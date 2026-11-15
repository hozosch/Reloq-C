"""Experimental Persian text-to-Eloquence-phone mapping.

The eSpeak library/data are used for text analysis, not for the sound.  The
output uses the private Finnish-based Persian acoustic probe (Q=7, X=1,
GLOTTAL=2), not
the ordinary Finnish voice.  This module is not installed in the NVDA driver.
"""
from __future__ import annotations

from dataclasses import dataclass
import re
import unicodedata

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
# for a glottal stop. An actual eSpeak w is rejected until it has its own
# distinct private representation.
CONSONANTS = {
    'b': 'b', 'd': 'd', 'dZ': 'J', 'f': 'f', 'g': 'g', 'h': 'h',
    'j': 'j', 'k': 'k', 'l': 'l', 'm': 'm', 'n': 'n', 'p': 'p',
    'q1': 'R', 'R': 'r', 'r': 'r', 's': 's', 'S': 'S', 't': 't',
    'tS': 'C', 'v': 'v', '?': 'w', 'x': 'G', 'z': 'z', 'Z': 'Z',
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
        if item == 'q' and i + 1 < len(items) and items[i + 1] == '1':
            item = 'q1'
            i += 1
        if item not in VOWELS and item not in CONSONANTS:
            raise UnsupportedPronunciation(f'unsupported eSpeak phone {item!r} in {word!r}')
        if item == '?' and result and result[-1].name == 'h':
            raise UnsupportedPronunciation('H followed by a glottal stop needs separate markers')
        if item not in VOWELS and (stress or long):
            raise UnsupportedPronunciation(f'unexpected stress or length on {item!r}')
        result.append(Phone(item, stress, long))
        i += 1
    if not any(p.name in VOWELS for p in result):
        raise UnsupportedPronunciation(f'word has no supported vowel: {word!r}')
    return tuple(result)


def word_tag(word: str) -> str:
    """Map the word to syllable-marked phones for the private acoustic probe."""
    items = word.strip('|').split('|')
    if '_' in items:
        # Like native phrase prediction, place a comma boundary in the
        # annotation stream. It is punctuation for the voice, not a spoken
        # name and not an extra phoneme.
        groups, group = [], []
        for item in items:
            if item == '_':
                groups.append(group)
                group = []
            else:
                group.append(item)
        groups.append(group)
        if any(not group for group in groups[1:-1]) or not any(groups):
            raise UnsupportedPronunciation('ambiguous repeated reference pause')
        return ','.join(word_tag('|'.join(group)) if group else '' for group in groups)
    phones = parse_word(word)
    vowel_indices = [i for i, p in enumerate(phones) if p.name in VOWELS]
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
                long = phone.long or (iran and position == 2)
                out.append(f"'{name}:'" if long else name)
            else:
                # Finnish otherwise loses a final H when another word follows.
                # h+w marks a short silent carrier for the native H impulse.
                out.append('hw' if phone.name == 'h' and position == len(phones) - 1
                           else CONSONANTS[phone.name])
        syllables.append('.' + str(phones[vowel_index].stress) + ''.join(out))
    return '`[' + ''.join(syllables) + ']'


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
                    for word in fragment.strip('| \t\r\n').split():
                        append('fa', word_tag(word), internal_word=True)
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


def native_tag_accepted(tag: str, native_phones: str) -> bool:
    """Finnish's phone display hides coda h although it renders its noise."""
    if tag in native_phones:
        return True
    vowel = max((i for i, ch in enumerate(tag) if ch in 'Aaeiou'), default=-1)
    if vowel < 0:
        return False
    displayed = tag[:vowel + 1] + tag[vowel + 1:].replace('h', '')
    return displayed != tag and displayed in native_phones


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
        if ((',' in tagged[left.end():right.start()]) !=
                (',' in native_phones[observed_left.end():observed_right.start()])):
            return False
    return True
