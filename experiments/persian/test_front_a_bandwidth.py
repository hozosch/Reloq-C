"""Check the small, scoped dev124 front-vowel resonance adjustment."""
import argparse
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    new, ns = voice(args.candidate, 0x90000)
    old, os = voice(args.previous, 0x90000)
    checks = 0
    try:
        for gender in (0, 1):
            for rate in (1, 3, 6):
                for speed in (40, 98, 170):
                    for engine in (new, old):
                        for parameter, value in ((GENDER, gender), (SAMPLE_RATE, rate), (NATIVE_SPEED, speed)):
                            engine.set_parameter(parameter, value)
                    for text in ('`[.0sAd]', '`[.0bAt]', '`[.0kAt]', "`[.0s'A:'d]"):
                        actual, frames = capture(new, ns, text, True)
                        previous, prior = capture(old, os, text, True)
                        def middle(rows):
                            rows = [row for row in rows if row.controls[2] > 25 and
                                    row.controls[8] < 1 and row.controls[9] > 400]
                            assert rows
                            return rows[len(rows) // 2]
                        mid, before = middle(frames), middle(prior)
                        assert len(actual.pcm) == len(previous.pcm) and actual.pcm != previous.pcm
                        assert all(abs(mid.controls[i] - before.controls[i]) < 1
                                   for i in (9, 10, 13, 14, 15, 17, 18, 19, 20))
                        assert before.controls[16] < mid.controls[16] <= before.controls[16] * 1.3 + 1
                        checks += 1
        for dialect, text in ((0x90000, 'Viisi sanaa.'), (0x10000, 'can'),
                              (0x10001, 'sad'), (0x40000, 'Ein deutscher Satz.')):
            a, x = voice(args.candidate, dialect)
            b, y = voice(args.previous, dialect)
            try:
                assert capture(a, x, text, False)[0].pcm == capture(b, y, text, False)[0].pcm
                checks += 1
            finally:
                a.close()
                b.close()
    finally:
        new.close()
        old.close()
    print(f'Passed {checks} front-vowel, context and isolation checks.')


if __name__ == '__main__':
    main()
