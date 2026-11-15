"""Conservative direct suffix templates, verified per stem against eSpeak.

The generated admission table is required: templates alone must never be
applied to an arbitrary word whose reference pronunciation was not checked.
"""
import re

SUFFIXES = ('ها', 'تر', 'م', 'ت', 'ش', 'ی')
SYLLABLE = re.compile(r'\.([012])([^.]*)')
CODAS = frozenset('bdfghjklmnprstvzSZGRCJ')
VOWELS = frozenset('aAeEiou')


def candidate(base, suffix):
    if suffix not in SUFFIXES or not base.startswith('`[') or not base.endswith(']'):
        return None
    syllables = SYLLABLE.findall(base[2:-1])
    if not syllables or ''.join('.' + stress + body for stress, body in syllables) != base[2:-1]:
        return None
    if suffix == 'م' and syllables[-1][1][-1:] in VOWELS:
        return base[:-1] + 'm]'
    if suffix not in ('ها', 'تر'):
        coda = syllables[-1][1][-1:]
        if coda not in CODAS or len(syllables[-1][1]) < 2:
            return None
        syllables[-1] = syllables[-1][0], syllables[-1][1][:-1]
        ending = '.0' + coda + {'م': 'Am', 'ت': 'At', 'ش': 'AS', 'ی': 'i'}[suffix]
    else:
        ending = '.0ha' if suffix == 'ها' else '.0tAr'
    count = len(syllables)
    stem = ''.join('.' + ('1' if i == count - 1 else '2' if i == 0 and count > 2 else '0') + body
                   for i, (_, body) in enumerate(syllables))
    return '`[' + stem + ending + ']'
