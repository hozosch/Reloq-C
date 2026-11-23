"""Protect R noise while smoothing U and upper R resonances in context."""
from __future__ import annotations

import argparse
import array
import math
from pathlib import Path
import sys

from test_persian_r_voice_resonance import components
from nvda_driver_probe import make_language_engine

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import EciIndex, NATIVE_SPEED, SAMPLE_RATE


def upper_power(pcm, start, end):
    samples = array.array('h'); samples.frombytes(pcm)
    if sys.byteorder != 'little':
        samples.byteswap()
    segment = samples[round(start*16):round(end*16)]
    assert len(segment) > 80, (start, end)
    windowed = [sample * (0.5 - 0.5 * math.cos(math.tau*i/(len(segment)-1)))
                for i, sample in enumerate(segment)]
    power = 0.0
    for hz in range(3000, 5001, 100):
        angle = math.tau * hz / 16000
        sine, cosine = math.sin(angle), math.cos(angle)
        re = im = 0.0; x, y = 1.0, 0.0
        for sample in windowed:
            re += sample*x; im += sample*y
            x, y = x*cosine-y*sine, x*sine+y*cosine
        power += re*re + im*im
    return power


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    cases = (('`[.0zo.1hur]', 3, 2), ('`[.0zo.0hu.1ri]', 3, 2),
             ('`[.0fo.1ri]', 2, None), ('`[.0Go.1ri]', 2, None),
             ('`[.0go.0ler.1ha.0je]', 4, None))
    checks = 0
    for rate in (1, 3, 6):
        for gender in (0, 1):
            for clock in (0, 1):
                for tag, r, u in cases:
                    old, before, old_r = components(args.previous, tag, r, gender, clock, rate)
                    new, after, new_r = components(args.library, tag, r, gender, clock, rate)
                    assert old.syllables == new.syllables and old_r == new_r
                    assert before[0] == after[0], (tag, rate, gender, clock, 'noise changed')
                    for a, b in zip(old.frames(), new.frames()):
                        assert a.tick == b.tick
                        assert all(a.controls[c] == b.controls[c] for c in range(62) if c not in (11, 12)), (
                            tag, rate, gender, clock, a.tick, 'source or formants changed')
                    if rate == 3:
                        start = old.syllables[r][0] / 10000
                        end = old.syllables[r+1][0] / 10000
                        lo = start + 8; hi = min(end-5, start+40)
                        old_power = upper_power(before[1], lo, hi)
                        new_power = upper_power(after[1], lo, hi)
                        assert old_power > 0 and new_power < old_power * .96, (
                            tag, gender, clock, 'R upper band', old_power, new_power)
                        if u is not None:
                            begin, finish = (x/10000 for x in old.syllables[u])
                            lo = begin + (finish-begin)*.3
                            hi = begin + (finish-begin)*.8
                            old_power = upper_power(before[1], lo, hi)
                            new_power = upper_power(after[1], lo, hi)
                            assert old_power > 0 and new_power < old_power * .9, (
                                tag, gender, clock, 'U upper band', old_power, new_power)
                    checks += 1
    for tag, r in (('`[.0ja.1vAr]', 4), ('`[.0pe.1sAr]', 4),
                   ('`[.1ruz]', 0), ('`[.0fu.1ra]', 2)):
        for gender in (0, 1):
            old, before, _ = components(args.previous, tag, r, gender, 0)
            new, after, _ = components(args.library, tag, r, gender, 0)
            assert old.syllables == new.syllables and before == after, (tag, gender)
    # A later U-R must not color an earlier U or a protected U-R liaison.
    for tag, r in (('`[.1u] `[.0zo.1hur]', 4),
                   ('`[.1ur] `[.1A] `[.0zo.1hur]', 6)):
        old, before, _ = components(args.previous, tag, r, 0, 0)
        new, after, _ = components(args.library, tag, r, 0, 0)
        assert old.syllables == new.syllables and before[0] == after[0], tag
        first_end = round(old.syllables[1][0] / 10000 * 32)
        assert before[1][:first_end] == after[1][:first_end], tag
        assert before[1] != after[1], tag
    for native in (False, True):
        owners = [make_language_engine(path.resolve(), path.resolve(), None,
                                       Path('/unavailable'), Path('/unavailable'),
                                       native_sentences=native)
                  for path in (args.previous, args.library)]
        try:
            for owner in owners:
                owner.set_parameter(SAMPLE_RATE, 3)
                owner.set_parameter(NATIVE_SPEED, 65)
            for text, change in (('ظهور', True), ('ظهور ناگهانی', True), ('ظهوری', True),
                                 ('فوری', True), ('گلرها', True), ('گلرهای', True),
                                 ('یاور', False), ('پسر', False), ('روز', False)):
                outputs = []
                for owner in owners:
                    pcm, marks = bytearray(), []
                    assert owner.speak((EciIndex(11), text, EciIndex(12)), pcm.extend,
                                       lambda value: marks.append(value))
                    outputs.append((bytes(pcm), marks))
                assert outputs[0][1] == outputs[1][1] == [11, 12], (text, native)
                assert (outputs[0][0] != outputs[1][0]) == change, (text, native)
                if text != 'گلرهای':
                    assert len(outputs[0][0]) == len(outputs[1][0]), (text, native)
        finally:
            for owner in owners:
                owner.close()
    print(f'Passed {checks} U/R upper-band and isolated-noise cases plus unaffected contexts.')


if __name__ == '__main__':
    main()
