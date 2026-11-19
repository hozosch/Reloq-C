"""Check reported first-vowel timing without replacing Persian vowel quality."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, GENDER, HEAD_SIZE, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
from test_persian_r_after_i_contexts import at_fraction, capture
from test_us_articulation import voice


CASES = (
    ('کوچک', "`[.0k'u:'.1CAk]", 1, (55, 70)),
    ('کوچکتر', "`[.0k'u:'.0CAk.1tAr]", 1, (53, 68)),
    ('کودک', "`[.0k'u:'.1dAk]", 1, (62, 78)),
    ('اصغر', "`[.0'A:'s.1RAr]", 0, (60, 75)),
    ('اکبر', "`[.0'A:'k.1bAr]", 0, (60, 75)),
    ('اخگر', "`[.0'A:'G.1gAr]", 0, (73, 89)),
    ('احمد', "`[.0'A:'h.1mAd]", 0, (82, 98)),
)
CONTROLS = ("`[.0f'u:'t.1bal]", '`[.0ma.1dAr]', "`[.1'A:'s.0RAr]",
            "`[.1k'u:'.0CAk]", "`[.1k'u:'k]", "`[.0k'u:'k]",
            "`[.0k'u:'k] `[.1da]",
            "`[.0ka.0m'u:'.1dAr]", "`[.1f'u:'t] `[.1bal]",
            "`[.0ti.1r'e:']", '`[.1zir.0GAt]', '`[.1ir]')


def vowel_duration(result, index):
    start, end = result.syllables[index]
    return (end - start) / 10000.0


def midpoint(result, index):
    return tuple(at_fraction(result, index, .5, col) for col in (9, 13, 15, 17))


def capture_sequence(engine, scope, sequence):
    previous = scope(1)
    try:
        return FrameCapture(engine._dll).capture(engine, sequence, phone_segments=True)
    finally:
        assert scope(previous) == 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='optional immediate pre-change build')
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    count = 0
    try:
        for rate in (1, 3, 6):
            for gender, head in ((0, 50), (1, 50), (1, 20)):
                for speed, clock in ((40, 0), (82, 0), (82, 1), (100, 0)):
                    for engine in (current, old):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                               (HEAD_SIZE, head), (NATIVE_SPEED, speed),
                                               (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                                engine.set_parameter(key, value)
                    for name, tag, index, limits in CASES:
                        candidate = capture(current, scope, tag)
                        duration = vowel_duration(candidate, index)
                        if speed == 82:
                            assert limits[0] <= duration <= limits[1], (name, rate, gender, head, clock, duration)
                        if old:
                            previous = capture(old, old_scope, tag)
                            before = vowel_duration(previous, index)
                            assert .65 < duration / before < .96, (name, rate, gender, head, speed, clock, before, duration)
                            a, b = midpoint(previous, index), midpoint(candidate, index)
                            # Durations change which time sample is nearest
                            # the vowel centre; no formant target is replaced.
                            assert all(abs(x-y) <= 12 for x, y in zip(a, b)), (name, a, b)
                        count += 1
                    # A vowel ending the preceding word must not make this
                    # word's first syllable look word-internal.
                    sequence = ('`[.1fa] ', EciIndex(51), "`[.0k'u:'.1CAk]")
                    after = capture_sequence(current, scope, sequence)
                    assert [mark for mark, _ in after.indexes] == [51]
                    if speed == 82:
                        assert 55 <= vowel_duration(after, 3) <= 70, (rate, gender, head, clock, 'word after a vowel')
                        for tag, index, bounds in (("`[.0k'u:'k]", 1, (90, 103)),
                                                   ("`[.0ka.0m'u:'.1dAr]", 3, (60, 75))):
                            control = capture(current, scope, tag)
                            assert bounds[0] <= vowel_duration(control, index) <= bounds[1], (
                                rate, gender, head, clock, tag, 'outside first-vowel context')
                    if old:
                        prior = capture_sequence(old, old_scope, sequence)
                        assert vowel_duration(after, 3) < vowel_duration(prior, 3), (
                            rate, gender, head, speed, clock, 'word after a vowel')
                        for tag in CONTROLS:
                            a = capture(old, old_scope, tag)
                            b = capture(current, scope, tag)
                            assert a.pcm == b.pcm and a.frame_bytes == b.frame_bytes, (
                                tag, rate, gender, head, speed, clock, 'outside timing context changed')
                        # Other language scopes must not inherit the rule.
                        a = capture(old, old_scope, CASES[0][1], False)
                        b = capture(current, scope, CASES[0][1], False)
                        assert a.pcm == b.pcm and a.frame_bytes == b.frame_bytes
    finally:
        current.close()
        if old:
            old.close()
    print(f'Passed {count} first-vowel timing, quality and context cases.')


if __name__ == '__main__':
    main()
