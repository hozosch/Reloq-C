"""Check the saved O-to-R resonance release without changing the R source."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE
from test_persian_r_after_i_contexts import capture, middle
from test_us_articulation import voice


def release_slope(result):
    start = result.syllables[1][1] / 10000
    end = result.syllables[2][0] / 10000
    rows = [row for row in result.frames() if start <= row.tick <= end]
    return max((abs(b.controls[17] - a.controls[17]) / (b.tick - a.tick)
                for a, b in zip(rows, rows[1:]) if b.tick > a.tick), default=0)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender, head in ((0, 50), (1, 50), (1, 20)):
                for speed in (40, 65, 82, 100, 170):
                    for engine in (current, previous):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                               (HEAD_SIZE, head), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    tag = '`[.0fo.1ri]'  # فوری
                    actual = capture(current, scope, tag)
                    if (rate, gender, head, speed) == (3, 0, 50, 82):
                        assert release_slope(actual) < 60, release_slope(actual)
                    if previous:
                        old = capture(previous, old_scope, tag)
                        assert actual.syllables == old.syllables
                        a, b = actual.frames(), old.frames()
                        assert len(a) == len(b)
                        for row, before in zip(a, b):
                            assert row.tick == before.tick
                            for column in range(62):
                                if column not in (15, 17):
                                    assert row.controls[column] == before.controls[column], (
                                        rate, gender, head, speed, row.tick, column)
                        begin, end = actual.syllables[1]
                        if end - begin >= 80000:
                            assert middle(actual, 1, (9, 13, 15, 17)) == middle(old, 1, (9, 13, 15, 17))
                        if (rate, gender, head, speed) == (3, 0, 50, 82):
                            assert release_slope(old) > 120
                            assert actual.pcm != old.pcm and len(actual.pcm) == len(old.pcm)
                        # The rule must not affect an O followed by another
                        # consonant, or ordinary Finnish when scope is off.
                        assert capture(current, scope, '`[.0fo.1di]').pcm == \
                               capture(previous, old_scope, '`[.0fo.1di]').pcm
                        assert capture(current, scope, tag, False).pcm == \
                               capture(previous, old_scope, tag, False).pcm
                    checks += 1
    finally:
        current.close()
        if previous:
            previous.close()
    print(f'Passed {checks} O-to-R transitions with unchanged vowel body and voice source.')


if __name__ == '__main__':
    main()
