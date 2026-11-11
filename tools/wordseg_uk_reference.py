"""Conservative comparison of native British SPR and reference IPA.

The English compound generator uses this only to compare sounds and primary
stress. Unknown IPA symbols decline a comparison; they are never discarded.
Unlike the US comparator, this keeps British LOT and PALM vowels distinct
and does not merge rhotic and non-rhotic pronunciations.
"""
from collections import defaultdict
import re

from referee import PHONES, VOWELS

_IPA = [
    ('tʃ','CH'), ('dʒ','JH'), ('eɪ','EY'), ('aɪ','AY'), ('ɔɪ','OY'),
    ('aʊ','AW'), ('əʊ','OW'), ('ɪə','IH AH'), ('eə','EH AH'), ('ʊə','UH AH'),
    ('ɑ','AA'), ('ɒ','LOT'), ('æ','AE'), ('ɐ','AH'), ('ʌ','AH'), ('ə','AH'),
    ('ɜ','ER'), ('ɛ','EH'), ('e','EH'), ('i','IY'), ('ɪ','IH'), ('ɔ','AO'),
    ('o','OW'), ('ʊ','UH'), ('u','UW'), ('p','P'), ('b','B'), ('t','T'),
    ('d','D'), ('k','K'), ('g','G'), ('ɡ','G'), ('f','F'), ('v','V'), ('θ','TH'),
    ('ð','DH'), ('s','S'), ('z','Z'), ('ʃ','SH'), ('ʒ','ZH'), ('m','M'),
    ('n','N'), ('ŋ','NG'), ('l','L'), ('ɹ','R'), ('r','R'), ('j','Y'),
    ('w','W'), ('h','HH'), ('ʔ','T'),
]
_VOWELS = VOWELS | {'LOT', 'ER'}


def ipa(text):
    text = re.sub(r'[\s/\[\].ːˑ]', '', text)
    out, pending, at = [], 0, 0
    while at < len(text):
        if text[at] in 'ˈˌ':
            pending = 1 if text[at] == 'ˈ' else 2
            at += 1
            continue
        for symbol, phones in _IPA:
            if text.startswith(symbol, at):
                for phone in phones.split():
                    out.append((phone, pending if phone in _VOWELS else None))
                    if phone in _VOWELS:
                        pending = 0
                at += len(symbol)
                break
        else:
            raise ValueError('unsupported British IPA symbol')
    if not out:
        raise ValueError('empty British IPA reading')
    return out


def native(text):
    text = re.sub(r'[`\[\] ]', '', text)
    out, stress, at = [], None, 0
    while at < len(text):
        c = text[at]
        if c == '.' and at + 1 < len(text) and text[at + 1].isdigit():
            stress, at = int(text[at + 1]), at + 2
            continue
        if c.isdigit():
            at += 1
            continue
        if c == '@':
            out.append(('LOT', stress)); at += 1
        elif c == 'L':
            out.extend((('AH', 0), ('L', None))); at += 1
        elif c == 'R':
            out.append(('ER' if stress in (1, 2) else 'AH', stress)); at += 1
        else:
            for symbol, phones in PHONES:
                if text.startswith(symbol, at):
                    for phone in phones.split():
                        mark = (0 if len(phones.split()) > 1 else stress)
                        out.append((phone, mark if phone in VOWELS else None))
                    at += len(symbol)
                    break
            else:
                raise ValueError('unsupported British native phone')
    return out


def _normalize(pairs):
    out = []
    for i, (phone, stress) in enumerate(pairs):
        if phone == 'NG' and i + 1 < len(pairs) and pairs[i + 1][0] in ('K', 'G'):
            phone = 'N'
        out.append((phone, stress))
    return out


def same(native_reading, reference):
    a, b = _normalize(native_reading), _normalize(reference)
    if len(a) != len(b):
        return False
    for (left, left_stress), (right, right_stress) in zip(a, b):
        if left == right:
            continue
        # The two references allow either schwa or a weak short i, but never
        # use that relaxation for a stressed vowel or to delete a consonant.
        if {left, right} <= {'AH', 'IH'} and left_stress != 1 and right_stress != 1:
            continue
        return False
    stresses_a = [stress for _, stress in a if stress is not None]
    stresses_b = [stress for _, stress in b if stress is not None]
    first_a = stresses_a.index(1) if 1 in stresses_a else -1
    first_b = stresses_b.index(1) if 1 in stresses_b else -1
    return first_a == first_b


def matches(reading, references):
    try:
        actual = native(reading)
    except ValueError:
        return False
    return any(same(actual, ref) for ref in references)


def read_references(path, wanted):
    result = defaultdict(list)
    for line in path.read_text(encoding='utf8').splitlines():
        word, separator, pronunciations = line.partition('\t')
        if not separator or word not in wanted:
            continue
        for pronunciation in pronunciations.split(','):
            try:
                result[word].append(ipa(pronunciation))
            except ValueError:
                continue
    return result
