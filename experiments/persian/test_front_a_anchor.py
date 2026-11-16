"""Check native contextual short-A and its targeted opening before Ach."""
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
    scope = (getattr(engine._dll, 'eciPersianAcousticScope', None) or
             engine._dll.reloq_fa_acoustic_scope)
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
    parser.add_argument('--baseline', type=Path,
                        help='previous contextual Persian probe before fixed alef')
    args = parser.parse_args()
    ach = ('`[.1bAG]', '`[.1sAGt]', '`[.1bAd.0bAGt]',
           '`[.1GAt]', '`[.1GA.0te]', '`[.1zir.0GAt]', '`[.1GAG]')
    before_ach = set(ach[:3])
    unchanged = ('`[.1A.0lef]', "`[.1s'A:'d]", '`[.1sAd]', '`[.0sA.1lam]',
                 '`[.1bAt]', "`[.1'i:'.0r'a:'n]", '`[.1Gu]')
    checks = 0
    for gender in (0, 1):
        for rate in (3, 6):
            current, scope = voice(args.probe, gender, rate)
            baseline, old_scope = voice(args.baseline, gender, rate) if args.baseline else (None, None)
            try:
                for tag in ach:
                    sample = capture(current, scope, tag)
                    rows = sample.frames()
                    # The earlier opening is retained before Ach, but is not
                    # applied to a vowel immediately after it.
                    opened = [r.controls[9] for r in rows if r.controls[2] > 0 and r.controls[8] == 0]
                    threshold = (770 if gender else 700) if tag in before_ach else (660 if gender else 600)
                    assert sum(value >= threshold for value in opened) >= (3 if 'zir' in tag else 15), (gender, rate, tag)
                    if baseline:
                        old = capture(baseline, old_scope, tag)
                        previous = old.frames()
                        prior = [r.controls[9] for r in previous if r.controls[2] > 0 and r.controls[8] == 0]
                        # Duration differences can change the number of frames;
                        # compare the vowel targets rather than aligned times.
                        delta = max(opened) - max(prior)
                        if tag in before_ach:
                            assert abs(delta) < 2, (tag, delta)
                        elif tag == '`[.1GAG]':
                            # Both surrounding fricatives can obscure the
                            # exact F1 peak of this very short vowel.
                            assert -220 < delta < -60, (tag, delta)
                        else:
                            expected = -125 if gender else -100
                            assert abs(delta - expected) < 3, (tag, delta)
                    checks += 1
                if baseline:
                    for tag in unchanged:
                        # Other vowels may receive a bounded duration change.
                        now = capture(current, scope, tag).frames()
                        old = capture(baseline, old_scope, tag).frames()
                        assert abs(max(r.controls[9] for r in now) - max(r.controls[9] for r in old)) < 2, tag
                    # The scoped adjustment must never affect ordinary Finnish.
                    for tag in ach:
                        assert capture(current, scope, tag, False).pcm == capture(baseline, old_scope, tag, False).pcm
            finally:
                current.close()
                if baseline:
                    baseline.close()
    print(f'Passed {checks} contextual short-A/Ach checks, voice/rate and isolation checks.')


if __name__ == '__main__':
    main()
