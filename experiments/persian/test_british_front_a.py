"""Compare Persian 100/صد and short Ä against British English sad."""
from __future__ import annotations

import argparse
from pathlib import Path
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def vowel(rows):
    # Ignore the preceding S, the following D, and their transitions. These
    # tests compare the voiced vowel nuclei, not overall word duration.
    rows = [r for r in rows if r.controls[2] > 25 and r.controls[8] < 1 and r.controls[9] > 400]
    assert len(rows) >= 12
    return rows[len(rows) // 2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='dev117 for exact control comparisons')
    args = parser.parse_args()
    fa, scope = voice(args.candidate, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    british, british_scope = voice(args.candidate, 0x10001)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 98, 170):
                    for item in (fa, old, british):
                        if item:
                            for parameter, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                                     (NATIVE_SPEED, speed)):
                                item.set_parameter(parameter, value)
                    _, gb_rows = capture(british, british_scope, 'sad', False)
                    gb = vowel(gb_rows)
                    # 100 and the written word صد intentionally have distinct
                    # length markers, and both should receive the same target.
                    for tag in ("`[.1s'A:'d]", '`[.1sAd]'):
                        current, rows = capture(fa, scope, tag, True)
                        mid = vowel(rows)
                        assert abs(mid.controls[15] - gb.controls[15]) < 100, (rate, gender, speed, tag)
                        assert round(mid.controls[20]) == (500 if gender else 380), (
                            rate, gender, speed, tag, mid.controls[20])
                        if old:
                            previous, old_rows = capture(old, old_scope, tag, True)
                            before = vowel(old_rows)
                            assert len(current.pcm) == len(previous.pcm) and len(rows) == len(old_rows)
                            assert current.pcm != previous.pcm
                            assert abs(mid.controls[15] - gb.controls[15]) < \
                                   abs(before.controls[15] - gb.controls[15]), (rate, gender, speed, tag)
                            assert all(a.tick == b.tick and all(a.controls[i] == b.controls[i]
                                       for i in range(62) if i not in (15, 20))
                                       for a, b in zip(rows, old_rows)), (rate, gender, speed, tag)
                            if not gender:
                                assert abs(mid.controls[20] - gb.controls[20]) < \
                                       abs(before.controls[20] - gb.controls[20]), (rate, gender, speed, tag)
                        checks += 1
                if old:
                    for tag in ('`[.1li]', '`[.1a]', '`[.1e]', '`[.1i]', '`[.1u]'):
                        a, _ = capture(fa, scope, tag, True)
                        b, _ = capture(old, old_scope, tag, True)
                        assert a.pcm == b.pcm, (rate, gender, tag)
                        checks += 1
        if old:
            # Scope isolation: ordinary Finnish, English and German voices
            # must remain bit-identical to the previous native build.
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
        if old:
            old.close()
    print(f'Passed {checks} British/Persian front-vowel and isolation checks.')


if __name__ == '__main__':
    main()
