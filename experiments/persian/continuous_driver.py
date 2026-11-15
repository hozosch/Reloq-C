"""Connect the selected sentence planner to one NVDA language job."""
from __future__ import annotations

import logging
import re
from bisect import bisect_left

if __package__:
    from .._reloq_eci_direct import (EciCapitalWord, EciIndex, EciLanguage, EciText,
                                     SAMPLE_RATE, SAMPLE_RATES, PROCESS_TAGS)
    from .._reloq_persian_frames import FrameCapture, FrameReplay
    from .._reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
    from .._reloq_persian_projection import ProjectedJob
    from .direct_fa_sequence import ScopedDirectPersianEngine, project
    from .continuous_plan import CapturedRows, context_plan, replay_marks, selected_sentence
    from .text_frontend import SpeechPart, UnsupportedPronunciation
else:
    from _reloq_eci_direct import (EciCapitalWord, EciIndex, EciLanguage, EciText,
                                    SAMPLE_RATE, SAMPLE_RATES, PROCESS_TAGS)
    from _reloq_persian_frames import FrameCapture, FrameReplay
    from _reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
    from _reloq_persian_projection import ProjectedJob
    from direct_fa_sequence import ScopedDirectPersianEngine, project
    from continuous_plan import CapturedRows, context_plan, replay_marks, selected_sentence
    from text_frontend import SpeechPart, UnsupportedPronunciation

_log = logging.getLogger(__name__)
_TAG = re.compile(r'`\[[^]]+\]')
_FINAL_U = re.compile(r"(?:u|'u:')\]$")
_INITIAL_A = re.compile(r"^`\[\.[012](?:a|'a:')")


class _PlanningVoice:
    """Activate the right owned handle before each private synthesis pass."""
    def __init__(self, owner, key, engine, *, validate=False):
        self.owner, self.key, self.engine, self.validate = owner, key, engine, validate
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
        result = self.raw._phonemes_for_probe(text)
        self._check()
        return result

    def speak(self, parts, on_pcm, on_index):
        self._check()
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
        if isinstance(part, EciText) and (part.character_mode or part.speed_control):
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
    prepared = project((*items, suffix) if suffix else items, engine._frontend,
                       allow_fallback=engine._allow_fallback, pinglish=engine._pinglish)
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
                   reset_cancellation=True):
    """Return None when this job must retain the established native route."""
    if reset_cancellation:
        owner._cancelled.clear()
    try:
        groups = _segments(owner, parts, fa_ids, english_id)
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
        sentence, events, guarded, validate = [], [], [], False
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
                        offset += len(value)
                sentence.append(SpeechPart('en-us', ''.join(text)))
        if any(part.text.rstrip().endswith(('.', '?', '!', ',', ';', ':', '،', '؟', '؛'))
               for part in sentence[:-1]):
            return None
        en_engine = owner._activate(en_key)
        german_key = owner._key(german_id, en_key[1])
        german_engine = owner._activate(german_key)
        fa = _PlanningVoice(owner, fa_key, fa_engine, validate=validate)
        en = _PlanningVoice(owner, en_key, en_engine)
        german = _PlanningVoice(owner, german_key, german_engine)
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
                            guarded_spans=guarded, check=check_cancel)
        original_counts = plan.counts
        selected_sentence(plan, sentence, en, german, rate, check=check_cancel)
        marks = replay_marks(plan, original_counts)
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
    key, items = owner._home, []
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
        prepared = project(items, engine._frontend, allow_fallback=engine._allow_fallback,
                           pinglish=engine._pinglish)
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
            else:
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
        voice = _PlanningVoice(owner, key, engine, validate=prepared.needs_native_validation)
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
    """Split ordinary multi-sentence jobs at unambiguous full stops."""
    if any(not isinstance(p, (str, EciText, EciIndex, EciLanguage, EciCapitalWord)) or
           isinstance(p, EciText) and (p.character_mode or p.speed_control) for p in parts):
        return ()
    full = ''.join(p if isinstance(p, str) else p.text if isinstance(p, EciText) else
                   p.original if isinstance(p, EciCapitalWord) else '' for p in parts)
    if not any(c in full for c in '.!?؟\n'):
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
            if char not in '.!?؟\n':
                continue
            after = full[absolute + 1:absolute + 2]
            if char == '.' and after and not after.isspace():
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
