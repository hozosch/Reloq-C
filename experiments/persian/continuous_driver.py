"""Connect the selected sentence planner to one NVDA language job."""
from __future__ import annotations

import logging
from collections import OrderedDict
import ctypes
import os
import re
from bisect import bisect_left
from dataclasses import replace

if __package__:
    from .._reloq_eci_direct import (EciCapitalWord, EciIndex, EciLanguage, EciPitch, EciPitchHz,
                                     EciProsody, EciText,
                                     SAMPLE_RATE, SAMPLE_RATES, PROCESS_TAGS, NATIVE_SPEED, NATIVE_WPM)
    from .._reloq_persian_frames import FrameCapture, FrameReplay, FrameStream, NativeFrame
    from .._reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
    from .._reloq_persian_projection import ProjectedJob
    from .direct_fa_sequence import ScopedDirectPersianEngine, _safe_native_punctuation, project, project_resilient
    from .continuous_plan import CapturedRows, context_plan, replay_marks, selected_sentence
    from .text_frontend import SpeechPart, UnsupportedPronunciation
else:
    from _reloq_eci_direct import (EciCapitalWord, EciIndex, EciLanguage, EciPitch, EciPitchHz,
                                    EciProsody, EciText,
                                    SAMPLE_RATE, SAMPLE_RATES, PROCESS_TAGS, NATIVE_SPEED, NATIVE_WPM)
    from _reloq_persian_frames import FrameCapture, FrameReplay, FrameStream, NativeFrame
    from _reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
    from _reloq_persian_projection import ProjectedJob
    from direct_fa_sequence import ScopedDirectPersianEngine, _safe_native_punctuation, project, project_resilient
    from continuous_plan import CapturedRows, context_plan, replay_marks, selected_sentence
    from text_frontend import SpeechPart, UnsupportedPronunciation

_log = logging.getLogger(__name__)
_TAG = re.compile(r'`\[[^]]+\]')
_FINAL_U = re.compile(r"(?:u|'u:')\]$")
_INITIAL_A = re.compile(r"^`\[\.[012](?:a|'a:')")
_DASHES = '-‐–—'
_UNICODE_DASHES = '‐–—'
# Even short mixed labels benefit from planning the independent contour on
# the already prepared second voice. Nothing from an utterance is retained;
# the single-core and unavailable-worker paths still use serial planning.
_PARALLEL_MIN = 1
_FAST_PILOT_MIN = 80
_FAST_ENGLISH_PREFIX_MIN_MS = 200


class _PlanningVoice:
    """Activate the right owned handle before each private synthesis pass."""
    def __init__(self, owner, key, engine, *, validate=False, prefix=()):
        self.owner, self.key, self.engine, self.validate = owner, key, engine, validate
        self.prefix = tuple(prefix)
        self._request_phones = {}
        self.raw = engine._voice if isinstance(engine, ScopedDirectPersianEngine) else engine
        self._dll, self._handle = self.raw._dll, self.raw._handle

    def _check(self):
        if self.owner._cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        self.owner._activate(self.key)
        if self.owner._cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')

    def _phonemes_for_probe(self, text):
        if text in self._request_phones:
            if self.owner._cancelled.is_set():
                raise InterruptedError('sentence planning was cancelled')
            return self._request_phones[text]
        self._check()
        result = self.raw._phonemes_for_probe(text)
        self._check()
        self._request_phones[text] = result
        return result

    def speak(self, parts, on_pcm, on_index):
        self._check()
        parts = (*self.prefix, *parts)
        if isinstance(self.engine, ScopedDirectPersianEngine):
            tagged_parts = tuple(EciText(p, segmentation_guarded=True) if isinstance(p, str) else p
                                 for p in parts)
            tagged = ''.join(p.text for p in tagged_parts if isinstance(p, EciText) and not p.speed_control)
            prepared = _safe_native_punctuation(ProjectedJob('', tagged, tagged_parts, self.validate))
            completed = self.engine._speak_prepared(prepared, on_pcm, on_index)
        else:
            completed = self.raw.speak(parts, on_pcm, on_index)
        if self.owner._cancelled.is_set():
            return False
        return completed


