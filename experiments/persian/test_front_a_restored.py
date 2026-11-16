"""Verify the isolated Persian front vowel against the dev121 build."""
from __future__ import annotations

import argparse
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--dev121', required=True, type=Path)
    args = parser.parse_args()
    current, scope = voice(args.candidate, 0x90000)
    old, old_scope = voice(args.dev121, 0x90000)
    count = 0
    try:
        for rate in range(7):
            for gender in (0, 1):
                for speed in (40, 98, 170):
                    for engine in (current, old):
                        for parameter, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                                 (NATIVE_SPEED, speed)):
                            engine.set_parameter(parameter, value)
                    for tag in ('`[.1A]', "`[.1'A:']"):
                        actual, _ = capture(current, scope, tag, True)
                        before, _ = capture(old, old_scope, tag, True)
                        assert actual.pcm == before.pcm, (rate, gender, speed, tag)
                        count += 1
    finally:
        current.close()
        old.close()
    print(f'Passed {count} isolated dev121 front-vowel waveform comparisons.')


if __name__ == '__main__':
    main()
