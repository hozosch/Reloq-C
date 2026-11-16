"""Exercise the real NVDA language/queue contract with a private FA voice.

This factory is only for tests. Its Persian IDs are not public ECI dialects.
Explicit and automatic English switches can use the selected continuous
sentence plan when the private host exposes frame capture and replay.
"""
from __future__ import annotations

from dataclasses import replace
from pathlib import Path
from bisect import bisect_right
import os
import re
import unicodedata

if __package__:
    from .direct_fa_sequence import ScopedDirectPersianEngine
    from .hybrid_fa_frontend import DirectFirstPersian
    from .pinglish_frontend import english_exception, latin_spans
    from .continuous_driver import bounded_jobs, bracket_jobs, continuation_comma, sentence_jobs, try_continuous, try_persian_glides
    from .._reloq_eci_direct import (DirectEciEngine, EciCapitalWord, EciIndex, EciLanguage,
                                     EciPitch, EciPitchHz, EciProsody, EciText, PAUSE_MODE)
    from .._reloq_languages import Language, LanguageEngine, US_ENGLISH, GERMAN, FINNISH
else:
    from direct_fa_sequence import ScopedDirectPersianEngine
    from hybrid_fa_frontend import DirectFirstPersian
    from pinglish_frontend import english_exception, latin_spans
    from continuous_driver import bounded_jobs, bracket_jobs, continuation_comma, sentence_jobs, try_continuous, try_persian_glides
    from _reloq_eci_direct import (DirectEciEngine, EciCapitalWord, EciIndex, EciLanguage,
                                  EciPitch, EciPitchHz, EciProsody, EciText, PAUSE_MODE)
    from _reloq_languages import Language, LanguageEngine, US_ENGLISH, GERMAN, FINNISH

PERSIAN_PROBE = Language(0x7f0000, 'Persian test', 'fa_IR', 'utf-8', US_ENGLISH.voices)
PINGLISH_PROBE = Language(0x7f0001, 'Persian (Pinglish) test', 'fa_Latn', 'utf-8', US_ENGLISH.voices)
PROFILES = {profile.id: profile for profile in
            (PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH, GERMAN, FINNISH)}


class OwnedPersianEngine(ScopedDirectPersianEngine):
    """One worker owns the acoustic handle and optional isolated reader."""
    def __init__(self, library, native_library, worker, espeak_library, data_parent, *, pinglish=False):
        frontend = DirectFirstPersian(native_library, worker, espeak_library, data_parent)
        try:
            super().__init__(Path(library), frontend, allow_fallback=True, pinglish=pinglish)
        except BaseException:
            frontend.close()
            raise

    def set_dictionary_path(self, path):
        self._voice.set_dictionary_path(path)

    def close(self):
        try:
            super().close()
        finally:
            self._frontend.close()


def _script_runs(text):
    """Preserve every character while separating Latin from other letters."""
    runs, prefix, current, kind = [], '', '', None
    for char in text:
        strong = ('en' if unicodedata.name(char, '').startswith('LATIN ') else
                  'fa' if char.isalpha() else None)
        if strong is None:
            if kind is None:
                prefix += char
            else:
                current += char
            continue
        if kind is None:
            kind, current, prefix = strong, prefix, ''
        elif strong != kind:
            runs.append((kind, current))
            kind, current = strong, ''
        current += char
    if kind is None:
        return (('fa', prefix),) if prefix else ()
    runs.append((kind, current))
    return tuple(runs)


