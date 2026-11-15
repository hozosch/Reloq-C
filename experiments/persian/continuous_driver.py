"""Connect the selected sentence planner to one NVDA language job."""
from __future__ import annotations

import logging
from collections import OrderedDict
import ctypes
import re
from bisect import bisect_left
from dataclasses import replace

if __package__:
    from .._reloq_eci_direct import (EciCapitalWord, EciIndex, EciLanguage, EciPitch, EciPitchHz,
                                     EciProsody, EciText,
                                     SAMPLE_RATE, SAMPLE_RATES, PROCESS_TAGS)
    from .._reloq_persian_frames import FrameCapture, FrameReplay, NativeFrame
    from .._reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
    from .._reloq_persian_projection import ProjectedJob
    from .direct_fa_sequence import ScopedDirectPersianEngine, project, project_resilient
    from .continuous_plan import CapturedRows, context_plan, replay_marks, selected_sentence
    from .text_frontend import SpeechPart, UnsupportedPronunciation
else:
    from _reloq_eci_direct import (EciCapitalWord, EciIndex, EciLanguage, EciPitch, EciPitchHz,
                                    EciProsody, EciText,
                                    SAMPLE_RATE, SAMPLE_RATES, PROCESS_TAGS)
    from _reloq_persian_frames import FrameCapture, FrameReplay, NativeFrame
    from _reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
    from _reloq_persian_projection import ProjectedJob
    from direct_fa_sequence import ScopedDirectPersianEngine, project, project_resilient
    from continuous_plan import CapturedRows, context_plan, replay_marks, selected_sentence
    from text_frontend import SpeechPart, UnsupportedPronunciation

_log = logging.getLogger(__name__)
_TAG = re.compile(r'`\[[^]]+\]')
_FINAL_U = re.compile(r"(?:u|'u:')\]$")
_INITIAL_A = re.compile(r"^`\[\.[012](?:a|'a:')")


