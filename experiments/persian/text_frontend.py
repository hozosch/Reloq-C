"""Experimental Persian text-to-Eloquence-phone mapping.

The eSpeak library/data are used for text analysis, not for the sound.  The
output uses the private Finnish-based Persian acoustic probe (Q=7, X=1,
GLOTTAL=2), not
the ordinary Finnish voice.  This module is not installed in the NVDA driver.
"""
from __future__ import annotations

from dataclasses import dataclass
import re
from typing import TYPE_CHECKING
import unicodedata

if TYPE_CHECKING:
    if __package__:
        from .espeak_ng_probe import EspeakPhonemeProbe
    else:
        from espeak_ng_probe import EspeakPhonemeProbe


class UnsupportedPronunciation(ValueError):
    """Do not silently substitute an unimplemented phone or language."""


@dataclass(frozen=True)
class SpeechPart:
    language: str                 # 'fa' for the probe, 'en-us' for ReLoq
    text: str


@dataclass(frozen=True)
class Phone:
    name: str
    stress: int = 0               # 0 unstressed, 1 primary, 2 secondary
    long: bool = False


# Lowercase r is Finnish's ordinary trill. Uppercase R is reserved *only in
# the private Q=7 probe* for Persian q; Q=7 changes it into a marked stop.
# G is the private X=1 German Ach-laut experiment; w is the private marker
# for a glottal stop. Reference /w/ uses the same available /v/ approximation
# as the direct Pinglish path until a separate approximant is implemented.
CONSONANTS = {
    'b': 'b', 'd': 'd', 'dZ': 'J', 'f': 'f', 'g': 'g', 'h': 'h',
    'j': 'j', 'k': 'k', 'l': 'l', 'm': 'm', 'n': 'n', 'p': 'p',
    'q1': 'R', 'R': 'r', 'r': 'r', 's': 's', 'S': 'S', 't': 't',
    'tS': 'C', 'v': 'v', 'w': 'v', '?': 'w', 'x': 'G', 'z': 'z', 'Z': 'Z',
}
VOWELS = {'a': 'A', 'A': 'a', 'e': 'e', 'i': 'i', 'o': 'o', 'u': 'u'}


def parse_word(word: str) -> tuple[Phone, ...]:
    """Parse one eSpeak word when phone separator is '|'."""
    items = word.strip('|').split('|')
    result = []
    i = 0
    while i < len(items):
        item = items[i]
        if not item:
            raise UnsupportedPronunciation(f'empty phone in {word!r}')
        stress = 1 if item.startswith("'") else 2 if item.startswith(',') else 0
        if stress:
            item = item[1:]
        long = item.endswith(':')
        if long:
            item = item[:-1]
        if (item == 'q' and long and i + 2 < len(items) and
                items[i + 1:i + 3] == ['q', '1']):
            # eSpeak can realize a geminated Persian uvular as a long /q/
            # closure followed by /q1/. Both share our Persian uvular slot;
            # retain the two uvular portions rather than spelling the word.
            result.append(Phone('q1'))
            item = 'q1'
            i += 2
            long = False
        elif item == 'q' and i + 1 < len(items) and items[i + 1] == '1':
            item = 'q1'
            i += 1
        if item not in VOWELS and item not in CONSONANTS:
            raise UnsupportedPronunciation(f'unsupported eSpeak phone {item!r} in {word!r}')
        if item == '?' and result and result[-1].name == 'h':
            raise UnsupportedPronunciation('H followed by a glottal stop needs separate markers')
        if item not in VOWELS and stress:
            raise UnsupportedPronunciation(f'unexpected stress on {item!r}')
        result.append(Phone(item, stress, long))
        i += 1
    if not any(p.name in VOWELS for p in result):
        raise UnsupportedPronunciation(f'word has no supported vowel: {word!r}')
    return tuple(result)


def _vowel_item(item: str) -> bool:
    return item.lstrip("',").rstrip(':') in VOWELS


