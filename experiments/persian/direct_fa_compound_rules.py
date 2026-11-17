"""Candidate compound patterns; generated per-stem admissions are required.

These reproduce specific existing eSpeak readings rather than predicting
unverified Persian compounds. The runtime must reject an unadmitted stem.
"""
from __future__ import annotations

import re

SUFFIXES = ('گر', '\u200cگر', '\u200cنمایش', '\u200cگذاری', '\u200cسازی')
SYLLABLE = re.compile(r'\.([012])([^.]*)')
TAILS = (None, None, '.0nA.1ma.0jeS]', '.0go.1za.0ri]', '.1sa.0zi]')


def candidate(base: str, kind: int, follows: bool) -> str | None:
    if not 0 <= kind < len(SUFFIXES) or not base.startswith('`[') or not base.endswith(']'):
        return None
    syllables = SYLLABLE.findall(base[2:-1])
    if not syllables or ''.join('.' + stress + body for stress, body in syllables) != base[2:-1]:
        return None
    if kind == 0:
        n = len(syllables)
        start = ''.join('.' + ('1' if index == n - 1 else '2' if index == 0 and n > 2 else '0') + body
                        for index, (_stress, body) in enumerate(syllables))
        return '`[' + start + '.0gAr]'
    if kind == 1:
        return base[:-1] + ('.0gAr]' if follows else '.1gAr]')
    return base[:-1] + TAILS[kind]