def _segments(owner, parts, fa_ids, english_id):
    key = owner._home
    result = []
    for part in parts:
        if isinstance(part, EciLanguage):
            key = owner._key(part.language, part.voice)
            continue
        if isinstance(part, EciCapitalWord):
            cache_key = (key, part.original, owner._dictionary_path,
                         tuple(sorted(owner._parameters.items())))
            if cache_key in owner._uppercase_cache:
                spelled = owner._uppercase_cache[cache_key]
                owner._uppercase_cache.move_to_end(cache_key)
            else:
                engine = owner._activate(key)
                probe = getattr(engine, 'spells_uppercase', None)
                try:
                    spelled = probe(part.original) if probe else None
                except Exception:
                    spelled = None
                if spelled is not None:
                    owner._uppercase_cache[cache_key] = spelled
                    if len(owner._uppercase_cache) > 4096:
                        owner._uppercase_cache.popitem(last=False)
            if owner._cancelled.is_set():
                raise InterruptedError('sentence planning was cancelled')
            part = EciText(part.segmented if spelled is False else part.original)
        if not isinstance(part, (str, EciText, EciIndex)):
            return None
        if isinstance(part, EciText) and part.speed_control:
            return None
        text = part if isinstance(part, str) else part.text if isinstance(part, EciText) else ''
        if '`' in text or key[0] not in (*fa_ids, english_id):
            return None
        if result and result[-1][0] == key:
            result[-1][1].append(part)
        else:
            result.append((key, [part]))
    # Whitespace at a language boundary belongs to the adjacent speech, not
    # to a separate empty acoustic segment.
    i = 0
    while i < len(result):
        _key, items = result[i]
        value = ''.join(p if isinstance(p, str) else p.text if isinstance(p, EciText) else '' for p in items)
        if not value.strip() and len(result) > 1:
            if i:
                result[i - 1][1].extend(items)
            else:
                result[i + 1][1][:0] = items
            del result[i]
        else:
            i += 1
    if not 2 <= len(result) <= 32 or {key[0] for key, _ in result}.isdisjoint(fa_ids):
        return None
    if not any(key[0] == english_id for key, _ in result):
        return None
    if len({key for key, _ in result if key[0] in fa_ids}) != 1:
        return None
    if len({key for key, _ in result if key[0] == english_id}) != 1:
        return None
    for _key, items in result:
        text = ''.join(p if isinstance(p, str) else p.text if isinstance(p, EciText) else '' for p in items)
        if not any(c.isalnum() for c in text):
            return None
    if sum(len(p if isinstance(p, str) else p.text if isinstance(p, EciText) else '')
           for _, items in result for p in items) > 1800:
        return None
    return result


def _project_part(engine, items, *, follows):
    original = ''.join(p if isinstance(p, str) else p.text if isinstance(p, EciText) else '' for p in items)
    # A following English word is also a following word for Persian stress.
    # The known sentinel supplies that context and is removed after mapping.
    suffix = (' salam' if engine._pinglish else ' سلام') if follows else ''
    source = (*items, suffix) if suffix else items
    try:
        prepared = project(source, engine._frontend,
                           allow_fallback=engine._allow_fallback, pinglish=engine._pinglish)
    except (ValueError, RuntimeError, OSError, UnicodeError):
        if engine._cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        prepared = project_resilient(source, engine._frontend, pinglish=engine._pinglish,
                                     validate=engine._phonemes_for_validation,
                                     cancelled=engine._cancelled.is_set)
    tagged = prepared.tagged
    limit = len(tagged)
    if suffix:
        sentinel = project((suffix.strip(),), engine._frontend,
                           allow_fallback=engine._allow_fallback, pinglish=engine._pinglish)
        last = tuple(_TAG.finditer(tagged))
        if not last or last[-1].group() != sentinel.tagged.strip() or tagged[last[-1].end():].strip():
            raise UnsupportedPronunciation('following-word context could not be removed')
        limit = last[-1].start()
        # Remove only the blank introduced with the sentinel.
        if limit and tagged[limit - 1] == ' ':
            limit -= 1
        tagged = tagged[:limit]
    marks, offset = [], 0
    for part in prepared.parts:
        if isinstance(part, EciText):
            offset += len(part.text)
        elif isinstance(part, EciIndex):
            marks.append((min(offset, limit), part.value))
        else:
            raise UnsupportedPronunciation('unsupported sentence event')
    if not tagged or not original.strip():
        raise UnsupportedPronunciation('empty Persian sentence segment')
    return tagged, marks, prepared.needs_native_validation


