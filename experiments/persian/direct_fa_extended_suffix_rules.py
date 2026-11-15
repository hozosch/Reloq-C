"""Candidate Persian endings, admitted only after reference verification."""
from __future__ import annotations

import re

SUFFIXES = ('ترین', 'هایی', 'هایم', 'هایت', 'هایش', 'هایمان', 'هایتان', 'هایشان',
            'هاش', 'هاشون', 'مان', 'تان', 'شان')
TAILS = (('tA', 'rin'), ('ha', "'i:'"), ('ha', 'jAm'), ('ha', 'jAt'), ('ha', 'jAS'),
         ('ha', 'je', 'man'), ('ha', 'je', 'tan'), ('ha', 'je', 'San'),
         ('haS',), ('ha', 'Sun'), ('{c}e', 'man'), ('{c}e', 'tan'), ('{c}e', 'San'))
SYLLABLE = re.compile(r'\.([012])([^.]*)')
CODAS = frozenset('bdfghjklmnprstvzSZGRCJ')


def candidate(base, kind, mode):
    if not 0 <= kind < len(SUFFIXES) or mode not in (0, 1) or not base.startswith('`[') or not base.endswith(']'):
        return None
    syllables = SYLLABLE.findall(base[2:-1])
    if not syllables or ''.join('.' + stress + body for stress, body in syllables) != base[2:-1]:
        return None
    bodies = [body for _, body in syllables]
    stresses = [int(stress) for stress, _ in syllables]
    tail = list(TAILS[kind])
    if kind >= 10:
        coda = bodies[-1][-1:]
        if coda not in CODAS or len(bodies[-1]) < 2:
            return None
        bodies[-1] = bodies[-1][:-1]
        tail[0] = coda + 'e'
    bodies.extend(tail)
    if mode == 0:
        primary = next((i for i, value in enumerate(stresses) if value == 1), 0)
        stresses.extend(2 if i - primary >= 2 and (i - primary) % 2 == 0 else 0
                        for i in range(len(stresses), len(bodies)))
    else:
        primary = max(0, len(bodies) - 2)
        stresses = [1 if i == primary else 2 if i == 0 and primary >= 2 else 0
                    for i in range(len(bodies))]
    return '`[' + ''.join('.' + str(stress) + body for stress, body in zip(stresses, bodies)) + ']'
