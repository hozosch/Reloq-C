"""Connect the selected sentence planner to one NVDA language job."""
from __future__ import annotations

import logging
from collections import OrderedDict
import ctypes
import os
import queue
import re
from bisect import bisect_left
from dataclasses import replace

if __package__:
    from .._reloq_eci_direct import (EciCapitalWord, EciIndex, EciLanguage, EciPitch, EciPitchHz,
                                     EciProsody, EciText, persistent_tag_control,
                                     SAMPLE_RATE, SAMPLE_RATES, PROCESS_TAGS, NATIVE_SPEED, NATIVE_WPM, ORIGINAL_ACOUSTIC_CLOCK)
    from .._reloq_persian_frames import FrameCapture, FrameReplay, FrameStream, NativeFrame, NativeGlideCandidate
    from .._reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
    from .._reloq_persian_projection import ProjectedJob
    from .direct_fa_sequence import ScopedDirectPersianEngine, _safe_native_punctuation, project, project_resilient
    from .continuous_plan import (CapturedRows, context_plan, replay_marks, selected_sentence,
                                  _bracket_boundary, fit_us_word_pitch, indexed_us_proxy, word_boundary)
    from .._reloq_languages import US_ENGLISH
    from .korean_contour import align_english_edges, contour
    from .text_frontend import SpeechPart, UnsupportedPronunciation
else:
    from _reloq_eci_direct import (EciCapitalWord, EciIndex, EciLanguage, EciPitch, EciPitchHz,
                                    EciProsody, EciText, persistent_tag_control,
                                    SAMPLE_RATE, SAMPLE_RATES, PROCESS_TAGS, NATIVE_SPEED, NATIVE_WPM, ORIGINAL_ACOUSTIC_CLOCK)
    from _reloq_persian_frames import FrameCapture, FrameReplay, FrameStream, NativeFrame, NativeGlideCandidate
    from _reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
    from _reloq_persian_projection import ProjectedJob
    from direct_fa_sequence import ScopedDirectPersianEngine, _safe_native_punctuation, project, project_resilient
    from continuous_plan import (CapturedRows, context_plan, replay_marks, selected_sentence,
                                 _bracket_boundary, fit_us_word_pitch, indexed_us_proxy, word_boundary)
    from _reloq_languages import US_ENGLISH
    from korean_contour import align_english_edges, contour
    from text_frontend import SpeechPart, UnsupportedPronunciation

def _original_persian_clock(voice):
    # Persian/Pinglish use the Finnish native handle, where automatic mode 2
    # is the fine clock. A user-selected original clock is represented by 1.
    return voice._dll.eciGetParam(voice._handle, ORIGINAL_ACOUSTIC_CLOCK) == 1


def _hold_acoustic_clock(rows, first=0, last=None):
    """Keep planned Persian controls on the original five-ms grid.

    The US contour and cross-phone bridges are applied after native capture;
    without this step they silently restore one-ms changes to selected tracks.
    Frame durations, indexes and the selected US contour itself stay intact.
    """
    last = len(rows) if last is None else last
    group, held = None, None
    for index in range(first, last):
        row = rows[index]
        current = int(row.tick) // 5
        if current != group:
            group = current
            held = tuple(row.controls[i] for i in range(1, 62))
        else:
            for i, value in enumerate(held, 1):
                # These otherwise-unused tracks carry exact phone boundaries
                # for the local resonance filters. Do not smear that metadata
                # into an adjacent phone when holding acoustic parameters.
                if i in (11, 12):
                    continue
                row.controls[i] = value


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


def _colon_ends_sentence(text, index):
    """Separate prose colons; keep adjoining clock, URL and drive syntax."""
    before, after = index - 1, index + 1
    if before >= 0 and after < len(text) and text[before].isdecimal() and text[after].isdecimal():
        return False
    if after < len(text) and text[after] in ('/', '\\'):
        prefix = re.search(r'(?<![A-Za-z0-9+.-])([A-Za-z][A-Za-z0-9+.-]*)$', text[:index])
        if prefix and (len(prefix.group(1)) == 1 or text[after:after + 2] == '//' or
                       after + 1 < len(text) and not text[after + 1].isspace() or
                       prefix.group(1).casefold() in ('file', 'http', 'https', 'ftp')):
            return False
    # A slash separated by whitespace is content after the colon, not a URI
    # scheme. The old lookahead accidentally suppressed the sentence break.
    return True


def _plain_colons(text):
    """Locate punctuation colons without inspecting phoneme-tag contents."""
    return [match.start() for match in re.finditer(r'`\[[^]]+\]|:', text)
            if match.group() == ':']


def _sentence_colons_as_periods(tagged, source):
    """Use the agreed full-stop contour after prose colons, not clock colons."""
    source_colons, tagged_colons = _plain_colons(source), _plain_colons(tagged)
    if len(source_colons) != len(tagged_colons):
        return tagged
    replace_at = {target for original, target in zip(source_colons, tagged_colons)
                  if _colon_ends_sentence(source, original)}
    return ''.join('.' if index in replace_at else char for index, char in enumerate(tagged))