def try_continuous(owner, parts, on_pcm, on_index, *, fa_ids, english_id, german_id,
                   reset_cancellation=True, end_pause_shortened=False):
    """Return None when this job must retain the established native route."""
    if reset_cancellation:
        owner._cancelled.clear()
    prefix, ordinary, saw_text = [], [], False
    for part in parts:
        is_control = isinstance(part, (EciPitch, EciPitchHz, EciProsody)) or (
            isinstance(part, EciText) and bool(part.speed_control))
        if is_control:
            if saw_text:
                return None
            prefix.append(part)
        else:
            ordinary.append(part)
            if isinstance(part, (str, EciText, EciCapitalWord)):
                value = part if isinstance(part, str) else part.text if isinstance(part, EciText) else part.original
                saw_text |= bool(value.strip())
    try:
        groups = _segments(owner, ordinary, fa_ids, english_id)
    except InterruptedError:
        return False
    if not groups:
        return None
    fa_key = next(key for key, _ in groups if key[0] in fa_ids)
    en_key = next(key for key, _ in groups if key[0] == english_id)
    fa_engine = owner._activate(fa_key)
    if not isinstance(fa_engine, ScopedDirectPersianEngine):
        return None
    if not FrameReplay.available(fa_engine._voice._dll):
        return None
    fa_engine._cancelled.clear()
    restores, stream_state = [], {}
    try:
        sentence, events, guarded, spelling, validate = [], [], [], [], False
        for segment, (key, items) in enumerate(groups):
            if owner._cancelled.is_set():
                return False
            if key[0] in fa_ids:
                tagged, marks, checked = _project_part(fa_engine, items, follows=segment < len(groups) - 1)
                sentence.append(SpeechPart('fa', tagged))
                events.extend((segment, offset, value) for offset, value in marks)
                validate |= checked
            else:
                text, offset = [], 0
                for part in items:
                    if isinstance(part, EciIndex):
                        events.append((segment, offset, part.value))
                    else:
                        value = part if isinstance(part, str) else part.text
                        text.append(value)
                        if isinstance(part, EciText) and part.segmentation_guarded and value:
                            guarded.append((segment, offset, offset + len(value)))
                        if isinstance(part, EciText) and part.character_mode and value:
                            spelling.append((segment, offset, offset + len(value)))
                        offset += len(value)
                sentence.append(SpeechPart('en-us', ''.join(text)))
        if any(part.text.rstrip().endswith(('.', '?', '!', ',', ';', ':', '،', '؟', '؛'))
               for part in sentence[:-1]):
            return None
        en_engine = owner._activate(en_key)
        german_key = owner._key(german_id, en_key[1])
        fa = _PlanningVoice(owner, fa_key, fa_engine, validate=validate, prefix=prefix)
        en = _PlanningVoice(owner, en_key, en_engine, prefix=prefix)
        for voice in (fa, en):
            voice.raw._cancelled.clear()
        if owner._cancelled.is_set():
            return False
        # The planner uses private phoneme tags even when the user disables
        # source-side tags. Restore the user's setting before leaving.
        def enable_tags(voice):
            previous = voice._dll.eciGetParam(voice._handle, PROCESS_TAGS)
            if previous < 0:
                raise RuntimeError('cannot read native tag setting')
            if previous != 1:
                voice.raw.set_parameter(PROCESS_TAGS, 1)
                restores.append((voice.raw, previous))
        enable_tags(en)
        german_voice = []
        def german():
            if not german_voice:
                voice = _PlanningVoice(owner, german_key, owner._activate(german_key), prefix=prefix)
                voice.raw._cancelled.clear()
                enable_tags(voice)
                german_voice.append(voice)
            return german_voice[0]
        rate = SAMPLE_RATES.get(fa._dll.eciGetParam(fa._handle, SAMPLE_RATE))
        if rate is None:
            raise ValueError('invalid sentence sample rate')
        multicore = (os.cpu_count() or 1) > 1
        parallel_worthy = sum(len(part.text) for part in sentence) >= _PARALLEL_MIN and multicore
        available_pilot = getattr(owner, '_persian_contour_worker', None) if multicore else None
        parallel_pilot = available_pilot if parallel_worthy else None
        def warm_parallel():
            if not parallel_worthy or getattr(owner, '_persian_contour_worker', None) is not None:
                return
            if __package__:
                from .parallel_pilot import ParallelPilot
            else:
                from parallel_pilot import ParallelPilot
            try:
                owner._persian_contour_worker = ParallelPilot(owner._dll)
            except (RuntimeError, OSError):
                # Thread allocation failure retains the serial planner.
                pass
        def check_cancel():
            if owner._cancelled.is_set():
                raise InterruptedError('sentence planning was cancelled')
        def start_prefix(item, bounds, source_marks, capture_api, *, minimum_ms=60):
            # Only the selected English-to-Ach transition can reach as far
            # as 200 ms back into the first segment. Other switch edits stay
            # within 30 ms; keep a 60-ms safety margin for those and start
            # short labels without changing any frames near their boundary.
            ach_boundary = (sentence[0].language == 'en-us' and sentence[1].language == 'fa' and
                            re.match(r'^\s*`\[\.[012]G', sentence[1].text) is not None)
            reserve = 250 if ach_boundary else 60
            boundary_ms = round(bounds[1] * 1000 / rate)
            first_end = bisect_left(item.ticks, boundary_ms)
            first = bisect_left(item.ticks, max(0, boundary_ms - reserve))
            if first < 6 or first >= first_end or item.ticks[first] - item.ticks[0] < minimum_ms:
                return False
            positions = []
            for segment, offset, _order, mark, value in source_marks:
                if segment:
                    continue
                source = bisect_left(item.ticks, round(item.indexes[mark] * 1000 / rate))
                local = (0 if offset == 0 else first_end if offset == len(sentence[0].text) else
                         max(0, min(first_end, source)))
                positions.append((local, value))
            if any(a[0] > b[0] for a, b in zip(positions, positions[1:])):
                raise RuntimeError('initial source indexes are not ordered')
            if capture_api.recompute_frication is not None:
                capture_api.recompute_frication(item.frames, first_end)
            else:
                for row in item.frames[:first_end]:
                    c = row.controls
                    row.s_frication = int(c[8] > 0 and (c[38] > c[37] + 10 or c[39] > c[37] + 10))
            first_voice = en if sentence[0].language == 'en-us' else fa
            first_key = en_key if first_voice is en else fa_key
            if first_voice is en:
                # All later English-phone lookups must happen before this
                # same handle enters replay. These values are request-local.
                for part in sentence:
                    if part.language == 'en-us':
                        en._request_phones[part.text] = en._phonemes_for_probe(part.text)
            owner._activate(first_key)
            stream = FrameStream(first_voice.raw, on_pcm, on_index, cancelled=owner._cancelled)
            stream_state.update(stream=stream, item=item, end=first_end, first=first,
                                positions=positions, prefix_complete=False, finishing=False)
            stream.append(item.frames, 0, first, tuple((at, value) for at, value in positions if at < first))
            stream_state['prefix_complete'] = True
            # The very first long utterance should not wait for a second
            # native handle to load. Prepare it only after its safe prefix
            # has been delivered, then use it for later uncached requests.
            if item.ticks[first] - item.ticks[0] >= 250:
                warm_parallel()
            return True
        tags = tuple(_TAG.finditer(sentence[0].text)) if sentence[0].language == 'fa' else ()
        first_has_glide = any(_FINAL_U.search(left.group()) and _INITIAL_A.match(right.group()) and
                              sentence[0].text[left.end():right.start()].isspace()
                              for left, right in zip(tags, tags[1:]))
        can_stream = (not first_has_glide and not spelling and FrameStream.available(en._dll))
        fast_english = False
        if (available_pilot and can_stream and sentence[0].language == 'en-us' and
                not any(isinstance(item, EciProsody) and item.parameter in (6, 8) or
                        isinstance(item, EciText) and item.speed_control for item in prefix)):
            # Predict only long English openings for which an early prefix
            # is likely to leave the required audio reserve. Reuse the same
            # per-request phone lookup later when forming the pilot proxy.
            speed = en._dll.eciGetParam(en._handle, NATIVE_SPEED)
            wpm = en._dll.eciGetParam(en._handle, NATIVE_WPM)
            ach = (sentence[1].language == 'fa' and
                   re.match(r'^\s*`\[\.[012]G', sentence[1].text) is not None)
            estimated_usable = (0 < wpm <= (250 if ach else 350) if wpm else
                                0 < speed <= (75 if ach else 100))
            if estimated_usable:
                displayed = en._phonemes_for_probe(sentence[0].text)
                syllables = sum(len(re.findall(r'\.[012]', match.group())) for match in _TAG.finditer(displayed))
                fast_english = syllables >= 4
                if fast_english:
                    parallel_pilot = available_pilot
        plan = context_plan(sentence, fa, en, rate, maximum=30000, source_events=events,
                            guarded_spans=guarded, spelling_spans=spelling, check=check_cancel,
                            trusted_fa=not validate, validate_in_capture=validate,
                            start_prefix=start_prefix if can_stream else None, parallel_pilot=parallel_pilot,
                            fast_initial=bool(parallel_pilot and can_stream and
                                              (len(sentence[0].text) >= _FAST_PILOT_MIN or fast_english)),
                            fast_minimum_ms=(_FAST_ENGLISH_PREFIX_MIN_MS if fast_english else 300))
        original_counts = plan.counts
        selected_sentence(plan, sentence, en, german, rate, check=check_cancel)
        marks = replay_marks(plan, original_counts)
        if stream_state:
            first = stream_state['first']
            size = first * ctypes.sizeof(NativeFrame)
            if ctypes.string_at(plan.frames, size) != ctypes.string_at(stream_state['item'].frames, size):
                _log.warning('The streamed prefix differed from the completed sentence plan')
            remaining = tuple((at - first, value) for at, value in marks if at >= first)
            stream_state['finishing'] = True
            stream_state['stream'].append(plan.frames, first, len(plan.frames), remaining)
            stream_state['stream'].close(flush=True)
            return True
    except InterruptedError:
        return False
    except (ValueError, RuntimeError, OSError) as error:
        if owner._cancelled.is_set():
            return False
        _log.warning('Cannot plan a continuous Persian sentence: %s', error)
        if stream_state:
            if not stream_state['prefix_complete'] or stream_state['finishing']:
                # A replay failure may already have emitted part of this
                # slice. Never restart it or duplicate audible speech.
                raise
            # Once the initial audio has escaped, never repeat it by falling
            # back to the whole job. Complete the first native context, then
            # speak the remaining original text with its own language engine.
            stream = stream_state['stream']
            if stream.voice._callback_error is not None:
                raise stream.voice._callback_error
            first = stream_state['first']
            tail = tuple((at - first, value) for at, value in stream_state['positions'] if at >= first)
            stream.append(stream_state['item'].frames, first, stream_state['end'], tail)
            stream.close(flush=True)
            for group_key, items in groups[1:]:
                if owner._cancelled.is_set():
                    return False
                engine = owner._activate(group_key)
                if not engine.speak((*prefix, *items), on_pcm, on_index):
                    return False
            return True
        return None
    finally:
        try:
            if stream_state and stream_state['stream'].state:
                try:
                    stream_state['stream'].close()
                except InterruptedError:
                    pass
        finally:
            for voice, previous in reversed(restores):
                voice.set_parameter(PROCESS_TAGS, previous)
    if owner._cancelled.is_set():
        return False
    owner._activate(fa_key)
    return FrameReplay(fa._dll).replay(fa.raw, plan.frames, marks, on_pcm, on_index,
                                      cancelled=owner._cancelled)


