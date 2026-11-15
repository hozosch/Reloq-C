"""Project a covered Persian text job and its ECI marks in one utterance.

This is a focused experiment, not an NVDA language. The optional reference
fallback requires verified word positions and native-accepted phoneme tags.
It fails before speech for uncovered input or unsafe controls.
"""
from __future__ import annotations

import ctypes
from dataclasses import replace
from pathlib import Path
import re
import sys
import threading
from typing import Sequence

if __package__:
    from .._reloq_eci_direct import (
        DirectEciEngine, EciIndex, EciPitch, EciPitchHz, EciProsody, EciText, PROCESS_TAGS,
    )
    from .._reloq_persian_projection import Command, ProjectedJob, PersianProjectionError, project_persian
    from .hybrid_fa_frontend import DirectFirstPersian
    from .reference_alignment import unpositioned_reference_input
    from .text_frontend import UnsupportedPronunciation, native_sequence_accepted
else:
    ROOT = Path(__file__).resolve().parents[2]
    sys.path.insert(0, str(ROOT / 'nvda'))
    from _reloq_eci_direct import (
        DirectEciEngine, EciIndex, EciPitch, EciPitchHz, EciProsody, EciText, PROCESS_TAGS,
    )
    from _reloq_persian_projection import Command, ProjectedJob, PersianProjectionError, project_persian
    from hybrid_fa_frontend import DirectFirstPersian
    from reference_alignment import unpositioned_reference_input
    from text_frontend import UnsupportedPronunciation, native_sequence_accepted

def project(parts: Sequence[str | EciText | Command], frontend: DirectFirstPersian,
            *, allow_fallback: bool = False, pinglish: bool = False) -> ProjectedJob:
    """Use direct analysis and, optionally, verified reference word anchors."""
    parts = tuple(parts)
    DirectEciEngine._prepare(parts, 'utf-8')
    spelled = [part for part in parts if isinstance(part, EciText) and part.character_mode]
    runs = []
    if spelled:
        if any(isinstance(part, EciText) and part.speed_control for part in parts):
            raise UnsupportedPronunciation('inline controls need separate handling')
        for part in parts:
            if not isinstance(part, (str, EciText)):
                continue
            value = part if isinstance(part, str) else part.text
            mode = isinstance(part, EciText) and part.character_mode
            if runs and runs[-1][0] == mode:
                runs[-1] = (mode, runs[-1][1] + value)
            elif value:
                runs.append((mode, value))
        parts = tuple(replace(part, character_mode=False) if isinstance(part, EciText) and part.character_mode else part
                      for part in parts)
    total = sum(len(part if isinstance(part, str) else part.text)
                for part in parts if isinstance(part, (str, EciText)))
    offset = 0
    internal_marks = False
    for part in parts:
        if isinstance(part, (str, EciText)):
            offset += len(part if isinstance(part, str) else part.text)
        elif 0 < offset < total:
            internal_marks = True
    used_fallback = False
    def analyze_spelled(text):
        pieces, output_bytes = [], 0
        mapping = [0] * (len(text.encode('utf8')) + 1)
        source_bytes = 0
        for char in text:
            width = len(char.encode('utf8'))
            name = (frontend.analyze_pinglish_character(char) if pinglish
                    else frontend.analyze_character(char))
            if name is None or len(name.parts) != 1:
                return None
            if pieces:
                pieces.append(' ')
                output_bytes += 1
            value = name.parts[0].text
            for index in range(source_bytes, source_bytes + width):
                mapping[index] = output_bytes
            pieces.append(value)
            output_bytes += len(value)
            source_bytes += width
            mapping[source_bytes] = output_bytes
        return ''.join(pieces), tuple(mapping)

    def analyze_regular(text):
        nonlocal used_fallback
        if pinglish:
            analysis = frontend.analyze_pinglish(text, positioned=internal_marks,
                                                allow_fallback=allow_fallback)
        else:
            analysis = frontend.analyze_direct(text)
        if analysis is None and allow_fallback and not pinglish:
            if internal_marks:
                analysis = frontend.analyze_positioned(text)
            elif unpositioned_reference_input(text):
                # With marks only at the beginning/end, no word alignment is
                # required. Avoid synthesizing discarded eSpeak audio.
                analysis = frontend.analyze(text)
                if len(analysis.parts) != 1 or analysis.parts[0].language != 'fa':
                    return None
                tagged = analysis.parts[0].text
                used_fallback = used_fallback or analysis.used_fallback
                return tagged, (0,) * len(text.encode('utf8')) + (len(tagged),)
        if analysis is None:
            return None
        if pinglish and analysis.source_map is None and not internal_marks:
            if len(analysis.parts) != 1 or analysis.parts[0].language != 'fa':
                return None
            tagged = analysis.parts[0].text
            used_fallback = used_fallback or analysis.used_fallback
            return tagged, (0,) * len(text.encode('utf8')) + (len(tagged),)
        if analysis.source_map is None or len(analysis.parts) > 1:
            raise PersianProjectionError('direct Persian source map is unavailable')
        used_fallback = used_fallback or analysis.used_fallback
        return analysis.parts[0].text if analysis.parts else '', analysis.source_map

    def analyze(text):
        if not spelled:
            return analyze_regular(text)
        if ''.join(value for _, value in runs) != text:
            raise PersianProjectionError('spelling runs do not match the source')
        pieces, mapping, position = [], [0], 0
        for mode, value in runs:
            result = analyze_spelled(value) if mode else analyze_regular(value)
            if result is None:
                return None
            tagged, local = result
            if pieces and tagged and pieces[-1] and not pieces[-1][-1].isspace() and not tagged[0].isspace():
                pieces.append(' ')
                position += 1
                mapping[-1] = position
            pieces.append(tagged)
            mapping.extend(position + item for item in local[1:])
            position += len(tagged)
        return ''.join(pieces), tuple(mapping)
    try:
        result = project_persian(parts, analyze, allow_symbols=bool(spelled))
        return replace(result, needs_native_validation=used_fallback)
    except PersianProjectionError as error:
        raise UnsupportedPronunciation(str(error)) from error


