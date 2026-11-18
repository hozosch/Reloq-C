"""Check the restored R and the vowel-local head-size experiment."""
from __future__ import annotations

import argparse
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def nucleus(rows):
    voiced = [r for r in rows if r.controls[2] >= 45 and r.controls[7] < 40 and r.controls[8] < 1]
    assert len(voiced) >= 10
    return voiced[len(voiced) // 2].controls


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='engine before the head-size and restored-R trial')
    args = parser.parse_args()
    current, scope = voice(args.candidate, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        for engine in (current, previous):
            if engine:
                for key, value in ((GENDER, 0), (SAMPLE_RATE, 3), (NATIVE_SPEED, 65)):
                    engine.set_parameter(key, value)
        # Both familiar NVDA labels use the earlier R instead of the rejected
        # after-I F3 boost. The E and its upper resonance are still unchanged.
        for tag, index, lower, upper in (("`[.0ti.1r'e:']", 100, 2140, 2200),
                                          ('`[.1zir.0GAt]', 175, 2135, 2200)):
            _, frames = capture(current, scope, tag, True)
            assert lower <= frames[index].controls[15] <= upper, (tag, 'restored R F3')
            if previous:
                _, old = capture(previous, old_scope, tag, True)
                assert old[index].controls[15] - frames[index].controls[15] >= 90, tag
                assert abs(old[index].controls[17] - frames[index].controls[17]) < 3, tag
            checks += 1
        for gender in (0, 1):
            for rate in (1, 3, 6):
                for speed in (40, 98, 170):
                    for engine in (current, previous):
                        if engine:
                            for key, value in ((GENDER, gender), (SAMPLE_RATE, rate), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    e_pcm, e_rows = capture(current, scope, '`[.1e]', True)
                    e = nucleus(e_rows)
                    assert abs(e[9] - (495 if gender else 450)) < 2
                    assert abs(e[13] - (2359 if gender else 1950)) < 3
                    if previous:
                        old_e, _ = capture(previous, old_scope, '`[.1e]', True)
                        assert e_pcm.pcm == old_e.pcm, 'unchanged E sounded different'
                    for vowel in 'aAiou':
                        pcm, rows = capture(current, scope, '`[.1' + vowel + ']', True)
                        if previous:
                            old_pcm, old_rows = capture(previous, old_scope, '`[.1' + vowel + ']', True)
                            assert pcm.pcm != old_pcm.pcm and len(rows) == len(old_rows), vowel
                            # F2 moves by three percent; F0, amplitude, noise,
                            # frame timing and bandwidths are unaffected.
                            new, old = nucleus(rows), nucleus(old_rows)
                            assert abs(new[13] / old[13] - 1.03) < .005, (gender, vowel, new[13], old[13])
                            for a, b in zip(rows, old_rows):
                                assert a.tick == b.tick
                                for col in (1, 2, 7, 8, 10, 14, 16, 18, 20):
                                    assert abs(a.controls[col] - b.controls[col]) < 1, (gender, vowel, col)
                        checks += 1
                    checks += 1
    finally:
        current.close()
        if previous:
            previous.close()
    print(f'Passed {checks} restored-R and vowel-local head-size checks.')


if __name__ == '__main__':
    main()
