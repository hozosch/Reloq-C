"""Protect Persian R source transitions without moving its pitch or timing."""
from __future__ import annotations

import argparse
import ctypes
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
    tuning = engine._dll.eciPersianTuningScope
    tuning.argtypes, tuning.restype = (ctypes.c_int,), ctypes.c_int
    previous = tuning(1)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 100):
                    for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                        engine.set_parameter(key, value)
                    tuning(0)
                    raw = capture(engine, scope, '`[.0fo.1ri]')
                    tuning(1)
                    shaped = capture(engine, scope, '`[.0fo.1ri]')
                    assert raw.syllables == shaped.syllables
                    before, after = raw.frames(), shaped.frames()
                    assert len(before) == len(after)
                    start, end = (round(value / 10000) for value in shaped.syllables[2])
                    for edge in (start, end):
                        a = next(i for i, frame in enumerate(before) if frame.tick == edge)
                        b = a + 1
                        original_jump = abs(before[b].controls[2] - before[a].controls[2])
                        shaped_jump = abs(after[b].controls[2] - after[a].controls[2])
                        assert original_jump > 8 and shaped_jump <= 8, (rate, gender, speed, edge, original_jump, shaped_jump)
                    # Formants, timing and pitch belong to the existing
                    # tuning; this change only shapes voice/noise sources.
                    for old, new in zip(before, after):
                        assert old.tick == new.tick and old.controls[0] == new.controls[0] and old.controls[1] == new.controls[1]
                    checks += 1
        # At fast speech a short coda must not lose its retained R.
        for key, value in ((SAMPLE_RATE, 1), (GENDER, 0), (NATIVE_SPEED, 170)):
            engine.set_parameter(key, value)
        tuning(0); raw = capture(engine, scope, '`[.1ar]')
        tuning(1); shaped = capture(engine, scope, '`[.1ar]')
        assert raw.syllables == shaped.syllables
        for old, new in zip(raw.frames(), shaped.frames()):
            assert tuple(old.controls[k] for k in (2, 7, 8)) == tuple(new.controls[k] for k in (2, 7, 8))
    finally:
        tuning(previous)
        engine.close()
    print(f'Passed {checks} Persian R source onsets/releases and fast-R preservation.')


if __name__ == '__main__':
    main()
