"""Shortened ECI pauses retain the spoken Mandarin-voice contour and release."""
from pathlib import Path
import argparse
from array import array
from math import log2, sqrt
import sys

from _reloq_eci_direct import (
    DirectEciEngine, EciIndex, GENDER, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK,
    PAUSE_MODE, PROCESS_TAGS, SAMPLE_RATE,
)


def pitch_frames(pcm):
    """Estimate voiced F0 at 10 ms steps for the 11.025 kHz test cases."""
    samples = array('h')
    samples.frombytes(pcm)
    if sys.byteorder != 'little':
        samples.byteswap()
    audio = list(samples[::2])
    width, step = 220, 55  # 40 ms / 10 ms after 2:1 decimation.
    result = {}
    for start in range(0, len(audio) - width, step):
        segment = audio[start:start + width]
        mean = sum(segment) / width
        window = [x - mean for x in segment]
        squares = [x * x for x in window]
        if sum(squares) < width * 60 * 60:
            continue
        cumulative = [0.0]
        for value in squares:
            cumulative.append(cumulative[-1] + value)
        correlations = []
        for lag in range(14, 80):
            left = cumulative[width - lag]
            right = cumulative[width] - cumulative[lag]
            denominator = sqrt(left * right) if left > 0 and right > 0 else 0
            correlations.append(sum(a * b for a, b in zip(window[:-lag], window[lag:])) /
                                denominator if denominator else 0)
        peaks = [i for i in range(1, len(correlations) - 1)
                 if correlations[i] > correlations[i - 1]
                 and correlations[i] >= correlations[i + 1] and correlations[i] > .6]
        if not peaks:
            continue
        best = max(correlations[i] for i in peaks)
        chosen = next(i for i in peaks if correlations[i] >= best * .88)
        result[start] = (11025 / 2) / (14 + chosen)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    count = pitch_count = 0
    for language, encodings in ((0x60000, ('utf8', 'gb18030')), (0x60001, ('utf8',))):
        for encoding in encodings:
            engine = DirectEciEngine(args.library.resolve(), dialect=language,
                                     encoding=encoding, native_voice=True)
            try:
                for key, value in ((PROCESS_TAGS, 1), (PAUSE_MODE, 0),
                                   (ORIGINAL_ACOUSTIC_CLOCK, 1)):
                    engine.set_parameter(key, value)
                for sample_rate in (1, 3):
                    engine.set_parameter(SAMPLE_RATE, sample_rate)
                    for speed in (50, 95):
                        engine.set_parameter(NATIVE_SPEED, speed)
                        for gender in (0, 1):
                            engine.set_parameter(GENDER, gender)

                            def render(parts):
                                pcm, marks = bytearray(), []
                                assert engine.speak(parts, pcm.extend,
                                                    lambda index: marks.append((index, len(pcm))))
                                return bytes(pcm), marks

                            # The p1 at a final punctuation mark must remove
                            # only silence, like the 32-bit ECI host. Include
                            # an English run both alone and after native text.
                            for text, punctuation in (('Hello', '!'), ('Hello', '?'),
                                                      ('你好，Hello', '!'), ('Tonhöhe', '.')):
                                normal = render([text, f' `p0{punctuation}', EciIndex(9)])
                                shortened = render([text, f' `p1{punctuation}', EciIndex(9)])
                                assert shortened[0] and len(shortened[0]) < len(normal[0]), (
                                    hex(language), encoding, sample_rate, speed, gender, text)
                                # A p1 command may differ by the last 5 ms
                                # of the release/silence boundary. Everything
                                # preceding it, including the voiced contour,
                                # must remain byte-identical.
                                sample_hz = 11025 if sample_rate == 1 else 22050
                                guard_bytes = 2 * (sample_hz // 200)
                                assert len(shortened[0]) > guard_bytes
                                assert normal[0].startswith(shortened[0][:-guard_bytes]), (
                                    hex(language), encoding, sample_rate, speed, gender, text,
                                    len(normal[0]), len(shortened[0]))
                                assert shortened[1] == [(9, len(shortened[0]))]
                                # ECI fragmentation must not change the voiced
                                # samples or the final index callback.
                                joined = render([text + f' `p1{punctuation}', EciIndex(9)])
                                assert joined == shortened
                                count += 1
                            if sample_rate == 1:
                                # ECI p1 at every mark should follow the same
                                # timing and F0 contour as our confirmed native
                                # all-pauses mode, including English/Chinese
                                # switches. Compare measured voiced frames; raw
                                # PCM can legitimately differ at transitions.
                                for plain, tagged in (
                                    ('Hello, world!', 'Hello `p1, world `p1!'),
                                    ('one, two, three.', 'one `p1, two `p1, three `p1.'),
                                    ('你好, Hello!', '你好 `p1, Hello `p1!'),
                                    ('Hello, 你好.', 'Hello `p1, 你好 `p1.'),
                                    ('Tonhöhe, Lautstärke.', 'Tonhöhe `p1, Lautstärke `p1.'),
                                    ('你好，世界！', '你好 `p1，世界 `p1！'),
                                    ('这是一个测试。欢迎使用。', '这是一个测试 `p1。欢迎使用 `p1。'),
                                    ('中文，测试，句子。', '中文 `p1，测试 `p1，句子 `p1。'),
                                    ('你好,世界!', '你好 `p1,世界 `p1!'),
                                ):
                                    engine.set_parameter(PAUSE_MODE, 2)
                                    native = render([plain, EciIndex(9)])[0]
                                    engine.set_parameter(PAUSE_MODE, 0)
                                    eci = render([tagged, EciIndex(9)])[0]
                                    assert abs(len(native) - len(eci)) <= 2 * (11025 // 200), (
                                        hex(language), encoding, speed, gender, plain,
                                        len(native), len(eci))
                                    left, right = pitch_frames(native), pitch_frames(eci)
                                    common = left.keys() & right.keys()
                                    deviations = sorted(abs(12 * log2(left[k] / right[k]))
                                                        for k in common)
                                    assert len(deviations) >= 4, (plain, 'insufficient voiced frames')
                                    assert deviations[int(.9 * (len(deviations) - 1))] < 1, (
                                        hex(language), encoding, speed, gender, plain, deviations)
                                    pitch_count += 1
            finally:
                engine.close()
    print(f'Passed {count} Mandarin final-index checks and {pitch_count} F0-contour comparisons.')


if __name__ == '__main__':
    main()
