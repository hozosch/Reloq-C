"""Check short Persian /æ/ timing in several positions and protect long vowels."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from test_fa_cluster_final_timing import ms
from test_persian_r_after_i_contexts import at_fraction, capture
from test_us_articulation import voice


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    cases = (('`[.1tAk]', 1), ('`[.0tA.1ki]', 1), ('`[.1tA]', 1), ('`[.1Aks]', 0))
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 100):
                    for engine in (current, old):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    short = capture(current, scope, cases[0][0])
                    long = capture(current, scope, "`[.1t'A:'k]")
                    assert 0 < ms(short, 1) < ms(long, 1), (rate, gender, speed)
                    for tag, index in cases:
                        now = capture(current, scope, tag)
                        assert ms(now, index) > 0
                        if old:
                            before = capture(old, old_scope, tag)
                            assert ms(now, index) < ms(before, index), (tag, rate, gender, speed, ms(now, index), ms(before, index))
                            # The center of the short vowel keeps its formants;
                            # small timing-dependent interpolation is permitted.
                            for column in (9, 13, 15, 17):
                                assert abs(at_fraction(now, index, .5, column) - at_fraction(before, index, .5, column)) <= 100, (tag, column, rate, gender, speed)
                        checks += 1
                    if old:
                        # Explicit long A, including an unstressed initial
                        # long marker, and other vowels keep their old sound.
                        for tag in ("`[.1t'A:'k]", "`[.0t'A:'.1ki]", '`[.1te]', '`[.1to]'):
                            assert capture(current, scope, tag).pcm == capture(old, old_scope, tag).pcm, (tag, rate, gender, speed)
                        for tag, _ in cases:
                            assert capture(current, scope, tag, False).pcm == capture(old, old_scope, tag, False).pcm, (tag, rate, gender, speed, 'outside Persian scope')
    finally:
        current.close()
        if old:
            old.close()
    print(f'Passed {checks} short Persian A timing contexts and protected long vowels.')


if __name__ == '__main__':
    main()
