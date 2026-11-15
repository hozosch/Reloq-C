"""Compare Persian /p t k/ after nasals with the previous scoped voice."""
from __future__ import annotations

import argparse
from pathlib import Path

from test_windows_acoustics import render
from _reloq_eci_direct import SAMPLE_RATES


NASAL_STOPS = (
    '`[.1am.0per]',      # amper: /m p/
    '`[.0em.1te.0han]',  # emtehan: /m t/
    '`[.1em.0kan]',      # emkan: /m k/
    '`[.0ran.1pa]',      # /n p/
    '`[.0en.1te.0Gab]',  # entekhab: /n t/
    '`[.1en.0kar]',      # enkar: /n k/
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--candidate', required=True, type=Path)
    args = parser.parse_args()
    checked = 0
    for rate in (1, 3, 6):
        for gender in (0, 1):
            for tag in NASAL_STOPS:
                old = render(args.previous, 0x90000, tag, scoped=True, rate=rate, gender=gender)
                new = render(args.candidate, 0x90000, tag, scoped=True, rate=rate, gender=gender)
                duration_ms = (len(new) - len(old)) * 500 / SAMPLE_RATES[rate]
                assert old != new and 0 < duration_ms <= 100, (tag, rate, gender, duration_ms)
                checked += 1
            # Other clusters and the accepted initial and intervocalic paths
            # must keep their existing acoustics exactly.
            for tag in ('`[.1skA]', '`[.1noR.0te]', '`[.1ka.0RAz]', '`[.1sa.0ka]',
                        '`[.0em.1da]', '`[.0en.1ga]'):
                old = render(args.previous, 0x90000, tag, scoped=True, rate=rate, gender=gender)
                new = render(args.candidate, 0x90000, tag, scoped=True, rate=rate, gender=gender)
                assert old == new, (tag, rate, gender)
            for language, text in ((0x90000, 'viisi.'), (0x40000, 'Guten Tag.'),
                                   (0x10000, 'important.')):
                old = render(args.previous, language, text, scoped=False, rate=rate, gender=gender)
                new = render(args.candidate, language, text, scoped=False, rate=rate, gender=gender)
                assert old == new, (language, rate, gender)
    print(f'Passed {checked} post-nasal stop checks and unaffected context isolation.')


if __name__ == '__main__':
    main()
