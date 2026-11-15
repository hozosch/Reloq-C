"""Conservatively align reference Persian words to their source positions.

Only the isolated reference helper is involved. Its synthesized audio is
discarded; the returned pronunciations still use the selected Eloquence
phones. Ambiguous expansions, languages or punctuation fail closed.
"""
from __future__ import annotations

import re
import unicodedata

if __package__:
    from .text_frontend import UnsupportedPronunciation, word_tags
else:
    from text_frontend import UnsupportedPronunciation, word_tags

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


def _tokens(text: str, *, pinglish: bool = False) -> tuple[tuple[int, int], ...] | None:
    """Recognize simple Persian or explicit Pinglish words."""
    def word_character(char: str) -> bool:
        return _word_character(char) or (pinglish and char.isascii() and char.isalpha())
    result = []
    position = 0
    while position < len(text):
        if text[position] in ' \t':
            position += 1
            continue
        if not word_character(text[position]) or not text[position].isalpha():
            return None
        start = position
        while position < len(text) and word_character(text[position]):
            position += 1
        result.append((start, position))
    return tuple(result)


def unpositioned_reference_input(text: str) -> bool:
    """Admit only simple Persian/number text, without raw engine controls."""
    digits = '0123456789۰۱۲۳۴۵۶۷۸۹٠١٢٣٤٥٦٧٨٩'
    punctuation = ' \t,.?!:;،؛؟٫٬+-−'
    return bool(text) and all(_word_character(char) or char in digits or
                              char in punctuation for char in text)


def unpositioned_pinglish_input(text: str) -> bool:
    """Allow simple transliteration, numbers and punctuation without tags."""
    return bool(text) and all(unpositioned_reference_input(char) or
                              (char.isascii() and char.isalpha()) for char in text)


def _reference_words(worker, core: str, tokens: tuple[tuple[int, int], ...],
                     *, voice: str = 'fa') -> tuple[str, ...] | None:
    """Require eSpeak's phoneme sequence and word events to agree exactly.

    Synthesis events omit some vowel length colons; length comes only from
    the independently retained text-to-phonemes pronunciation.
    """
    clauses = worker.translate(core, voice=voice, separator='|')
    words = tuple(word for clause in clauses for word in clause.phonemes.split())
    events = worker.events(core, voice=voice)
    grouped: list[tuple[int, list[str]]] = []
    for event in events:
        if event.kind == 1:
            grouped.append((event.position - 1, []))
        elif event.kind == 7:
            # Clause-final silence is outside the word. An underscore at the
            # word's own position is an actual short-pause phone and must
            # match the reference before becoming an internal comma.
            if event.phone in ('_', '_:') and (
                    not grouped or event.position - 1 != grouped[-1][0]):
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


def source_map_for_reference(text: str, parts, worker, *, pinglish: bool = False) -> tuple[int, ...] | None:
    """Return a verified UTF-8-byte to tag-byte map or decline the input.

    This deliberately covers simple Persian or Pinglish words and punctuation,
    not numbers, spelling mode or mixed-language speech. It never
    changes a pronunciation when an alignment cannot be established.
    """
    if len(parts) != 1 or parts[0].language != 'fa':
        return None
    tagged = parts[0].text
    source_tokens = []
    reference_tags = []
    for fragment in _FRAGMENT.finditer(text):
        raw = fragment.group()
        core = raw.strip(' \t')
        if not core:
            continue
        leading = len(raw) - len(raw.lstrip(' \t'))
        tokens = _tokens(core, pinglish=pinglish)
        if not tokens:
            return None
        words = _reference_words(worker, core, tokens, voice='fa-latn' if pinglish else 'fa')
        if words is None:
            return None
        try:
            reference_tags.extend(word_tags(words))
        except UnsupportedPronunciation:
            return None
        start = fragment.start() + leading
        source_tokens.extend((start + a, start + b) for a, b in tokens)
    if not source_tokens or len(source_tokens) != len(reference_tags):
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
                while target_end < out_end and tagged[target_end] in ' \t~':
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
    for i, ((start, end), tag) in enumerate(zip(source_tokens, reference_tags)):
        if tag.startswith('~'):
            # The private onset marker maps to the intervening source space.
            tag = tag[1:]
        next_start = source_tokens[i + 1][0] if i + 1 < len(source_tokens) else len(text)
        following = text[end:next_start].lstrip(' \t')
        if tag.endswith(',') and following and following[0] in ',.?!:;،؛؟':
            # The explicit source punctuation replaces the coincident
            # inferred boundary and remains mapped to its own source bytes.
            tag = tag[:-1]
        candidates = (tag[1:], tag) if tag.startswith(',') else (tag,)
        chosen = None
        for candidate in candidates:
            tag_start = tagged.find(candidate, out_cursor)
            if tag_start < 0:
                continue
            if candidate != tag and not tagged[:tag_start].rstrip(' \t').endswith(tuple(',.?!:;')):
                continue
            if gap(source_cursor, start, out_cursor, tag_start):
                chosen = (tag_start, candidate)
                break
        if chosen is None:
            return None
        tag_start, tag = chosen
        for b in range(byte_offsets[start], byte_offsets[end]):
            result[b] = tag_start
        tag_end = tag_start + len(tag)
        result[byte_offsets[end]] = tag_end
        source_cursor, out_cursor = end, tag_end
    if not gap(source_cursor, len(text), out_cursor, len(tagged)):
        return None
    return tuple(result)