def try_persian_glides(owner, parts, on_pcm, on_index, *, fa_ids):
    """Apply the selected U-to-A glide inside an otherwise Persian job."""
    key, items, prefix, saw_text = owner._home, [], [], False
    for part in parts:
        if isinstance(part, EciLanguage):
            next_key = owner._key(part.language, part.voice)
            if items and next_key != key:
                return None
            key = next_key
        elif isinstance(part, (str, EciText, EciIndex)):
            if isinstance(part, EciText) and (part.character_mode or part.speed_control):
                return None
            items.append(part)
            if isinstance(part, (str, EciText)):
                saw_text = True
        elif isinstance(part, (EciPitch, EciPitchHz, EciProsody)) and not saw_text:
            prefix.append(part)
        else:
            return None
    if key[0] not in fa_ids or not items:
        return None
    engine = owner._activate(key)
    if not isinstance(engine, ScopedDirectPersianEngine) or not FrameReplay.available(engine._voice._dll):
        return None
    engine._cancelled.clear()
    if owner._cancelled.is_set():
        return False
    try:
        try:
            prepared = project(items, engine._frontend, allow_fallback=engine._allow_fallback,
                               pinglish=engine._pinglish)
        except (ValueError, RuntimeError, OSError, UnicodeError):
            if owner._cancelled.is_set():
                return False
            prepared = project_resilient(items, engine._frontend, pinglish=engine._pinglish,
                                         validate=engine._phonemes_for_validation,
                                         cancelled=owner._cancelled.is_set)
        if prefix:
            prepared = replace(prepared, parts=tuple(prefix) + tuple(prepared.parts))
        matches = tuple(_TAG.finditer(prepared.tagged))
        tags = tuple(match.group() for match in matches)
        separators = tuple(prepared.tagged[a.end():b.start()] for a, b in zip(matches, matches[1:]))
        if not any(_FINAL_U.search(a) and _INITIAL_A.match(b) and 'w' not in a and 'w' not in b and sep.isspace()
                   for a, b, sep in zip(tags, tags[1:], separators)):
            return engine._speak_prepared(prepared, on_pcm, on_index)
        inserts, source, offset, next_mark = [], [], 0, 1
        for part in prepared.parts:
            if isinstance(part, EciText):
                offset += len(part.text)
            elif isinstance(part, EciIndex):
                inserts.append((offset, len(inserts), next_mark))
                source.append((next_mark, offset, part.value))
                next_mark += 1
            elif not isinstance(part, (EciPitch, EciPitchHz, EciProsody)):
                return None
        words = {}
        for word, match in enumerate(matches):
            if word:
                inserts.append((match.start(), -1, next_mark))
                words[word] = next_mark
                next_mark += 1
        sequence, position = [], 0
        for at, _order, mark in sorted(inserts):
            sequence.extend((prepared.tagged[position:at], EciIndex(mark)))
            position = at
        sequence.append(prepared.tagged[position:])
        voice = _PlanningVoice(owner, key, engine, validate=prepared.needs_native_validation,
                               prefix=prefix)
        voice.raw._cancelled.clear()
        if owner._cancelled.is_set():
            return False
        captured = FrameCapture(voice._dll).capture(voice, sequence, maximum=30000)
        item = CapturedRows.from_capture(captured)
        rate = SAMPLE_RATES.get(voice._dll.eciGetParam(voice._handle, SAMPLE_RATE))
        if rate is None:
            raise ValueError('invalid Persian sample rate')
        boundaries = {word: round(item.indexes[mark] * 1000 / rate) for word, mark in words.items()}
        controls = [row.controls for row in item.frames]
        glides = plan_u_a_glides(controls, item.ticks, tags, separators, boundaries)
        if not glides:
            return engine._speak_prepared(prepared, on_pcm, on_index)
        apply_u_a_glides(controls, item.ticks, glides)
        marks = []
        for mark, offset, value in source:
            frame = (0 if offset == 0 else len(item.frames) if offset == len(prepared.tagged) else
                     min(len(item.frames), bisect_left(item.ticks, round(item.indexes[mark] * 1000 / rate))))
            marks.append((frame, value))
    except InterruptedError:
        return False
    except (ValueError, RuntimeError, OSError) as error:
        if owner._cancelled.is_set():
            return False
        _log.debug('Persian vowel glide unavailable: %s', error)
        return None
    if owner._cancelled.is_set():
        return False
    owner._activate(key)
    return FrameReplay(voice._dll).replay(voice.raw, item.frames, marks, on_pcm, on_index,
                                         cancelled=owner._cancelled)


