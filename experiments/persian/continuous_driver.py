"""Connect the selected sentence planner to one NVDA language job."""
from __future__ import annotations

import logging
import re

if __package__:
    from .._reloq_eci_direct import (EciCapitalWord, EciIndex, EciLanguage, EciText,
                                     SAMPLE_RATE, SAMPLE_RATES, PROCESS_TAGS)
    from .._reloq_persian_frames import FrameReplay
    from .._reloq_persian_projection import ProjectedJob
    from .direct_fa_sequence import ScopedDirectPersianEngine, project
    from .continuous_plan import context_plan, replay_marks, selected_sentence
    from .text_frontend import SpeechPart, UnsupportedPronunciation
else:
    from _reloq_eci_direct import (EciCapitalWord, EciIndex, EciLanguage, EciText,
                                    SAMPLE_RATE, SAMPLE_RATES, PROCESS_TAGS)
    from _reloq_persian_frames import FrameReplay
    from _reloq_persian_projection import ProjectedJob
    from direct_fa_sequence import ScopedDirectPersianEngine, project
    from continuous_plan import context_plan, replay_marks, selected_sentence
    from text_frontend import SpeechPart, UnsupportedPronunciation

_log = logging.getLogger(__name__)
_TAG = re.compile(r'`\[[^]]+\]')


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


def try_continuous(owner, parts, on_pcm, on_index, *, fa_ids, english_id, german_id):
    """Return None when this job must retain the established native route."""
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
        sentence, events, validate = [], [], False
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
        plan = context_plan(sentence, fa, en, rate, maximum=30000, source_events=events)
        original_counts = plan.counts
        selected_sentence(plan, sentence, en, german, rate)
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