def word_tag(word: str, *, initial_glottal: bool = False) -> str:
    """Map the word to syllable-marked phones for the private acoustic probe."""
    items = word.strip('|').split('|')
    if '_' in items:
        # A private separator marks a Finnish vowel onset after the reference
        # pause. It is replaced by a space before the native parser sees it.
        if any(a == b == '_' for a, b in zip(items, items[1:])) or all(i == '_' for i in items):
            raise UnsupportedPronunciation('ambiguous repeated reference pause')
        groups = [[]]
        separators = []
        leading = False
        for index, item in enumerate(items):
            if item == '_':
                if (index + 1 < len(items) and _vowel_item(items[index + 1]) and
                        (index > 0 or initial_glottal) and
                        (not groups[-1] or groups[-1][-1] != '?')):
                    if groups[-1]:
                        separators.append('~')
                        groups.append([])
                    else:
                        leading = True
            else:
                groups[-1].append(item)
        tags = [word_tag('|'.join(group)) for group in groups]
        result = tags[0]
        for separator, tag in zip(separators, tags[1:]):
            result += separator + tag
        return ('~' if leading else '') + result
    if initial_glottal and items and _vowel_item(items[0]):
        return '~' + word_tag(word)
    phones = parse_word(word)
    vowel_indices = [i for i, p in enumerate(phones) if p.name in VOWELS]
    # eSpeak represents the first /æ/ of alāmat with its own long-phone
    # marker. Mapping that marker to a Finnish long vowel more than doubles
    # the initial nucleus relative to the later /æ/. Preserve the reference
    # phones and stress but use the ordinary Persian front vowel for this
    # word family, including its inflected and compounded forms. This does
    # not impose a general /æ/ duration rule on other words.
    alamat = (len(phones) >= 6 and phones[0].name == 'a' and phones[0].long and
              tuple(phone.name for phone in phones[:6]) == ('a', 'l', 'A', 'm', 'a', 't'))
    # In ʿalāve, the eSpeak a: annotation likewise does not mean that
    # Finnish's long front vowel should be used. Preserve the stressed back
    # vowel and final E, including the spelling with an attached be-prefix.
    names = tuple(phone.name for phone in phones)
    alave_start = (0 if names[:5] == ('a', 'l', 'A', 'v', 'e') else
                   2 if names[:7] == ('b', 'e', 'a', 'l', 'A', 'v', 'e') else None)
    if alave_start is not None and not phones[alave_start].long:
        alave_start = None
    starts = [0]
    for previous, current in zip(vowel_indices, vowel_indices[1:]):
        # Retain the last consonant as the next onset, and also retain a
        # preceding consonant with a glide. This reproduces the accepted
        # Salam and donya examples; other clusters still need listening tests.
        between = current - previous - 1
        onset = min(between, 1)
        if between >= 2 and phones[current - 1].name in ('j', 'w'):
            onset = 2
        starts.append(current - onset)
    starts.append(len(phones))
    # The already accepted Iran example uses an explicitly long Finnish a.
    # Do not generalise this exception to every unstressed back vowel.
    iran = tuple((p.name, p.stress, p.long) for p in phones) == (
        ('i', 1, True), ('R', 0, False), ('A', 0, False), ('n', 0, False))
    syllables = []
    for index, vowel_index in enumerate(vowel_indices):
        out = []
        for position in range(starts[index], starts[index + 1]):
            phone = phones[position]
            if phone.name in VOWELS:
                name = VOWELS[phone.name]
                long = (phone.long and not (alamat and position == 0) and
                        position != alave_start) or (iran and position == 2)
                out.append(f"'{name}:'" if long else name)
            else:
                # Finnish otherwise loses a final H when another word follows.
                # h+w marks a short silent carrier for the native H impulse.
                consonant = CONSONANTS[phone.name] * (2 if phone.long else 1)
                out.append(consonant + 'w' if phone.name == 'h' and position == len(phones) - 1
                           else consonant)
        syllables.append('.' + str(phones[vowel_index].stress) + ''.join(out))
    return '`[' + ''.join(syllables) + ']'


def word_tags(words: tuple[str, ...] | list[str]) -> tuple[str, ...]:
    """Apply explicit reference pauses across words, not across punctuation."""
    result = []
    trailing_pause = False
    previous_glottal = False
    for index, word in enumerate(words):
        items = word.strip('|').split('|')
        leading_pause = bool(items and items[0] == '_')
        onset = index > 0 and (trailing_pause or leading_pause) and not previous_glottal
        result.append(word_tag(word, initial_glottal=onset))
        trailing_pause = bool(items and items[-1] == '_')
        actual = [item for item in items if item != '_']
        previous_glottal = bool(actual and actual[-1] == '?')
    return tuple(result)


def _latin_piece_spans(text: str) -> list[tuple[int, int]]:
    spans = []
    start = None
    for i, char in enumerate(text):
        latin = unicodedata.name(char, '').startswith('LATIN ')
        continuation = start is not None and unicodedata.category(char).startswith('M')
        if latin or continuation:
            if start is None:
                start = i
        elif start is not None:
            spans.append((start, i))
            start = None
    if start is not None:
        spans.append((start, len(text)))
    return spans


