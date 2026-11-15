"""Project a covered Persian text job and its ECI marks in one utterance.

This is a focused experiment, not an NVDA language. The optional reference
fallback requires verified word positions and native-accepted phoneme tags.
It fails before speech for uncovered input or unsafe controls.
"""
from __future__ import annotations

import ctypes
from dataclasses import replace
from pathlib import Path
import sys
import threading
from typing import Sequence

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (
    DirectEciEngine, EciIndex, EciPitch, EciPitchHz, EciProsody, EciText,
    PROCESS_TAGS,
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
    if spelled:
        texts = [part if isinstance(part, str) else part.text
                 for part in parts if isinstance(part, (str, EciText))]
        if len(spelled) != 1 or len(spelled[0].text) != 1 or ''.join(texts) != spelled[0].text:
            raise UnsupportedPronunciation('only one Persian letter can be named at a time')
        if spelled[0].speed_control:
            raise UnsupportedPronunciation('inline controls need separate handling')
        parts = tuple(replace(part, character_mode=False) if part is spelled[0] else part
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
    def analyze(text):
        nonlocal used_fallback
        if spelled:
            analysis = (frontend.analyze_pinglish_character(text) if pinglish
                        else frontend.analyze_character(text))
        elif pinglish:
            analysis = frontend.analyze_pinglish(text, positioned=internal_marks,
                                                allow_fallback=allow_fallback)
        else:
            analysis = frontend.analyze_direct(text)
        if spelled and analysis is None:
            return None
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
                used_fallback = analysis.used_fallback
                return tagged, (0,) * len(text.encode('utf8')) + (len(tagged),)
        if analysis is None:
            return None
        if pinglish and analysis.source_map is None and not internal_marks:
            if len(analysis.parts) != 1 or analysis.parts[0].language != 'fa':
                return None
            tagged = analysis.parts[0].text
            used_fallback = analysis.used_fallback
            return tagged, (0,) * len(text.encode('utf8')) + (len(tagged),)
        if analysis.source_map is None or len(analysis.parts) > 1:
            raise PersianProjectionError('direct Persian source map is unavailable')
        used_fallback = analysis.used_fallback
        return analysis.parts[0].text if analysis.parts else '', analysis.source_map
    try:
        result = project_persian(parts, analyze)
        return replace(result, needs_native_validation=used_fallback)
    except PersianProjectionError as error:
        raise UnsupportedPronunciation(str(error)) from error


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
            if prepared.needs_native_validation:
                # Unknown reference pronunciations must also be accepted by
                # the actual Finnish-based phoneme parser before any audio.
                self._voice._cancelled.clear()
                if self._cancelled.is_set():
                    return False
                try:
                    shown = self._voice._phonemes_for_probe(prepared.tagged)
                except RuntimeError:
                    if self._cancelled.is_set():
                        return False
                    raise
                if not native_sequence_accepted(prepared.tagged, shown):
                    raise UnsupportedPronunciation('reference pronunciation is not accepted by the native parser')
                if self._cancelled.is_set():
                    return False
            return self._voice.speak(prepared.parts, pcm, index) and not self._cancelled.is_set()
        finally:
            if self._scope(previous) < 0:
                raise RuntimeError('cannot restore Persian acoustic scope')

    def cancel(self) -> None:
        self._cancelled.set()
        cancel_frontend = getattr(self._frontend, 'cancel', None)
        if cancel_frontend is not None:
            cancel_frontend()
        self._voice.cancel()

    def close(self) -> None:
        self._voice.close()
