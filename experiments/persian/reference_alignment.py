"""Conservatively align reference Persian words to their source positions.

Only the isolated reference helper is involved. Its synthesized audio is
discarded; the returned pronunciations still use the selected Eloquence
phones. Ambiguous expansions, languages or punctuation fail closed.
"""
from __future__ import annotations

import re
import unicodedata

from text_frontend import UnsupportedPronunciation, word_tag

_TAG = re.compile(r'`\[[^]]+\]')
_FRAGMENT = re.compile(r'[^,.?!:;،؛؟]+')
_PUNCTUATION = {'،': ',', '؛': ';', '؟': '?'}


def _word_character(char: str) -> bool:
    if char in ('\u200c', '\u200d'):
        return True
    cp = ord(char)
    arabic = (0x0600 <= cp <= 0x06ff or 0x0750 <= cp <= 0x077f or
              0x08a0 <= cp <= 0x08ff or 0xfb50 <= cp <= 0xfdff or
              0xfe70 <= cp <= 0xfeff)
    return arabic and unicodedata.category(char)[0] in ('L', 'M')


def _tokens(text: str) -> tuple[tuple[int, int], ...] | None:
    """Recognize only simple Persian words separated by spaces or tabs."""
    result = []
    position = 0
    while position < len(text):
        if text[position] in ' \t':
            position += 1
            continue
        if not _word_character(text[position]) or not text[position].isalpha():
            return None
        start = position
        while position < len(text) and _word_character(text[position]):
            position += 1
        result.append((start, position))
    return tuple(result)


def unpositioned_reference_input(text: str) -> bool:
    """Admit only simple Persian/number text, without raw engine controls."""
    digits = '0123456789۰۱۲۳۴۵۶۷۸۹٠١٢٣٤٥٦٧٨٩'
    punctuation = ' \t,.?!:;،؛؟٫٬+-−'
    return bool(text) and all(_word_character(char) or char in digits or
                              char in punctuation for char in text)


def _reference_words(worker, core: str, tokens: tuple[tuple[int, int], ...]) -> tuple[str, ...] | None:
    """Require eSpeak's phoneme sequence and word events to agree exactly.

    Synthesis events omit some vowel length colons; length comes only from
    the independently retained text-to-phonemes pronunciation.
    """
    clauses = worker.translate(core, separator='|')
    words = tuple(word for clause in clauses for word in clause.phonemes.split())
    events = worker.events(core)
    grouped: list[tuple[int, list[str]]] = []
    for event in events:
        if event.kind == 1:
            grouped.append((event.position - 1, []))
        elif event.kind == 7:
            if event.phone in ('_', '_:'):
                continue
            if not grouped or event.position - 1 != grouped[-1][0]:
                return None
            grouped[-1][1].append(event.phone.rstrip(':'))
        else:
            return None
    if len(tokens) != len(words) or len(grouped) != len(words):
        return None
    for (source_start, _), word, (event_start, phones) in zip(tokens, words, grouped):
        reference = tuple(phone.rstrip(':') for phone in word.strip('|').split('|'))
        if event_start != source_start or tuple(phones) != reference:
            return None
    return words


def source_map_for_reference(text: str, parts, worker) -> tuple[int, ...] | None:
    """Return a verified UTF-8-byte to tag-byte map or decline the input.

    This deliberately covers Persian-only words and simple punctuation, not
    numbers, spelling mode, Pinglish or mixed-language speech. It never
    changes a pronunciation when an alignment cannot be established.
    """
    if len(parts) != 1 or parts[0].language != 'fa':
        return None
    tagged = parts[0].text
    tags = tuple(_TAG.finditer(tagged))
    source_tokens = []
    reference_tags = []
    for fragment in _FRAGMENT.finditer(text):
        raw = fragment.group()
        core = raw.strip(' \t')
        if not core:
            continue
        leading = len(raw) - len(raw.lstrip(' \t'))
        tokens = _tokens(core)
        if not tokens:
            return None
        words = _reference_words(worker, core, tokens)
        if words is None:
            return None
        try:
            reference_tags.extend(word_tag(word) for word in words)
        except UnsupportedPronunciation:
            return None
        start = fragment.start() + leading
        source_tokens.extend((start + a, start + b) for a, b in tokens)
    if (not source_tokens or len(source_tokens) != len(tags) or
            tuple(match.group() for match in tags) != tuple(reference_tags)):
        return None
    byte_offsets = [0]
    for char in text:
        byte_offsets.append(byte_offsets[-1] + len(char.encode('utf8')))
    result = [0] * (byte_offsets[-1] + 1)

    def gap(start: int, end: int, out_start: int, out_end: int) -> bool:
        # The reference can collapse runs of whitespace. Preserve the
        # surrounding word boundaries and map intervening marks monotonically.
        i, out = start, out_start
        while i < end:
            char = text[i]
            if char in ' \t':
                source_end = i
                while source_end < end and text[source_end] in ' \t':
                    source_end += 1
                target_end = out
                while target_end < out_end and tagged[target_end] in ' \t':
                    target_end += 1
                if target_end == out:
                    return False
                for k in range(i, source_end):
                    value = out + (k - i) * (target_end - out) // (source_end - i)
                    for b in range(byte_offsets[k], byte_offsets[k + 1]):
                        result[b] = value
                i, out = source_end, target_end
            else:
                expected = _PUNCTUATION.get(char, char)
                if expected not in ',.?!:;' or out >= out_end or tagged[out] != expected:
                    return False
                for b in range(byte_offsets[i], byte_offsets[i + 1]):
                    result[b] = out
                i += 1
                out += 1
        result[byte_offsets[end]] = out
        return out == out_end

    source_cursor = out_cursor = 0
    for (start, end), tag in zip(source_tokens, tags):
        if not gap(source_cursor, start, out_cursor, tag.start()):
            return None
        for b in range(byte_offsets[start], byte_offsets[end]):
            result[b] = tag.start()
        result[byte_offsets[end]] = tag.end()
        source_cursor, out_cursor = end, tag.end()
    if not gap(source_cursor, len(text), out_cursor, len(tagged)):
        return None
    return tuple(result)
