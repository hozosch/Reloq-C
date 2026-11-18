"""Check the measured Persian back-A step towards German oft, in context."""
from __future__ import annotations

import argparse
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def back_nucleus(rows, expected_f1):
    matches = [r for r in rows if r.controls[2] > 25 and r.controls[8] < 1 and
               abs(r.controls[9] - expected_f1) < 14]
    assert len(matches) >= 2, 'no settled Persian back-A frames'
    return matches[len(matches) // 2]


def german_o_nucleus(rows):
    # At the fastest speed the short German vowel has fewer than eight
    # fully settled frames; selecting its middle still measures the target.
    matches = [r for r in rows if r.controls[2] > 25 and r.controls[8] < 1 and
               r.controls[9] > 300]
    assert len(matches) >= 2, 'no settled German short-O frames'
    return matches[len(matches) // 2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    fa, scope = voice(args.candidate, 0x90000)
    german, german_scope = voice(args.candidate, 0x40000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    columns = (9, 13, 15, 17, 19)
    # Measured dev141 nuclei at head size 50, not a claim of acoustic
    # equivalence between the voices. Later voice scaling is sex-dependent.
    baseline = {0: (656, 1000, 2620, 3700, 3900),
                1: (820, 1070, 2998, 4200, 4485)}
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 98, 170):
                    for engine in (fa, german, old):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                               (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    de = german_o_nucleus(capture(german, german_scope, 'oft', False)[1])
                    # Native female F1 scaling is nonlinear: the same raw
                    # adjustment produces roughly a half-step there, while
                    # the other measured values remain near a quarter-step.
                    expected = []
                    for column, before in zip(columns, baseline[gender]):
                        share = 0 if column == 19 else 0.5 if gender and column == 9 else 0.25
                        target = (1 - share) * before + share * de.controls[column]
                        if not (gender and column == 9):
                            target *= 1.03
                        if gender and column in (17, 19):
                            target = min(target, 4200 if column == 17 else 4600)
                        expected.append(target)
                    for tag in ('`[.1a]', '`[.1sad]', '`[.1ran]', '`[.1nam]',
                                '`[.1i.0ran]'):
                        current, rows = capture(fa, scope, tag, True)
                        row = back_nucleus(rows, expected[0])
                        for column, target in zip(columns, expected):
                            assert abs(row.controls[column] - target) < 14, (
                                rate, gender, speed, tag, column, row.controls[column], target)
                        if old:
                            previous, before_rows = capture(old, old_scope, tag, True)
                            assert len(current.pcm) == len(previous.pcm) and current.pcm != previous.pcm
                            assert len(rows) == len(before_rows)
                            assert all(a.tick == b.tick and abs(a.controls[1] - b.controls[1]) < .01
                                       for a, b in zip(rows, before_rows))
                        checks += 1
                    if old:
                        # The E vowel remains unchanged by the head-size trial.
                        assert capture(fa, scope, '`[.1e]', True)[0].pcm == \
                               capture(old, old_scope, '`[.1e]', True)[0].pcm
        if old:
            for dialect, text in ((0x90000, 'Viisi sanaa.'), (0x10000, 'can'),
                                  (0x40000, 'oft und Tag')):
                a, sa = voice(args.candidate, dialect)
                b, sb = voice(args.previous, dialect)
                try:
                    assert capture(a, sa, text, False)[0].pcm == capture(b, sb, text, False)[0].pcm
                finally:
                    a.close()
                    b.close()
    finally:
        fa.close()
        german.close()
        if old:
            old.close()
    print(f'Passed {checks} German-guided Persian back-vowel comparisons.')


if __name__ == '__main__':
    main()
