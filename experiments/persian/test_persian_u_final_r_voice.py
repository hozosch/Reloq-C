"""Exercise the voiced-only adjustment to Persian final R after U."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from nvda_driver_probe import make_language_engine
from test_persian_r_after_i_contexts import capture
from test_us_articulation import voice


def render(owner, text):
    pcm, marks = bytearray(), []
    assert owner.speak((EciIndex(11), text, EciIndex(12)), pcm.extend,
                       lambda value: marks.append((value, len(pcm))))
    assert pcm and marks == [(11, 0), (12, len(pcm))], (text, marks)
    return bytes(pcm), marks


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--library', required=True, type=Path)
    ap.add_argument('--previous', type=Path)
    args = ap.parse_args()
    current, scope = voice(args.library, 0x90000)
    prior, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    changed = (('`[.0zo.1hur]', 3), ('`[.1ur]', 1), ('`[.1ur] `[.1di]', 1))
    unchanged = ('`[.0fo.1ri]', '`[.1AzAr]', '`[.1or]', '`[.1ar]',
                 '`[.1sefr]', '`[.1ru]', '`[.1ur] `[.1A]')
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 100, 170):
                    for clock in (0, 1):
                        for v in (current, prior):
                            if v:
                                for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                                   (NATIVE_SPEED, speed), (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                                    v.set_parameter(key, value)
                        for tag, r_index in changed:
                            result = capture(current, scope, tag)
                            start = result.syllables[r_index][0] / 10000
                            end = result.syllables[r_index + 1][0] / 10000
                            rows = result.frames()
                            assert any(start <= row.tick < end and row.controls[4] > 0 for row in rows), (
                                rate, gender, speed, clock, tag, 'voiced R profile missing')
                            if clock:
                                for a, b in zip(rows, rows[1:]):
                                    if a.controls[4] != b.controls[4]:
                                        assert b.tick % 5 == 0, (tag, b.tick, 'original clock')
                            if prior:
                                before = capture(prior, old_scope, tag)
                                old = before.frames()
                                assert result.syllables == before.syllables and len(result.pcm) == len(before.pcm)
                                assert len(rows) == len(old)
                                affected = False
                                for a, b in zip(rows, old):
                                    assert a.tick == b.tick
                                    for col in range(62):
                                        if col != 4:
                                            assert a.controls[col] == b.controls[col], (tag, a.tick, col)
                                    delta = a.controls[4] - b.controls[4]
                                    assert 0 <= delta <= 3, (tag, a.tick, delta)
                                    if delta:
                                        assert start <= a.tick < end + (5 if clock else 0), (tag, a.tick)
                                        affected = True
                                assert affected, (rate, gender, speed, clock, tag, 'voiced R profile unchanged')
                                # At the fastest speed the entire R may be
                                # shorter than one glottal period. A source
                                # filter must not create an extra pulse.
                                a, b = result.syllables[r_index]
                                if b - a >= 150000:
                                    assert result.pcm != before.pcm, (rate, gender, speed, clock, tag, 'no voiced change')
                            checks += 1
                        if prior:
                            for tag in unchanged:
                                a, b = capture(current, scope, tag), capture(prior, old_scope, tag)
                                assert a.pcm == b.pcm and a.frame_bytes == b.frame_bytes, (
                                    rate, gender, speed, clock, tag)
                                checks += 1
        owners = [make_language_engine(path.resolve(), path.resolve(), None, Path('/unavailable'), Path('/unavailable'))
                  for path in ((args.previous, args.library) if args.previous else (args.library,))]
        try:
            for text, should_change in (('ظهور ناگهانی', True), ('فوری', False),
                                        ('دوشنبه 9 آذر heading level 4', False)):
                output = [render(owner, text) for owner in owners]
                if prior:
                    assert len(output[0][0]) == len(output[1][0]) and output[0][1] == output[1][1]
                    assert (output[0][0] != output[1][0]) == should_change, text
                checks += 1
        finally:
            for owner in owners:
                owner.close()
    finally:
        current.close()
        if prior:
            prior.close()
    print(f'Passed {checks} voiced U-final-R, timing, clock and unchanged-context checks.')


if __name__ == '__main__':
    main()
