"""Measure the Persian front vowel against British and US references.

The previous test equated a closer F3 value with a clearer vowel.  The
listening feedback disproved that assumption, so this test examines the
rendered upper spectrum as well as the native vowel and scope boundaries.
"""
from __future__ import annotations

import argparse
import array
import ctypes
import math
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameReplay, NativeFrame


def vowel(rows):
    rows = [r for r in rows if r.controls[2] > 25 and r.controls[8] < 1 and r.controls[9] > 400]
    # The US consonant release overlaps the edge of the short vowel. At
    # higher rates eleven fully settled frames remain in this test word.
    assert len(rows) >= 8
    return rows[len(rows) // 2]


def spectrum(engine, scope, row, selected, gender):
    """Replay a steady nucleus at matched F0 and measure its harmonic bands."""
    frames = (NativeFrame * 100)()
    raw = ctypes.string_at(ctypes.byref(row), ctypes.sizeof(row))
    for i in range(len(frames)):
        ctypes.memmove(ctypes.byref(frames, i * ctypes.sizeof(row)), raw, len(raw))
        frames[i].tick = i
        frames[i].controls[1] = 2000 if gender else 1200
    pcm = bytearray()
    previous = scope(int(selected))
    try:
        assert FrameReplay(engine._dll).replay(engine, frames, (), pcm.extend, lambda _: None)
    finally:
        scope(previous)
    samples = array.array('h')
    samples.frombytes(pcm)
    samples = samples[16 * 40:16 * 90]
    assert len(samples) == 800
    window = [s * (0.5 - 0.5 * math.cos(2 * math.pi * i / (len(samples) - 1)))
              for i, s in enumerate(samples)]
    # The source is periodic at the imposed F0. Measure every harmonic and
    # normalize the band powers; this avoids relying on an optional FFT module.
    pitch = 200 if gender else 120
    powers = [0.0] * 6
    ranges = ((200, 500), (500, 1000), (1000, 2000),
              (2000, 3000), (3000, 4000), (4000, 6000))
    for hz in range(pitch, 6000, pitch):
        band = next((i for i, (lo, hi) in enumerate(ranges) if lo <= hz < hi), None)
        if band is None:
            continue
        coefficient = 2.0 * math.cos(2 * math.pi * hz / 16000)
        a = b = 0.0
        for sample in window:
            c = sample + coefficient * a - b
            b, a = a, c
        powers[band] += max(0.0, a*a + b*b - coefficient*a*b)
    total = sum(powers)
    assert total > 0
    return tuple(10 * math.log10(max(p / total, 1e-12)) for p in powers)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='optional dev118 for comparison')
    args = parser.parse_args()
    fa, scope = voice(args.candidate, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    british, british_scope = voice(args.candidate, 0x10001)
    us, us_scope = voice(args.candidate, 0x10000)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 98, 170):
                    for item in (fa, old, british, us):
                        if item:
                            for parameter, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                                     (NATIVE_SPEED, speed)):
                                item.set_parameter(parameter, value)
                    _, gb_rows = capture(british, british_scope, 'sad', False)
                    _, us_rows = capture(us, us_scope, 'sad', False)
                    for tag in ("`[.1s'A:'d]", '`[.1sAd]'):
                        current, rows = capture(fa, scope, tag, True)
                        mid = vowel(rows)
                        assert all(mid.controls[i] > 0 for i in (9, 10, 13, 14, 15, 16, 19, 20))
                        if old:
                            before, old_rows = capture(old, old_scope, tag, True)
                            old_mid = vowel(old_rows)
                            # F1/F2 frequencies and damping are outside this
                            # upper-resonance adjustment.
                            assert all(abs(mid.controls[i] - old_mid.controls[i]) < 1
                                       for i in (9, 10, 13, 14)), (rate, gender, speed, tag)
                            assert current.pcm != before.pcm
                        if (rate, speed, tag) == (3, 98, "`[.1s'A:'d]"):
                            levels = spectrum(fa, scope, mid, True, gender)
                            gb = spectrum(british, british_scope, vowel(gb_rows), False, gender)
                            target = spectrum(us, us_scope, vowel(us_rows), False, gender)
                            if old:
                                previous_levels = spectrum(old, old_scope, vowel(old_rows), True, gender)
                                # Guard against losing upper-band energy, but
                                # judge spectral shape with the independent
                                # multi-pitch harmonic test rather than a
                                # single broad-band distance.
                                assert all(levels[i] >= previous_levels[i] - 1 for i in (3, 4, 5)), (
                                    gender, levels, previous_levels, target, gb)
                            print('matched-F0 front vowel', gender, 'FA', levels,
                                  'GB', gb, 'US', target)
                        checks += 1
        if old:
            for dialect, text in ((0x90000, 'Viisi sanaa.'), (0x10001, 'sad'),
                                  (0x10000, 'sad'), (0x40000, 'Ein deutscher Satz.')):
                a, s = voice(args.candidate, dialect)
                b, t = voice(args.previous, dialect)
                try:
                    current, _ = capture(a, s, text, False)
                    previous, _ = capture(b, t, text, False)
                    assert current.pcm == previous.pcm, (dialect, text)
                    checks += 1
                finally:
                    a.close()
                    b.close()
    finally:
        fa.close()
        british.close()
        us.close()
        if old:
            old.close()
    print(f'Passed {checks} Persian/English front-vowel and isolation checks.')


if __name__ == '__main__':
    main()
