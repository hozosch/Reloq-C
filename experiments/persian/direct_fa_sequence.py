"""Project a covered Persian text job and its ECI marks in one utterance.

This is a focused experiment, not an NVDA language or a general fallback.
It keeps the native frontend's word/number-level position accuracy and fails
before synthesis for uncovered words, unsupported symbols or unsafe controls.
"""
from __future__ import annotations

import ctypes
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
from text_frontend import UnsupportedPronunciation

def project(parts: Sequence[str | EciText | Command], frontend: DirectFirstPersian) -> ProjectedJob:
    """Use the native-only frontend with the reusable NVDA projection code."""
    def analyze(text):
        analysis = frontend.analyze_direct(text)
        if analysis is None:
            return None
        if analysis.source_map is None or len(analysis.parts) > 1:
            raise PersianProjectionError('direct Persian source map is unavailable')
        return analysis.parts[0].text if analysis.parts else '', analysis.source_map
    try:
        return project_persian(parts, analyze)
    except PersianProjectionError as error:
        raise UnsupportedPronunciation(str(error)) from error


class ScopedDirectPersianEngine:
    """Private synchronous ECI experiment with request-scoped Persian sound."""

    def __init__(self, library: Path, frontend: DirectFirstPersian):
        self._owner = threading.get_ident()
        self._frontend = frontend
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
        prepared = project(parts, self._frontend)
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
            return self._voice.speak(prepared.parts, pcm, index) and not self._cancelled.is_set()
        finally:
            if self._scope(previous) < 0:
                raise RuntimeError('cannot restore Persian acoustic scope')

    def cancel(self) -> None:
        self._cancelled.set()
        self._voice.cancel()

    def close(self) -> None:
        self._voice.close()
