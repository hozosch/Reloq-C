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
import unicodedata

if __package__:
    from .espeak_worker import EspeakWorker
    from .pinglish_frontend import latin_name, normalize_pinglish
    from .persian_symbols import SYMBOL_TAGS, UNICODE_TAG
    from .reference_alignment import (source_map_for_reference, unpositioned_pinglish_input,
                                      unpositioned_reference_input)
    from .text_frontend import SpeechPart, UnsupportedPronunciation, translate_text
else:
    from espeak_worker import EspeakWorker
    from pinglish_frontend import latin_name, normalize_pinglish
    from persian_symbols import SYMBOL_TAGS, UNICODE_TAG
    from reference_alignment import (source_map_for_reference, unpositioned_pinglish_input,
                                     unpositioned_reference_input)
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
    trailing_internal = False

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
                if trailing_internal:
                    previous = parts[-1]
                    parts[-1] = SpeechPart(previous.language, re.sub(r',([ \t]*)$', r'\1', previous.text))
                append(parts[-1].language, mark)
            else:
                pending += mark
            trailing_internal = False
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
            content = part.text
            before = (parts[-1].text if parts and parts[-1].language == part.language else '') + prefix
            if (part.language == 'fa' and content.startswith(',') and
                    before.rstrip(' \t').endswith(tuple(',.?!:;'))):
                content = content[1:]
            append(part.language, prefix + content + suffix)
            trailing_internal = part.language == 'fa' and content.endswith(',')
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
        self._pinglish_word = getattr(self._dll, 'reloq_fa_pinglish_word', None)
        if self._pinglish_word is not None:
            self._pinglish_word.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
            self._pinglish_word.restype = ctypes.c_int
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
        normalized = unicodedata.normalize('NFKC', char)
        normalized = {'ك': 'ک', 'ي': 'ی', 'ى': 'ی'}.get(normalized, normalized)
        value = self._dll.reloq_fa_direct_letter(ord(normalized)) if len(normalized) == 1 else None
        tag = value.decode('ascii') if value is not None else SYMBOL_TAGS.get(char)
        if tag is None:
            name = latin_name(char)
            if name is not None:
                tag = name.text
        if tag is None:
            # Keep unsupported Unicode identifiable in explicit spelling
            # instead of dropping the entire speech job. Hex letters use the
            # same Latin names as Pinglish; digits use Persian digit names.
            pieces = [UNICODE_TAG, SYMBOL_TAGS['+']]
            for digit in format(ord(char), 'X'):
                name = latin_name(digit)
                pieces.append(SYMBOL_TAGS[digit] if name is None else name.text)
            tag = ' '.join(pieces)
        mapping = (0,) * len(char.encode('utf8')) + (len(tag),)
        return PersianAnalysis((SpeechPart('fa', tag),), mapping, False)

    def analyze_pinglish_character(self, char: str) -> PersianAnalysis | None:
        """Use Persian names for Latin letters in the explicit Pinglish profile."""
        if not isinstance(char, str) or len(char) != 1 or char == '\0':
            raise ValueError('one Unicode character is required')
        return self.analyze_character(char)

    def _direct_pinglish(self, text: str) -> PersianAnalysis | None:
        """Run native Latin rules alongside verified Persian words and numbers."""
        if self._pinglish_word is None:
            return None
        words = tuple(re.finditer(r'[A-Za-z]+', text))
        if not words:
            return None
        source = text.encode('utf8')
        offsets = [0]
        for char in text:
            offsets.append(offsets[-1] + len(char.encode('utf8')))
        mapping = [0] * (len(source) + 1)
        pieces, position, cursor = [], 0, 0

        def ordinary(first, last, *, follows):
            nonlocal position
            if first == last:
                return True
            value = text[first:last]
            # A known following Latin word is also a following word for the
            # preceding Persian function-word stress.
            trial = value + (' سلام' if follows else '')
            status, tagged, local, _ = self._direct(trial)
            if status != 0:
                return False
            count = offsets[last] - offsets[first]
            end = local[count]
            pieces.append(tagged[:end])
            for i in range(count + 1):
                mapping[offsets[first] + i] = position + local[i]
            position += end
            return True

        for match in words:
            if not ordinary(cursor, match.start(), follows=True):
                return None
            word = match.group()
            if len(word) == 1:
                named = latin_name(word)
                if named is None:
                    return None
                tag = named.text
            else:
                encoded = word.encode('ascii')
                buffer = ctypes.create_string_buffer(max(2048, len(encoded) * 12 + 16))
                status = self._pinglish_word(encoded, len(encoded), buffer, len(buffer))
                if status != 1:
                    return None
                tag = buffer.value.decode('ascii')
            start, end = offsets[match.start()], offsets[match.end()]
            for i in range(start, end):
                mapping[i] = position
            pieces.append(tag)
            position += len(tag)
            mapping[end] = position
            cursor = match.end()
        if not ordinary(cursor, len(text), follows=False):
            return None
        return PersianAnalysis((SpeechPart('fa', ''.join(pieces)),), tuple(mapping), False)

    def analyze_pinglish(self, text: str, *, positioned: bool,
                         allow_fallback: bool) -> PersianAnalysis | None:
        """Analyze explicit Pinglish, preserving confirmed Persian word sounds.

        Marks are mapped through whole-word aliases and, when necessary,
        verified reference events. Unknown mixed languages are not guessed.
        """
        if not isinstance(text, str) or '\0' in text:
            raise ValueError('text must be a string without NUL')
        if not text:
            return self.analyze_direct(text)
        normalized, original_to_normalized = normalize_pinglish(text)
        if not unpositioned_pinglish_input(normalized):
            return None
        direct = self.analyze_direct(normalized)
        if direct is None:
            direct = self._direct_pinglish(normalized)
        if direct is not None:
            assert direct.source_map is not None
            mapping = tuple(direct.source_map[pos] for pos in original_to_normalized)
            return PersianAnalysis(direct.parts, mapping, False)
        if not allow_fallback or not any(c.isalpha() for c in normalized):
            return None
        if not positioned:
            parts = self._fallback_with_numbers(normalized, pinglish=True)
            return PersianAnalysis(parts, None, True) if len(parts) == 1 and parts[0].language == 'fa' else None
        # A preceding spelling-mode request can leave eSpeak's first event
        # pass out of sync. Retry the complete analysis once; each attempt
        # still has to verify every word and phoneme before it can be used.
        for _ in range(2):
            if any(c.isdecimal() for c in normalized):
                aligned = self._positioned_numbers(normalized, pinglish=True)
                if aligned is not None and aligned.source_map is not None:
                    return PersianAnalysis(aligned.parts,
                                           tuple(aligned.source_map[pos] for pos in original_to_normalized), True)
            else:
                parts = self._fallback_with_numbers(normalized, pinglish=True)
                if len(parts) != 1 or parts[0].language != 'fa':
                    return None
                mapped = source_map_for_reference(normalized, parts, self._fallback(), pinglish=True)
                if mapped is not None:
                    return PersianAnalysis(parts, tuple(mapped[pos] for pos in original_to_normalized), True)
        return None

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

    def _number_template(self, text, spans, *, pinglish):
        """Use a known word as a checked stand-in for contextual analysis."""
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
            template = ''.join(chunks)
            parts = _translate_with_native_punctuation(self._fallback(), template, pinglish=pinglish)
            if sum(part.text.count(tag) for part in parts if part.language == 'fa') != len(spans):
                continue
            return marker, tag, template, parts
        raise UnsupportedPronunciation('could not align numbers with the reference text')

    def _fallback_with_numbers(self, text: str, *, pinglish: bool = False) -> tuple[SpeechPart, ...]:
        """Keep numeric values direct while analysing surrounding whole clauses."""
        spans = self._numeric_spans(text)
        if not spans:
            return _translate_with_native_punctuation(self._fallback(), text, pinglish=pinglish)
        _, tag, _, parts = self._number_template(text, spans, pinglish=pinglish)
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

    def _positioned_numbers(self, text: str, *, pinglish: bool) -> PersianAnalysis | None:
        """Verify each stand-in's word events before mapping native numbers."""
        spans = self._numeric_spans(text)
        if not spans:
            return None
        marker, tag, template, parts = self._number_template(text, spans, pinglish=pinglish)
        if len(parts) != 1 or parts[0].language != 'fa':
            return None
        template_map = source_map_for_reference(template, parts, self._fallback(), pinglish=pinglish)
        if template_map is None:
            return None
        raw = text.encode('utf8')
        original_to_template = [0] * (len(raw) + 1)
        template_position = original_position = 0
        intervals = []
        for start, end, numeric_tag in spans:
            first = len(text[:start].encode('utf8'))
            last = len(text[:end].encode('utf8'))
            for i in range(original_position, first):
                original_to_template[i] = template_position + i - original_position
            template_position += first - original_position
            mark_first = template_position
            for i in range(first, last):
                original_to_template[i] = mark_first
            template_position += len(marker.encode('utf8'))
            original_to_template[last] = template_position
            intervals.append((template_map[mark_first], template_map[template_position], numeric_tag))
            original_position = last
        for i in range(original_position, len(raw) + 1):
            original_to_template[i] = template_position + i - original_position
        if original_to_template[-1] != len(template.encode('utf8')):
            return None
        tagged = parts[0].text
        target_map = [0] * (len(tagged) + 1)
        chunks, source_position, target_position = [], 0, 0
        for first, last, numeric_tag in intervals:
            if first < source_position or tagged[first:last] != tag:
                return None
            chunks.append(tagged[source_position:first])
            for i in range(source_position, first):
                target_map[i] = target_position + i - source_position
            target_position += first - source_position
            for i in range(first, last):
                target_map[i] = target_position
            chunks.append(numeric_tag)
            target_position += len(numeric_tag)
            target_map[last] = target_position
            source_position = last
        chunks.append(tagged[source_position:])
        for i in range(source_position, len(tagged) + 1):
            target_map[i] = target_position + i - source_position
        final = ''.join(chunks)
        mapping = tuple(target_map[template_map[i]] for i in original_to_template)
        return PersianAnalysis((SpeechPart('fa', final),), mapping, True)

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
        if not unpositioned_reference_input(text):
            return None
        if not any(c.isalpha() for c in text):
            return None
        for _ in range(2):
            if any(c.isdecimal() for c in text):
                result = self._positioned_numbers(text, pinglish=False)
                if result is not None:
                    return result
            else:
                parts = _translate_with_native_punctuation(self._fallback(), text)
                mapping = source_map_for_reference(text, parts, self._fallback())
                if mapping is not None:
                    return PersianAnalysis(parts, mapping, True)
        return None

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
