"""Check contextual timing of initial unstressed I without changing its sound."""
import argparse
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, GENDER, HEAD_SIZE, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from test_initial_vowel_timing import capture_sequence
from test_persian_r_after_i_contexts import at_fraction, capture
from test_us_articulation import voice


CASES = (
    ("`[.0'i:'.0ta.0li.1a]", '`[.0i.0ta.0li.1a]', 0),
    ("`[.0'i:'.0ta.0li.0a.1'i:']", "`[.0i.0ta.0li.0a.1'i:']", 0),
    ("`[.0'i:'.0ta.1ra]", '`[.0i.0ta.1ra]', 0),
    ("`[.0m'i:'.0na.1su]", '`[.0mi.0na.1su]', 1),
    ("`[.0t'i:'.0ka.1ra]", '`[.0ti.0ka.1ra]', 1),
)
CONTROLS = (
    "`[.0'i:'.1r'a:']",                 # Two syllables, as in Iran.
    "`[.1'i:'.0ta.0li.0a]",            # Initial syllable is stressed.
    "`[.0ba.0t'i:'.1a]",                # I is not the initial nucleus.
    "`[.0k'u:'.0ta.1la]",               # Previously selected U timing.
    "`[.0'A:'.0ta.1la]",                # Previously selected A timing.
)


def duration(result, index):
    first, last = result.syllables[index]
    return (last - first) / 10000.0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    translate = current._dll.eciPersianTranslateUtf8
    translate.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                          ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_size_t))
    pinglish = current._dll.eciPersianPinglishWord
    pinglish.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    current._dll.eciPersianRelease.argtypes = (ctypes.c_void_p,)
    actual_tags = []
    for text, roman in (('ایتالیا', 'italia'), ('ایتالیایی', 'italiai')):
        raw = text.encode('utf8'); output = ctypes.c_void_p(); mapping = ctypes.c_void_p(); error = ctypes.c_size_t()
        assert translate(raw, len(raw), ctypes.byref(output), ctypes.byref(mapping), ctypes.byref(error)) == 0
        try:
            tag = ctypes.string_at(output).decode('ascii')
        finally:
            current._dll.eciPersianRelease(output); current._dll.eciPersianRelease(mapping)
        encoded = roman.encode('ascii'); target = ctypes.create_string_buffer(2048)
        assert pinglish(encoded, len(encoded), target, len(target)) == 1
        assert target.value.decode('ascii') == tag and "'i:'" in tag, (text, roman, tag)
        actual_tags.append((tag, tag.replace("'i:'", 'i', 1)))
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender, head in ((0, 50), (1, 20)):
                for speed, clock in ((40, 0), (65, 0), (82, 0), (82, 1), (100, 0)):
                    for engine in (current, old):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender), (HEAD_SIZE, head),
                                               (NATIVE_SPEED, speed), (ORIGINAL_ACOUSTIC_CLOCK, clock)):
                                engine.set_parameter(key, value)
                    for long_tag, short_tag, index in CASES:
                        actual = capture(current, scope, long_tag)
                        short = capture(current, scope, short_tag)
                        assert duration(actual, index) == duration(short, index), (
                            long_tag, rate, gender, speed, duration(actual, index), duration(short, index))
                        if old:
                            before = capture(old, old_scope, long_tag)
                            assert duration(actual, index) < duration(before, index), (
                                long_tag, rate, gender, speed, duration(actual, index), duration(before, index))
                            for column in (9, 13, 15, 17):
                                assert abs(at_fraction(actual, index, .5, column) -
                                           at_fraction(before, index, .5, column)) <= 50, (long_tag, column)
                        checks += 1
                    for long_tag, short_tag in actual_tags:
                        actual, short = capture(current, scope, long_tag), capture(current, scope, short_tag)
                        assert duration(actual, 0) == duration(short, 0), (long_tag, rate, gender, speed)
                    if old:
                        for tag in CONTROLS:
                            actual = capture(current, scope, tag)
                            before = capture(old, old_scope, tag)
                            assert actual.pcm == before.pcm and actual.frame_bytes == before.frame_bytes, (
                                tag, rate, gender, speed, 'outside contextual timing rule')
                        sequence = ('`[.1fa] ', EciIndex(51), CASES[0][0])
                        actual = capture_sequence(current, scope, sequence)
                        before = capture_sequence(old, old_scope, sequence)
                        assert [mark for mark, _ in actual.indexes] == [51]
                        assert duration(actual, 2) < duration(before, 2), (rate, gender, speed)
                        sequence = ('`[.1fa] ', EciIndex(52), CONTROLS[0])
                        actual = capture_sequence(current, scope, sequence)
                        before = capture_sequence(old, old_scope, sequence)
                        assert actual.pcm == before.pcm and actual.frame_bytes == before.frame_bytes, (
                            rate, gender, speed, 'a preceding word must not change Iran')
    finally:
        current.close()
        if old:
            old.close()
    print(f'Passed {checks} contextual long-word timing and vowel quality cases.')


if __name__ == '__main__':
    main()
