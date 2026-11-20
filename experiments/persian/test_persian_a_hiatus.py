"""Check a short weak back A between vowels before a stressed vowel."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from test_persian_r_after_i_contexts import capture, middle
from test_us_articulation import voice


CASES = (
    ("`[.0'i:'.0ta.0li.0a.1'i:']", 5),
    ("`[.0ma.0i.0a.1'u:']", 3),
    ('`[.0me.0a.1i]', 2),
    ("`[.0mu.0a.1'e:']", 2),
)
CONTROLS = (
    "`[.0'i:'.0ta.0li.1a]",  # stressed and final, as in italia
    "`[.0mi.0'a:'.1'i:']",   # long A
    "`[.0mi.1a.0'i:']",      # stressed A
    "`[.0mi.0a.0'i:']",      # no following stressed vowel
    "`[.0mi] `[.0a.1'i:']",  # real word boundary
    "`[.0ma.1'i:']",         # no preceding vowel
    "`[.0'i:'.1r'a:']",      # Iranian A and R
)


def duration(item, index):
    start, end = item.syllables[index]
    return end - start


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checked = 0
    try:
        for rate in (1, 3, 6):
            for gender, head in ((0, 50), (1, 50), (1, 20)):
                for speed, clock in ((40, 0), (65, 0), (82, 0), (82, 1), (100, 0)):
                    for engine in (current, previous):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (HEAD_SIZE, head),
                                               (NATIVE_SPEED, speed), (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                                engine.set_parameter(key, value)
                    for tag, index in CASES:
                        result = capture(current, scope, tag)
                        assert duration(result, index) > 0, (tag, rate, speed)
                        if previous:
                            old = capture(previous, old_scope, tag)
                            assert duration(result, index) < duration(old, index), (tag, rate, gender, speed, clock)
                            for adjacent in (index - 1, index + 1):
                                assert duration(result, adjacent) == duration(old, adjacent), (tag, adjacent, rate, speed)
                            a, b = middle(result, index, (9, 13, 15)), middle(old, index, (9, 13, 15))
                            assert max(abs(x - y) for x, y in zip(a, b)) <= 60, (tag, rate, speed, a, b)
                            plain_old = capture(previous, old_scope, tag, False)
                            plain_new = capture(current, scope, tag, False)
                            assert plain_old.pcm == plain_new.pcm and plain_old.frame_bytes == plain_new.frame_bytes
                        checked += 1
                    if previous:
                        for tag in CONTROLS:
                            old, result = capture(previous, old_scope, tag), capture(current, scope, tag)
                            assert old.pcm == result.pcm and old.frame_bytes == result.frame_bytes, (tag, rate, gender, speed, clock)
    finally:
        current.close()
        if previous:
            previous.close()
    print(f'Passed {checked} weak-A contexts, adjacent vowels, resonance and unchanged controls.')


if __name__ == '__main__':
    main()
