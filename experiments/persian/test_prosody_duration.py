"""Compare Persian vowel timing to the accepted consonant-era baseline.

The classifier is intentionally an acoustic proxy, not a gold phonetic
alignment. Both recordings use the same phonemes and lexical stress.
"""
import argparse
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def vowel_duration(rows):
    return sum(row.controls[0] for row in rows if row.controls[2] > 25 and
               row.controls[8] < 5 and row.controls[9] > 500)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', type=Path, required=True)
    parser.add_argument('--previous', type=Path, required=True)
    args = parser.parse_args()
    new, ns = voice(args.candidate, 0x90000)
    old, os = voice(args.previous, 0x90000)
    comparisons = increased = 0
    try:
        for gender in (0, 1):
            for rate in (1, 3, 6):
                for speed in (40, 98, 170):
                    for engine in (new, old):
                        for parameter, value in ((GENDER, gender), (SAMPLE_RATE, rate),
                                                 (NATIVE_SPEED, speed)):
                            engine.set_parameter(parameter, value)
                    for tag in ('`[.1pA]', '`[.0pA]', '`[.1tA]', '`[.1kA]',
                                '`[.1mA]', '`[.1nA]', '`[.1Ap]', '`[.1At]',
                                '`[.1Ak]', "`[.1'A:'t]"):
                        actual, frames = capture(new, ns, tag, True)
                        previous, prior = capture(old, os, tag, True)
                        a, b = vowel_duration(frames), vowel_duration(prior)
                        assert a >= b and a > 0, (gender, rate, speed, tag, a, b)
                        assert len(actual.pcm) >= len(previous.pcm), tag
                        increased += a > b
                        comparisons += 1
        assert increased >= comparisons * .9, (increased, comparisons)
        for dialect, text in ((0x90000, 'Viisi sanaa.'), (0x10000, 'A normal sentence.'),
                              (0x40000, 'Ein deutscher Satz.')):
            a, x = voice(args.candidate, dialect)
            b, y = voice(args.previous, dialect)
            try:
                assert capture(a, x, text, False)[0].pcm == capture(b, y, text, False)[0].pcm
            finally:
                a.close()
                b.close()
    finally:
        new.close()
        old.close()
    print(f'Passed {comparisons} scoped Persian vowel-duration comparisons ({increased} longer).')


if __name__ == '__main__':
    main()
