"""Check contextual shortening of the vowel in yek without altering /k/."""
import argparse
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def e_duration(rows):
    # Approximate voiced, E-like frames; this is not a human segmentation.
    return sum(row.controls[0] for row in rows if row.controls[2] > 25 and
               row.controls[8] < 5 and 400 <= row.controls[9] <= 600 and
               row.controls[13] > 1700)


def e_nucleus_duration(rows, gender):
    """Measure the stable E nucleus, excluding the voiced US-like /j/."""
    f1, f2 = ((495, 2420) if gender else (450, 2000))
    return sum(row.controls[0] for row in rows if row.controls[2] > 25 and
               row.controls[8] < 5 and abs(row.controls[9] - f1) < 20 and
               abs(row.controls[13] - f2) < 60)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    fa, scope = voice(args.candidate, 0x90000)
    old, prior_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        for gender in (0, 1):
            for rate in (1, 3, 6):
                for speed in (40, 98, 170):
                    for engine in (fa, old):
                        if engine:
                            for setting, value in ((GENDER, gender), (SAMPLE_RATE, rate),
                                                   (NATIVE_SPEED, speed)):
                                engine.set_parameter(setting, value)
                    durations, nuclei = {}, {}
                    for tag in ('`[.1jek]', '`[.0jek]', '`[.1je.0ki]',
                                '`[.1tek]', '`[.1lek]', '`[.1sefr]'):
                        current, rows = capture(fa, scope, tag, True)
                        duration = e_duration(rows)
                        assert duration > 0, (gender, rate, speed, tag)
                        durations[tag] = duration
                        nuclei[tag] = e_nucleus_duration(rows, gender)
                        if old:
                            previous, prior = capture(old, prior_scope, tag, True)
                            before = e_duration(prior)
                            if tag in ('`[.1jek]', '`[.0jek]'):
                                assert duration <= before, (gender, rate, speed, tag, duration, before)
                                if speed == 98:
                                    assert duration < before, (gender, rate, tag, duration, before)
                                assert duration >= before * .7, (tag, duration, before)
                                assert len(current.pcm) <= len(previous.pcm), (tag, gender, rate, speed)
                            else:
                                # The general long-vowel cap also affects slow
                                # speech; do not mistake it for the yek rule.
                                assert duration >= before * .8, (tag, duration, before)
                        checks += 1
                    assert nuclei['`[.1jek]'] > 0 and nuclei['`[.1tek]'] > 0
                    assert nuclei['`[.1jek]'] <= nuclei['`[.1tek]'] + 3, (
                        gender, rate, speed, nuclei)
    finally:
        fa.close()
        if old:
            old.close()
    print(f'Passed {checks} contextual Persian E duration checks.')


if __name__ == '__main__':
    main()
