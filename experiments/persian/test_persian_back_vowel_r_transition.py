"""Check back-vowel/R transitions while preserving the accepted R source."""
from __future__ import annotations
import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from test_persian_r_after_i_contexts import capture
from test_us_articulation import voice

CASES = ('`[.0zo.1hur]', '`[.0fo.1ri]', '`[.1ur] `[.1di]')
UNAFFECTED = ('`[.0fo.1di]', '`[.1ir]', '`[.1i.0ro]')


def max_slope(result, left, right, column):
    start, end = result.syllables[left][1] / 10000, result.syllables[right][0] / 10000
    rows = [r for r in result.frames() if start <= r.tick <= end]
    return max((abs(b.controls[column] - a.controls[column]) / (b.tick - a.tick)
                for a, b in zip(rows, rows[1:]) if b.tick > a.tick), default=0)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (65, 82, 100):
                    for clock in (0, 1):
                        for engine in (current, old):
                            if engine:
                                for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                                   (NATIVE_SPEED, speed), (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                                    engine.set_parameter(key, value)
                        for tag in CASES:
                            new = capture(current, scope, tag)
                            if (rate, gender, speed, clock) == (3, 0, 82, 0):
                                if tag == CASES[0]:
                                    # Do not trade a softened onset for an
                                    # abrupt catch-up at the start of R.
                                    assert max_slope(new, 2, 3, 13) <= 60
                                    assert max_slope(new, 2, 3, 17) <= 80
                                elif tag == CASES[1]:
                                    assert max_slope(new, 1, 2, 13) <= 15
                                    assert max_slope(new, 2, 3, 13) <= 12.1
                            if old:
                                before = capture(old, old_scope, tag)
                                assert new.syllables == before.syllables and len(new.pcm) == len(before.pcm)
                                r_index = (4, 2, 1)[CASES.index(tag)]
                                r_start, r_end = (value / 10000 for value in new.syllables[r_index])
                                for x, y in zip(new.frames(), before.frames()):
                                    assert x.tick == y.tick
                                    for col in range(62):
                                        if col not in (13, 17) or r_start <= x.tick < r_end:
                                            assert x.controls[col] == y.controls[col], (tag, rate, gender, speed, clock, x.tick, col)
                                assert capture(current, scope, tag, False).pcm == capture(old, old_scope, tag, False).pcm
                            checks += 1
                        if old:
                            for tag in UNAFFECTED:
                                assert capture(current, scope, tag).pcm == capture(old, old_scope, tag).pcm, (tag, rate, gender, speed, clock)
    finally:
        current.close()
        if old:
            old.close()
    print(f'Passed {checks} back-vowel/R transitions and isolation cases.')


if __name__ == '__main__':
    main()
