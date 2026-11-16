"""Compare Persian /m/ and /n/ trajectories with the US native voice."""
from __future__ import annotations

import argparse
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def nasal(engine, scope, phone, context, onset, persian):
    vowel = ("'i:'" if persian else 'i') if context == 'i' else (
        'E' if context == 'e' and not persian else context)
    tag = f'`[.1{phone}{vowel}]' if onset else f'`[.1{vowel}{phone}]'
    _, frames = capture(engine, scope, tag, persian)
    # Nasal pole/zero separation distinguishes the voiced nasal portion
    # from the following or preceding ordinary vowel in these examples.
    rows = [row for row in frames if row.controls[2] > 15 and row.controls[8] < 2
            and row.controls[29] - row.controls[27] >= 40]
    assert rows, ('missing voiced nasal', tag)
    return len(rows), rows[len(rows) // 2].controls


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    args = parser.parse_args()
    fa, fa_scope = voice(args.candidate, 0x90000)
    us, us_scope = voice(args.candidate, 0x10000)
    count = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 98, 170):
                    for engine in (fa, us):
                        for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                           (NATIVE_SPEED, speed)):
                            engine.set_parameter(key, value)
                    for phone in 'mn':
                        for context in 'Aaeiou':
                            for onset in (True, False):
                                a, x = nasal(fa, fa_scope, phone, context, onset, True)
                                b, y = nasal(us, us_scope, phone, context, onset, False)
                                case = rate, gender, speed, phone, context, onset
                                assert abs(a-b) <= max(35, round(.30*b)), (case, a, b)
                                for col, tolerance in ((2, 12), (9, 90), (13, 250),
                                                       (15, 350), (27, 60), (29, 160)):
                                    assert abs(x[col]-y[col]) <= tolerance, (case, col, x[col], y[col])
                                count += 1
    finally:
        fa.close()
        us.close()
    print(f'Passed {count} US/Persian nasal onset, coda and resonance comparisons.')


if __name__ == '__main__':
    main()
