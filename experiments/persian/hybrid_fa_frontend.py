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
from reference_alignment import source_map_for_reference
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
_NUMBER_RUN = re.compile(r'[0-9\u0660-\u0669\u06f0-\u06f9][0-9\u0660-\u0669\u06f0-\u06f9.,\u066b\u066c]*')


def _reference_numeric_spelling(text: str) -> str:
    result = []
    for char in text:
        value = ord(char)
        if 0x660 <= value <= 0x669:
            result.append(chr(ord('0') + value - 0x660))
        elif 0x6f0 <= value <= 0x6f9:
            result.append(chr(ord('0') + value - 0x6f0))
        else:
            result.append('.' if char == '\u066b' else ',' if char == '\u066c' else char)
    return ''.join(result)


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
        self._dll.reloq_fa_direct_number.argtypes = (
            ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t),
            ctypes.c_void_p, ctypes.c_size_t,
        )
        self._dll.reloq_fa_direct_number.restype = ctypes.c_int
        self._dll.reloq_fa_direct_free.argtypes = (ctypes.c_void_p,)
        self._dll.reloq_fa_direct_letter.argtypes = (ctypes.c_uint32,)
        self._dll.reloq_fa_direct_letter.restype = ctypes.c_char_p
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

    def _direct(self, text: str):
        encoded = text.encode('utf8')
        output = ctypes.c_void_p()
        mapping = ctypes.POINTER(ctypes.c_size_t)()
        offset = ctypes.c_size_t()
        status = self._dll.reloq_fa_direct_text(encoded, len(encoded), ctypes.byref(output),
                                                 ctypes.byref(mapping), ctypes.byref(offset))
        if status:
            return status, '', None, offset.value
        try:
            tagged = ctypes.string_at(output).decode('ascii')
            source_map = tuple(mapping[i] for i in range(len(encoded) + 1))
        finally:
            self._dll.reloq_fa_direct_free(output)
            self._dll.reloq_fa_direct_free(mapping)
        return 0, tagged, source_map, 0

    def analyze_direct(self, text: str) -> PersianAnalysis | None:
        """Return only a fully covered native Persian analysis, without eSpeak."""
        if not isinstance(text, str) or '\0' in text:
            raise ValueError('text must be a string without NUL')
        with self._guard:
            if self._closed:
                raise RuntimeError('Persian text frontend is closed')
        status, tagged, source_map, offset = self._direct(text)
        if status == 0:
            parts = (SpeechPart('fa', tagged),) if tagged else ()
            return PersianAnalysis(parts, source_map, False)
        if status == 1:
            return None
        if status == 2:
            raise UnsupportedPronunciation(f'ambiguous or unsupported Persian number at UTF-8 offset {offset}')
        if status == -2:
            raise MemoryError('native Persian text allocation failed')
        raise ValueError(f'invalid native Persian input at UTF-8 offset {offset}')

    def analyze_character(self, char: str) -> PersianAnalysis | None:
        """Name one explicitly requested Persian letter without a worker."""
        if not isinstance(char, str) or len(char) != 1 or char == '\0':
            raise ValueError('one Unicode character is required')
        with self._guard:
            if self._closed:
                raise RuntimeError('Persian text frontend is closed')
        value = self._dll.reloq_fa_direct_letter(ord(char))
        if value is None:
            return None
        tag = value.decode('ascii')
        mapping = (0,) * len(char.encode('utf8')) + (len(tag),)
        return PersianAnalysis((SpeechPart('fa', tag),), mapping, False)

    def _numeric_spans(self, text: str) -> list[tuple[int, int, str]]:
        """Find validated numbers without consuming sentence punctuation."""
        spans = []
        cursor = 0
        for match in _NUMBER_RUN.finditer(text):
            if any(mark in text[cursor:match.start()] for mark in ('\u066b', '\u066c')):
                raise UnsupportedPronunciation('unattached Persian numeric separator')
            canonical = _reference_numeric_spelling(match.group()).encode('ascii')
            used = ctypes.c_size_t()
            output = ctypes.create_string_buffer(2048)
            status = self._dll.reloq_fa_direct_number(canonical, len(canonical),
                                                       ctypes.byref(used), output, len(output))
            if status != 0:
                raise UnsupportedPronunciation('ambiguous or unsupported Persian number')
            start, end = match.start(), match.start() + used.value
            numeric_tag = output.value.decode('ascii')
            if start and text[start - 1] in '+-\u2212':
                start -= 1
                status, numeric_tag, _, _ = self._direct(text[start:end])
                if status != 0:
                    raise UnsupportedPronunciation('unsupported signed Persian number')
            if ((start and text[start - 1].isalpha()) or
                (end < len(text) and text[end].isalpha())):
                raise UnsupportedPronunciation('numbers attached to letters need separate analysis')
            if start and text[start - 1].isdecimal():
                raise UnsupportedPronunciation('a numeric range needs separate analysis')
            spans.append((start, end, numeric_tag))
            cursor = end
        if any(mark in text[cursor:] for mark in ('\u066b', '\u066c')):
            raise UnsupportedPronunciation('unattached Persian numeric separator')
        return spans

    def _fallback_with_numbers(self, text: str, *, pinglish: bool = False) -> tuple[SpeechPart, ...]:
        """Keep numeric values direct while analysing surrounding whole clauses.

        A checked invariant Persian word marks each number for eSpeak's
        contextual analysis. The marker's tag is then replaced with native
        numeric tags. Ambiguous marker alignment is never guessed. The source
        map still belongs to future NVDA integration.
        """
        spans = self._numeric_spans(text)
        if not spans:
            return _translate_with_native_punctuation(self._fallback(), text, pinglish=pinglish)
        for marker in ('سلام', 'ایران', 'نقطه', 'خوب', 'کتاب', 'دنیا'):
            if marker in text:
                continue
            status, tag, _, _ = self._direct(marker)
            if status or not tag:
                continue
            chunks, cursor = [], 0
            for start, end, _ in spans:
                chunks.extend((text[cursor:start], marker))
                cursor = end
            chunks.append(text[cursor:])
            parts = _translate_with_native_punctuation(self._fallback(), ''.join(chunks), pinglish=pinglish)
            if sum(part.text.count(tag) for part in parts if part.language == 'fa') != len(spans):
                continue
            values = iter(item[2] for item in spans)
            output = []
            for part in parts:
                if part.language == 'fa':
                    pieces = part.text.split(tag)
                    content = pieces[0]
                    for suffix in pieces[1:]:
                        content += next(values) + suffix
                    part = SpeechPart(part.language, content)
                output.append(part)
            return tuple(output)
        raise UnsupportedPronunciation('could not align numbers with the reference text')

    def analyze(self, text: str, *, pinglish: bool = False) -> PersianAnalysis:
        if not isinstance(text, str) or '\0' in text:
            raise ValueError('text must be a string without NUL')
        with self._guard:
            if self._closed:
                raise RuntimeError('Persian text frontend is closed')
        if not pinglish and _native_english(text):
            # Pass the original text and punctuation to the English ECI path.
            return PersianAnalysis((SpeechPart('en-us', text),), tuple(range(len(text) + 1)), False)
        if pinglish:
            return PersianAnalysis(self._fallback_with_numbers(text, pinglish=True), None, True)
        if text and not any(char.isalpha() or char.isdecimal() for char in text):
            return PersianAnalysis(translate_text(self._fallback(), text), None, True)
        direct = self.analyze_direct(text)
        return direct if direct is not None else PersianAnalysis(self._fallback_with_numbers(text), None, True)

    def analyze_positioned(self, text: str) -> PersianAnalysis | None:
        """Direct analysis, or a reference result with verified word anchors.

        The reference route currently accepts only Persian words and simple
        punctuation. Unknown alignment is never inferred from word order.
        """
        direct = self.analyze_direct(text)
        if direct is not None:
            return direct
        if any(c.isascii() and c.isalnum() or c.isdecimal() for c in text):
            return None
        if not any(c.isalpha() for c in text):
            return None
        parts = _translate_with_native_punctuation(self._fallback(), text)
        mapping = source_map_for_reference(text, parts, self._fallback())
        return PersianAnalysis(parts, mapping, True) if mapping is not None else None

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
