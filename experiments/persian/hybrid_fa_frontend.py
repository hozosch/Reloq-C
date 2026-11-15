"""Private direct-first Persian analysis with whole-clause eSpeak fallback.

This experiment has no NVDA integration. Native complete-word rules are used
when a whole input is covered. When they cannot cover it, translate the entire
input with the isolated reference worker so word context and English routing
are not broken into independently analysed fragments.
"""
from __future__ import annotations

import ctypes
from dataclasses import dataclass
from pathlib import Path
import threading

from espeak_worker import EspeakWorker
from text_frontend import SpeechPart, translate_text


@dataclass(frozen=True)
class PersianAnalysis:
    parts: tuple[SpeechPart, ...]
    # UTF-8 source byte -> tag byte, only at word granularity. Fallback does
    # not currently provide a reliable source map.
    source_map: tuple[int, ...] | None
    used_fallback: bool


class DirectFirstPersian:
    def __init__(self, native_library: Path, worker: Path, espeak_library: Path, data_parent: Path):
        self._dll = ctypes.CDLL(str(native_library.resolve()))
        self._dll.reloq_fa_direct_text.argtypes = (
            ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
            ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)), ctypes.POINTER(ctypes.c_size_t),
        )
        self._dll.reloq_fa_direct_text.restype = ctypes.c_int
        self._dll.reloq_fa_direct_free.argtypes = (ctypes.c_void_p,)
        self._worker_paths = (worker, espeak_library, data_parent)
        self._worker = None
        self._guard = threading.Lock()
        self._closed = False

    def _fallback(self) -> EspeakWorker:
        with self._guard:
            if self._closed:
                raise RuntimeError('Persian text frontend is closed')
            if self._worker is None:
                self._worker = EspeakWorker(*self._worker_paths)
            return self._worker

    def analyze(self, text: str, *, pinglish: bool = False) -> PersianAnalysis:
        if not isinstance(text, str) or '\0' in text:
            raise ValueError('text must be a string without NUL')
        with self._guard:
            if self._closed:
                raise RuntimeError('Persian text frontend is closed')
        if pinglish:
            return PersianAnalysis(translate_text(self._fallback(), text, pinglish=True), None, True)
        encoded = text.encode('utf8')
        output = ctypes.c_void_p()
        mapping = ctypes.POINTER(ctypes.c_size_t)()
        offset = ctypes.c_size_t()
        status = self._dll.reloq_fa_direct_text(encoded, len(encoded), ctypes.byref(output),
                                                 ctypes.byref(mapping), ctypes.byref(offset))
        if status == 0:
            try:
                tagged = ctypes.string_at(output).decode('ascii')
                source_map = tuple(mapping[i] for i in range(len(encoded) + 1))
            finally:
                self._dll.reloq_fa_direct_free(output)
                self._dll.reloq_fa_direct_free(mapping)
            parts = (SpeechPart('fa', tagged),) if tagged else ()
            return PersianAnalysis(parts, source_map, False)
        if status == 1:
            return PersianAnalysis(translate_text(self._fallback(), text), None, True)
        if status == -2:
            raise MemoryError('native Persian text allocation failed')
        raise ValueError(f'invalid native Persian input at UTF-8 offset {offset.value}')

    def cancel(self) -> None:
        with self._guard:
            worker = self._worker
        if worker is not None:
            worker.cancel()

    def close(self) -> None:
        with self._guard:
            self._closed = True
            worker, self._worker = self._worker, None
        if worker is not None:
            worker.close()

    def __enter__(self):
        return self

    def __exit__(self, *_args):
        self.close()
