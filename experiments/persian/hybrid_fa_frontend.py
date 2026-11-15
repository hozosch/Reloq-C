"""Private direct-first Persian analysis with isolated eSpeak fallback.

This experiment has no NVDA integration. Native complete-word rules are used
when a whole input is covered. In other inputs, retain native punctuation
boundaries while using the reference worker to analyse each clause. This is
still a research frontend, not the NVDA speech or symbol-processing API.
"""
from __future__ import annotations

import ctypes
from dataclasses import dataclass
from pathlib import Path
import re
import threading

from espeak_worker import EspeakWorker
from text_frontend import SpeechPart, UnsupportedPronunciation, translate_text


@dataclass(frozen=True)
class PersianAnalysis:
    parts: tuple[SpeechPart, ...]
    # UTF-8 source byte -> tag byte, only at word granularity. Fallback does
    # not currently provide a reliable source map.
    source_map: tuple[int, ...] | None
    used_fallback: bool


def _native_english(text: str) -> bool:
    """Only route unambiguously English ASCII directly to the English core."""
    return (any('a' <= c.lower() <= 'z' for c in text)
            and all(c.isascii() and (c.isalnum() or c in ' \t\r\n.,?!:;\'"()-')
                    for c in text))


_BOUNDARIES = {',': ',', '،': ',', ';': ';', '؛': ';',
               '?': '?', '؟': '?', '.': '.', '!': '!', ':': ':'}


def _translate_with_native_punctuation(probe, text: str, *, pinglish: bool = False) -> tuple[SpeechPart, ...]:
    """Analyse clauses, then pass source punctuation to the owning core.

    This path deliberately does not split numeric input: commas and periods
    may be part of a number. Numeric analysis and symbol-level handling need
    their own integration. Within a clause eSpeak still analyses the complete
    fragment, including any English switch.
    """
    if any(char.isdecimal() for char in text) or not any(c in _BOUNDARIES for c in text):
        return translate_text(probe, text, pinglish=pinglish)
    parts: list[SpeechPart] = []
    pending = ''

    def append(language: str, content: str) -> None:
        if not content:
            return
        if parts and parts[-1].language == language:
            parts[-1] = SpeechPart(language, parts[-1].text + content)
        else:
            parts.append(SpeechPart(language, content))

    for fragment in re.split(r'([,.?!:;،؛؟])', text):
        if not fragment:
            continue
        if fragment in _BOUNDARIES:
            mark = _BOUNDARIES[fragment]
            if parts:
                append(parts[-1].language, mark)
            else:
                pending += mark
            continue
        core = fragment.strip()
        if not core:
            if parts:
                append(parts[-1].language, fragment)
            else:
                pending += fragment
            continue
        left = len(fragment) - len(fragment.lstrip())
        right = len(fragment.rstrip())
        leading, trailing = fragment[:left], fragment[right:]
        if not pinglish and _native_english(core):
            translated = (SpeechPart('en-us', core),)
        else:
            translated = translate_text(probe, core, pinglish=pinglish)
        if not translated:
            raise ValueError('reference produced no speech for a nonempty clause')
        for index, part in enumerate(translated):
            prefix = pending + leading if index == 0 else ''
            suffix = trailing if index == len(translated) - 1 else ''
            append(part.language, prefix + part.text + suffix)
        pending = ''
    if pending:
        if not parts:
            raise ValueError('punctuation without speech has no language assignment')
        append(parts[-1].language, pending)
    return tuple(parts)


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
        if '\u066b' in text or '\u066c' in text:
            raise UnsupportedPronunciation('Persian decimal and grouping separators need numeric handling')
        if not pinglish and _native_english(text):
            # Pass the original text and punctuation to the English ECI path.
            return PersianAnalysis((SpeechPart('en-us', text),), tuple(range(len(text) + 1)), False)
        if pinglish:
            return PersianAnalysis(_translate_with_native_punctuation(self._fallback(), text, pinglish=True), None, True)
        if text and not any(char.isalpha() for char in text):
            return PersianAnalysis(translate_text(self._fallback(), text), None, True)
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
            return PersianAnalysis(_translate_with_native_punctuation(self._fallback(), text), None, True)
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
