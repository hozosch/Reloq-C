#!/usr/bin/env python3
"""Prevent accidentally labeling the observed native baseline as reconstructed G2P."""
from pathlib import Path
import hashlib
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
EXPECTED_DEU = '8d917e07fdd59cf2cdd8e6f8c2118f18904cb09965d381baa779a7d0eb5aefe0'
FUNCTIONS = ('break_into_ger_morphs', 'strip_inflection', 'end_morph_analysis',
             'create_ger_syllables', 'assign_ger_stress', 'find_compound')

def main():
    paths = [Path(x) for x in sys.argv[1:]]
    for p in paths:
        got = hashlib.sha256(p.read_bytes()).hexdigest()
        if got != EXPECTED_DEU:
            raise SystemExit(f'Wrong original deu.so reference: {p}; SHA256={got}')
        print(f'Original reference verified: {p} ({got})')
    files = [ROOT/'src/reloq_german_g2p.c',ROOT/'src/reloq_german_morphology.c',
             ROOT/'reference-apple/WORD-ANALYSIS-BEHAVIOR-DE.md']
    for f in files:
        if not f.is_file(): raise SystemExit(f'Missing {f}')
    inventory=(ROOT/'reference-apple/WORD-ANALYSIS-BEHAVIOR-DE.md').read_text(encoding='utf-8')
    for name in FUNCTIONS:
        if not re.search(r'\b'+name+r'\b',inventory):
            raise SystemExit(f'Missing function in research notes: {name}')
    print('Original analysis stage references present. C G2P still contains approximations.')
    print('This is a source/research consistency check, not a phonetic equivalence test.')

if __name__=='__main__': main()