def _persian_part_runs(parts, home):
    """Keep neutral fragments with their script across NVDA text/index pieces."""
    result, entries, logical = {}, [], home

    def flush():
        if not entries:
            return
        text = ''.join(value for _, value, _ in entries)
        runs = _script_runs(text)
        if runs:
            spans, at = [], 0
            for kind, value in runs:
                spans.append((at, at + len(value), kind))
                at += len(value)
            position = 0
            for index, value, ordinary in entries:
                end = position + len(value)
                if ordinary:
                    pieces = tuple((kind, value[max(a, position) - position:min(b, end) - position])
                                   for a, b, kind in spans if a < end and b > position)
                    if pieces:
                        result[index] = pieces
                position = end
        entries.clear()

    for index, part in enumerate(parts):
        if isinstance(part, EciLanguage):
            flush()
            logical = (part.language, part.voice)
        elif logical[0] == PERSIAN_PROBE.id and isinstance(part, (str, EciText)):
            text = part if isinstance(part, str) else part.text
            if ('`' in text or isinstance(part, EciText) and
                    (part.character_mode or part.is_control)):
                flush()
            else:
                entries.append((index, text, True))
        elif logical[0] == PERSIAN_PROBE.id and isinstance(part, EciCapitalWord):
            runs = _script_runs(part.original)
            if runs and all(kind == 'en' for kind, _ in runs):
                entries.append((index, part.original, False))
            else:
                flush()
        elif not isinstance(part, (EciIndex, EciPitch, EciPitchHz, EciProsody)):
            flush()
    flush()
    return result


def _pinglish_part_runs(parts, home):
    """Find English exceptions across text chunks without moving commands."""
    result, entries, logical = {}, [], home

    def flush():
        if not entries:
            return
        text = ''.join(value for _, value in entries)
        words = tuple(latin_spans(text))
        spans = [(start, end) for start, end in words if english_exception(text[start:end])]
        for number in re.finditer(r'[+\-−]?[0-9]+(?:[.,][0-9]+)*', text):
            previous = next(((a, b) for a, b in reversed(words) if b <= number.start()), None)
            following = next(((a, b) for a, b in words if a >= number.end()), None)
            if (previous and english_exception(text[previous[0]:previous[1]]) and
                    not text[previous[1]:number.start()].strip()):
                spans.append((previous[1], number.end()))
            elif (previous is None and following and english_exception(text[following[0]:following[1]]) and
                  not text[number.end():following[0]].strip()):
                spans.append((number.start(), following[0]))
        offset = 0
        for index, value in entries:
            end = offset + len(value)
            cuts = sorted({offset, end, *(edge for a, b in spans for edge in (a, b)
                                          if offset < edge < end)})
            pieces = []
            for a, b in zip(cuts, cuts[1:]):
                english = any(left <= a and b <= right for left, right in spans)
                pieces.append((english, value[a - offset:b - offset]))
            if any(english for english, _ in pieces):
                result[index] = tuple(pieces)
            offset = end
        entries.clear()

    for index, part in enumerate(parts):
        if isinstance(part, EciLanguage):
            flush()
            logical = (part.language, part.voice)
        elif logical[0] == PINGLISH_PROBE.id and isinstance(part, (str, EciText)):
            value = part if isinstance(part, str) else part.text
            if ('`' in value or isinstance(part, EciText) and
                    (part.character_mode or part.is_control)):
                flush()
            else:
                entries.append((index, value))
        elif not isinstance(part, (EciIndex, EciPitch, EciPitchHz, EciProsody)):
            flush()
    flush()
    return result


_BRACKET_CONTINUATION = re.compile(r'[)\]}](?=\s*(?::|$))')


