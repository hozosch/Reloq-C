"""Check source pitch changes at coarticulated word boundaries."""
import argparse
from pathlib import Path
import array

from _reloq_eci_direct import (DirectEciEngine, EciIndex, NATIVE_INFLECTION,
                               NATIVE_SPEED, PITCH_GLIDE_MS, PROCESS_TAGS,
                               SAMPLE_RATE)
from test_eci_german_clock_native import render
from test_eci_inline_queue import TEXTS

RATES = {0: 8000, 1: 11025, 3: 16000, 2: 22050, 4: 32000, 5: 44100, 6: 48000}


def first_difference(a, b):
    left, right = array.array('h'), array.array('h')
    left.frombytes(a)
    right.frombytes(b)
    for i, (x, y) in enumerate(zip(left, right)):
        if x != y:
            return i
    return None


def engine(path, language, rate=3, speed=30):
    result = DirectEciEngine(path, dialect=language, encoding='utf8', native_voice=True)
    for key, value in ((PROCESS_TAGS, 1), (SAMPLE_RATE, rate),
                       (NATIVE_SPEED, speed), (NATIVE_INFLECTION, 0)):
        result.set_parameter(key, value)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    checks = 0
    for rate, hz in RATES.items():
        for speed in (20, 30, 50, 80, 100):
            current = engine(args.library, 0x40000, rate, speed)
            try:
                current.set_parameter(PITCH_GLIDE_MS, 0)
                prefix = '`vf0 `vbhz125 ma '
                low, marks = render(current, [prefix, EciIndex(1), '`vbhz185 ma'])
                high, other_marks = render(current, [prefix, EciIndex(1), '`vbhz190 ma'])
                assert marks == other_marks and [n for n, _ in marks] == [1]
                assert len(low) == len(high)
                first = first_difference(low, high)
                assert first is not None
                start = marks[0][1] // 2
                # The source mark is the beginning of the preceding vowel's
                # coarticulation. Its pitch must remain stable until the
                # projected transition, at both slow and fast speech rates.
                assert 0.004 <= (first - start) / hz < 0.080, (rate, speed, first, start)
                current.set_parameter(PITCH_GLIDE_MS, 80)
                glided, glided_marks = render(current, [prefix, EciIndex(1), '`vbhz185 ma'])
                assert glided_marks == marks and len(glided) == len(low)
                assert low[:start * 2] == glided[:start * 2] and low != glided
                checks += 4
            finally:
                current.close()

    cases = [(language, left, word + suffix)
             for language, (left, word, suffix) in TEXTS.items()]
    cases.append((0x130000, 'A ', 'voz ye clara.'))
    for language, left, right in cases:
        current = engine(args.library, language)
        previous = engine(args.baseline, language) if args.baseline else None
        try:
            plain = left + right
            if previous:
                assert render(current, [plain]) == render(previous, [plain]), (hex(language), 'plain speech')
                checks += 1
            # Two real changes preserve the same native syntax, so the first
            # PCM difference identifies the pitch itself rather than tokenization.
            prefix = '`vf0 `vbhz125 ' + left + '`vbhz'
            for duration in (0, 80):
                current.set_parameter(PITCH_GLIDE_MS, duration)
                low, _ = render(current, [prefix + '185 ' + right])
                high, _ = render(current, [prefix + '190 ' + right])
                assert low and len(low) == len(high), (hex(language), duration, 'duration')
                change = first_difference(low, high)
                assert change is not None and 0 < change < len(low) // 2, (hex(language), duration, 'pitch')
                checks += 2
        finally:
            current.close()
            if previous:
                previous.close()
    # Native CJK VB tags and exact-Hz tags must start at the same acoustic
    # transition; a native VB command must not leak through earlier.
    for language, left, right in ((0x60000, '你好', '世界'),
                                  (0x80000, '今日は', '世界'),
                                  (0xa0000, '안녕하세요', '세상')):
        current = engine(args.library, language)
        try:
            for duration in (0, 80):
                current.set_parameter(PITCH_GLIDE_MS, duration)
                hz_prefix = '`vf0 `vbhz125 ' + left + ' `vbhz'
                vb_prefix = '`vf0 `vb65 ' + left + ' `vb'
                low_hz, _ = render(current, [hz_prefix + '185 ' + right])
                high_hz, _ = render(current, [hz_prefix + '190 ' + right])
                low_vb, _ = render(current, [vb_prefix + '70 ' + right])
                high_vb, _ = render(current, [vb_prefix + '90 ' + right])
                hz_start, vb_start = first_difference(low_hz, high_hz), first_difference(low_vb, high_vb)
                assert len(low_hz) == len(high_hz) and len(low_vb) == len(high_vb)
                assert hz_start is not None and vb_start is not None
                assert abs(hz_start - vb_start) <= 16_000 * 0.015, (hex(language), duration, hz_start, vb_start)
                checks += 3
        finally:
            current.close()
    print(f'Pitch boundaries: {checks} rate, language, timing and PCM checks passed')


if __name__ == '__main__':
    main()
