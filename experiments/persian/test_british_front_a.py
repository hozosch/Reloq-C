"""Check the Persian front vowel between British sad and German Tag."""
from __future__ import annotations

import argparse
import array
import ctypes
import math
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE
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
    german, german_scope = voice(args.candidate, 0x40000)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 98, 170):
                    for item in (fa, old, british, german):
                        if item:
                            for parameter, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                                     (NATIVE_SPEED, speed)):
                                item.set_parameter(parameter, value)
                    british.set_parameter(HEAD_SIZE, 46)
                    _, gb_rows = capture(british, british_scope, 'sad', False)
                    reference = vowel(gb_rows)
                    _, de_rows = capture(german, german_scope, 'Tag', False)
                    german_reference = vowel(de_rows)
                    for tag in ("`[.1s'A:'d]", '`[.1sAd]'):
                        current, rows = capture(fa, scope, tag, True)
                        mid = vowel(rows)
                        assert all(mid.controls[i] > 0 for i in (9, 10, 13, 14, 15, 16, 19, 20))
                        # Frequencies, bandwidths and source tilt are all
                        # checked, not just one upper resonance.
                        for i in (4, 9, 10, 13, 14, 15, 16, 17, 18, 19, 20):
                            is_frequency = i in (9, 13, 15, 17, 19)
                            target = (.75 * reference.controls[i] + .25 * german_reference.controls[i]
                                      if is_frequency else reference.controls[i])
                            tolerance = 18 if is_frequency else .5 if i == 4 else 5
                            assert abs(mid.controls[i] - target) < tolerance, (
                                rate, gender, speed, tag, i, mid.controls[i], target)
                        if old:
                            before, old_rows = capture(old, old_scope, tag, True)
                            # Only the vowel sound changed; the word duration
                            # and the pitch plan retain the Persian baseline.
                            assert len(current.pcm) == len(before.pcm)
                            assert len(rows) == len(old_rows)
                            assert all(a.tick == b.tick and abs(a.controls[1] - b.controls[1]) < .01
                                       for a, b in zip(rows, old_rows))
                            assert current.pcm != before.pcm
                        if (rate, speed, tag) == (3, 98, "`[.1s'A:'d]"):
                            levels = spectrum(fa, scope, mid, True, gender)
                            gb = spectrum(british, british_scope, vowel(gb_rows), False, gender)
                            assert max(abs(a - b) for a, b in zip(levels, gb)) < 8, (gender, levels, gb)
                            print('matched-F0 front vowel', gender, 'FA', levels, 'GB', gb)
                        checks += 1
        # The upper-formant defaults and bandwidths differ by voice and head
        # size. They must follow a slightly smaller British reference, not a
        # fixed calibration made for the default male voice.
        for gender in (0, 1):
            for head in (0, 25, 50, 75, 100):
                for engine in (fa, british, german):
                    for parameter, value in ((GENDER, gender), (HEAD_SIZE, head),
                                             (SAMPLE_RATE, 3), (NATIVE_SPEED, 98)):
                        engine.set_parameter(parameter, value)
                british.set_parameter(HEAD_SIZE, max(0, head - 4))
                actual = vowel(capture(fa, scope, '`[.1sAd]', True)[1])
                expected = vowel(capture(british, british_scope, 'sad', False)[1])
                de = vowel(capture(german, german_scope, 'Tag', False)[1])
                for i in (4, 9, 10, 13, 14, 15, 16, 17, 18, 19, 20):
                    is_frequency = i in (9, 13, 15, 17, 19)
                    if is_frequency:
                        # The native female/head-size transform is nonlinear
                        # at its extremes. Keep the result between the two
                        # measured reference voices, with a small allowance
                        # for the unavailable negative British head setting.
                        low, high = sorted((expected.controls[i], de.controls[i]))
                        margin = max(12, expected.controls[i] * .026)
                        assert low - margin <= actual.controls[i] <= high + margin, (
                            gender, head, i, actual.controls[i], low, high)
                    else:
                        tolerance = .5 if i == 4 else 5
                        assert abs(actual.controls[i] - expected.controls[i]) < tolerance, (
                            gender, head, i, actual.controls[i], expected.controls[i])
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
        german.close()
        if old:
            old.close()
    print(f'Passed {checks} Persian/English front-vowel and isolation checks.')


if __name__ == '__main__':
    main()