def sentence_jobs(parts, home):
    """Split mixed speech at unambiguous native phrase punctuation."""
    if any(not isinstance(p, (str, EciText, EciIndex, EciLanguage, EciCapitalWord)) or
           isinstance(p, EciText) and (p.character_mode or p.speed_control) for p in parts):
        return ()
    full = ''.join(p if isinstance(p, str) else p.text if isinstance(p, EciText) else
                   p.original if isinstance(p, EciCapitalWord) else '' for p in parts)
    if not any(c in full for c in '.!?؟,;:،؛\n'):
        return ()
    jobs, pending, key, offset = [], [], home, 0
    for part in parts:
        if isinstance(part, EciLanguage):
            key = (part.language, part.voice)
            pending.append(part)
            continue
        if isinstance(part, EciIndex):
            pending.append(part)
            continue
        if isinstance(part, EciCapitalWord):
            pending.append(part)
            offset += len(part.original)
            continue
        value = part if isinstance(part, str) else part.text
        begin = 0
        for i, char in enumerate(value):
            absolute = offset + i
            if char not in '.!?؟,;:،؛\n':
                continue
            after = full[absolute + 1:absolute + 2]
            if char in '.,;:،؛' and after and not after.isspace():
                continue
            if char == '.':
                before = re.search(r'([A-Za-z]+)$', full[:absolute])
                # Do not break common abbreviations into separate synthesis
                # requests; the native language core interprets them.
                if before and (len(before.group(1)) == 1 or
                               before.group(1).casefold() in ('dr', 'mr', 'mrs', 'ms', 'prof', 'etc')):
                    continue
            fragment = value[begin:i + 1]
            if fragment:
                pending.append(fragment if isinstance(part, str) else EciText(
                    fragment, segmentation_guarded=part.segmentation_guarded))
            if any(isinstance(p, (str, EciText, EciCapitalWord)) and
                   (p if isinstance(p, str) else p.text if isinstance(p, EciText) else p.original).strip()
                   for p in pending):
                jobs.append(tuple(pending))
                pending = [EciLanguage(*key)]
            begin = i + 1
        fragment = value[begin:]
        if fragment:
            pending.append(fragment if isinstance(part, str) else EciText(
                fragment, segmentation_guarded=part.segmentation_guarded))
        offset += len(value)
    if any(isinstance(p, (str, EciText, EciCapitalWord)) and
           (p if isinstance(p, str) else p.text if isinstance(p, EciText) else p.original).strip()
           for p in pending):
        jobs.append(tuple(pending))
    elif jobs:
        jobs[-1] += tuple(p for p in pending if isinstance(p, EciIndex))
    return tuple(jobs) if len(jobs) > 1 else ()


