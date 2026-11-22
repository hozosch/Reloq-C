"""Exercise the real NVDA language/queue contract with a private FA voice.

This factory is only for tests. Its Persian IDs are not public ECI dialects.
Explicit and automatic English switches can use the selected continuous
sentence plan when the private host exposes frame capture and replay.
"""
from __future__ import annotations

from dataclasses import replace
from pathlib import Path
from bisect import bisect_right
import ctypes
import os
import re
import unicodedata

if __package__:
    from .direct_fa_sequence import ScopedDirectPersianEngine
    from .hybrid_fa_frontend import DirectFirstPersian
    from .pinglish_frontend import english_exception, latin_fold, latin_spans
    from .continuous_driver import bounded_jobs, bracket_jobs, continuation_comma, sentence_jobs, try_continuous, try_persian_glides
    from .._reloq_eci_direct import (DirectEciEngine, EciCapitalWord, EciIndex, EciLanguage,
                                     EciPitch, EciPitchHz, EciProsody, EciText, PAUSE_MODE)
    from .._reloq_languages import Language, LanguageEngine, US_ENGLISH, GERMAN, FINNISH, carry_speech_context
else:
    from direct_fa_sequence import ScopedDirectPersianEngine
    from hybrid_fa_frontend import DirectFirstPersian
    from pinglish_frontend import english_exception, latin_fold, latin_spans
    from continuous_driver import bounded_jobs, bracket_jobs, continuation_comma, sentence_jobs, try_continuous, try_persian_glides
    from _reloq_eci_direct import (DirectEciEngine, EciCapitalWord, EciIndex, EciLanguage,
                                  EciPitch, EciPitchHz, EciProsody, EciText, PAUSE_MODE)
    from _reloq_languages import Language, LanguageEngine, US_ENGLISH, GERMAN, FINNISH, carry_speech_context

PERSIAN_PROBE = Language(0x140000, 'Persian test', 'fa_IR', 'utf-8', US_ENGLISH.voices)
PINGLISH_PROBE = Language(0x140001, 'Persian (Pinglish) test', 'fa_Latn', 'utf-8', US_ENGLISH.voices)
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
        self._emoji_character = getattr(self._voice._dll, 'eciUnicodeIsEmoji', None)
        if self._emoji_character is not None:
            self._emoji_character.argtypes = (ctypes.c_uint32,)
            self._emoji_character.restype = ctypes.c_int

    def is_emoji_character(self, char):
        if self._emoji_character is not None:
            return bool(self._emoji_character(ord(char)))
        # Compatibility with older test libraries: keep possible emoji rather
        # than suppressing them when the shared Unicode index is unavailable.
        cp = ord(char)
        return unicodedata.category(char) in ('So', 'Sk') and (
            0x2600 <= cp <= 0x27bf or 0x1f000 <= cp <= 0x1faff)

    def set_dictionary_path(self, path):
        self._voice.set_dictionary_path(path)

    def speak(self, parts, on_pcm, on_index):
        # LanguageEngine's separated fallback flushes each language through
        # its own engine. It must not bypass Persian contour planning after
        # punctuation, inline controls, or an unrelated language switch.
        planner = getattr(self, '_contour_fallback', None)
        if planner is not None:
            result = planner(parts, on_pcm, on_index)
            if result is not None:
                return result
        return super().speak(parts, on_pcm, on_index)

    def close(self):
        try:
            super().close()
        finally:
            self._frontend.close()