def _persian_sentence_starts(tagged, source, matches, separators):
    """Find sentence words, resolving colons against the original text."""
    source_colons, tagged_colons = _plain_colons(source), _plain_colons(tagged)
    if len(source_colons) == len(tagged_colons):
        boundaries = {target for original, target in zip(source_colons, tagged_colons)
                      if _colon_ends_sentence(source, original)}
    else:
        # A rare projection may omit punctuation. Be conservative rather than
        # split a clock or an address when source-to-tag pairing is unclear.
        boundaries = set()
    result = [0]
    for i, gap in enumerate(separators):
        start = matches[i].end()
        if (any(char in gap for char in '.!?') or
                any(start + offset in boundaries for offset, char in enumerate(gap) if char == ':')):
            result.append(i + 1)
    return result


class _PlanningVoice:
    """Activate the right owned handle before each private synthesis pass."""
    def __init__(self, owner, key, engine, *, validate=False, prefix=(), source=''):
        self.owner, self.key, self.engine, self.validate = owner, key, engine, validate
        self.prefix = tuple(prefix)
        self.source = source
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
            def normalized_space(value):
                return ''.join(' ' if ord(char) > 127 and char.isspace() else char for char in value)
            tagged_parts = tuple(EciText(normalized_space(p), segmentation_guarded=True)
                                 if isinstance(p, str) else
                                 replace(p, text=normalized_space(p.text))
                                 if isinstance(p, EciText) and not p.is_control else p
                                 for p in parts)
            tagged = ''.join(p.text for p in tagged_parts if isinstance(p, EciText) and not p.is_control)
            prepared = _safe_native_punctuation(ProjectedJob(self.source, tagged, tagged_parts, self.validate))
            completed = self.engine._speak_prepared(prepared, on_pcm, on_index, neutralize_stress=False)
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
        if isinstance(part, EciText) and part.is_control:
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


def _project_part(engine, items, *, follows, preceded=False):
    original = ''.join(p if isinstance(p, str) else p.text if isinstance(p, EciText) else '' for p in items)
    # A following English word is also a following word for Persian stress.
    # The known sentinel supplies that context and is removed after mapping.
    suffix = (' salam' if engine._pinglish else ' سلام') if follows else ''
    # A Persian enclitic following an English word has a left host even
    # though script routing has placed that host in a separate segment.
    enclitic = (r'(?:را|است|ra{1,2}|ast)' if engine._pinglish else r'(?:را|است)')
    prefix = ('salam ' if engine._pinglish else 'سلام ') if (
        preceded and re.match(r'^[ \t]*' + enclitic + r'(?=$|[ \t.,?!:;،؟؛)\]}])',
                              original, re.IGNORECASE)) else ''
    source = ((*((prefix,) if prefix else ()), *items, *((suffix,) if suffix else ()))
              if prefix or suffix else items)
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
    start = 0
    if prefix:
        sentinel = project((prefix.strip(),), engine._frontend,
                           allow_fallback=engine._allow_fallback, pinglish=engine._pinglish)
        first = sentinel.tagged.strip()
        if not tagged.startswith(first + ' '):
            raise UnsupportedPronunciation('preceding-word context could not be removed')
        start = len(first) + 1
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
    tagged = tagged[start:limit]
    marks, offset = [], 0
    for part in prepared.parts:
        if isinstance(part, EciText):
            offset += len(part.text)
        elif isinstance(part, EciIndex):
            marks.append((max(0, min(offset, limit) - start), part.value))
        else:
            raise UnsupportedPronunciation('unsupported sentence event')
    if not tagged or not original.strip():
        raise UnsupportedPronunciation('empty Persian sentence segment')
    return tagged, marks, prepared.needs_native_validation


