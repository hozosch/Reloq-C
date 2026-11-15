"""Check intermediate E/A at word ends without altering internal vowels."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from listening import render
from salam_samples import (DONYA_ESPEAK, IRAN_PREFERRED, NO_TE_LONG, NO_TE_SHORT,
                           SALAM, TA_LONG, TA_SHORT)
from test_selected_probe import KEYS, set_probe
from test_vowel_timing import capture


def nucleus(engine: DirectEciEngine, text: str, phone: int, final: int) -> int:
    _, rows = capture(engine, text, 1, final=final)
    values = [row[5] for row in rows if row[0] == phone]
    assert values, ('missing target nucleus', text, rows)
    return values[0]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x90000,
                             encoding='utf8', native_voice=True)
    checks = 0
    try:
        engine.set_parameter(PROCESS_TAGS, 1)
        engine.set_parameter(SAMPLE_RATE, 3)
        pairs = ((NO_TE_SHORT, NO_TE_LONG, 29),
                 (TA_SHORT, TA_LONG, 34),
                 (DONYA_ESPEAK, "`[.1do.0nj'a:']", 34))
        for gender in (0, 1):
            for speed in (40, 50, 98, 200):
                engine.set_parameter(GENDER, gender)
                engine.set_parameter(NATIVE_SPEED, speed)
                for short, long, phone in pairs:
                    assert short in engine._phonemes_for_probe(short) and long in engine._phonemes_for_probe(long)
                    low = nucleus(engine, short, phone, 0)
                    high = nucleus(engine, long, phone, 0)
                    target = (low + high) / 2
                    assert low < high, ('unadjusted timing order', short, long)
                    for text in (short, long):
                        actual = nucleus(engine, text, phone, 1)
                        # The two underlying Finnish quantity paths apply
                        # context differently, so this is a close rather than
                        # a claimed exact midpoint for every possible word.
                        assert low < actual < high and abs(actual - target) <= target * .035, (
                            'not near the midpoint', gender, speed, text, low, high, actual)
                        checks += 1
                for text in (SALAM, IRAN_PREFERRED, '`[.1sefr]', "`[.1s'e:'fr]"):
                    assert text in engine._phonemes_for_probe(text), ('invalid control', text)
                    set_probe(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1, s=1)
                    ordinary = render(engine, text)
                    set_probe(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1, s=1, final=1)
                    assert render(engine, text) == ordinary, ('internal vowel changed', gender, speed, text)
                    checks += 1
        engine.set_parameter(GENDER, 0)
        engine.set_parameter(NATIVE_SPEED, 50)
        for suffix in ('.', '?', ' `[.1si]'):
            for text, phone in ((TA_LONG, 34), (NO_TE_LONG, 29)):
                actual = nucleus(engine, text + suffix, phone, 1)
                ordinary = nucleus(engine, text + suffix, phone, 0)
                assert actual < ordinary, ('word-end detection failed', text, suffix, actual, ordinary)
                checks += 1
        for dialect in (0x10000, 0x40000, 0x30000, 0x130000):
            other = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                other.set_parameter(PROCESS_TAGS, 1)
                other.set_parameter(SAMPLE_RATE, 3)
                set_probe(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1, s=1)
                before = render(other, 'Salam?')
                set_probe(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1, s=1, final=1)
                assert render(other, 'Salam?') == before, ('other language changed', dialect)
                checks += 1
            finally:
                other.close()
        print(f'Passed {checks} final-vowel, word-boundary and isolation checks.')
    finally:
        engine.close()
        for key in (*KEYS, 'RELOQ_FA_TRACE_NUC'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