_TAG = re.compile(r'`\[[^]]+\]')
_DOUBLE_J = re.compile(r'J(\.[012])?J')


def _mark_geminated_affricates(parts):
    """Keep one J per geminate and identify its native word/phone ordinal.

    Projected events can occur between tags but never inside a tag. A literal
    D followed by ZH can resemble native J after phoneme expansion, so leave
    such ambiguous words on the old route instead of changing a single J.
    """
    selected, word, output = [], 0, []

    def change(match):
        nonlocal word
        word += 1
        tag = match.group()
        if word > 32767 or re.search(r'd(?:\.[012])?Z', tag):
            return tag
        pos, ordinal, pieces = 0, 0, []
        for doubled in _DOUBLE_J.finditer(tag):
            prefix = tag[pos:doubled.start()]
            ordinal += prefix.count('J') + 1
            if ordinal > 65535 or len(selected) >= 32767:
                # Reject rather than silently selecting a different phone.
                raise UnsupportedPronunciation('too many marked affricates')
            pieces.extend((prefix, doubled.group(1) or '', 'J'))
            selected.append((word << 16) | ordinal)
            pos = doubled.end()
        return ''.join(pieces) + tag[pos:] if pieces else tag

    for part in parts:
        output.append(replace(part, text=_TAG.sub(change, part.text))
                      if isinstance(part, EciText) else part)
    return tuple(output), tuple(selected)