def _bracket_continuations(parts):
    """Apply native punctuation precedence to a closing bracket.

    A bare closing bracket at EOF receives the native comma contour. Before
    a colon, let the colon itself determine the contour; remove the intervening
    closing bracket only from the internal synthesis input. Character mode,
    explicit sentence endings and user-supplied ECI tags are untouched.
    """
    if not any(any(char in (part if isinstance(part, str) else part.text
                            if isinstance(part, EciText) else '') for char in ')]}')
               for part in parts):
        return parts
    text, spans, length = [], [], 0
    for index, part in enumerate(parts):
        value = (part if isinstance(part, str) else part.text if isinstance(part, EciText) else
                 part.original if isinstance(part, EciCapitalWord) else None)
        if value is None or isinstance(part, EciText) and part.is_control:
            continue
        spelled = isinstance(part, EciText) and part.character_mode
        text.append(value)
        spans.append((length, length + len(value), index, spelled or isinstance(part, EciCapitalWord)))
        length += len(value)
    value = ''.join(text)
    # Raw ECI tags can contain brackets or control their own prosody.
    if '`' in value or not any(char.isalnum() for char in value):
        return parts
    edits = {}
    ends = [end for _start, end, _index, _spelled in spans]
    def position(offset):
        index = bisect_right(ends, offset)
        if index >= len(spans):
            return None
        start, _end, part_index, spelled = spans[index]
        return part_index, offset - start, spelled
    span_index = 0
    for match in _BRACKET_CONTINUATION.finditer(value):
        offset = match.start()
        while span_index < len(spans) and spans[span_index][1] <= offset:
            span_index += 1
        if span_index == len(spans):
            break
        start, _end, index, spelled = spans[span_index]
        if spelled:
            continue
        previous = offset - 1
        while previous >= 0 and (value[previous].isspace() or value[previous] in ')]}'):
            previous -= 1
        if previous < 0 or value[previous] in '.!?؟':
            continue
        following = offset + 1
        while following < len(value) and value[following].isspace():
            following += 1
        if following < len(value) and value[following] == ':':
            # Remove only the closing-bracket cluster and adjacent spaces;
            # keep the colon and any source indexes in their original order.
            selected = [position(i) for i in range(previous + 1, following)]
            if not selected or any(item is None or item[2] for item in selected):
                continue
            for part_index, local, _ in selected:
                edits.setdefault(part_index, {})[local] = ''
        else:
            edits.setdefault(index, {})[offset - start] = value[offset] + ','
    if not edits:
        return parts
    result = list(parts)
    for index, changes in edits.items():
        part = result[index]
        value = part if isinstance(part, str) else part.text
        value = ''.join(changes.get(offset, char) for offset, char in enumerate(value))
        result[index] = value if isinstance(part, str) else replace(part, text=value)
    return tuple(result)