def _normal_english_phones(text: str) -> tuple[str, ...]:
    # Stress belongs to the English voice when the original text is routed.
    # Keep individual phone boundaries to avoid ambiguous string matching.
    return tuple(item.lstrip("',") for item in re.split(r'[|\s]+', text)
                 if item and item != ';')


def _english_reference(probe: EspeakPhonemeProbe, word: str, *, voice: str = 'fa') -> tuple[str, ...]:
    clauses = probe.translate(word, voice=voice, separator='|')
    if len(clauses) != 1:
        raise UnsupportedPronunciation(f'cannot match Latin text {word!r}')
    match = re.fullmatch(r'\s*\(en\)(.*?)\(fa\)\s*', clauses[0].phonemes)
    if not match:
        raise UnsupportedPronunciation(f'Latin text is not an English fallback: {word!r}')
    return _normal_english_phones(match.group(1))


def _punctuation(terminator: int) -> str:
    contour = terminator & 0x7000
    if contour == 0x1000:
        return ','
    if contour == 0x2000:
        return '?'
    if contour == 0x3000:
        return '!'
    if contour == 0 and not (terminator & 0x10000):  # explicit boundary, not EOF
        return '.' if terminator & 0x80000 else ','
    return ''


def translate_text(probe: EspeakPhonemeProbe, text: str, *, pinglish: bool = False) -> tuple[SpeechPart, ...]:
    """Analyse whole clauses, routing verified Latin fallback to ReLoq US English.

    Latin-to-English alignment is checked against eSpeak's pronunciation of
    each original Latin span. Complex unmatched URLs, symbols or switches are
    rejected, rather than losing or inventing text. Pinglish is an explicit
    separate mode and is sent through eSpeak's fa-latn analysis instead.
    """
    voice = 'fa-latn' if pinglish else 'fa'
    clauses = probe.translate(text, voice=voice, separator='|')
    spans = _latin_piece_spans(text)
    if pinglish:
        # Most Latin spans are Persian transliteration in this mode. Retain
        # only those eSpeak independently recognises as English fallback.
        english_spans = []
        for start, end in spans:
            try:
                _english_reference(probe, text[start:end], voice=voice)
            except UnsupportedPronunciation:
                continue
            english_spans.append((start, end))
        spans = english_spans
    cursor = 0
    parts: list[SpeechPart] = []
    trailing_internal = False

    def append(language: str, content: str, *, space: bool = True,
               internal_word: bool = False, punctuation: bool = False) -> None:
        nonlocal trailing_internal
        if not content:
            return
        if internal_word and content.startswith('~'):
            if parts and parts[-1].language == language:
                # The marker itself becomes the one native word separator.
                space = False
            else:
                content = content[1:]
        if punctuation and trailing_internal and parts and parts[-1].language == language:
            # The predictor also keeps a real punctuation boundary in
            # preference to an inferred one at exactly the same location.
            prev = parts[-1]
            parts[-1] = SpeechPart(language, re.sub(r',([ \t]*)$', r'\1', prev.text))
            trailing_internal = False
        if internal_word and content.startswith(',') and parts and parts[-1].language == language:
            if parts[-1].text.rstrip(' \t').endswith(tuple(',.?!:;')):
                content = content[1:]
        if parts and parts[-1].language == language:
            prev = parts[-1]
            parts[-1] = SpeechPart(language, prev.text + (' ' if space else '') + content)
        else:
            parts.append(SpeechPart(language, content))
        trailing_internal = internal_word and content.endswith(',')

    for clause in clauses:
        fragments = re.split(r'(\(en\)|\(fa\))', clause.phonemes)
        language = 'fa'
        for fragment in fragments:
            if fragment == '(en)':
                if language != 'fa':
                    raise UnsupportedPronunciation('unexpected English language switch')
                language = 'en-us'
            elif fragment == '(fa)':
                if language != 'en-us':
                    raise UnsupportedPronunciation('unexpected Persian language switch')
                language = 'fa'
            elif fragment.strip('| \t\r\n'):
                if language == 'fa':
                    words = fragment.strip('| \t\r\n').split()
                    for tag in word_tags(words):
                        append('fa', tag, internal_word=True)
                else:
                    wanted = _normal_english_phones(fragment)
                    found: tuple[str, ...] = ()
                    first = cursor
                    while len(found) < len(wanted) and cursor < len(spans):
                        start, end = spans[cursor]
                        if cursor > first and not text[spans[cursor - 1][1]:start].isspace():
                            raise UnsupportedPronunciation('unmatched punctuation inside Latin text')
                        found += _english_reference(probe, text[start:end], voice=voice)
                        if wanted[:len(found)] != found:
                            raise UnsupportedPronunciation('cannot align English fallback to source text')
                        cursor += 1
                    if found != wanted or cursor == first:
                        raise UnsupportedPronunciation('incomplete English fallback alignment')
                    append('en-us', text[spans[first][0]:spans[cursor - 1][1]])
        if language != 'fa':
            raise UnsupportedPronunciation('unterminated English fallback')
        punctuation = _punctuation(clause.terminator)
        if punctuation and parts:
            append(parts[-1].language, punctuation, space=False, punctuation=True)
    if cursor != len(spans):
        raise UnsupportedPronunciation('Latin text was not accounted for by eSpeak')
    return tuple(parts)


