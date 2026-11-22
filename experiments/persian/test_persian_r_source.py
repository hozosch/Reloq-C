"""Preserve the established Persian R source and its timing."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from test_persian_r_after_i_contexts import capture
from test_us_articulation import voice


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='optional pre-source-change reference')
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        # This is a measured, previously accepted medial R source. The failed
        # new slew limiter changed these values to 44/26 at its onset and
        # 13/33 at the following source step.
        for key, value in ((SAMPLE_RATE, 3), (GENDER, 0), (NATIVE_SPEED, 82)):
            current.set_parameter(key, value)
        medial = capture(current, scope, '`[.0fo.1ri]')
        frames = {row.tick: row.controls for row in medial.frames()}
        assert tuple(frames[97][j] for j in (2, 7, 8)) == (5, 25, 40)
        assert tuple(frames[107][j] for j in (2, 7, 8)) == (40, 45, 60)
        assert tuple(frames[118][j] for j in (2, 7, 8)) == (56, 34, 50)
        final = capture(current, scope, '`[.0zo.1hur]')
        frames = {row.tick: row.controls for row in final.frames()}
        assert tuple(frames[227][j] for j in (2, 7, 8)) == (15, 15, 0)
        assert tuple(frames[243][j] for j in (2, 7, 8)) == (25, 20, 55)
        checks += 2
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 100, 170):
                    for engine in (current, previous):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    for tag in ('`[.0fo.1ri]', '`[.0zo.1hur]', '`[.1ir]', '`[.1ur] `[.1di]'):
                        actual = capture(current, scope, tag)
                        if previous:
                            baseline = capture(previous, old_scope, tag)
                            assert actual.syllables == baseline.syllables, (rate, gender, speed, tag, 'timing')
                            a, b = actual.frames(), baseline.frames()
                            assert len(a) == len(b), (rate, gender, speed, tag, 'frames')
                            for x, y in zip(a, b):
                                assert x.tick == y.tick
                                for column in (0, 1, 2, 7, 8, 35, 36, 37, 38, 39, 40, 41, 42):
                                    assert x.controls[column] == y.controls[column], (
                                        rate, gender, speed, tag, x.tick, column)
                        checks += 1
    finally:
        current.close()
        if previous:
            previous.close()
    print(f'Passed {checks} Persian R source and timing preservation cases.')


if __name__ == '__main__':
    main()