def bracket_jobs(parts, home, *, fa_ids, english_id):
    """Separate language cores at explicit bracket and dash boundaries.

    Native index callbacks do not always mark the acoustic start of the
    following word. At a real boundary, a separate native job keeps
    the entire next word in its own voice and avoids proxy synthesis. Marks
    remain in source order and active inline controls are copied forward.
    """
    parts = list(parts)
    result, pending, controls, key = [], [], OrderedDict(), home
    segment = []
    split_started = False

    def text_and_mode(item):
        if isinstance(item, str):
            return item, False
        if isinstance(item, EciText) and not item.speed_control:
            return item.text, item.character_mode
        if isinstance(item, EciCapitalWord):
            return item.original, False
        return '', False

    def edge(items, backwards):
        for item in reversed(items) if backwards else items:
            value, spelled = text_and_mode(item)
            value = value.strip()
            if value:
                return (value[-1] if backwards else value[0]), spelled
        return '', False

    def has_speech(items):
        return any(any(not c.isspace() and c not in '()[]{}-‐–—.,;:!?؟،؛' for c in text_and_mode(item)[0])
                   for item in items)

    def has_mixed_speech(items):
        active, spoken = home, set()
        for item in items:
            if isinstance(item, EciLanguage):
                active = (item.language, item.voice)
            elif has_speech((item,)):
                spoken.add(active[0])
        return english_id in spoken and bool(spoken.intersection(fa_ids))

    def neighbour(items, backwards):
        for item in reversed(items) if backwards else items:
            if isinstance(item, EciLanguage):
                break
            value, spelled = text_and_mode(item)
            if value:
                return (value[-1] if backwards else value[0]), spelled
        return '', False

    def normalise_persian_dash(items, backwards):
        # Direct Persian word projection treats ASCII hyphen as a boundary;
        # map other silent dash marks at a language edge to the same route.
        positions = range(len(items) - 1, -1, -1) if backwards else range(len(items))
        for position in positions:
            item = items[position]
            if isinstance(item, EciLanguage):
                break
            value, spelled = text_and_mode(item)
            if not value.strip():
                continue
            if spelled or not isinstance(item, (str, EciText)):
                return False
            offset = len(value.rstrip()) - 1 if backwards else len(value) - len(value.lstrip())
            if value[offset] in _UNICODE_DASHES:
                value = value[:offset] + '-' + value[offset + 1:]
                items[position] = value if isinstance(item, str) else replace(item, text=value)
            return True
        return False

    def next_nonspace(items, backwards):
        for item in reversed(items) if backwards else items:
            if isinstance(item, EciLanguage):
                break
            value, spelled = text_and_mode(item)
            if value.strip():
                return (value.rstrip()[-1] if backwards else value.lstrip()[0]), spelled
        return '', False

    mixed_request = has_mixed_speech(parts)
    for index, part in enumerate(parts):
        if isinstance(part, EciLanguage):
            selected = (part.language, part.voice)
            right = []
            for following in parts[index + 1:]:
                if isinstance(following, EciLanguage):
                    break
                right.append(following)
            left_char, left_spelled = edge(segment, True)
            right_char, right_spelled = edge(right, False)
            changed = selected != key and {selected[0], key[0]} <= {english_id, *fa_ids}
            has_both = (selected[0] == english_id and key[0] in fa_ids or
                        key[0] == english_id and selected[0] in fa_ids)
            boundary = ((left_char in '()[]{}' + _DASHES and left_char and not left_spelled) or
                        (right_char in '()[]{}' + _DASHES and right_char and not right_spelled))
            if changed and has_both and left_char and right_char and boundary:
                split_pending, rest = list(pending), parts[index + 1:]
                if left_char in _UNICODE_DASHES and key[0] in fa_ids:
                    if not normalise_persian_dash(split_pending, True):
                        pending.append(part)
                        key, segment = selected, []
                        continue
                if right_char in _UNICODE_DASHES and selected[0] in fa_ids:
                    if not normalise_persian_dash(rest, False):
                        pending.append(part)
                        key, segment = selected, []
                        continue
                parts[index + 1:] = rest
                result.append((tuple(split_pending), True))
                pending = [part, *controls.values()]
                split_started = True
            else:
                pending.append(part)
            key, segment = selected, []
            continue
        # Parentheses already mark a short native phrase boundary even when
        # both sides use the same language. In a mixed request, stop planning
        # at that boundary so later NVDA role/status text is rendered by its
        # own core and does not delay the beginning of the sentence.
        value, spelled = text_and_mode(part)
        if (isinstance(part, (str, EciText)) and value and not spelled and '`' not in value and
                any(c in value for c in '()[]{}' + _DASHES)):
            start = 0
            for position, char in enumerate(value):
                if position < start or char not in '()[]{}' + _DASHES:
                    continue
                if char in _DASHES:
                    # A spaced dash inside an English run already makes a
                    # native pause. Do not split English compounds (or
                    # Pinglish suffixes and signed numbers) at bare hyphens.
                    left, left_spelled = (value[position - 1], False) if position else neighbour(segment, True)
                    right, right_spelled = ((value[position + 1], False) if position + 1 < len(value)
                                            else neighbour(parts[index + 1:], False))
                    if (key[0] != english_id or not left.isspace() or not right.isspace() or
                            left_spelled or right_spelled):
                        continue
                    before = [*segment, value[:position]]
                    after = [value[position + 1:], *parts[index + 1:]]
                    left_word, _ = next_nonspace(before, True)
                    right_word, _ = next_nonspace(after, False)
                    if left_word.isdigit() and right_word.isdigit():
                        continue
                end = position + 1
                while end < len(value) and value[end].isspace():
                    end += 1
                fragment = value[start:end]
                item = replace(part, text=fragment) if isinstance(part, EciText) else fragment
                pending.append(item)
                segment.append(item)
                following = [value[end:], *parts[index + 1:]]
                if ((split_started or has_mixed_speech(pending) or
                     char in _DASHES and mixed_request) and
                        has_speech(segment) and has_speech(following)):
                    result.append((tuple(pending), True))
                    pending = [EciLanguage(*key), *controls.values()]
                    segment = []
                    split_started = True
                start = end
            if start < len(value):
                fragment = value[start:]
                item = replace(part, text=fragment) if isinstance(part, EciText) else fragment
                pending.append(item)
                segment.append(item)
            continue
        pending.append(part)
        segment.append(part)
        if isinstance(part, (EciPitch, EciPitchHz, EciProsody)):
            slot = ('pitch' if isinstance(part, (EciPitch, EciPitchHz)) else
                    'speed' if part.parameter in (6, 8) else part.parameter)
            controls.pop(slot, None)
            controls[slot] = part
        elif isinstance(part, EciText) and part.speed_control:
            controls.pop('speed', None)
            controls['speed'] = EciText(part.speed_control + ' ')
    result.append((tuple(pending), False))
    return tuple(result)


