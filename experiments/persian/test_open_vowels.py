"""Check short /o/ contexts and the separate /e/ listening candidate."""
from __future__ import annotations

import argparse
from itertools import groupby
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def nucleus(engine, scope, tag, gender, phone, *, scaled=True):
    _, frames = capture(engine, scope, tag, True)
    f1 = ((498 if gender else 453) if scaled else (484 if gender else 440)) if phone == 'o' else (495 if gender else 450)
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
                    reference = nucleus(fa, scope, '`[.1do]', gender, 'o')
                    for tag, phone in (
                            ('`[.1do]', 'o'), ('`[.1no]', 'o'), ('`[.1mo]', 'o'),
                            ('`[.1don]', 'o'), ('`[.1do.0nja]', 'o'),
                            ('`[.1dos]', 'o'), ('`[.1do.0ma]', 'o'),
                            ('`[.1des]', 'e')):
                        row = nucleus(fa, scope, tag, gender, phone)
                        if phone == 'o':
                            expected_b1 = 300 if gender else 120
                            assert abs(row.controls[10]-expected_b1) < 2, tag
                            assert abs(row.controls[16]-120) < 2, tag
                            if tag in ('`[.1no]', '`[.1mo]'):
                                assert abs(row.controls[4]-reference.controls[4]) < 1, tag
                                assert abs(row.controls[17]-reference.controls[17]) < 2, tag
                        else:
                            assert abs(row.controls[10]-(335 if gender else 155)) < 2, tag
                            assert abs(row.controls[16]-125) < 2, tag
                        if old:
                            before = nucleus(old, old_scope, tag, gender, phone, scaled=False)
                            # The vowel centres retain their identity. The
                            # surrounding consonant's transitions may differ.
                            assert abs(row.controls[9]-before.controls[9] * (1.03 if phone == 'o' else 1)) < 3
                            if phone == 'e':
                                assert abs(row.controls[13]-before.controls[13]) < 30
                        count += 1
                    # Two's /o/ is the target in all these consonant
                    # contexts, including after Ach and before/after nasals.
                    # Compare the stable nucleus, not the transitions.
                    for word in ('po', 'to', 'ko', 'bo', 'go', 'fo', 'so', 'So',
                                 'zo', 'lo', 'ro', 'vo', 'jo', 'Go', 'ho', 'no',
                                 'mo', 'os', 'on', 'om', 'ot', 'ok', 'ov', 'dom',
                                 'dor', 'do.0nja', 'no.0ro', 'mo.0ro', 'so.0ro'):
                        tag = '`[.1' + word + ']'
                        row = nucleus(fa, scope, tag, gender, 'o')
                        for column in (4, 9, 10, 13, 14, 15, 16, 17, 18, 19, 20):
                            assert abs(row.controls[column] - reference.controls[column]) < 2, (
                                gender, rate, speed, tag, column,
                                row.controls[column], reference.controls[column])
                        count += 1
    finally:
        fa.close()
        if old:
            old.close()
    print(f'Passed {count} contextual Persian short-vowel comparisons.')


if __name__ == '__main__':
    main()
