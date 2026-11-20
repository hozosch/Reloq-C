"""Verify French nasal acoustics, US timing and uncolored Persian vowels."""
from __future__ import annotations

import argparse
import array
import math
from pathlib import Path
from statistics import median
import sys

from test_us_articulation import voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import FrameCapture
from test_persian_r_after_i_contexts import capture, middle


def duration(result, index):
    start, end = result.syllables[index]
    return end - start


def french_onset(engine, scope, word, columns):
    previous = scope(0)
    try:
        result = FrameCapture(engine._dll).capture(engine, (word,))
        assert result and result.pcm
        rows = [row for row in result.frames() if row.tick <= 8 and row.controls[2] > 0]
        assert rows, word
        return tuple(median(row.controls[column] for row in rows) for column in columns)
    finally:
        assert scope(previous) == 0


def _band_energy(samples, rate, center, q):
    """Two biquads give a bounded, dependency-free spectral comparison."""
    omega = 2 * math.pi * center / rate
    alpha = math.sin(omega) / (2 * q)
    b0 = alpha / (1 + alpha)
    a1 = -2 * math.cos(omega) / (1 + alpha)
    a2 = (1 - alpha) / (1 + alpha)
    values = samples
    for _ in range(2):
        x1 = x2 = y1 = y2 = 0.0
        output = []
        for x in values:
            y = b0*x - b0*x2 - a1*y1 - a2*y2
            x2, x1, y2, y1 = x1, x, y1, y
            output.append(y)
        values = output
    steady = values[64:]
    assert steady
    return sum(value*value for value in steady) / len(steady)


def upper_to_low_db(result, index, rate):
    start, end = (value / 10000 for value in result.syllables[index])
    pcm = array.array('h'); pcm.frombytes(result.pcm)
    if sys.byteorder != 'little':
        pcm.byteswap()
    samples = pcm[int((start + .2*(end-start))*rate/1000):
                  int((start + .8*(end-start))*rate/1000)]
    lower = _band_energy(samples, rate, 600, .8)
    upper = _band_energy(samples, rate, 3200, 2.0)
    assert lower > 0 and upper > 0
    return 10 * math.log10(upper / lower)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='optional previous Persian build')
    args = parser.parse_args()
    fa, fa_scope = voice(args.candidate, 0x90000)
    us, us_scope = voice(args.candidate, 0x10000)
    fr, fr_scope = voice(args.candidate, 0x30000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    count = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 140):
                    for engine in (fa, us, fr, previous):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    # Compare stable source and resonance controls to real
                    # French words; contextual F2/F3 and timing may differ.
                    columns = (2, 4, 7, 9, 10, 14, 16, 17, 19, 27)
                    for phone, word in (('m', 'mime'), ('n', 'niche')):
                        nasal = capture(fa, fa_scope, f'`[.1{phone}im]')
                        actual = middle(nasal, 0, columns)
                        original = french_onset(fr, fr_scope, word, columns)
                        assert all(abs(a-b) <= 1 for a, b in zip(actual, original)), (
                            rate, gender, speed, phone, actual, original)
                    # Check both ends and six vowels. Preserve the adopted US
                    # phone timings while allowing the French sound targets.
                    for phone in 'mn':
                        for vowel in 'Aaeiou':
                            for onset in (True, False):
                                fa_vowel = "'i:'" if vowel == 'i' else vowel
                                us_vowel = 'E' if vowel == 'e' else vowel
                                fa_tag = f'`[.1{phone}{fa_vowel}]' if onset else f'`[.1{fa_vowel}{phone}]'
                                us_tag = f'`[.1{phone}{us_vowel}]' if onset else f'`[.1{us_vowel}{phone}]'
                                nasal, nucleus = (0, 1) if onset else (1, 0)
                                selected = capture(fa, fa_scope, fa_tag, True)
                                english = capture(us, us_scope, us_tag, False)
                                case = rate, gender, speed, fa_tag
                                assert duration(selected, nasal) == duration(english, nasal), (case, 'US timing')
                                av, pole, zero = middle(selected, nucleus, (2, 27, 29))
                                assert av > 30 and abs(pole-zero) <= 1, (case, 'vowel', av, pole, zero)
                                if previous:
                                    prior = capture(previous, old_scope, fa_tag, True)
                                    assert selected.syllables == prior.syllables, (case, 'phone durations')
                                    a, b = selected.frames(), prior.frames()
                                    assert len(a) == len(b)
                                    assert all(x.tick == y.tick and x.controls[1] == y.controls[1]
                                               for x, y in zip(a, b)), (case, 'pitch')
                                count += 1
                    # Compare every vowel between nasals to itself in
                    # isolation: no inherited tilt or shifted resonances in
                    # the nucleus, while the consonant transitions remain.
                    for vowel in 'aAeiou':
                        alone = capture(fa, fa_scope, f'`[.1{vowel}]')
                        cols = (4, 9, 13, 15, 17, 19, 27, 29)
                        neutral = middle(alone, 0, cols)
                        for phone in 'mn':
                            surrounded = capture(fa, fa_scope, f'`[.1{phone}{vowel}{phone}]')
                            actual = middle(surrounded, 1, cols)
                            for col, a, b, tolerance in zip(cols, actual, neutral, (1, 2, 3, 3, 3, 3, 1, 1)):
                                assert abs(a-b) <= tolerance, (rate, gender, speed, phone, vowel, col, a, b)
                            if (phone, vowel) in (('n', 'u'), ('m', 'i')):
                                # Measure synthesized PCM, normalized against
                                # its lower band. The old U tilt and I
                                # allophone lose several dB here despite an
                                # unchanged overall vowel level.
                                difference = (upper_to_low_db(surrounded, 1, SAMPLE_RATES[rate]) -
                                              upper_to_low_db(alone, 0, SAMPLE_RATES[rate]))
                                assert difference >= -2.5, (rate, gender, speed, phone, vowel, difference)
                    for phone, vowel in (('n', "'u:'"), ('m', "'i:'")):
                        alone = capture(fa, fa_scope, f'`[.1{vowel}]')
                        surrounded = capture(fa, fa_scope, f'`[.1{phone}{vowel}{phone}]')
                        cols = (4, 9, 13, 15, 17, 19, 27, 29)
                        neutral, actual = middle(alone, 0, cols), middle(surrounded, 1, cols)
                        assert all(abs(a-b) <= 3 for a, b in zip(actual, neutral)), (
                            rate, gender, speed, phone, vowel, actual, neutral)
        if previous:
            # Do not modify ordinary Finnish or words without an adjacent
            # nasal in the Persian scope.
            for tag in ('`[.1li]', '`[.1sir]', '`[.1pAt]', '`[.1ha]'):
                for selected in (False, True):
                    a = capture(fa, fa_scope, tag, selected)
                    b = capture(previous, old_scope, tag, selected)
                    assert a.pcm == b.pcm and a.frame_bytes == b.frame_bytes, (tag, selected)
            for tag in ('`[.1nun]', '`[.1mim]'):
                a, b = capture(fa, fa_scope, tag, False), capture(previous, old_scope, tag, False)
                assert a.pcm == b.pcm and a.frame_bytes == b.frame_bytes, (tag, 'Finnish isolation')
    finally:
        fa.close(); us.close(); fr.close()
        if previous:
            previous.close()
    print(f'Passed {count} French nasal, US duration and uncolored-vowel cases.')


if __name__ == '__main__':
    main()