def continuation_comma(parts):
    """Ask the current native core for a nonfinal separated phrase tone."""
    parts = tuple(parts)
    def visible(part):
        return (part if isinstance(part, str) else part.text if isinstance(part, EciText) and
                not part.speed_control and not part.character_mode else
                part.original if isinstance(part, EciCapitalWord) else '')
    tail = ''.join(visible(part) for part in parts).rstrip().rstrip('()[]{}').rstrip()
    if tail.endswith(('.', '!', '?', '؟')):
        return parts
    for index in range(len(parts) - 1, -1, -1):
        part = parts[index]
        value = visible(part)
        if not value.strip():
            continue
        # Do not override an explicit sentence or question ending inside a
        # closing bracket. The comma is internal, not spoken text.
        return parts[:index + 1] + (',',) + parts[index + 1:]
    return parts


def bounded_jobs(parts, home, *, maximum=1200, max_switches=24):
    """Bound long requests while retaining source marks and active controls."""
    total = sum(len(part if isinstance(part, str) else part.text if isinstance(part, EciText)
                    and not part.speed_control else part.original if isinstance(part, EciCapitalWord) else '')
                for part in parts)
    switches = sum(isinstance(part, EciLanguage) for part in parts)
    if total <= maximum and switches <= max_switches:
        return (tuple(parts),)
    if maximum < 64 or max_switches < 2:
        raise ValueError('invalid bounded speech size')
    jobs, pending, key, size, changes = [], [], home, 0, 0
    controls = {}

    def flush():
        nonlocal pending, size, changes
        if pending:
            jobs.append(tuple(pending))
        pending = [EciLanguage(*key), *controls.values()]
        size, changes = 0, 0

    for part in parts:
        if isinstance(part, EciLanguage):
            if changes >= max_switches and size:
                flush()
            key = (part.language, part.voice)
            pending.append(part)
            changes += 1
            continue
        if isinstance(part, (EciPitch, EciPitchHz, EciProsody)):
            slot = ('pitch' if isinstance(part, (EciPitch, EciPitchHz)) else
                    'speed' if part.parameter in (6, 8) else part.parameter)
            controls.pop(slot, None)
            controls[slot] = part
            pending.append(part)
            continue
        if isinstance(part, EciText) and part.speed_control:
            controls.pop('speed', None)
            controls['speed'] = EciText(part.speed_control + ' ')
            pending.append(part)
            continue
        if isinstance(part, EciCapitalWord):
            if size and size + len(part.original) > maximum:
                flush()
            if len(part.original) <= maximum:
                pending.append(part)
                size += len(part.original)
                continue
            part = EciText(part.original)
        if not isinstance(part, (str, EciText)):
            pending.append(part)
            continue
        value = part if isinstance(part, str) else part.text
        while value:
            if size >= maximum:
                flush()
            room = maximum - size
            if len(value) <= room:
                take = len(value)
            else:
                take = max(value.rfind(' ', 0, room + 1), value.rfind('\n', 0, room + 1)) + 1
                if take <= room // 3:
                    take = room
            piece, value = value[:take], value[take:]
            pending.append(piece if isinstance(part, str) else EciText(
                piece, part.character_mode, part.speed_control, part.segmentation_guarded))
            size += len(piece)
            if value:
                flush()
    if pending:
        jobs.append(tuple(pending))
    return tuple(jobs)