def try_continuous(owner, parts, on_pcm, on_index, *, fa_ids, english_id, german_id,
                   reset_cancellation=True, end_pause_shortened=False):
    """Return None when this job must retain the established native route."""
    # One-shot ECI controls must be played exactly once, not separately in
    # the two voices used by the joint contour pilot.
    if any(isinstance(part, EciText) and part.tag_control for part in parts):
        return None
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
                tagged, marks, checked = _project_part(fa_engine, items, follows=segment < len(groups) - 1,
                                                       preceded=segment > 0)
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
        original_fa_clock = _original_persian_clock(fa.raw)
        rate = SAMPLE_RATES.get(fa._dll.eciGetParam(fa._handle, SAMPLE_RATE))
        if rate is None:
            raise ValueError('invalid sentence sample rate')
        # The request-local parallel worker owns a separate US pilot handle.
        korean_available = False
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
            if korean_available and sentence[0].language == 'en-us':
                # The last voiced English frame can precede the language
                # boundary by more than the usual 60-ms reserve. Do not
                # stream any frame that may need the Korean edge blend.
                voiced = next((item.frames[i].tick for i in range(first_end - 1, -1, -1)
                               if item.frames[i].controls[2] > 10 and item.frames[i].controls[1] > 0), None)
                if voiced is not None:
                    first = min(first, bisect_left(item.ticks, max(0, voiced - 45)))
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
            if korean_available and sentence[0].language == 'fa':
                if bisect_left(item.ticks, round(bounds[0] * 1000 / rate)) != 0:
                    return False
                pointer = ctypes.cast(item.frames, ctypes.POINTER(NativeFrame))
                if contour(owner).apply(pointer, first_end, sentence[0].text, fa.raw,
                                        owner._cancelled, continuation=True, prefix=prefix):
                    stream_state['korean_first'] = tuple(item.frames[i].controls[1] for i in range(first_end))
            first_voice = en if sentence[0].language == 'en-us' else fa
            if first_voice is fa and original_fa_clock:
                _hold_acoustic_clock(item.frames, 0, first_end)
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
                                positions=positions, prefix_complete=False, finishing=False,
                                prefix_bytes=ctypes.string_at(item.frames, first * ctypes.sizeof(NativeFrame)))
            stream.append(item.frames, 0, first, tuple((at, value) for at, value in positions if at < first))
            flush_prefix = getattr(on_pcm, 'flush_prefix', None)
            if flush_prefix is not None:
                flush_prefix()
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
        can_stream = not first_has_glide and not spelling and FrameStream.available(en._dll)
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
                            fast_initial=bool(os.environ.get('RELOQ_FA_STREAM_PREFIX', '1') == '1' and
                                              parallel_pilot and can_stream and
                                              (sentence[0].language == 'fa' or
                                               len(sentence[0].text) >= _FAST_PILOT_MIN or fast_english)),
                            fast_minimum_ms=(_FAST_ENGLISH_PREFIX_MIN_MS if fast_english else 60),
                            us_pilot=True)
        original_counts = plan.counts
        if korean_available:
            # The existing planner retains the English context's untouched
            # pitch before it blends switch edges to the Finnish pilot.
            # Restore it, then fit only those edges to the Korean-derived
            # Persian contour after the normal acoustic transitions.
            english_context = plan.contexts['en-us']
            original_pitch = english_context.original_pitch
            if original_pitch is not None:
                offset = 0
                for i, (part, count) in enumerate(zip(sentence, plan.counts)):
                    if part.language == 'en-us':
                        first = bisect_left(english_context.ticks, round(plan.edges['en-us'][i] * 1000 / rate))
                        last = bisect_left(english_context.ticks, round(plan.edges['en-us'][i + 1] * 1000 / rate))
                        if last - first != count:
                            raise RuntimeError('English original pitch length changed')
                        for local in range(count):
                            plan.frames[offset + local].controls[1] = original_pitch[first + local]
                    offset += count
        selected_sentence(plan, sentence, en, german, rate, check=check_cancel)
        if korean_available:
            offset = 0
            for i, (part, count) in enumerate(zip(sentence, plan.counts)):
                if part.language == 'fa' and count:
                    check_cancel()
                    if i == 0 and 'korean_first' in stream_state:
                        pitches = stream_state['korean_first']
                        if len(pitches) != count:
                            raise RuntimeError('streamed Persian pitch length changed')
                        for frame, pitch in enumerate(pitches):
                            plan.frames[frame].controls[1] = pitch
                    else:
                        pointer = ctypes.cast(ctypes.byref(plan.frames, offset * ctypes.sizeof(NativeFrame)),
                                              ctypes.POINTER(NativeFrame))
                        contour(owner).apply(pointer, count, part.text, fa.raw, owner._cancelled,
                                             continuation=i + 1 < len(sentence),
                                             preceded=i > 0, prefix=prefix)
                offset += count
            align_english_edges(plan, sentence, _bracket_boundary)
        if original_fa_clock:
            offset = 0
            for part, count in zip(sentence, plan.counts):
                if part.language == 'fa' and count:
                    _hold_acoustic_clock(plan.frames, offset, offset + count)
                offset += count
        marks = replay_marks(plan, original_counts)
        if stream_state:
            first = stream_state['first']
            size = first * ctypes.sizeof(NativeFrame)
            if ctypes.string_at(plan.frames, size) != stream_state['prefix_bytes']:
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
                result = None
                if group_key[0] in fa_ids:
                    result = try_persian_glides(owner, (EciLanguage(*group_key), *prefix, *items),
                                                 on_pcm, on_index, fa_ids=fa_ids)
                if result is None:
                    result = engine.speak((*prefix, *items), on_pcm, on_index)
                if not result:
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
    """Apply Persian glides and an experimental US-derived pitch contour."""
    key, items, prefix, saw_text, explicit = owner._home, [], [], False, False
    for part in parts:
        if isinstance(part, EciLanguage):
            next_key = owner._key(part.language, part.voice)
            if items and next_key != key:
                return None
            key = next_key
        elif isinstance(part, EciText) and part.is_control:
            if part.tag_control and _TAG.fullmatch(part.tag_control):
                items.append(part)
                explicit = saw_text = True
            elif not saw_text and (part.speed_control or persistent_tag_control(part) is not None):
                prefix.append(part)
            else:
                return None
        elif isinstance(part, (str, EciText, EciIndex)):
            items.append(part)
            if isinstance(part, (str, EciText)):
                saw_text = True
        elif isinstance(part, (EciPitch, EciPitchHz, EciProsody)) and saw_text:
            # Keep controls in the native capture when NVDA changes pitch or
            # speed inside the Persian part of a sentence.
            items.append(part)
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
    pending_pilot = None
    pending_target = None
    pilot_worker = None
    target_worker = None
    stream = None
    streamed_frames = 0
    streamed_marks = ()
    try:
        if explicit:
            # For explicit phonetic-only requests, plan the same selected
            # contour as ordinary Persian speech. One-shot controls are
            # converted once, then carried as immutable pronunciation text;
            # normal words and ambiguous geminated J keep the established
            # fallback until their event projection is implemented.
            prepared_parts, source_text, tagged_text = [], [], []
            for part in items:
                if isinstance(part, EciText) and part.is_control:
                    if re.search(r'J(?:\.[012])?J', part.text):
                        return None
                    converted = engine._convert_user_phonetic(part)
                    part = replace(converted, tag_control=None, speed_control=None,
                                   segmentation_guarded=True)
                    prepared_parts.append(part)
                    tagged_text.append(part.text)
                elif isinstance(part, (str, EciText)):
                    value = part if isinstance(part, str) else part.text
                    if any(c not in ' \t\r\n.,?!:;()[]{}،؛؟' for c in value):
                        return None
                    value = value.replace('،', ',').replace('؛', ';').replace('؟', '?')
                    prepared_parts.append(EciText(value, segmentation_guarded=True))
                    source_text.append(value)
                    tagged_text.append(value)
                else:
                    prepared_parts.append(part)
            tagged = ''.join(tagged_text)
            if re.search(r'\]`\[', tagged):
                return None
            prepared = ProjectedJob(''.join(source_text), tagged, tuple(prepared_parts), False)
        else:
            try:
                prepared = project(items, engine._frontend, allow_fallback=engine._allow_fallback,
                                   pinglish=engine._pinglish)
            except (ValueError, RuntimeError, OSError, UnicodeError):
                if owner._cancelled.is_set():
                    return False
                prepared = project_resilient(items, engine._frontend, pinglish=engine._pinglish,
                                             validate=engine._phonemes_for_validation,
                                             cancelled=owner._cancelled.is_set)
        matches = tuple(_TAG.finditer(prepared.tagged))
        tags = tuple(match.group() for match in matches)
        separators = tuple(prepared.tagged[a.end():b.start()] for a, b in zip(matches, matches[1:]))
        glide_words = tuple(i + 1 for i, (a, b, sep) in enumerate(zip(tags, tags[1:], separators))
                            if _FINAL_U.search(a) and _INITIAL_A.match(b) and 'w' not in a and
                            'w' not in b and sep.isspace())
        has_glide = bool(glide_words)
        korean_available = False
        us_available = bool(matches)
        if not has_glide and not korean_available and not us_available:
            return engine._speak_prepared(replace(prepared, parts=(*prefix, *prepared.parts)), on_pcm, on_index)
        inserts, source, offset, next_mark, internal_pitch = [], [], 0, 1, False
        for part in prepared.parts:
            if isinstance(part, EciText):
                offset += len(part.text)
            elif isinstance(part, EciIndex):
                inserts.append((offset, len(inserts), next_mark))
                source.append((next_mark, offset, part.value))
                next_mark += 1
            elif isinstance(part, (EciPitch, EciPitchHz, EciProsody)):
                inserts.append((offset, len(inserts), part))
                internal_pitch |= isinstance(part, EciPitch) and offset < len(prepared.tagged)
            else:
                return None
        starts = (_persian_sentence_starts(prepared.tagged, prepared.source, matches, separators)
                  if korean_available and matches else [])
        sentence_words = set(starts[1:])
        # A glide needs its own start and neighbouring word boundaries.
        # Unrelated word marks can make the native callback report later
        # sentence marks before the preceding sentence has actually ended.
        glide_boundaries = {j for i in glide_words for j in (i - 1, i, i + 1) if 0 < j < len(matches)}
        words = {}
        us_boundaries = set(range(1, len(matches))) if us_available else set()
        for word in sorted(sentence_words | glide_boundaries | us_boundaries):
            boundary = word_boundary(prepared.tagged, matches, word)
            inserts.append((boundary, -1, next_mark))
            words[word] = next_mark
            next_mark += 1
        sequence, position = [], 0
        us_marks = set(words.values()) if us_available else set()
        for at, _order, mark in sorted(inserts, key=lambda entry: entry[:2]):
            sequence.extend((prepared.tagged[position:at], EciIndex(mark) if isinstance(mark, int) else mark))
            if (isinstance(mark, int) and mark in us_marks and at < len(prepared.tagged) and
                    prepared.tagged[at] in '.,?!:;()[]{}'):
                sequence.append(' ')
            position = at
        sequence.append(prepared.tagged[position:])
        # DirectEciEngine applies Hertz pitch and speed/volume controls to
        # the whole queued utterance, even if supplied at its end. Match
        # that established behavior in the donor. EciPitch is queued and
        # its trailing reset must not change the preceding word.
        donor_prefix = (*prefix, *(part for part in items if isinstance(part, (EciPitchHz, EciProsody))))
        voice = _PlanningVoice(owner, key, engine, validate=prepared.needs_native_validation,
                               prefix=prefix, source=prepared.source)
        voice.raw._cancelled.clear()
        if owner._cancelled.is_set():
            return False
        english = proxy = None
        previous_tags = 1
        word_marks = tuple(words[word] for word in range(1, len(matches)))
        tagged_donor = _sentence_colons_as_periods(prepared.tagged, prepared.source)
        stream_candidate = (os.environ.get('RELOQ_FA_STREAM_PREFIX', '1') == '1' and
                            (os.cpu_count() or 1) > 1 and len(matches) >= 4 and
                            not prepared.needs_native_validation and
                            not internal_pitch and 1 not in glide_words and
                            all(offset in (0, len(prepared.tagged)) for _, offset, _ in source) and
                            all(re.fullmatch(r'\s*(?:`00\s*)?(?:[()\[\]{},،]\s*(?:`00\s*)?)*', sep)
                                for sep in separators) and
                            bool(word_marks) and
                            FrameStream.available(voice._dll) and
                            getattr(voice._dll, 'eciPersianCaptureCurrent', None) is not None and
                            getattr(owner, '_persian_capture_worker', None) is not None and
                            getattr(owner, '_persian_contour_worker', None) is not None)
        if us_available:
            english_key = owner._key(US_ENGLISH.id, key[1])
            english_engine = owner._activate(english_key)
            english = _PlanningVoice(owner, english_key, english_engine, prefix=donor_prefix)
            # A cancelled navigation request may have stopped this handle.
            # Clear only the stale stop while holding the owner's guard.
            with owner._guard:
                if owner._cancelled.is_set():
                    return False
                english.raw._cancelled.clear()
            previous_tags = english._dll.eciGetParam(english._handle, PROCESS_TAGS)
            if previous_tags < 0:
                raise RuntimeError('cannot read US tag setting')
            # The US pilot owns a separate handle. Begin its full capture
            # while the Persian handle captures the very same request; the
            # result is still fitted before any audio or index is emitted.
            # Short labels and single numbers benefit too: their target and
            # donor planning otherwise add up on the navigation path.
            if matches and (os.cpu_count() or 1) > 1:
                pilot_worker = getattr(owner, '_persian_contour_worker', None)
                if pilot_worker is not None and not stream_candidate:
                    try:
                        pending_pilot = pilot_worker.submit_proxy(
                            tagged=tagged_donor, marks=word_marks, prefix=donor_prefix,
                            parameters=owner._engine_parameters[english_key[0]],
                            dictionary=owner._dictionary_path, cancelled=owner._cancelled,
                            maximum=30000)
                    except (RuntimeError, OSError):
                        pending_pilot = None
        capture_api = FrameCapture(voice._dll)
        captured_rate = SAMPLE_RATES.get(voice._dll.eciGetParam(voice._handle, SAMPLE_RATE))
        original_clock = _original_persian_clock(voice.raw)
        captured = None
        # A verified prefix can be replayed while two independent captures
        # continue. Do not use this path for source marks inside a
        # phrase, inline pitch, reference validation, or a glide across the
        # first word: those need the complete established plan first.
        stream_ok = stream_candidate
        target_worker = getattr(owner, '_persian_capture_worker', None)
        if stream_ok and target_worker is not None and pilot_worker is not None:
            ready = queue.Queue()
            snapshot_at = word_marks[1]
            def publish(kind):
                return lambda value: ready.put((kind, value))
            pending_target = target_worker.submit_snapshot(
                parts=sequence, prefix=prefix, validate=False, source=prepared.source,
                parameters=owner._engine_parameters[key[0]], dictionary=owner._dictionary_path,
                cancelled=owner._cancelled, maximum=30000, snapshot_at=snapshot_at,
                on_snapshot=publish('target'))
            pending_pilot = pilot_worker.submit_proxy_snapshot(
                tagged=tagged_donor, marks=word_marks, prefix=donor_prefix,
                parameters=owner._engine_parameters[english_key[0]], dictionary=owner._dictionary_path,
                cancelled=owner._cancelled, maximum=30000, snapshot_at=snapshot_at,
                on_snapshot=publish('donor'))
            pending_target.add_done_callback(lambda _future: ready.put(('target_done', None)))
            pending_pilot.add_done_callback(lambda _future: ready.put(('donor_done', None)))
            snapshots, finished = {}, set()
            while len(snapshots) < 2 and not any(name + '_done' in finished and name not in snapshots
                                                 for name in ('target', 'donor')):
                if owner._cancelled.is_set():
                    raise InterruptedError('sentence planning was cancelled')
                try:
                    which, value = ready.get(timeout=.05)
                except queue.Empty:
                    continue
                if value is None:
                    finished.add(which)
                else:
                    snapshots[which] = value
            if len(snapshots) == 2:
                partial = CapturedRows.from_capture(snapshots['target'], native=capture_api.frame_ticks)
                donor_partial = CapturedRows.from_capture(snapshots['donor'], native=capture_api.frame_ticks)
                rate = captured_rate
                if rate and word_marks[0] in partial.indexes and word_marks[0] in donor_partial.indexes:
                    fit = fit_us_word_pitch(partial, (0, partial.indexes[word_marks[0]]),
                                            donor_partial, (0, donor_partial.indexes[word_marks[0]]),
                                            (), (), rate, capture_api,
                                            syllable_counts=(len(re.findall(r'\.[012]', tags[0])),))
                    boundary = round(partial.indexes[word_marks[0]] * 1000 / rate)
                    end = bisect_left(partial.ticks, max(0, boundary - 25))
                    if fit and end > 8:
                        if original_clock:
                            _hold_acoustic_clock(partial.frames)
                        streamed_marks = tuple((0, value) for _, offset, value in source if offset == 0)
                        owner._activate(key)
                        stream = FrameStream(voice.raw, on_pcm, on_index, cancelled=owner._cancelled)
                        stream.append(partial.frames, 0, end, streamed_marks)
                        streamed_frames = end
            try:
                captured = pending_target.result()
            except (ValueError, RuntimeError, OSError):
                if stream is None or owner._cancelled.is_set():
                    raise
                # If a worker failed after handing off the first prefix,
                # retry its full capture without replaying the prefix.
                pending_target = target_worker.submit(
                    parts=sequence, prefix=prefix, validate=False,
                    parameters=owner._engine_parameters[key[0]], dictionary=owner._dictionary_path,
                    cancelled=owner._cancelled, maximum=30000, audio=False)
                captured = pending_target.result()
            pending_target = None
        if captured is None:
            captured = capture_api.capture(voice, sequence, maximum=30000, audio=False)
        item = CapturedRows.from_capture(captured, native=capture_api.frame_ticks)
        sentence_item = item
        if sentence_words and (glide_boundaries - sentence_words or
                               any(0 < offset < len(prepared.tagged) for _, offset, _ in source)):
            # Obtain sentence marks without the extra source/glide indexes:
            # they can shift native callbacks by an entire preceding word.
            # Keep all controls, and only use the second pass if its native
            # frame clock agrees with the actual captured utterance.
            clean_inserts = [(matches[word].start(), -1, words[word]) for word in starts[1:]]
            clean_inserts.extend((at, order, event) for at, order, event in inserts
                                 if isinstance(event, (EciPitch, EciPitchHz, EciProsody)))
            clean_sequence, position = [], 0
            for at, _order, event in sorted(clean_inserts, key=lambda entry: entry[:2]):
                clean_sequence.extend((prepared.tagged[position:at],
                                       EciIndex(event) if isinstance(event, int) else event))
                position = at
            clean_sequence.append(prepared.tagged[position:])
            clean_capture = capture_api.capture(voice, clean_sequence, maximum=30000, audio=False)
            sentence_item = CapturedRows.from_capture(clean_capture, native=capture_api.frame_ticks)
            if tuple(sentence_item.ticks) != tuple(item.ticks):
                raise ValueError('sentence-only capture changed the native frame clock')
        pitch_ratios = None
        if internal_pitch and (korean_available or us_available):
            # Measure only the native effect of the queued pitch commands.
            # With those commands removed, the same phonemes must have an
            # identical frame clock; their pitch ratio can then be applied
            # to the Korean contour without restoring the Finnish melody.
            plain = capture_api.capture(
                voice, tuple(part for part in sequence if not isinstance(part, EciPitch)),
                maximum=30000, audio=False)
            reference = CapturedRows.from_capture(plain, native=capture_api.frame_ticks)
            if tuple(item.ticks) != tuple(reference.ticks):
                raise ValueError('inline pitch changed the native frame clock')
            pitch_ratios = tuple(a.controls[1] / b.controls[1]
                                 if a.controls[1] > 0 and b.controls[1] > 0 else 1.0
                                 for a, b in zip(item.frames, reference.frames))
        rate = captured_rate
        if rate is None:
            raise ValueError('invalid Persian sample rate')
        boundaries = {word: round(item.indexes[mark] * 1000 / rate) for word, mark in words.items()}
        sentence_boundaries = {word: round(sentence_item.indexes[words[word]] * 1000 / rate)
                               for word in starts[1:]}
        glides = ()
        if has_glide:
            applied = ctypes.c_size_t()
            native_glides = getattr(capture_api, 'apply_glides', None)
            if native_glides is not None and all(word in boundaries and
                    (word == 1 or word - 1 in boundaries) for word in glide_words):
                candidates = (NativeGlideCandidate * len(glide_words))(*(
                    NativeGlideCandidate(word, boundaries.get(word - 1, 0), boundaries[word],
                        boundaries.get(word + 1, item.ticks[-1] + 1)) for word in glide_words))
                status = native_glides(item.frames, len(item.frames), candidates, len(candidates),
                                       1, ctypes.byref(applied))
            else:
                status = -1
            if status == 0:
                glides = applied.value
            else:
                # Old libraries or unverified boundary clocks retain the
                # previous planner. The native function fails before edits.
                controls = [row.controls for row in item.frames]
                glides = plan_u_a_glides(controls, item.ticks, tags, separators, boundaries)
                apply_u_a_glides(controls, item.ticks, glides)
        if not glides and not korean_available and not us_available:
            return engine._speak_prepared(replace(prepared, parts=(*prefix, *prepared.parts)), on_pcm, on_index)
        transferred = False
        if us_available:
            donor_capture = None
            if pending_pilot is not None:
                future, pending_pilot = pending_pilot, None
                try:
                    donor_capture = future.result()
                except InterruptedError:
                    raise
                except (ValueError, RuntimeError, OSError):
                    if owner._cancelled.is_set():
                        raise InterruptedError('sentence planning was cancelled')
            if donor_capture is None:
                if previous_tags != 1:
                    english.raw.set_parameter(PROCESS_TAGS, 1)
                try:
                    proxy = indexed_us_proxy(tagged_donor, english, word_marks)
                    donor_capture = FrameCapture(english._dll).capture(
                        english, proxy, maximum=30000, audio=False)
                finally:
                    if previous_tags != 1:
                        english.raw.set_parameter(PROCESS_TAGS, previous_tags)
            donor = CapturedRows.from_capture(donor_capture, native=capture_api.frame_ticks)
            target = reference if pitch_ratios is not None else item
            transferred = fit_us_word_pitch(
                target, (0, len(target.pcm) // 2), donor, (0, len(donor.pcm) // 2),
                word_marks, word_marks, rate, capture_api,
                check=lambda: (_ for _ in ()).throw(InterruptedError()) if owner._cancelled.is_set() else None,
                syllable_counts=tuple(len(re.findall(r'\.[012]', tag)) for tag in tags))
            if not transferred and word_marks:
                # Closely spaced NVDA indexes can coincide with a native
                # word boundary. Keep the same US donor over the whole
                # phrase if its per-word alignment cannot be trusted.
                transferred = fit_us_word_pitch(
                    target, (0, len(target.pcm) // 2), donor, (0, len(donor.pcm) // 2),
                    (), (), rate, capture_api,
                    check=lambda: (_ for _ in ()).throw(InterruptedError()) if owner._cancelled.is_set() else None)
            if not transferred and not any(row.controls[2] > 0 and row.controls[1] > 0
                                           for row in target.frames):
                # An entirely unvoiced sound has no audible pitch to fit.
                transferred = True
            if not transferred:
                # Some very short or weak proxies do not yield two voiced
                # US frames. Use a single voiced US syllable for this rare
                # emergency contour, preserving the ending's punctuation.
                # This affects only pitch, never the Persian phones or timing.
                ending = prepared.source.rstrip()[-1:]
                punctuation = '?' if ending in ('?', '؟') else ',' if ending in (',', '،') else '.'
                if previous_tags != 1:
                    english.raw.set_parameter(PROCESS_TAGS, 1)
                try:
                    rescue = CapturedRows.from_capture(FrameCapture(english._dll).capture(
                        english, ('`[.1ma]' + punctuation,), maximum=30000, audio=False))
                finally:
                    if previous_tags != 1:
                        english.raw.set_parameter(PROCESS_TAGS, previous_tags)
                transferred = fit_us_word_pitch(
                    target, (0, len(target.pcm) // 2), rescue, (0, len(rescue.pcm) // 2),
                    (), (), rate, capture_api,
                    check=lambda: (_ for _ in ()).throw(InterruptedError()) if owner._cancelled.is_set() else None)
            if transferred and pitch_ratios is not None:
                for frame in range(len(item.frames)):
                    if item.frames[frame].controls[1] > 0 and target.frames[frame].controls[1] > 0:
                        item.frames[frame].controls[1] = target.frames[frame].controls[1] * pitch_ratios[frame]
        if korean_available and matches:
            # Plan each sentence over its own validated native frame span.
            text_edges = [0] + [matches[index].start() for index in starts[1:]] + [len(prepared.tagged)]
            frame_edges = [0] + [bisect_left(item.ticks, sentence_boundaries[index])
                                 for index in starts[1:]] + [len(item.frames)]
            for i, (first, last) in enumerate(zip(frame_edges, frame_edges[1:])):
                if first >= last:
                    continue
                # The older transfer API normalizes its donor to the target's
                # median. Use the no-command reference as that target so a
                # queued pitch change is applied exactly once in either API.
                target = reference.frames if pitch_ratios is not None else item.frames
                pointer = ctypes.cast(ctypes.byref(target, first * ctypes.sizeof(NativeFrame)),
                                      ctypes.POINTER(NativeFrame))
                tagged = prepared.tagged[text_edges[i]:text_edges[i + 1]]
                applied = contour(owner).apply(pointer, last - first, tagged, voice.raw, owner._cancelled,
                                               prefix=donor_prefix)
                if applied and pitch_ratios is not None:
                    for frame in range(first, last):
                        if item.frames[frame].controls[1] > 0 and target[frame].controls[1] > 0:
                            item.frames[frame].controls[1] = target[frame].controls[1] * pitch_ratios[frame]
                transferred |= applied
        if not glides and not transferred:
            if stream is not None:
                stream.close(flush=False)
                stream = None
                return False
            return engine._speak_prepared(replace(prepared, parts=(*prefix, *prepared.parts)), on_pcm, on_index)
        if original_clock:
            _hold_acoustic_clock(item.frames)
        marks = []
        for mark, offset, value in source:
            frame = (0 if offset == 0 else len(item.frames) if offset == len(prepared.tagged) else
                     min(len(item.frames), bisect_left(item.ticks, round(item.indexes[mark] * 1000 / rate))))
            marks.append((frame, value))
    except InterruptedError:
        if stream is not None:
            try:
                stream.close(flush=False)
            except (InterruptedError, RuntimeError):
                pass
        return False
    except (ValueError, RuntimeError, OSError) as error:
        if stream is not None:
            try:
                stream.close(flush=False)
            except (InterruptedError, RuntimeError):
                pass
        if owner._cancelled.is_set():
            return False
        _log.debug('Persian vowel glide unavailable: %s', error)
        return False if streamed_frames else None
    except BaseException:
        if stream is not None:
            try:
                stream.close(flush=False)
            except (InterruptedError, RuntimeError):
                pass
        raise
    finally:
        if pending_target is not None:
            if not pending_target.cancel():
                target_worker.cancel()
            try:
                pending_target.result()
            except BaseException:
                pass
        if pending_pilot is not None:
            if not pending_pilot.cancel():
                pilot_worker.cancel()
            try:
                pending_pilot.result()
            except BaseException:
                pass
    if owner._cancelled.is_set():
        if stream is not None:
            try:
                stream.close(flush=False)
            except (InterruptedError, RuntimeError):
                pass
        return False
    owner._activate(key)
    if stream is not None:
        try:
            remaining = tuple((at - streamed_frames, value) for at, value in marks
                              if at >= streamed_frames)
            stream.append(item.frames, streamed_frames, len(item.frames), remaining)
            stream.close(flush=True)
            stream = None
            return True
        finally:
            if stream is not None:
                stream.close(flush=False)
    return FrameReplay(voice._dll).replay(voice.raw, item.frames, marks, on_pcm, on_index,
                                         cancelled=owner._cancelled)


def sentence_jobs(parts, home, *, hard_only=False):
    """Split speech at unambiguous native phrase punctuation.

    For a single Persian language, split only complete sentences. This lets
    NVDA start a long passage without planning every later sentence, while
    keeping commas and other clause boundaries in their original context.
    """
    if any(not isinstance(p, (str, EciText, EciIndex, EciLanguage, EciCapitalWord)) or
           isinstance(p, EciText) and (p.character_mode or p.is_control) for p in parts):
        return ()
    full = ''.join(p if isinstance(p, str) else p.text if isinstance(p, EciText) else
                   p.original if isinstance(p, EciCapitalWord) else '' for p in parts)
    if hard_only and '`' in full:
        return ()
    punctuation = '.!?؟:\n' if hard_only else '.!?؟,;:،؛\n'
    if not any(c in full for c in punctuation):
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
            if char not in punctuation:
                continue
            after = full[absolute + 1:absolute + 2]
            if hard_only and char not in ':\n' and after and not after.isspace():
                continue
            if char == ':' and not _colon_ends_sentence(full, absolute):
                continue
            if char in '.,;،؛' and after and not after.isspace():
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
        if isinstance(item, EciText) and not item.is_control:
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
            controls['speed'] = EciText(part.speed_control + ' ', speed_control=part.speed_control)
        elif isinstance(part, EciText):
            replay = persistent_tag_control(part)
            if replay is not None:
                slot, control = replay
                controls.pop(slot, None)
                controls[slot] = control
    result.append((tuple(pending), False))
    return tuple(result)


def continuation_comma(parts):
    """Ask the current native core for a nonfinal separated phrase tone."""
    parts = tuple(parts)
    def visible(part):
        return (part if isinstance(part, str) else part.text if isinstance(part, EciText) and
                not part.is_control and not part.character_mode else
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
                    and not part.is_control else part.original if isinstance(part, EciCapitalWord) else '')
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
            controls['speed'] = EciText(part.speed_control + ' ', speed_control=part.speed_control)
            pending.append(part)
            continue
        if isinstance(part, EciText) and part.tag_control:
            replay = persistent_tag_control(part)
            if replay is not None:
                slot, control = replay
                controls.pop(slot, None)
                controls[slot] = control
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
            pending.append(piece if isinstance(part, str) else replace(part, text=piece))
            size += len(piece)
            if value:
                flush()
    if pending:
        jobs.append(tuple(pending))
    return tuple(jobs)
