"""Verify Finnish /m, n/ targets with US phone timing in Persian speech."""
from __future__ import annotations

import argparse
from pathlib import Path

from test_us_articulation import voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from test_persian_r_after_i_contexts import capture, middle


def duration(result, index):
    start, end = result.syllables[index]
    return end - start


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='optional previous Persian build')
    args = parser.parse_args()
    fa, fa_scope = voice(args.candidate, 0x90000)
    us, us_scope = voice(args.candidate, 0x10000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    count = changed = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 140):
                    for engine in (fa, us, previous):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    for phone in 'mn':
                        for vowel in 'Aaeiou':
                            for onset in (True, False):
                                fa_vowel = "'i:'" if vowel == 'i' else vowel
                                us_vowel = 'E' if vowel == 'e' else vowel
                                fa_tag = f'`[.1{phone}{fa_vowel}]' if onset else f'`[.1{fa_vowel}{phone}]'
                                us_tag = f'`[.1{phone}{us_vowel}]' if onset else f'`[.1{us_vowel}{phone}]'
                                nasal, nucleus = (0, 1) if onset else (1, 0)
                                selected = capture(fa, fa_scope, fa_tag, True)
                                finnish = capture(fa, fa_scope, fa_tag, False)
                                english = capture(us, us_scope, us_tag, False)
                                case = rate, gender, speed, fa_tag
                                assert duration(selected, nasal) == duration(english, nasal), (case, 'US timing')
                                # Mid-nasal targets are the Finnish ones, with a
                                # small interpolation tolerance on the nasal zero.
                                cols = (2, 9, 13, 15, 27, 29)
                                actual = middle(selected, nasal, cols)
                                original = middle(finnish, nasal, cols)
                                for column, a, b, tolerance in zip(cols, actual, original, (3, 1, 1, 1, 1, 20)):
                                    assert abs(a-b) <= tolerance, (case, column, a, b)
                                # The vowel midpoint is voiced and its nasal
                                # pole and zero cancel instead of weakening it.
                                av, pole, zero = middle(selected, nucleus, (2, 27, 29))
                                assert av > 30 and abs(pole-zero) <= 1, (case, 'vowel', av, pole, zero)
                                if previous:
                                    prior = capture(previous, old_scope, fa_tag, True)
                                    assert selected.syllables == prior.syllables, (case, 'phone durations')
                                    left, right = selected.frames(), prior.frames()
                                    assert len(left) == len(right)
                                    assert all(a.tick == b.tick and a.controls[1] == b.controls[1]
                                               for a, b in zip(left, right)), (case, 'pitch')
                                    assert middle(selected, nucleus, cols) == middle(prior, nucleus, cols), (
                                        case, 'vowel midpoint changed')
                                    changed += selected.frame_bytes != prior.frame_bytes
                                count += 1
                    # The reported repeated-nasal contexts keep the vowel's
                    # own target and use the same US duration at both ends.
                    for phone, vowel in (('n', "'u:'"), ('m', "'i:'")):
                        tag = f'`[.1{phone}{vowel}{phone}]'
                        selected = capture(fa, fa_scope, tag, True)
                        english = capture(us, us_scope, tag, False)
                        for nasal in (0, 2):
                            assert duration(selected, nasal) == duration(english, nasal), (tag, rate, gender, speed)
                        av, pole, zero = middle(selected, 1, (2, 27, 29))
                        assert av > 30 and abs(pole-zero) <= 1, (tag, rate, gender, speed)
                        if previous:
                            prior = capture(previous, old_scope, tag, True)
                            assert selected.syllables == prior.syllables
                            assert middle(selected, 1, (2, 9, 13, 15, 27, 29)) == middle(
                                prior, 1, (2, 9, 13, 15, 27, 29))
        if previous:
            assert changed >= count // 2, (changed, count)
    finally:
        fa.close()
        us.close()
        if previous:
            previous.close()
    print(f'Passed {count} Finnish nasal targets, US durations, pitch and vowel-center checks.')


if __name__ == '__main__':
    main()