class ScopedDirectPersianEngine:
    """Private synchronous ECI experiment with request-scoped Persian sound."""

    def __init__(self, library: Path, frontend: DirectFirstPersian, *, allow_fallback: bool = False,
                 pinglish: bool = False):
        self._owner = threading.get_ident()
        self._frontend = frontend
        self._allow_fallback = allow_fallback
        self._pinglish = pinglish
        self._cancelled = threading.Event()
        self._voice = DirectEciEngine(library.resolve(), dialect=0x90000,
                                      encoding='utf8', native_voice=True)
        try:
            self._voice.set_parameter(PROCESS_TAGS, 1)
            self._scope = self._voice._dll.reloq_fa_acoustic_scope
            self._scope.argtypes = (ctypes.c_int,)
            self._scope.restype = ctypes.c_int
            self._onsets = self._voice._dll.reloq_fa_native_onsets
            self._onsets.argtypes = (ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t)
            self._onsets.restype = ctypes.c_int
            self._geminates = self._voice._dll.reloq_fa_native_geminates
            self._geminates.argtypes = (ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t)
            self._geminates.restype = ctypes.c_int
        except BaseException:
            self._voice.close()
            raise

    def set_parameter(self, parameter: int, value: int) -> None:
        if parameter == PROCESS_TAGS:
            if type(value) is not int or value not in (0, 1):
                raise ValueError('tag setting must be 0 or 1')
            # Internal tags are required; source-side user tags still fail
            # direct preflight even when native tag parsing remains enabled.
            self._voice.set_parameter(PROCESS_TAGS, 1)
            return
        self._voice.set_parameter(parameter, value)

    def speak(self, parts: Sequence[str | EciText | Command], on_pcm, on_index) -> bool:
        if threading.get_ident() != self._owner:
            raise RuntimeError('Persian engine belongs to its creating worker')
        self._cancelled.clear()
        try:
            prepared = project(parts, self._frontend, allow_fallback=self._allow_fallback,
                               pinglish=self._pinglish)
        except (RuntimeError, OSError):
            if self._cancelled.is_set():
                return False
            raise
        if self._cancelled.is_set():
            return False
        tags = tuple(re.finditer(r'`\[[^]]+\]', prepared.tagged))
        marked, checked = [], 0
        for index, (left, right) in enumerate(zip(tags, tags[1:]), start=2):
            between = prepared.tagged[left.end():right.start()]
            if '~' in between:
                if between != '~':
                    raise UnsupportedPronunciation('ambiguous native onset boundary')
                marked.append(index)
                checked += 1
        if checked != prepared.tagged.count('~') or any(word > 32767 for word in marked):
            raise UnsupportedPronunciation('native onset positions are unavailable')
        native_parts = tuple(replace(part, text=part.text.replace('~', ' '))
                             if isinstance(part, EciText) else part for part in prepared.parts)
        native_parts, geminates = _mark_geminated_affricates(native_parts)
        native_tagged = ''.join(part.text for part in native_parts if isinstance(part, EciText))
        previous = self._scope(1)
        if previous < 0:
            raise RuntimeError('cannot enable Persian acoustic scope')
        def pcm(data):
            if self._cancelled.is_set():
                self._voice.cancel()
            else:
                on_pcm(data)
        def index(value):
            if self._cancelled.is_set():
                self._voice.cancel()
            else:
                on_index(value)
        try:
            selected = (ctypes.c_uint32 * len(marked))(*marked) if marked else None
            if self._onsets(selected, len(marked)) < 0:
                raise RuntimeError('cannot select Finnish vowel onsets')
            selected_geminates = (ctypes.c_uint32 * len(geminates))(*geminates) if geminates else None
            if self._geminates(selected_geminates, len(geminates)) < 0:
                raise RuntimeError('cannot select Persian geminated affricates')
            if prepared.needs_native_validation:
                # Unknown reference pronunciations must also be accepted by
                # the actual Finnish-based phoneme parser before any audio.
                self._voice._cancelled.clear()
                if self._cancelled.is_set():
                    return False
                try:
                    shown = self._voice._phonemes_for_probe(native_tagged)
                except RuntimeError:
                    if self._cancelled.is_set():
                        return False
                    raise
                if not native_sequence_accepted(native_tagged, shown):
                    raise UnsupportedPronunciation('reference pronunciation is not accepted by the native parser')
                if self._cancelled.is_set():
                    return False
            return self._voice.speak(native_parts, pcm, index) and not self._cancelled.is_set()
        finally:
            reset = self._onsets(None, 0)
            reset_geminates = self._geminates(None, 0)
            if self._scope(previous) < 0 or reset < 0 or reset_geminates < 0:
                raise RuntimeError('cannot restore Persian acoustic scope')

    def cancel(self) -> None:
        self._cancelled.set()
        cancel_frontend = getattr(self._frontend, 'cancel', None)
        if cancel_frontend is not None:
            cancel_frontend()
        self._voice.cancel()

    def close(self) -> None:
        self._voice.close()