def _tag_signature(tag: str):
    """Separate phoneme order from the parser's movable syllable boundaries."""
    if not tag.startswith('`[') or not tag.endswith(']'):
        return None
    body = tag[2:-1]
    syllables = re.findall(r'\.([012])([^.]+)', body)
    if not syllables or ''.join('.' + stress + phones for stress, phones in syllables) != body:
        return None
    stresses = tuple((phone, stress) for stress, phones in syllables
                     for phone in phones if phone in 'Aaeiou')
    if not stresses:
        return None
    return ''.join(phones for _, phones in syllables), stresses


def _same_native_phones(expected: str, actual: str) -> bool:
    """Allow only the Finnish display's hidden postvocalic coda H."""
    left = right = 0
    while left < len(expected):
        if right < len(actual) and expected[left] == actual[right]:
            left += 1
            right += 1
            continue
        if expected[left] != 'h':
            return False
        before, after = left - 1, left + 1
        while before >= 0 and expected[before] in "':":
            before -= 1
        while after < len(expected) and expected[after] in "':":
            after += 1
        if (before < 0 or expected[before] not in 'Aaeiou' or
                after < len(expected) and expected[after] not in 'bcdfghjklmnprstvzSZGRCJw'):
            return False
        left += 1
    return right == len(actual)


def native_tag_accepted(tag: str, native_phones: str) -> bool:
    """Keep phone order and vowel stress even when the native display differs.

    Finnish can move H between adjacent syllables or omit coda H from the
    display while still rendering it. Neither is a reason to spell the word.
    Other missing or reordered phonemes and altered vowel stresses are errors.
    """
    if tag in native_phones:
        return True
    expected = _tag_signature(tag)
    if expected is None:
        return False
    for match in re.finditer(r'`\[[^]]+\]', native_phones):
        actual = _tag_signature(match.group())
        if actual is not None and expected[1] == actual[1] and _same_native_phones(expected[0], actual[0]):
            return True
    # Retain the previously accepted native display for final clusters such
    # as vowel+r+h; Finnish still renders that final H even when hiding it.
    vowel = max((i for i, ch in enumerate(tag) if ch in 'Aaeiou'), default=-1)
    displayed = tag[:vowel + 1] + tag[vowel + 1:].replace('h', '')
    return vowel >= 0 and displayed != tag and displayed in native_phones


def native_sequence_accepted(tagged: str, native_phones: str) -> bool:
    """Check every phone group and comma boundary in the native reading."""
    pattern = r'`\[[^]]+\]'
    expected = tuple(re.finditer(pattern, tagged))
    actual = tuple(re.finditer(pattern, native_phones))
    if (len(expected) != len(actual) or
            any(not native_tag_accepted(a.group(), b.group()) for a, b in zip(expected, actual))):
        return False
    for left, right, observed_left, observed_right in zip(expected, expected[1:], actual, actual[1:]):
        # The native parser may coalesce repeated written commas. Preserve
        # whether the boundary exists, while leaving its normal punctuation
        # policy to the core.
        source_gap = tagged[left.end():right.start()]
        shown_gap = native_phones[observed_left.end():observed_right.start()]
        if (',' in source_gap) != (',' in shown_gap):
            # The provider can omit a comma from its phoneme display when a
            # closing bracket already contributes the same phrase boundary.
            # Keep the check for ordinary commas and for unexpected commas.
            if not (',' in source_gap and ',' not in shown_gap and
                    any(char in source_gap for char in ')]}')):
                return False
    return True