class PersianProbeLanguageEngine(LanguageEngine):
    """Use English for explicit and clear Latin-script boundaries."""

    def select_voice(self, language, voice):
        super().select_voice(language, voice)
        if (self._home[0] not in (PERSIAN_PROBE.id, PINGLISH_PROBE.id) or
                not isinstance(self._engines.get(self._home[0]), ScopedDirectPersianEngine)):
            return
        if (getattr(self, '_persian_contour_worker', None) is None and
                (os.cpu_count() or 1) > 1):
            if __package__:
                from .parallel_pilot import ParallelPilot
            else:
                from parallel_pilot import ParallelPilot
            try:
                # Load a second voice handle in its own thread while NVDA is
                # selecting the voice. No speech results are retained.
                self._persian_contour_worker = ParallelPilot(self._dll)
            except (RuntimeError, OSError):
                pass
        if (US_ENGLISH.id in self._languages and
                not getattr(self, '_persian_english_prepared', False)):
            home = self._home
            try:
                english = self._activate(self._key(US_ENGLISH.id, home[1]))
                # Trigger the native rules' one-time initialization while
                # this voice is selected. Discard the neutral probe entirely;
                # every real text is analyzed again with current dictionaries.
                # Selection runs on the owning queue after earlier speech has
                # stopped. Clear a previous stop on this idle handle; a new
                # stop can still interrupt the probe through the same guard.
                with self._guard:
                    english._cancelled.clear()
                english._phonemes_for_probe('hello')
                self._persian_english_prepared = True
            except (AttributeError, OSError, RuntimeError, UnicodeError, ValueError):
                pass
            finally:
                self._activate(home)

    def cancel(self):
        super().cancel()
        worker = getattr(self, '_persian_contour_worker', None)
        if worker is not None:
            worker.cancel()

    def close(self):
        worker = getattr(self, '_persian_contour_worker', None)
        if worker is not None:
            self._persian_contour_worker = None
            worker.close()
        super().close()

    def speak(self, parts, on_pcm, on_index):
        self._assert_owner()
        def valid_text(value):
            return ''.join('\u2400' if char == '\0' else '\ufffd' if 0xd800 <= ord(char) <= 0xdfff
                           else char for char in value)
        # ECI's UTF-8 API cannot receive NUL or lone UTF-16 surrogates. Keep
        # their positions as identifiable Unicode replacement characters.
        parts = tuple(valid_text(part) if isinstance(part, str) else
                      replace(part, text=valid_text(part.text)) if isinstance(part, EciText) else
                      replace(part, original=valid_text(part.original), segmented=valid_text(part.segmented))
                      if isinstance(part, EciCapitalWord) else part for part in parts)
        logical = actual = self._home
        output = []

        def select(key):
            nonlocal actual
            if key != actual:
                # Events at a boundary describe the next text, rather than
                # making the old engine synthesize an event-only fragment.
                events = []
                while output and isinstance(output[-1], (EciIndex, EciPitch, EciPitchHz, EciProsody)):
                    events.append(output.pop())
                output.append(EciLanguage(*key))
                output.extend(reversed(events))
                actual = key

        persian_runs = _persian_part_runs(parts, logical)
        pinglish_runs = _pinglish_part_runs(parts, logical)
        for position, part in enumerate(parts):
            if isinstance(part, EciLanguage):
                logical = self._key(part.language, part.voice)
                select(logical)
                continue
            text = part if isinstance(part, str) else part.text if isinstance(part, EciText) else None
            ordinary_text = (text is not None and '`' not in text and
                             (not isinstance(part, EciText) or
                              not part.character_mode and not part.is_control))
            if position in pinglish_runs and logical[0] == PINGLISH_PROBE.id:
                english = self._key(US_ENGLISH.id, logical[1])
                for use_english, fragment in pinglish_runs[position]:
                    select(english if use_english else logical)
                    output.append(replace(part, text=fragment) if isinstance(part, EciText) else fragment)
                continue
            if position in persian_runs and logical[0] == PERSIAN_PROBE.id:
                english = self._key(US_ENGLISH.id, logical[1])
                for kind, fragment in persian_runs[position]:
                    select(english if kind == 'en' else logical)
                    output.append(replace(part, text=fragment) if isinstance(part, EciText) else fragment)
                continue
            if logical[0] == PERSIAN_PROBE.id and ordinary_text:
                runs = _script_runs(text)
                if any(kind == 'en' for kind, _ in runs):
                    english = self._key(US_ENGLISH.id, logical[1])
                    for kind, fragment in runs:
                        select(english if kind == 'en' else logical)
                        output.append(replace(part, text=fragment) if isinstance(part, EciText) else fragment)
                    continue
            if isinstance(part, EciCapitalWord) and logical[0] == PERSIAN_PROBE.id:
                runs = _script_runs(part.original)
                if runs and all(kind == 'en' for kind, _ in runs):
                    select(self._key(US_ENGLISH.id, logical[1]))
                    output.append(part)
                    continue
            if (isinstance(part, EciCapitalWord) and logical[0] == PINGLISH_PROBE.id and
                    english_exception(part.original)):
                select(self._key(US_ENGLISH.id, logical[1]))
                output.append(part)
                continue
            if text is not None or isinstance(part, EciCapitalWord):
                select(logical)
            output.append(part)
        prior_uppercase = None
        try:
            # Sentence splitting is needed only when this request contains a
            # Persian/English switch. Keep unrelated languages on their
            # original native utterance path.
            key, spoken = self._home, set()
            for item in output:
                if isinstance(item, EciLanguage):
                    key = self._key(item.language, item.voice)
                elif isinstance(item, (str, EciText, EciCapitalWord)) and not (
                        isinstance(item, EciText) and item.is_control):
                    value = item if isinstance(item, str) else item.text if isinstance(item, EciText) else item.original
                    if value.strip():
                        spoken.add(key[0])
            allowed = {US_ENGLISH.id, PERSIAN_PROBE.id, PINGLISH_PROBE.id}
            if spoken & {PERSIAN_PROBE.id, PINGLISH_PROBE.id}:
                output = _bracket_continuations(output)
            if self._home[0] in (PERSIAN_PROBE.id, PINGLISH_PROBE.id) or spoken & {PERSIAN_PROBE.id, PINGLISH_PROBE.id}:
                # The shared language engine also serves Chinese and other
                # voices. Preserve their cache while using a fresh, temporary
                # capitalization table for each Persian-containing request.
                prior_uppercase = self._uppercase_cache
                self._uppercase_cache = type(prior_uppercase)()
            mixed = (spoken <= allowed and US_ENGLISH.id in spoken and
                     bool(spoken & {PERSIAN_PROBE.id, PINGLISH_PROBE.id}))
            jobs = sentence_jobs(output, self._home) if mixed else ()
            if not jobs:
                jobs = (tuple(output),)
            split = tuple(piece for job in jobs for piece in bracket_jobs(
                job, self._home, fa_ids=(PERSIAN_PROBE.id, PINGLISH_PROBE.id),
                english_id=US_ENGLISH.id)) if mixed else tuple((job, False) for job in jobs)
            jobs = []
            for job, shortened_end in split:
                chunks = bounded_jobs(job, self._home) if spoken & {PERSIAN_PROBE.id, PINGLISH_PROBE.id} else (job,)
                jobs.extend((chunk, shortened_end and i == len(chunks) - 1)
                            for i, chunk in enumerate(chunks))
            self._cancelled.clear()
            for job, shortened_end in jobs:
                if self._cancelled.is_set():
                    return False
                raw, old_pause = None, None
                if shortened_end:
                    job = continuation_comma(job)
                    ending = self._home
                    for item in job:
                        if isinstance(item, EciLanguage):
                            ending = self._key(item.language, item.voice)
                    ending_engine = self._activate(ending)
                    raw = ending_engine._voice if isinstance(ending_engine, ScopedDirectPersianEngine) else ending_engine
                    dll, handle = getattr(raw, '_dll', None), getattr(raw, '_handle', None)
                    old_pause = dll.eciGetParam(handle, PAUSE_MODE) if dll and handle else None
                    if old_pause == 0:
                        # Suppress only the native end-of-utterance silence;
                        # the bracket's actual speech and melody are intact.
                        raw.set_parameter(PAUSE_MODE, 1)
                try:
                    result = try_continuous(self, job, on_pcm, on_index,
                                            fa_ids=(PERSIAN_PROBE.id, PINGLISH_PROBE.id),
                                            english_id=US_ENGLISH.id, german_id=GERMAN.id,
                                            reset_cancellation=False, end_pause_shortened=old_pause == 0)
                    if result is None and not self._cancelled.is_set():
                        result = try_persian_glides(self, job, on_pcm, on_index,
                                                    fa_ids=(PERSIAN_PROBE.id, PINGLISH_PROBE.id))
                    if result is None and not self._cancelled.is_set():
                        result = super().speak(job, on_pcm, on_index, reset_cancellation=False)
                finally:
                    if old_pause == 0:
                        raw.set_parameter(PAUSE_MODE, old_pause)
                if not result or self._cancelled.is_set():
                    return False
            return True
        finally:
            try:
                self._activate(self._home)
            finally:
                if prior_uppercase is not None:
                    self._uppercase_cache = prior_uppercase


def make_language_engine(library, native_library, worker, espeak_library, data_parent,
                         *, language=PERSIAN_PROBE.id, voice='1', languages=None):
    """Construct the regular LanguageEngine with a test-only Persian factory."""
    def factory(dll, *, dialect, encoding, native_voice):
        if dialect in (PERSIAN_PROBE.id, PINGLISH_PROBE.id):
            if encoding != 'utf-8' or not native_voice:
                raise ValueError('the Persian test requires a native UTF-8 voice')
            return OwnedPersianEngine(dll, native_library, worker, espeak_library, data_parent,
                                      pinglish=dialect == PINGLISH_PROBE.id)
        return DirectEciEngine(dll, dialect=dialect, encoding=encoding, native_voice=native_voice)
    return PersianProbeLanguageEngine(library, language=language, voice=voice,
                                      languages=PROFILES if languages is None else languages,
                                      engine_factory=factory)
