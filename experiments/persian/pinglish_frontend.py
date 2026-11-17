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
    'soal': 'سوال', 'soaal': 'سوال', 'sual': 'سوال', 'badbakht': 'بدبخت', 'badbacht': 'بدبخت',
    'farsi': 'فارسی', 'faarsi': 'فارسی', 'ketab': 'کتاب', 'ketaab': 'کتاب',
    'eslash': 'اسلش',
    'man': 'من', 'u': 'او', 'oo': 'او', 'ou': 'او',
    'amad': 'آمد', 'aamad': 'آمد',
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


def latin_spans(text: str) -> tuple[tuple[int, int], ...]:
    """Keep Latin letters and their combining marks in the same word."""
    result, start = [], None
    for index, char in enumerate(text):
        latin = char.isalpha() and unicodedata.name(char, '').startswith('LATIN ')
        mark = start is not None and unicodedata.category(char).startswith('M')
        if latin or mark:
            if start is None:
                start = index
        elif start is not None:
            result.append((start, index))
            start = None
    if start is not None:
        result.append((start, len(text)))
    return tuple(result)


def english_exception(word: str) -> bool:
    """Match the fa-Latn rules that explicitly defer a word to English.

    In the pinned fa_rules the Pinglish entries for ph and th are empty;
    eSpeak falls back to English for the whole word in these cases. Known
    Persian aliases take precedence, and spelling mode remains independent.
    """
    if latin_spans(word) != ((0, len(word)),):
        return False
    value = ''.join(char for char in word.casefold() if not unicodedata.category(char).startswith('M'))
    if any(not char.isascii() and char.isalpha() for char in word):
        return True
    if value in ALIASES:
        return False
    # eSpeak's fa-Latn path sends most precomposed non-ASCII Latin words
    # to English; do the same for unsupported Latin letters rather than
    # spelling a whole word as individual characters.
    return 'ph' in value or 'th' in value


def _connected(char: str) -> bool:
    return (char.isalnum() or unicodedata.category(char).startswith('M') or
            char in ('\u200c', '\u200d', "'", '’'))


def normalize_pinglish(text: str) -> tuple[str, tuple[int, ...]]:
    """Replace only known whole-word spellings, retaining a UTF-8 byte map."""
    # eSpeak ignores combining accents attached to ASCII Latin letters in
    # fa-Latn. Remove only those marks, preserving the source byte mapping.
    source_to_plain, plain, at, latin = [0], [], 0, False
    for char in text:
        keep = not (latin and unicodedata.category(char).startswith('M'))
        encoded_char = char.encode('utf8')
        start = at
        if keep:
            plain.append(char)
            latin = char.isascii() and char.isalpha()
            at += len(encoded_char)
        source_to_plain.extend(start + offset if keep else start for offset in range(1, len(encoded_char) + 1))
    text = ''.join(plain)
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
    return ''.join(chunks), tuple(mapping[position] for position in source_to_plain)


def latin_name(char: str) -> SpeechPart | None:
    if len(char) != 1 or not char.isascii() or not char.isalpha():
        return None
    tag = LATIN_NAMES.get(char.lower())
    return SpeechPart('fa', tag) if tag is not None else None
