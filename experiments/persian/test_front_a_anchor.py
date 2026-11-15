"""Check the Persian short-A nucleus against the existing alef reference."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, PAUSE_MODE, PROCESS_TAGS, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture


def voice(path, gender, rate):
    engine = DirectEciEngine(path.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    engine.set_parameter(PROCESS_TAGS, 1)
    engine.set_parameter(PAUSE_MODE, 1)
    engine.set_parameter(GENDER, gender)
    engine.set_parameter(SAMPLE_RATE, rate)
    scope = engine._dll.reloq_fa_acoustic_scope
    scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
    return engine, scope


def capture(engine, scope, tag, enabled=True):
    previous = scope(int(enabled))
    try:
        result = FrameCapture(engine._dll).capture(engine, (tag,))
        assert result and result.pcm and result.frames()
        return result
    finally:
        assert scope(previous) == int(enabled)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', required=True, type=Path)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    checks = 0
    for gender in (0, 1):
        for rate in (3, 6):
            current, scope = voice(args.probe, gender, rate)
            baseline, old_scope = voice(args.baseline, gender, rate) if args.baseline else (None, None)
            try:
                alef = capture(current, scope, '`[.1A.0lef]')
                rows = alef.frames()
                first = next(row for row in rows if row.controls[2] > 0)
                columns = (9, 15, 17, 19, 21, 23, 25, 10, 14, 16, 18, 20, 22, 24, 26)
                target = tuple(first.controls[i] for i in columns)
                def anchored(row):
                    return (row.controls[2] > 0 and
                            all(abs(row.controls[i] - ref) < .1 for i, ref in zip(columns, target)))
                reference = [row.controls[13] for row in rows if anchored(row)]
                assert len(reference) >= 20
                lower, upper = min(reference) - 1, max(reference) + 1
                for tag in ("`[.1s'A:'d]", '`[.1sAd]', '`[.0sA.1lam]', '`[.1bAd.0bAGt]'):
                    sample = capture(current, scope, tag)
                    target_rows = [row for row in sample.frames() if anchored(row)]
                    assert len(target_rows) >= 10, (gender, rate, tag)
                    assert all(lower <= row.controls[13] <= upper for row in target_rows), (gender, rate, tag)
                    if baseline:
                        previous = capture(baseline, old_scope, tag)
                        assert len(sample.pcm) == len(previous.pcm), (gender, rate, tag)
                        assert [row.controls[1] for row in sample.frames()] == [
                            row.controls[1] for row in previous.frames()], (gender, rate, tag)
                    checks += 1
                if baseline:
                    # The reference letter itself and the accepted back
                    # vowel keep their sound, for both native voice types.
                    assert alef.pcm == capture(baseline, old_scope, '`[.1A.0lef]').pcm
                    for tag in ('`[.1Gu]', "`[.1'i:'.0r'a:'n]"):
                        assert capture(current, scope, tag).pcm == capture(baseline, old_scope, tag).pcm
                    # The Finnish voice outside the scoped Persian profile
                    # must not acquire the Persian short-vowel target.
                    tag = '`[.1sAd]'
                    assert capture(current, scope, tag, False).pcm == capture(baseline, old_scope, tag, False).pcm
            finally:
                current.close()
                if baseline:
                    baseline.close()
    print(f'Passed {checks} short-A targets, voice/rate and isolation checks.')


if __name__ == '__main__':
    main()
