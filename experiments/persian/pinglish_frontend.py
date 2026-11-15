"""Small, explicit Pinglish aliases and direct Latin character names."""
from __future__ import annotations

import re
import unicodedata

if __package__:
    from .text_frontend import SpeechPart
else:
    from text_frontend import SpeechPart

# These are spelling aliases for pronunciations already confirmed in the
# Persian-script voice, not a general transliterator or English detection.
ALIASES = {
    'salam': 'سلام', 'salaam': 'سلام', 'donya': 'دنیا', 'donyaa': 'دنیا',
    'iran': 'ایران', 'iraan': 'ایران', 'khub': 'خوب', 'khoob': 'خوب',
    'nokte': 'نقطه', 'nokteh': 'نقطه', 'noghte': 'نقطه', 'noghteh': 'نقطه',
    'kaghaz': 'کاغذ', 'kakhaz': 'کاغذ', 'sefr': 'صفر', 'yek': 'یک',
    'soal': 'سوال', 'soaal': 'سوال', 'sual': 'سوال', 'badbakht': 'بدبخت',
    'farsi': 'فارسی', 'faarsi': 'فارسی', 'ketab': 'کتاب', 'ketaab': 'کتاب',
    'man': 'من',
}
LATIN_NAMES = {
    'a': '`[.1ej]', 'b': '`[.1bi]', 'c': '`[.1si]', 'd': '`[.1di]',
    'e': '`[.1i]', 'f': '`[.1ef]', 'g': '`[.1Ji]', 'h': '`[.1ejC]',
    'i': '`[.1aj]', 'j': '`[.1Jej]', 'k': '`[.1kej]', 'l': '`[.1el]',
    'm': '`[.1em]', 'n': '`[.1en]', 'o': '`[.1o]', 'p': '`[.1pi]',
    'q': '`[.1ki.0ju]', 'r': '`[.1ar]', 's': '`[.1es]', 't': '`[.1ti]',
    'u': '`[.1ju]', 'v': '`[.1vi]', 'w': '`[.0dA.1be.0lju]',
    'x': '`[.1eks]', 'y': '`[.1vaj]', 'z': '`[.1zi]',
}
_LATIN = re.compile(r'[A-Za-z]+')


def _connected(char: str) -> bool:
    return (char.isalnum() or unicodedata.category(char).startswith('M') or
            char in ('\u200c', '\u200d', "'", '’'))


def normalize_pinglish(text: str) -> tuple[str, tuple[int, ...]]:
    """Replace only known whole-word spellings, retaining a UTF-8 byte map."""
    encoded = text.encode('utf8')
    char_to_byte = [0]
    for char in text:
        char_to_byte.append(char_to_byte[-1] + len(char.encode('utf8')))
    mapping = [0] * (len(encoded) + 1)
    chunks, source_pos, target_pos = [], 0, 0

    def emit(start: int, end: int, replacement: str | None = None) -> None:
        nonlocal target_pos
        first, last = char_to_byte[start], char_to_byte[end]
        value = text[start:end] if replacement is None else replacement
        target = value.encode('utf8')
        chunks.append(value)
        for byte in range(first, last):
            mapping[byte] = target_pos + (byte - first if replacement is None else 0)
        target_pos += len(target)
        mapping[last] = target_pos

    for match in _LATIN.finditer(text):
        source = match.group()
        before = text[match.start() - 1] if match.start() else ''
        after = text[match.end()] if match.end() < len(text) else ''
        target = ALIASES.get(source.lower())
        if not target or (before and _connected(before)) or (after and _connected(after)):
            continue
        emit(source_pos, match.start())
        emit(match.start(), match.end(), target)
        source_pos = match.end()
    emit(source_pos, len(text))
    return ''.join(chunks), tuple(mapping)


def latin_name(char: str) -> SpeechPart | None:
    if len(char) != 1 or not char.isascii() or not char.isalpha():
        return None
    tag = LATIN_NAMES.get(char.lower())
    return SpeechPart('fa', tag) if tag is not None else None
