"""Keep the accepted final U-R while easing the voiced R-to-I transition."""
from __future__ import annotations

import argparse
from pathlib import Path

from nvda_driver_probe import make_language_engine
from test_persian_r_voice_resonance import components
from test_persian_u_r_upper_voice import upper_power

from _reloq_eci_direct import EciIndex, NATIVE_SPEED, SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    cases = (('`[.0zo.0hu.1ri]', 3), ('`[.0fo.1ri]', 2),
             ('`[.0Go.1ri]', 2), ('`[.0ja.0vA.1ri]', 4),
             ('`[.0se.1ri]', 2))
    unchanged = (('`[.0zo.1hur]', 3), ('`[.0ja.1vAr]', 4),
                 ('`[.0pe.1sAr]', 4), ('`[.1ruz]', 0), ('`[.0fu.1ra]', 2))
    checks = 0
    for rate in (1, 3, 6):
        for gender in (0, 1):
            for clock in (0, 1):
                for tag, r in cases:
                    old, before, old_r = components(args.previous, tag, r, gender, clock, rate)
                    new, after, new_r = components(args.library, tag, r, gender, clock, rate)
                    assert old.syllables == new.syllables and old_r == new_r
                    assert before[0] == after[0], (tag, rate, gender, clock, 'noise changed')
                    for a, b in zip(old.frames(), new.frames()):
                        assert a.tick == b.tick
                        assert all(a.controls[c] == b.controls[c] for c in range(62) if c not in (11, 12)), (
                            tag, rate, gender, clock, a.tick, 'acoustic target changed')
                    assert before[1] != after[1], (tag, rate, gender, clock, 'no voice change')
                    if rate == 3:
                        start = old.syllables[r][0] / 10000
                        end = old.syllables[r+1][0] / 10000
                        lo, hi = start + 8, min(end - 5, start + 40)
                        assert upper_power(after[1], lo, hi) < upper_power(before[1], lo, hi) * .94, (
                            tag, gender, clock, 'R upper voice')
                        lo, hi = end + 3, end + 18
                        assert upper_power(after[1], lo, hi) < upper_power(before[1], lo, hi) * .90, (
                            tag, gender, clock, 'I onset upper voice')
                        # The later vowel is not under the new correction.
                        for frame in new.frames():
                            if frame.tick >= end + 30:
                                assert not 16419 <= frame.controls[11] < 16420, (tag, frame.tick)
                        # The U before R, when present, keeps its accepted PCM.
                        if tag.startswith('`[.0zo.'):
                            limit = round((start - 3) * 32)
                            assert before[1][:limit] == after[1][:limit], (tag, 'preceding U changed')
                    checks += 1
                for tag, r in unchanged:
                    old, before, _ = components(args.previous, tag, r, gender, clock, rate)
                    new, after, _ = components(args.library, tag, r, gender, clock, rate)
                    assert old.syllables == new.syllables and before == after, (tag, rate, gender, clock)
    for native in (False, True):
        owners = [make_language_engine(path.resolve(), path.resolve(), None,
                                       Path('/unavailable'), Path('/unavailable'),
                                       native_sentences=native)
                  for path in (args.previous, args.library)]
        try:
            for owner in owners:
                owner.set_parameter(SAMPLE_RATE, 3)
                owner.set_parameter(NATIVE_SPEED, 65)
            for text, change in (('ظهور', False), ('ظهور ناگهانی', False),
                                 ('ظهوری', True), ('فوری', True), ('خوری', True),
                                 ('یاوری', True), ('گلرهای', False),
                                 ('یاور', False), ('پسر', False),
                                 ('ظهوری heading level 4', True),
                                 ('فوری heading level 4', True),
                                 ('ظهور heading level 4', False),
                                 ('پنجشنبه 5 آذر heading level 4', False)):
                results = []
                for owner in owners:
                    pcm, marks = bytearray(), []
                    assert owner.speak((EciIndex(11), text, EciIndex(12)), pcm.extend,
                                       lambda value: marks.append(value))
                    results.append((bytes(pcm), marks))
                assert results[0][1] == results[1][1] == [11, 12], (text, native)
                assert len(results[0][0]) == len(results[1][0]), (text, native)
                assert (results[0][0] != results[1][0]) == change, (text, native)
        finally:
            for owner in owners:
                owner.close()
    print(f'Passed {checks} R-before-I noise, resonance and timing cases plus unchanged final R.')


if __name__ == '__main__':
    main()
