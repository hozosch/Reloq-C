"""Check short /o/ before /n/ and the separate /e/ listening candidate."""
from __future__ import annotations

import argparse
from itertools import groupby
from pathlib import Path

from test_british_front_a import spectrum
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def nucleus(engine, scope, tag, gender, phone):
    _, frames = capture(engine, scope, tag, True)
    f1 = (484 if gender else 440) if phone == 'o' else (495 if gender else 450)
    def matches(row):
        c = row.controls
        f2 = 700 < c[13] < 1550 if phone == 'o' else c[13] > 1700
        return c[2] > 25 and c[8] < 2 and abs(c[9]-f1) < 2 and f2
    groups = [list(group) for selected, group in groupby(frames, matches) if selected]
    groups = [group for group in groups if len(group) >= 3]
    assert groups, (tag, gender, phone)
    return groups[0][len(groups[0]) // 2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='optional dev122 comparison')
    args = parser.parse_args()
    fa, scope = voice(args.candidate, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    count = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 98, 170):
                    for engine in (fa, old):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                               (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    for tag, phone, modified in (
                            ('`[.1don]', 'o', True), ('`[.1do.0nja]', 'o', True),
                            ('`[.1dos]', 'o', False), ('`[.1do.0ma]', 'o', False),
                            ('`[.1des]', 'e', True)):
                        row = nucleus(fa, scope, tag, gender, phone)
                        if phone == 'o':
                            expected_b1 = (300 if gender else 120) + (60 if modified else 0)
                            assert abs(row.controls[10]-expected_b1) < 2, tag
                            assert abs(row.controls[16]-(100 if modified else 120)) < 2, tag
                        else:
                            assert abs(row.controls[10]-(335 if gender else 155)) < 2, tag
                            assert abs(row.controls[16]-150) < 2, tag
                        if old:
                            before = nucleus(old, old_scope, tag, gender, phone)
                            # The vowel centres retain their identity. The
                            # surrounding consonant's transitions may differ.
                            assert abs(row.controls[9]-before.controls[9]) < 2
                            assert abs(row.controls[13]-before.controls[13]) < 30
                            if phone == 'o':
                                change = row.controls[10]-before.controls[10]
                                assert abs(change-(60 if modified else 0)) < 2, (tag, change)
                                assert abs(row.controls[16]-before.controls[16] -
                                           (-20 if modified else 0)) < 2, tag
                            else:
                                assert abs(row.controls[10]-before.controls[10]-35) < 2
                                assert abs(row.controls[16]-before.controls[16]+50) < 2
                            if rate == 3 and speed == 98 and modified:
                                new = spectrum(fa, scope, row, True, gender)
                                previous = spectrum(old, old_scope, before, True, gender)
                                # Both candidates add presence without simply
                                # forcing the /æ/ formant frequencies onto /o/.
                                band = 3 if phone == 'o' else 4
                                assert new[band] > previous[band]+.25, (tag, gender, new, previous)
                        count += 1
    finally:
        fa.close()
        if old:
            old.close()
    print(f'Passed {count} contextual Persian short-vowel comparisons.')


if __name__ == '__main__':
    main()
