"""Compare the selected initial vowels against the actual short timing."""
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
    ('کوچک', "`[.0k'u:'.1CAk]", '`[.0ku.1CAk]', 1),
    ('کوچکتر', "`[.0k'u:'.0CAk.1tAr]", '`[.0ku.0CAk.1tAr]', 1),
    ('کودک', "`[.0k'u:'.1dAk]", '`[.0ku.1dAk]', 1),
    ('اصغر', "`[.0'A:'s.1RAr]", '`[.0As.1RAr]', 0),
    ('اکبر', "`[.0'A:'k.1bAr]", '`[.0Ak.1bAr]', 0),
    ('اخگر', "`[.0'A:'G.1gAr]", '`[.0AG.1gAr]', 0),
    ('احمد', "`[.0'A:'h.1mAd]", '`[.0Ah.1mAd]', 0),
    ('علی', "`[.0'A:'.1li]", '`[.0A.1li]', 0),
    ('خواجه‌ئیان', "`[.1Ga.0J'e:'.0'i:'.1an]", "`[.1Ga.0J'e:'.0i.1an]", 5),
)
CONTROLS = ("`[.0f'u:'t.1bal]", '`[.0ma.1dAr]', "`[.1'A:'s.0RAr]",
            "`[.1k'u:'.0CAk]", "`[.1k'u:'k]", "`[.0k'u:'k]",
            "`[.0k'u:'k] `[.1da]", "`[.0ka.0m'u:'.1dAr]",
            "`[.1f'u:'t] `[.1bal]", "`[.0ti.1r'e:']", '`[.1ir]',
            "`[.1Ga.0J'e:'.1'i:'.0an]", "`[.1Ga.0J'e:'.0'i:'.0on]",
            "`[.1Ga.0J'e:'] `[.0'i:'.1an]")


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
                for speed, clock in ((40, 0), (65, 0), (82, 0), (82, 1), (100, 0)):
                    for engine in (current, old):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                               (HEAD_SIZE, head), (NATIVE_SPEED, speed),
                                               (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                                engine.set_parameter(key, value)
                    for name, tag, short_tag, index in CASES:
                        candidate = capture(current, scope, tag)
                        short = capture(current, scope, short_tag)
                        duration, reference = vowel_duration(candidate, index), vowel_duration(short, index)
                        if "'A:'" in tag:
                            # The short front A has a shorter base duration
                            # in every context. The explicitly long marker
                            # retains the earlier initial-vowel timing.
                            assert duration > reference, (name, rate, gender, head, speed, clock, duration, reference)
                        else:
                            assert duration == reference, (name, rate, gender, head, speed, clock, duration, reference)
                        if old:
                            previous = capture(old, old_scope, tag)
                            before = vowel_duration(previous, index)
                            assert duration < before, (name, rate, gender, head, speed, clock, before, duration)
                            a, b = midpoint(previous, index), midpoint(candidate, index)
                            assert all(abs(x-y) <= 30 for x, y in zip(a, b)), (name, a, b)
                        count += 1
                    # A preceding word's vowel must not hide the new word's
                    # first vowel or make a one-syllable word eligible.
                    long_seq = ('`[.1fa] ', EciIndex(51), "`[.0k'u:'.1CAk]")
                    short_seq = ('`[.1fa] ', EciIndex(51), '`[.0ku.1CAk]')
                    after = capture_sequence(current, scope, long_seq)
                    reference = capture_sequence(current, scope, short_seq)
                    assert [mark for mark, _ in after.indexes] == [51]
                    assert vowel_duration(after, 3) == vowel_duration(reference, 3)
                    if old:
                        prior = capture_sequence(old, old_scope, long_seq)
                        assert vowel_duration(after, 3) < vowel_duration(prior, 3)
                        for tag in CONTROLS:
                            a = capture(old, old_scope, tag)
                            b = capture(current, scope, tag)
                            assert a.pcm == b.pcm and a.frame_bytes == b.frame_bytes, (
                                tag, rate, gender, head, speed, clock, 'outside timing context changed')
                        for item in (CASES[0], CASES[-1]):
                            a = capture(old, old_scope, item[1], False)
                            b = capture(current, scope, item[1], False)
                            assert a.pcm == b.pcm and a.frame_bytes == b.frame_bytes
    finally:
        current.close()
        if old:
            old.close()
    print(f'Passed {count} exact short-vowel timing, quality and context cases.')


if __name__ == '__main__':
    main()
