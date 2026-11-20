"""Named native sound probes with explicit editable phone spans.

Consonants are synthesized with a carrier vowel; the editor isolates the
selected phone using the provider's actual acoustic intervals, not spelling
or fixed-duration guesses.  A few phones such as /h/ are represented by the
following vowel's onset and are identified separately below.
"""
from __future__ import annotations
from dataclasses import dataclass


@dataclass(frozen=True)
class Phone:
    key: str
    label: str
    tag: str
    first: int = 0
    count: int = 1
    onset: bool = False


def _vowels():
    return (
        Phone('a','A wie in Iran, lang','`[.1a]'),
        Phone('A','Ä, kurzes A','`[.1A]'),
        Phone('e','E wie in drei','`[.1e]'),
        Phone('i','I','`[.1i]'),
        Phone('o','O wie in zwei','`[.1o]'),
        Phone('u','U','`[.1u]'),
    )


_CONSONANTS = (
    ('p','P'),('t','T'),('k','K'),('b','B'),('d','D'),('g','G'),
    ('f','F'),('s','stimmloses S'),('z','stimmhaftes S'),
    ('S','Sch'),('Z','Sch wie in Journal'),('m','M'),('n','N'),
    ('l','L'),('j','J'),('r','R'),
)


def _consonants(items):
    return tuple(Phone(key, label + ' vor A', '`[.1' + key + 'a]') for key,label in items)


_PERSIAN = _vowels() + _consonants(_CONSONANTS) + (
    Phone('h','H vor A','`[.1ha]',onset=True),
    Phone('G','Ach-Laut vor A','`[.1Ga]'),
    Phone('R','Zäpfchenlaut vor A','`[.1Ra]'),
    Phone('C','Tsch vor A','`[.1Ca]',count=2),
    Phone('J','Dsch vor A','`[.1Ja]',count=2),
    Phone('r_i','R vor I','`[.1ri]'),
    Phone('i_r','R nach I, am Wortende','`[.1ir]',first=1),
)


def _generic_vowels():
    return tuple(Phone(k, label, '`[.1' + k + ']') for k,label in
                 (('a','A'),('A','kurzes A'),('e','E'),('i','I'),('o','O'),('u','U')))


_GERMAN = _generic_vowels() + _consonants(_CONSONANTS) + (
    Phone('h','H vor A','`[.1ha]',onset=True),
    Phone('r_i','R vor I','`[.1ri]'),
    Phone('i_r','R nach I, am Wortende','`[.1ir]',first=1),
)
_ENGLISH = _generic_vowels() + _consonants(tuple((('y','J') if k=='j' else (k,v)) for k,v in _CONSONANTS)) + (
    Phone('h','H vor A','`[.1ha]',onset=True),
    Phone('w','W vor A','`[.1wa]'),
    Phone('C','Tsch vor A','`[.1Ca]',count=2),
    Phone('J','Dsch vor A','`[.1Ja]',count=2),
)
_BRITISH = tuple(Phone(key, label, '`[.1' + key + ']') for key,label in (
    ('i','I wie in fleece'), ('I','kurzes I wie in kit'), ('E','E wie in dress'),
    ('A','kurzes A wie in trap'), ('a','A wie in bath'), ('@','O wie in lot'),
    ('c','O wie in thought'), ('U','U wie in foot'), ('u','U wie in goose'),
    ('H','A wie in strut'), ('R','Vokal wie in nurse'), ('x','Schwa wie in comma'),
    ('e','EI wie in face'), ('o','OU wie in goat'), ('Y','AI wie in price'),
    ('W','AU wie in mouth'), ('O','OI wie in choice'),
)) + _ENGLISH[len(_generic_vowels()):]
_JAPANESE = tuple(p for p in _generic_vowels() if p.key!='A') + _consonants(tuple(
    (k,v) for k,v in _CONSONANTS if k in 'ptkbdgfszmn r'.replace(' ',''))) + (
    Phone('h','H vor A','`[.1ha]',onset=True),
    Phone('S','Sch vor A','`[.1Sa]'),
)

PHONES = {'Persisch': _PERSIAN, 'Pinglish': _PERSIAN, 'Deutsch': _GERMAN,
          'Englisch (USA)': _ENGLISH, 'Englisch (Großbritannien)': _BRITISH,
          'Japanisch': _JAPANESE}


def find(language, key):
    return next((p for p in PHONES.get(language, ()) if p.key == key), None)


def interval(phone, spans):
    """Return the selected target and carrier end in native milliseconds."""
    if not spans:
        raise ValueError('Das Sprachpaket meldet keine Lautgrenzen. Bitte prüfen, ob der Eloquence-C-Testbuild 1.5.0-dev191 oder neuer installiert ist.')
    if phone.onset:
        begin, end = 0.0, spans[0][0] / 10000.0
        if end < 1:
            raise ValueError('Das H hat in dieser Stimme keinen getrennten Beginn. Bitte einen anderen Laut wählen.')
    else:
        first, after = phone.first, phone.first + phone.count
        if first >= len(spans):
            raise ValueError('Der ausgewählte Laut fehlt in dieser Stimme.')
        begin = spans[first][0] / 10000.0
        end = spans[after][0] / 10000.0 if after < len(spans) else spans[after - 1][1] / 10000.0
    if not 0 <= begin < end <= 60000:
        raise ValueError('Ungültige Zeitgrenzen für den ausgewählten Laut.')
    return begin, end
