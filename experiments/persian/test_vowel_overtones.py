"""Check the /u/ restoration and /o/ after nasals against the 'two' reference."""
import argparse
from pathlib import Path

from test_british_front_a import spectrum
from test_open_vowels import nucleus
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def u_mid(engine, scope, gender):
    _, rows = capture(engine, scope, '`[.1u]', True)
    target = 299 if gender else 280
    selected = [row for row in rows if row.controls[2] > 25 and row.controls[8] < 2 and
                abs(row.controls[9] - target) < 2]
    assert selected
    return selected[len(selected) // 2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--dev121', required=True, type=Path)
    args = parser.parse_args()
    new, ns = voice(args.candidate, 0x90000)
    old, os = voice(args.previous, 0x90000)
    restored, rs = voice(args.dev121, 0x90000)
    checks = 0
    try:
        for gender in (0, 1):
            for rate in (1, 3, 6):
                for speed in (40, 98, 170):
                    for engine in (new, old, restored):
                        for key, value in ((GENDER, gender), (SAMPLE_RATE, rate), (NATIVE_SPEED, speed)):
                            engine.set_parameter(key, value)
                    now, baseline = u_mid(new, ns, gender), u_mid(restored, rs, gender)
                    for column in (9, 10, 13, 14, 15):
                        assert abs(now.controls[column] - baseline.controls[column]) < 2, (
                            gender, rate, speed, column, now.controls[column], baseline.controls[column])
                    assert now.controls[16] < baseline.controls[16]
                    checks += 1
                    reference = nucleus(new, ns, '`[.1do]', gender, 'o')
                    for word in ('no', 'mo', 'nos', 'mos'):
                        after = nucleus(new, ns, '`[.1' + word + ']', gender, 'o')
                        assert abs(after.controls[4] - reference.controls[4]) < 1, (gender, rate, speed, word)
                        if word in ('no', 'mo'):
                            assert abs(after.controls[17] - reference.controls[17]) < 2, (gender, word)
                        if rate == 3 and speed == 98:
                            before = nucleus(old, os, '`[.1' + word + ']', gender, 'o')
                            goal = spectrum(new, ns, reference, True, gender)
                            actual = spectrum(new, ns, after, True, gender)
                            previous = spectrum(old, os, before, True, gender)
                            assert abs(actual[4] - goal[4]) < abs(previous[4] - goal[4]), (
                                gender, word, goal, actual, previous)
                        checks += 1
    finally:
        new.close()
        old.close()
        restored.close()
    print(f'Passed {checks} /u/ restoration and nasal-to-/o/ checks.')


if __name__ == '__main__':
    main()
