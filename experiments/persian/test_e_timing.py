"""Verify the eSpeak-informed E starting duration in the private probe."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from listening import render
from test_selected_probe import KEYS, set_probe
from test_vowel_timing import capture


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    pairs = (
        ('`[.1pA]', '`[.1pe]'),
        ('`[.0sA.1lam]', '`[.0se.1lam]'),
        ('`[.1tA.0ma]', '`[.1te.0ma]'),
        ("`[.1p'A:']", "`[.1p'e:']"),
        ("`[.0s'A:'.1lam]", "`[.0s'e:'.1lam]"),
    )
    count = 0
    try:
        previous = DirectEciEngine(args.previous.resolve(), dialect=0x90000,
                                   encoding='utf8', native_voice=True)
        probe = DirectEciEngine(args.probe.resolve(), dialect=0x90000,
                                encoding='utf8', native_voice=True)
        try:
            for engine in (previous, probe):
                engine.set_parameter(PROCESS_TAGS, 1)
                engine.set_parameter(SAMPLE_RATE, 3)
            for gender in (0, 1):
                for speed in (40, 98, 200):
                    for engine in (previous, probe):
                        engine.set_parameter(GENDER, gender)
                        engine.set_parameter(NATIVE_SPEED, speed)
                    for a, e in pairs:
                        assert a in probe._phonemes_for_probe(a) and e in probe._phonemes_for_probe(e)
                        set_probe(a=1, h=5, l=4, rise=2)
                        assert render(previous, e) == render(probe, e), ('disabled E changed', gender, speed, e)
                        _, a_rows = capture(probe, a, 1)
                        _, e_rows = capture(probe, e, 1)
                        a_time = next(row[5] for row in a_rows if row[0] == 31)
                        e_time = next(row[5] for row in e_rows if row[0] == 29)
                        # Identical eSpeak base values do not cancel all
                        # contextual effects; compare the actual nuclei.
                        assert abs(e_time - a_time) <= max(12000, a_time // 50), (
                            'E / front-A duration', gender, speed, a, e, a_time, e_time)
                        count += 2
                    # In the simple matched context, all three vowels have
                    # the same Finnish starting value and /A/ gets eSpeak's
                    # 200:160 multiplier. Other contexts can adjust further.
                    _, back_rows = capture(probe, '`[.1pa]', 1)
                    _, front_rows = capture(probe, '`[.1pA]', 1)
                    back_time = next(row[5] for row in back_rows if row[0] == 34)
                    front_time = next(row[5] for row in front_rows if row[0] == 31)
                    assert abs(back_time * 4 - front_time * 5) <= 8, (
                        'back-A / front-A base ratio', gender, speed, back_time, front_time)
                    count += 1
        finally:
            previous.close()
            probe.close()
        for dialect in (0x10000, 0x40000, 0x30000, 0x130000):
            old = DirectEciEngine(args.previous.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            new = DirectEciEngine(args.probe.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                for engine in (old, new):
                    engine.set_parameter(PROCESS_TAGS, 1)
                    engine.set_parameter(SAMPLE_RATE, 3)
                set_probe(a=1, q=5, h=5, l=4, vowel=1, rise=2)
                assert render(old, 'Salam?') == render(new, 'Salam?'), ('language isolation', dialect)
                count += 1
            finally:
                old.close()
                new.close()
        print(f'Passed {count} E timing, stress, speed and language-isolation checks.')
    finally:
        for key in (*KEYS, 'RELOQ_FA_TRACE_NUC'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
