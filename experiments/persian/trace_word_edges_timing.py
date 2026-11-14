"""Inspect word boundaries and Finnish nucleus timing in the private probe."""
from __future__ import annotations
import argparse
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES
from listening import render
from salam_samples import (SALAM, QALAM, PERSIAN_PHRASE, IRAN_FINAL_STRESS,
                           IRAN_FINAL_STRESS_LONG_A, DONYA_FINAL_STRESS_LONG_A)
from test_native_probe import IRAN
from test_selected_probe import KEYS, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x90000,
                             encoding='utf8', native_voice=True)
    try:
        engine.set_parameter(SAMPLE_RATE, 3)
        engine.set_parameter(PROCESS_TAGS, 1)
        set_probe(a=1, q=5, h=5, l=2, rise=2)
        os.environ['RELOQ_FA_TRACE_L_EDGES'] = '1'
        os.environ['RELOQ_FA_TRACE_NUC'] = '1'
        cases = (
            ('medial L, explicit Salam', SALAM),
            ('medial L, explicit Qalam', QALAM),
            ('final L, explicit', '`[.1Al]'),
            ('final L, explicit then another tag', '`[.1Al] `[.1sa]'),
            ('L before T in the same tag', '`[.1Al.0ta]'),
            ('medial L, Finnish spelling', 'sala'),
            ('final L, Finnish spelling', 'sal'),
            ('final L before following word', 'sal sana'),
            ('L before T inside word', 'salta'),
            ('final L before punctuation', 'sal?'),
            ('Iran, previous', IRAN),
            ('Iran, final stress and Finnish short-A', IRAN_FINAL_STRESS),
            ('Iran, final stress and Finnish long-A', IRAN_FINAL_STRESS_LONG_A),
            ('Donyā, final stress and Finnish long-A', DONYA_FINAL_STRESS_LONG_A),
            ('Donyā, first stress', '`[.1do.0nja]'),
            ('Donyā, final stress', '`[.0do.1nja]'),
            ('previous sentence', PERSIAN_PHRASE),
            ('final stress sentence', SALAM + ' `[.0do.1nja]'),
        )
        for label, text in cases:
            print(f'\nBEGIN {label}: {text}', flush=True)
            pcm = render(engine, text)
            print(f'END {label}: whole utterance {len(pcm)*500/SAMPLE_RATES[3]:.1f} ms', flush=True)
    finally:
        for key in (*KEYS, 'RELOQ_FA_TRACE_L_EDGES', 'RELOQ_FA_TRACE_NUC'):
            os.environ.pop(key, None)
        engine.close()


if __name__ == '__main__':
    main()
