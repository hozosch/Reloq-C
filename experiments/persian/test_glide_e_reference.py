"""Check the Persian /j/ against US /y/ and E against the 'three' nucleus."""
from __future__ import annotations

import argparse
from collections import Counter
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def first_voiced(rows):
    return next(row.controls for row in rows if row.controls[2] > 10)


def e_nucleus(rows, gender):
    first, second = (495, 2420) if gender else (450, 2000)
    candidates = [row.controls for row in rows if row.controls[2] > 25 and
                  row.controls[8] < 5 and abs(row.controls[9] - first) < 20 and
                  abs(row.controls[13] - second) < 60]
    if len(candidates) < 5:
        raise AssertionError('no stable e nucleus')
    formants = (9, 13, 15, 17, 19)
    return Counter(tuple(round(c[i]) for i in formants) for c in candidates).most_common(1)[0][0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    fa, scope = voice(args.candidate, 0x90000)
    us, us_scope = voice(args.candidate, 0x10000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        for gender in (0, 1):
            for rate in (1, 3, 6):
                for speed in (40, 98, 170):
                    for engine in (fa, us, old):
                        if engine:
                            for key, value in ((GENDER, gender), (SAMPLE_RATE, rate), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    for vowel in 'aAeio u'.replace(' ', ''):
                        a, ar = capture(fa, scope, '`[.1j' + vowel + ']', True)
                        _, br = capture(us, us_scope, '`[.1y' + ('E' if vowel == 'e' else vowel) + ']', False)
                        left, right = first_voiced(ar), first_voiced(br)
                        for column in (2, 7, 9, 13, 15, 16):
                            assert abs(left[column] - right[column]) <= 1, (
                                gender, rate, speed, vowel, column, left[column], right[column])
                        if old:
                            previous, before = capture(old, old_scope, '`[.1j' + vowel + ']', True)
                            assert len(a.pcm) == len(previous.pcm) and len(ar) == len(before), (
                                gender, rate, speed, vowel, 'changed phone timing')
                        checks += 1
                    tags = ('`[.1se]', '`[.1jek]', '`[.1pe]', '`[.1te]', '`[.1ke]',
                            '`[.1he]', '`[.1me]', '`[.1ne]', '`[.1le]', '`[.1re]', "`[.1s'e:']")
                    expected = None
                    for tag in tags:
                        current, rows = capture(fa, scope, tag, True)
                        actual = e_nucleus(rows, gender)
                        if expected is None:
                            expected = actual
                        assert actual == expected, (gender, rate, speed, tag, actual, expected)
                        if old:
                            previous, before = capture(old, old_scope, tag, True)
                            assert len(current.pcm) == len(previous.pcm) and len(rows) == len(before), (
                                gender, rate, speed, tag, 'changed vowel timing')
                        checks += 1
    finally:
        fa.close(); us.close()
        if old:
            old.close()
    print(f'Passed {checks} Persian glide and vowel reference comparisons.')


if __name__ == '__main__':
    main()
