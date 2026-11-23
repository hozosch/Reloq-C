"""Check the voiced U-to-final-R transition separately from its noise."""
from __future__ import annotations

import argparse
from pathlib import Path

from test_persian_r_voice_resonance import band_power, components


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    tag = '`[.0zo.1hur]'
    for rate in (1, 3, 6):
        for gender in (0, 1):
            for clock in (0, 1):
                old, old_parts, old_r = components(args.previous, tag, 3, gender, clock, rate)
                new, new_parts, new_r = components(args.library, tag, 3, gender, clock, rate)
                assert old.syllables == new.syllables and old_r == new_r
                assert old_parts[0] == new_parts[0], (rate, gender, clock, 'noise changed')
                if rate == 3:
                    start = old.syllables[2][1] / 10000
                    transition = (start, old_r[0])
                    before = band_power(old_parts[1], transition)
                    after = band_power(new_parts[1], transition)
                    assert before > 0 and after < before * .9, (gender, clock, before, after)
                for a, b in zip(old.frames(), new.frames()):
                    assert a.tick == b.tick
                    assert all(a.controls[c] == b.controls[c] for c in range(62) if c not in (11, 12)), (
                        rate, gender, clock, a.tick, 'source or formant controls changed')
    # A U-R followed by a vowel has its own accepted articulation and must
    # not acquire the final-R transition marker.
    for tag, phone in (('`[.0fu.1ra]', 2), ('`[.1ur] `[.1a]', 1)):
        for gender in (0, 1):
            old, old_parts, _ = components(args.previous, tag, phone, gender, 0)
            new, new_parts, _ = components(args.library, tag, phone, gender, 0)
            assert old.syllables == new.syllables and old_parts == new_parts, (tag, gender)
    print('Passed U-to-final-R voice transition, unchanged noise and vowel-followed R.')


if __name__ == '__main__':
    main()
