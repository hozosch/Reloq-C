"""Check the Persian R release into back A across voices and timing modes."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
from test_us_articulation import capture, voice


def phones(engine, scope, tag):
    previous = scope(1)
    try:
        result = FrameCapture(engine._dll).capture(engine, (tag,), phone_segments=True)
        return result, result.frames()
    finally:
        scope(previous)


def edge(rows, interval, first):
    a, b = (x / 10000 for x in interval)
    within = [row for row in rows if a <= row.tick <= b]
    assert within, interval
    return within[0] if first else within[-1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    current, scope = voice(args.candidate, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000)
    cases = (("`[.1ra]", 0, 1), ("`[.1r'a:']", 0, 1),
             ("`[.0'i:'.1r'a:'n]", 1, 2), ('`[.0ta.1ra]', 2, 3))
    controls = ('`[.1ri]', '`[.1ro]', '`[.1ru]', '`[.1rA]', '`[.1pa]', '`[.1ir]')
    checks = improved = 0
    try:
        for gender in (0, 1):
            for head in (22, 50, 80):
                for rate in (1, 3, 6):
                    for speed in (55, 98):
                        for engine in (current, previous):
                            for key, value in ((GENDER, gender), (HEAD_SIZE, head),
                                               (SAMPLE_RATE, rate), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                        for tag, r_index, a_index in cases:
                            new, nr = phones(current, scope, tag)
                            old, br = phones(previous, old_scope, tag)
                            assert new.syllables == old.syllables and len(new.pcm) == len(old.pcm), tag
                            r_new, r_old = edge(nr, new.syllables[r_index], False), edge(br, old.syllables[r_index], False)
                            a_new, a_old = edge(nr, new.syllables[a_index], True), edge(br, old.syllables[a_index], True)
                            assert all(abs(a_new.controls[c]-a_old.controls[c]) < 0.01 for c in (9, 13, 15, 17)), tag
                            for column in (15, 17):
                                before = abs(r_old.controls[column] - a_old.controls[column])
                                after = abs(r_new.controls[column] - a_new.controls[column])
                                assert after <= before + 1, (gender, head, rate, speed, tag, column, before, after)
                                improved += after + 50 < before
                            checks += 1
                        for tag in controls:
                            assert capture(current, scope, tag, True)[0].pcm == \
                                   capture(previous, old_scope, tag, True)[0].pcm, (gender, head, rate, speed, tag)
                            checks += 1
        assert improved >= 100, improved
        print(f'Passed {checks} R-to-back-A and unaffected-vowel comparisons; {improved} upper-formant gaps narrowed.')
    finally:
        current.close()
        previous.close()


if __name__ == '__main__':
    main()
