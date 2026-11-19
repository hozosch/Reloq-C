"""Check compact unstressed /i/ between vowels before a stressed nucleus."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from test_persian_r_after_i_contexts import capture
from test_us_articulation import voice


CASES = (
    ("`[.1Ga.0J'e:'.0'i:'.1an]", "`[.1Ga.0J'e:'.0i.1an]", 5),
    ("`[.1Ga.0n'e:'.0'i:'.1an]", "`[.1Ga.0n'e:'.0i.1an]", 4),
    ("`[.0me.0'i:'.1on]", '`[.0me.0i.1on]', 2),
    ("`[.0ma.0'i:'.1en]", '`[.0ma.0i.1en]', 2),
    ("`[.0mo.0'i:'.1an]", '`[.0mo.0i.1an]', 2),
    ("`[.0mu.0'i:'.1an]", '`[.0mu.0i.1an]', 2),
)
CONTROLS = (
    "`[.1Ga.0J'e:'.1'i:'.0an]",  # The /i/ itself carries stress.
    "`[.1Ga.0J'e:'.0'i:'.0an]",  # No following stressed vowel.
    "`[.1Ga.0J'e:'] `[.0'i:'.1an]",  # A real word boundary.
    "`[.0m'i:'.1an]",  # A long /i/ after a consonant.
    '`[.0mi.1an]',
    "`[.0f'u:'t.1bal]",
    '`[.1zir.0GAt]',
)


def duration(capture_result, index):
    first, last = capture_result.syllables[index]
    return last - first


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    count = 0
    try:
        for rate in (1, 3, 6):
            for gender, head in ((0, 50), (1, 50), (1, 20)):
                for speed, clock in ((40, 0), (82, 0), (82, 1), (100, 0), (140, 0)):
                    for engine in (current, old):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (HEAD_SIZE, head),
                                               (NATIVE_SPEED, speed), (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                                engine.set_parameter(key, value)
                    for long_tag, short_tag, index in CASES:
                        result = capture(current, scope, long_tag)
                        short = capture(current, scope, short_tag)
                        measured = duration(result, index)
                        assert 0 < measured == duration(short, index), (long_tag, rate, gender, speed, clock)
                        if old:
                            previous = capture(old, old_scope, long_tag)
                            assert measured < duration(previous, index), (long_tag, rate, gender, speed, clock)
                            # The rule changes the hiatus nucleus, not the
                            # duration of either neighbouring vowel.
                            for adjacent in (index - 1, index + 1):
                                assert duration(result, adjacent) == duration(previous, adjacent), (
                                    long_tag, rate, gender, speed, clock, adjacent)
                            plain_before = capture(old, old_scope, long_tag, False)
                            plain_after = capture(current, scope, long_tag, False)
                            assert plain_before.pcm == plain_after.pcm
                            assert plain_before.frame_bytes == plain_after.frame_bytes
                        count += 1
                    for long_tag, short_tag in (
                            ("`[.1Ga.0J'e:'.1'i:'.0an]", "`[.1Ga.0J'e:'.1i.0an]"),
                            ("`[.1Ga.0J'e:'] `[.0'i:'.1an]", "`[.1Ga.0J'e:'] `[.0i.1an]")):
                        long_result = capture(current, scope, long_tag)
                        short_result = capture(current, scope, short_tag)
                        assert duration(long_result, 5) > duration(short_result, 5), (
                            long_tag, rate, gender, speed, clock)
                    if old:
                        for tag in CONTROLS:
                            before = capture(old, old_scope, tag)
                            after = capture(current, scope, tag)
                            assert before.pcm == after.pcm and before.frame_bytes == after.frame_bytes, (
                                tag, rate, gender, speed, clock)
    finally:
        current.close()
        if old:
            old.close()
    print(f'Passed {count} scoped hiatus timings, adjacent vowels and boundary controls.')


if __name__ == '__main__':
    main()