def _script_runs(text):
    """Preserve every character while separating Latin from other letters."""
    # An English "4 of 14" is one phrase, even if the preceding Persian
    # text or an NVDA index ends immediately before its first digit.
    english_numbers = [(match.start(), match.end()) for match in
                       re.finditer(r'(?<![\w.])[0-9]+[ \t]+of[ \t]+[0-9]+(?![\w.])',
                                   text, re.IGNORECASE)]
    runs, prefix, current, kind = [], '', '', None
    for index, char in enumerate(text):
        in_number = any(start <= index < end for start, end in english_numbers)
        ipa_stress = (char in ('\u02c8', '\u02cc') and index + 1 < len(text) and
                      unicodedata.name(text[index + 1], '').startswith('LATIN '))
        strong = ('en' if in_number or ipa_stress or unicodedata.name(char, '').startswith('LATIN ') else
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


def try_native_persian_sentence(owner, parts, on_pcm, on_index, *, fa_ids):
    """Use one native Persian/US analysis for short ordinary sentences.

    The native backend has not yet acquired the established mixed-script,
    internal-prosody or incremental long-text paths. Those requests retain
    the current planner until their native equivalents pass the same tests.
    """
    key, text, offsets, values = owner._home, bytearray(), [], []
    saw_text = False
    for part in parts:
        if isinstance(part, EciLanguage):
            candidate = owner._key(part.language, part.voice)
            if saw_text and candidate != key:
                return None
            key = candidate
        elif isinstance(part, EciIndex):
            offsets.append(len(text)); values.append(part.value)
        elif isinstance(part, (str, EciText)):
            if isinstance(part, EciText) and (part.character_mode or part.is_control or
                    part.segmentation_guarded or part.internal_punctuation):
                return None
            value = part if isinstance(part, str) else part.text
            if '`' in value:
                return None
            text.extend(value.encode('utf-8'))
            saw_text = saw_text or bool(value)
        else:
            return None
    if key[0] not in fa_ids or not text or len(text) > 256 or len(offsets) > 4096:
        return None
    source = bytes(text).decode('utf-8')
    if len(source) <= 1 or len(source) > 64 or len(source.split()) > 3:
        return None
    engine = owner._activate(key)
    if not isinstance(engine, ScopedDirectPersianEngine) or getattr(engine._frontend, '_reference_available', False):
        return None
    target = engine._voice
    speak = getattr(target._dll, 'eciPersianSpeakUsNativeUtf8', None)
    if speak is None:
        return None
    pilot_key = owner._key(US_ENGLISH.id, key[1])
    pilot = owner._activate(pilot_key)
    owner._activate(key)
    with owner._guard:
        if owner._cancelled.is_set():
            return False
        engine._cancelled.clear()
        target._cancelled.clear()
        pilot._cancelled.clear()
        owner._native_persian_pilot = pilot
    size, integer = ctypes.c_size_t, ctypes.c_int
    speak.argtypes = (ctypes.c_void_p, ctypes.c_void_p, ctypes.c_char_p, size, integer,
                      ctypes.POINTER(size), ctypes.POINTER(integer), size, size, ctypes.POINTER(size))
    speak.restype = integer
    positions = (size * len(offsets))(*offsets) if offsets else None
    indexes = (integer * len(values))(*values) if values else None
    error = size()
    emitted = False
    def pcm(value):
        nonlocal emitted
        emitted = True
        on_pcm(value)
    def index(value):
        nonlocal emitted
        emitted = True
        on_index(value)
    target._callback_error = None
    target._on_pcm, target._on_index = pcm, index
    try:
        status = speak(target._handle, pilot._handle, bytes(text), len(text),
                       int(key[0] == PINGLISH_PROBE.id), positions, indexes,
                       len(offsets), 30000, ctypes.byref(error))
        if target._callback_error is not None:
            raise target._callback_error
        if owner._cancelled.is_set():
            return False
        if status == 1 and not emitted:
            return None
        if status != 0:
            raise RuntimeError(f'Native Persian sentence failed ({status}, byte {error.value})')
        return True
    finally:
        target._on_pcm = target._on_index = None
        with owner._guard:
            owner._native_persian_pilot = None


class PersianProbeLanguageEngine(LanguageEngine):
    """Use English for explicit and clear Latin-script boundaries."""

    def _activate(self, key):
        engine = super()._activate(key)
        if isinstance(engine, OwnedPersianEngine):
            def plan_segment(parts, on_pcm, on_index):
                if not getattr(self, '_fallback_contour_enabled', False):
                    return None
                if self._cancelled.is_set():
                    return False
                return try_persian_glides(self, (EciLanguage(*key), *parts), on_pcm, on_index,
                                          fa_ids=(PERSIAN_PROBE.id, PINGLISH_PROBE.id))
            engine._contour_fallback = plan_segment
        return engine

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
                if os.environ.get('RELOQ_FA_STREAM_PREFIX', '1') == '1':
                    self._persian_capture_worker = ParallelPilot(self._dll, persian=True)
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
        with self._guard:
            pilot = getattr(self, '_native_persian_pilot', None)
            if pilot is not None:
                pilot.cancel()
        korean = getattr(self, '_persian_korean_contour', None)
        if korean is not None:
            korean.cancel()
        worker = getattr(self, '_persian_contour_worker', None)
        if worker is not None:
            worker.cancel()
        worker = getattr(self, '_persian_capture_worker', None)
        if worker is not None:
            worker.cancel()

    def close(self):
        korean = getattr(self, '_persian_korean_contour', None)
        if korean is not None:
            self._persian_korean_contour = None
            korean.close()
        worker = getattr(self, '_persian_contour_worker', None)
        if worker is not None:
            self._persian_contour_worker = None
            worker.close()
        worker = getattr(self, '_persian_capture_worker', None)
        if worker is not None:
            self._persian_capture_worker = None
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
        parts = carry_speech_context(parts)
        # Consult the native text policy before Unicode-name and script
        # routing. An isolated character or explicit spelling must still be
        # named; indexes between ordinary fragments remain in their places.
        ordinary = tuple(part if isinstance(part, str) else part.text
                         for part in parts if isinstance(part, str) or
                         isinstance(part, EciText) and not part.character_mode and not part.is_control)
        if any(unicodedata.category(char) == 'Cf' for text in ordinary for char in text):
            engine = self._activate(self._home)
            is_ignorable = (getattr(engine._voice._dll, 'eciPersianIgnorableTextCharacter', None)
                            if isinstance(engine, OwnedPersianEngine) else None)
            if (is_ignorable is not None and
                    any(not is_ignorable(ord(char)) for text in ordinary if '`' not in text for char in text)):
                filtered, logical = [], self._home
                for part in parts:
                    if isinstance(part, EciLanguage):
                        logical = self._key(part.language, part.voice)
                    text = part if isinstance(part, str) else part.text if isinstance(part, EciText) else None
                    if (logical[0] in (PERSIAN_PROBE.id, PINGLISH_PROBE.id) and text and '`' not in text and
                            (not isinstance(part, EciText) or not part.character_mode and not part.is_control)):
                        text = ''.join(char for char in text if not is_ignorable(ord(char)))
                        part = replace(part, text=text) if isinstance(part, EciText) else text
                    filtered.append(part)
                parts = tuple(filtered)
        # Use character names only for an isolated character; emoji remain
        # available in continuous speech. NVDA's configured substitutions
        # have already been applied before this point.
        source_text = ''.join(part if isinstance(part, str) else part.text
                              for part in parts if isinstance(part, str) or
                              isinstance(part, EciText) and not part.is_control)
        single_scalar = len(source_text) == 1
        isolated = (len(source_text.strip()) == 1 or
                    single_scalar and ord(source_text) < 32 and source_text not in '\t\r\n')
        def adjoining(index, direction):
            # Source indexes and voice controls do not divide a word.
            index += direction
            while 0 <= index < len(parts):
                candidate = parts[index]
                if isinstance(candidate, (EciIndex, EciPitch, EciPitchHz, EciProsody)):
                    index += direction
                    continue
                if isinstance(candidate, str):
                    value = candidate
                elif isinstance(candidate, EciText) and not (candidate.character_mode or
                        candidate.is_control or candidate.segmentation_guarded):
                    value = candidate.text
                else:
                    break
                if '`' in value:
                    break
                if value:
                    return value[0] if direction > 0 else value[-1]
                index += direction
            return ''
        expanded, logical, names, previous_char = [], self._home, {}, None
        for part_index, part in enumerate(parts):
            if isinstance(part, EciLanguage):
                logical = self._key(part.language, part.voice)
                previous_char = None
                expanded.append(part)
                continue
            text = part if isinstance(part, str) else part.text if isinstance(part, EciText) else None
            if (logical[0] not in (PERSIAN_PROBE.id, PINGLISH_PROBE.id) or not text or '`' in text or
                    isinstance(part, EciText) and (part.character_mode or part.is_control or
                                                   part.segmentation_guarded)):
                if text is not None:
                    previous_char = None
                expanded.append(part)
                continue
            normalized_chars = []
            contextual = (not (isolated or single_scalar) or
                          isinstance(part, EciText) and part.source_in_context)
            for char in text:
                if (char == '\ufe0f' and previous_char is not None and
                        not (isinstance(part, EciText) and part.segmentation_guarded) and
                        (0x2600 <= ord(previous_char) <= 0x27bf or 0x1f000 <= ord(previous_char) <= 0x1faff)):
                    continue
                silent_control = ((ord(char) < 32 and char not in '\t\r\n') or ord(char) == 0x7f)
                normalized_chars.append(' ' if (contextual and silent_control) or
                                        (ord(char) > 127 and char.isspace()) else char)
                previous_char = char
            normalized = ''.join(normalized_chars)
            if normalized != text:
                text = normalized
                part = replace(part, text=text) if isinstance(part, EciText) else text
            positions = []
            for at, char in enumerate(text):
                cp = ord(char)
                category = unicodedata.category(char)
                latin = unicodedata.name(char, '').startswith('LATIN ')
                next_char = (text[at + 1] if at + 1 < len(text) else adjoining(part_index, 1)) if char in ('\u02c8', '\u02cc') else ''
                ipa_stress = bool(next_char and unicodedata.name(next_char, '').startswith('LATIN '))
                prev_char = (text[at - 1] if at else adjoining(part_index, -1)) if category.startswith('M') else ''
                combining_text = bool(prev_char and (prev_char.isalpha() or
                                      unicodedata.category(prev_char).startswith('M')))
                # The Persian frontend already treats silent angle quotes as
                # brackets; retain that contour in multi-character speech.
                if (cp < 128 or 0x600 <= cp <= 0x8ff or 0xfb50 <= cp <= 0xfeff or
                        char in '\u200c\u200d\ufe0e\ufe0f' or
                        not isolated and char in '«»' or
                        char.isspace() or char.isdecimal() or latin or ipa_stress or combining_text or
                        logical[0] == PINGLISH_PROBE.id and category.startswith('M')):
                    continue
                engine = self._activate(logical)
                emoji = isinstance(engine, OwnedPersianEngine) and engine.is_emoji_character(char)
                if not isolated and not emoji:
                    # Keep dash punctuation as a dash: it separates language
                    # runs and affects sentence timing even when its Unicode
                    # name must not be spoken in running text.
                    replacement = '-' if category == 'Pd' else ' '
                    positions.append((at, replacement, False))
                    continue
                if char not in names:
                    if not isinstance(engine, OwnedPersianEngine):
                        names[char] = None
                    else:
                        named = engine._frontend.analyze_character(char).parts[0]
                        names[char] = named.text if named.language == 'en-us' else None
                if names[char] is not None:
                    positions.append((at, names[char], True))
            if not positions:
                expanded.append(part)
                continue
            start = 0
            for at, name, announce in positions:
                if at > start:
                    expanded.append(replace(part, text=text[start:at]) if isinstance(part, EciText) else text[start:at])
                if announce:
                    expanded.extend((EciLanguage(US_ENGLISH.id, logical[1]), name, EciLanguage(*logical)))
                else:
                    expanded.append(replace(part, text=name) if isinstance(part, EciText) else name)
                start = at + 1
            if start < len(text):
                expanded.append(replace(part, text=text[start:]) if isinstance(part, EciText) else text[start:])
        parts = tuple(expanded)
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
            # Explicitly spelled Latin letters in the Persian-script voice
            # use the real US-English letter names and acoustics. Keep each
            # surrounding Persian fragment, character mode and indexes in
            # place. Pinglish deliberately retains its Persian letter names.
            if (logical[0] == PERSIAN_PROBE.id and isinstance(part, EciText) and
                    part.character_mode and not part.is_control and text and
                    any('A' <= ch <= 'Z' or 'a' <= ch <= 'z' for ch in text)):
                english = self._key(US_ENGLISH.id, logical[1])
                engine = self._activate(logical)
                segments = []
                for char in text:
                    key, fragment, mode = logical, char, True
                    if 'A' <= char <= 'Z' or 'a' <= char <= 'z':
                        key = english
                    elif isinstance(engine, OwnedPersianEngine):
                        named = engine._frontend.analyze_character(char).parts[0]
                        if named.language == 'en-us':
                            if latin_fold(char) is not None:
                                key = english
                            else:
                                key, fragment, mode = english, named.text, False
                    # Keep English Unicode names as separate words, while
                    # retaining native spelling of adjacent plain letters.
                    if mode and segments and segments[-1][0] == key and segments[-1][2]:
                        segments[-1] = (key, segments[-1][1] + fragment, mode)
                    else:
                        segments.append((key, fragment, mode))
                for key, fragment, mode in segments:
                    select(key)
                    output.append(replace(part, text=fragment, character_mode=mode))
                continue
            # Name an individually navigated character with the real English
            # engine when no Persian label exists. Routing it as a language
            # run also keeps Korean pitch planning away from the English name
            # and preserves indexes around the switch.
            if (logical[0] in (PERSIAN_PROBE.id, PINGLISH_PROBE.id) and text and
                    (len(text) == 1 or isinstance(part, EciText) and part.character_mode) and
                    (any(ord(char) > 127 for char in text) or isolated or
                     isinstance(part, EciText) and part.character_mode) and
                    not (text in ('\ufe0e', '\ufe0f') and
                         not (isinstance(part, EciText) and part.character_mode)) and
                    not (isinstance(part, EciText) and part.is_control)):
                engine = self._activate(logical)
                if isinstance(engine, OwnedPersianEngine):
                    named = []
                    for char in text:
                        analyze = (engine._frontend.analyze_pinglish_character
                                   if logical[0] == PINGLISH_PROBE.id
                                   else engine._frontend.analyze_character)
                        named_part = analyze(char).parts[0]
                        if named_part and named_part.language == 'en-us' and latin_fold(char) is not None:
                            name = char if logical[0] == PERSIAN_PROBE.id else None
                        else:
                            name = (named_part.text if named_part and named_part.language == 'en-us' else None)
                        named.append((char, name))
                    if any(name is not None for _, name in named):
                        for char, name in named:
                            select(self._key(US_ENGLISH.id, logical[1]) if name is not None else logical)
                            fragment = char if name is None else name
                            output.append(replace(part, text=fragment,
                                                  character_mode=name is None or fragment == char)
                                          if isinstance(part, EciText) else fragment)
                        continue
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
            if mixed:
                jobs = sentence_jobs(output, self._home)
            elif len(spoken) == 1 and spoken <= {PERSIAN_PROBE.id, PINGLISH_PROBE.id}:
                # A full stop, question or exclamation ends a Persian
                # sentence too. Plan later sentences after the first has
                # started, without cutting the intervening clause contours.
                jobs = sentence_jobs(output, self._home, hard_only=True)
            else:
                jobs = ()
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
                    if result is None and getattr(self, '_native_sentences', False) and not self._cancelled.is_set():
                        result = try_native_persian_sentence(self, job, on_pcm, on_index,
                            fa_ids=(PERSIAN_PROBE.id, PINGLISH_PROBE.id))
                    if result is None and not self._cancelled.is_set():
                        result = try_persian_glides(self, job, on_pcm, on_index,
                                                    fa_ids=(PERSIAN_PROBE.id, PINGLISH_PROBE.id))
                    if result is None and not self._cancelled.is_set():
                        previous = getattr(self, '_fallback_contour_enabled', False)
                        self._fallback_contour_enabled = True
                        try:
                            result = super().speak(job, on_pcm, on_index, reset_cancellation=False)
                        finally:
                            self._fallback_contour_enabled = previous
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
                         *, language=PERSIAN_PROBE.id, voice='1', languages=None, native_sentences=False):
    """Construct the regular LanguageEngine with a test-only Persian factory."""
    def factory(dll, *, dialect, encoding, native_voice):
        if dialect in (PERSIAN_PROBE.id, PINGLISH_PROBE.id):
            if encoding != 'utf-8' or not native_voice:
                raise ValueError('the Persian test requires a native UTF-8 voice')
            return OwnedPersianEngine(dll, native_library, worker, espeak_library, data_parent,
                                      pinglish=dialect == PINGLISH_PROBE.id)
        return DirectEciEngine(dll, dialect=dialect, encoding=encoding, native_voice=native_voice)
    engine = PersianProbeLanguageEngine(library, language=language, voice=voice,
                                        languages=PROFILES if languages is None else languages,
                                        engine_factory=factory)
    engine._native_sentences = bool(native_sentences)
    return engine
