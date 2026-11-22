"""Protect Persian R source transitions without moving its pitch or timing."""
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
    args = parser.parse_args()
    engine, scope = voice(args.library, 0x90000)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 100):
                    for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                        engine.set_parameter(key, value)
                    shaped = capture(engine, scope, '`[.0fo.1ri]')
                    after = shaped.frames()
                    start, end = (round(value / 10000) for value in shaped.syllables[2])
                    for edge in (start, end):
                        a = next(i for i, frame in enumerate(after) if frame.tick == edge)
                        b = a + 1
                        shaped_jump = abs(after[b].controls[2] - after[a].controls[2])
                        assert shaped_jump <= 8, (rate, gender, speed, edge, shaped_jump)
                    # The beginning and release remain distinct: smoothing
                    # must not flatten the R into its neighbouring vowels.
                    r = [row.controls[2] for row in after if start <= row.tick <= end]
                    assert r and min(r) < max(r), (rate, gender, speed, r)
                    checks += 1
        # At fast speech a short coda must not lose its retained R.
        for key, value in ((SAMPLE_RATE, 1), (GENDER, 0), (NATIVE_SPEED, 170)):
            engine.set_parameter(key, value)
        shaped = capture(engine, scope, '`[.1ar]')
        r = [row for row in shaped.frames() if row.controls[8] >= 40 and row.controls[37] >= 60 and
             35 <= row.controls[38] < 55 and 15 <= row.controls[2] <= 30]
        assert len(r) >= 5
    finally:
        engine.close()
    print(f'Passed {checks} Persian R source onsets/releases and fast-R preservation.')


if __name__ == '__main__':
    main()