class _PlanningVoice:
    """Activate the right owned handle before each private synthesis pass."""
    def __init__(self, owner, key, engine, *, validate=False, prefix=()):
        self.owner, self.key, self.engine, self.validate = owner, key, engine, validate
        self.prefix = tuple(prefix)
        self.raw = engine._voice if isinstance(engine, ScopedDirectPersianEngine) else engine
        self._dll, self._handle = self.raw._dll, self.raw._handle

    def _check(self):
        if self.owner._cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        self.owner._activate(self.key)
        if self.owner._cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')

    def _phonemes_for_probe(self, text):
        self._check()
        cache = getattr(self.owner, '_persian_phoneme_cache', None)
        if cache is None:
            cache = self.owner._persian_phoneme_cache = OrderedDict()
        key = (self.key, text, self.owner._dictionary_path,
               tuple(sorted(self.owner._parameters.items())), self.owner._acoustic_clocks,
               self._dll.eciGetParam(self._handle, PROCESS_TAGS))
        if key in cache:
            cache.move_to_end(key)
            return cache[key]
        result = self.raw._phonemes_for_probe(text)
        self._check()
        cache[key] = result
        if len(cache) > 4096:
            cache.popitem(last=False)
        return result

    def speak(self, parts, on_pcm, on_index):
        self._check()
        parts = (*self.prefix, *parts)
        if isinstance(self.engine, ScopedDirectPersianEngine):
            tagged_parts = tuple(EciText(p, segmentation_guarded=True) if isinstance(p, str) else p
                                 for p in parts)
            tagged = ''.join(p.text for p in tagged_parts if isinstance(p, EciText))
            prepared = ProjectedJob('', tagged, tagged_parts, self.validate)
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
                                     validate=engine._voice._phonemes_for_probe,
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
    if not hasattr(fa_engine._voice._dll, 'reloq_fa_replay_frames'):
        return None
    fa_engine._cancelled.clear()
    restores = []
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
        # Repeated NVDA labels usually differ only in their index values.
        # Cache bounded *control frames*, never rendered audio, and remap the
        # fresh indices on replay. Voice settings and dictionary changes
        # are part of the key; the owning worker alone accesses this cache.
        cache = getattr(owner, '_persian_plan_cache', None)
        if cache is None:
            cache = owner._persian_plan_cache = OrderedDict()
        key = (fa_key, en_key, tuple(sentence), tuple((a, b) for a, b, _ in events),
               tuple(guarded), tuple(spelling), tuple(prefix), validate,
               owner._dictionary_path, tuple(sorted(owner._parameters.items())), owner._acoustic_clocks,
               end_pause_shortened)
        cached = cache.get(key)
        if cached is not None:
            cache.move_to_end(key)
            data, count, points = cached
            frames = (NativeFrame * count).from_buffer_copy(data)
            marks = tuple((position, events[ordinal][2]) for position, ordinal in points)
            if owner._cancelled.is_set():
                return False
            owner._activate(fa_key)
            return FrameReplay(fa_engine._voice._dll).replay(fa_engine._voice, frames, marks,
                on_pcm, on_index, cancelled=owner._cancelled)
        en_engine = owner._activate(en_key)
        german_key = owner._key(german_id, en_key[1])
        german_engine = owner._activate(german_key)
        fa = _PlanningVoice(owner, fa_key, fa_engine, validate=validate, prefix=prefix)
        en = _PlanningVoice(owner, en_key, en_engine, prefix=prefix)
        german = _PlanningVoice(owner, german_key, german_engine, prefix=prefix)
        for voice in (fa, en, german):
            voice.raw._cancelled.clear()
        if owner._cancelled.is_set():
            return False
        # The planner uses private phoneme tags even when the user disables
        # source-side tags. Restore the user's setting before leaving.
        for voice in (en, german):
            previous = voice._dll.eciGetParam(voice._handle, PROCESS_TAGS)
            if previous < 0:
                raise RuntimeError('cannot read native tag setting')
            if previous != 1:
                voice.raw.set_parameter(PROCESS_TAGS, 1)
                restores.append((voice.raw, previous))
        rate = SAMPLE_RATES.get(fa._dll.eciGetParam(fa._handle, SAMPLE_RATE))
        if rate is None:
            raise ValueError('invalid sentence sample rate')
        def check_cancel():
            if owner._cancelled.is_set():
                raise InterruptedError('sentence planning was cancelled')
        plan = context_plan(sentence, fa, en, rate, maximum=30000, source_events=events,
                            guarded_spans=guarded, spelling_spans=spelling, check=check_cancel)
        original_counts = plan.counts
        selected_sentence(plan, sentence, en, german, rate, check=check_cancel)
        marks = replay_marks(plan, original_counts)
        if len(marks) == len(events):
            size = len(plan.frames) * ctypes.sizeof(NativeFrame)
            if size <= 4 * 1024 * 1024:
                data = ctypes.string_at(plan.frames, size)
                cache[key] = (data, len(plan.frames), tuple((position, i) for i, (position, _) in enumerate(marks)))
                # Put an upper bound on captured control-frame memory.
                while len(cache) > 12 or sum(len(value[0]) for value in cache.values()) > 12 * 1024 * 1024:
                    cache.popitem(last=False)
    except InterruptedError:
        return False
    except (ValueError, RuntimeError, OSError) as error:
        if owner._cancelled.is_set():
            return False
        _log.warning('Cannot plan a continuous Persian sentence: %s', error)
        return None
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
    if not isinstance(engine, ScopedDirectPersianEngine) or not hasattr(engine._voice._dll, 'reloq_fa_replay_frames'):
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
                                         validate=engine._voice._phonemes_for_probe,
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
    """Separate language cores at explicit bracket phrase boundaries.

    Native index callbacks do not always mark the acoustic start of the
    following word. At a real bracket boundary, a separate native job keeps
    the entire next word in its own voice and avoids proxy synthesis. Marks
    remain in source order and active inline controls are copied forward.
    """
    parts = tuple(parts)
    result, pending, controls, key = [], [], OrderedDict(), home
    segment = []

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
            bracket = ((left_char in '()[]{}' and left_char and not left_spelled) or
                       (right_char in '()[]{}' and right_char and not right_spelled))
            if changed and has_both and left_char and right_char and bracket:
                result.append((tuple(pending), True))
                pending = [part, *controls.values()]
            else:
                pending.append(part)
            key, segment = selected, []
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
