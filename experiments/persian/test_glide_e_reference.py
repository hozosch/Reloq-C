"""Check German initial /j/, unchanged medial /j/, and the 'three' E."""
from __future__ import annotations

import argparse
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE


def first_voiced(rows):
    return next(row.controls for row in rows if row.controls[2] > 10)


def e_nucleus(rows, gender):
    first = 495 if gender else 450
    candidates = [row.controls for row in rows if row.controls[2] > 25 and
                  row.controls[8] < 5 and abs(row.controls[9] - first) < 20 and
                  1700 < row.controls[13] < 2600]
    if len(candidates) < 5:
        raise AssertionError('no stable e nucleus')
    # The old se has a rising F2/F3 trajectory, rather than a static point.
    # Take the same proportion through the vowel; neighbouring consonants can
    # still contribute a few transition frames on either side.
    c = candidates[len(candidates) * 2 // 3]
    return tuple(round(c[i]) for i in (9, 13, 15))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='optional dev151 library')
    parser.add_argument('--older', type=Path, help='optional dev150 library')
    args = parser.parse_args()
    fa, scope = voice(args.candidate, 0x90000)
    german, german_scope = voice(args.candidate, 0x40000)
    previous, previous_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    old, old_scope = voice(args.older, 0x90000) if args.older else (None, None)
    checks = 0
    try:
        for gender in (0, 1):
            for rate in (1, 3, 6):
                for speed in (40, 98, 170):
                    for engine in (fa, german, previous, old):
                        if engine:
                            for key, value in ((GENDER, gender), (SAMPLE_RATE, rate), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    for vowel in 'aAeio u'.replace(' ', ''):
                        a, ar = capture(fa, scope, '`[.1j' + vowel + ']', True)
                        _, br = capture(german, german_scope, '`[.1j' + vowel + ']', False)
                        left, right = first_voiced(ar), first_voiced(br)
                        # Compare the first three resonances and their widths.
                        # Source loudness, pitch, and upper resonances also
                        # depend on the surrounding language and voice.
                        for column in (9, 10, 13, 14, 15, 16):
                            assert abs(left[column] - right[column]) <= 1, (
                                gender, rate, speed, vowel, column, left[column], right[column])
                        if previous:
                            earlier, before = capture(previous, previous_scope, '`[.1j' + vowel + ']', True)
                            assert len(a.pcm) == len(earlier.pcm) and len(ar) == len(before), (
                                gender, rate, speed, vowel, 'changed phone timing')
                            assert a.pcm != earlier.pcm, (gender, rate, speed, vowel, 'initial J unchanged')
                        checks += 1
                    for tag, initial in (('`[.1aja]', False), ('`[.1aj]', False),
                                         ('`[.0mi.1jan.0gin]', False),
                                         ('`[.1ma] `[.1ja]', True)):
                        current, rows = capture(fa, scope, tag, True)
                        # In these controlled syllables, B3 distinguishes the
                        # original glide from the US donor. Check an initial
                        # J after a whole preceding word as well as medial and
                        # word-final J, without including uppercase /dzh/.
                        glide = [r.controls for r in rows if r.controls[2] > 20 and
                                 r.controls[9] < (360 if gender else 330) and
                                 r.controls[13] > (1950 if gender else 1700)]
                        assert any(c[16] >= 450 if initial else 145 <= c[16] <= 155
                                   for c in glide), (gender, rate, speed, tag)
                        if not initial and previous:
                            earlier, _ = capture(previous, previous_scope, tag, True)
                            assert current.pcm == earlier.pcm, (gender, rate, speed, tag)
                        checks += 1
                    tags = ('`[.1se]', '`[.1jek]', '`[.1pe]', '`[.1te]', '`[.1ke]',
                            '`[.1he]', '`[.1me]', '`[.1ne]', '`[.1le]', '`[.1re]', "`[.1s'e:']")
                    expected = None
                    for tag in tags:
                        current, rows = capture(fa, scope, tag, True)
                        actual = e_nucleus(rows, gender)
                        if expected is None:
                            expected = actual
                        assert abs(actual[0] - expected[0]) <= 20 and \
                               abs(actual[1] - expected[1]) <= 50 and \
                               abs(actual[2] - expected[2]) <= 70, (
                                   gender, rate, speed, tag, actual, expected)
                        if old:
                            earlier, before = capture(old, old_scope, tag, True)
                            assert len(current.pcm) == len(earlier.pcm) and len(rows) == len(before), (
                                gender, rate, speed, tag, 'changed vowel timing')
                            if tag == '`[.1se]':
                                assert current.pcm == earlier.pcm, (gender, rate, speed, 'three reference changed')
                        checks += 1
        # The German core reduces the onset voice for very small female
        # heads. Verify the threshold on both sides rather than reading the
        # unrelated Finnish state flag with the same generated name.
        for gender in (0, 1):
            for head in (22, 29, 30, 50):
                for engine in (fa, german):
                    for key, value in ((GENDER, gender), (HEAD_SIZE, head),
                                       (SAMPLE_RATE, 3), (NATIVE_SPEED, 82)):
                        engine.set_parameter(key, value)
                _, actual = capture(fa, scope, '`[.1je]', True)
                _, reference = capture(german, german_scope, '`[.1je]', False)
                assert abs(first_voiced(actual)[2] - first_voiced(reference)[2]) <= 1, (gender, head)
                checks += 1
    finally:
        fa.close(); german.close()
        if old:
            old.close()
        if previous:
            previous.close()
    print(f'Passed {checks} Persian glide and vowel reference comparisons.')


if __name__ == '__main__':
    main()
