"""The accepted tuning survives old profile settings and changes of host."""
import argparse
import array
import math
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, HIGH_RATE_PROFILE,
    SAMPLE_RATE, SAMPLE_RATES, NATIVE_SPEED, NATIVE_VOLUME, GENDER, HEAD_SIZE, BREATHINESS)
from _reloq_languages import LANGUAGES


def capture(engine, text):
    output, indices = [], []
    size = 0
    def pcm(data):
        nonlocal size
        output.append(data)
        size += len(data)
    assert engine.speak([text, EciIndex(19)], pcm, lambda n: indices.append((n, size)))
    data = b''.join(output)
    assert data and indices == [(19, len(data))]
    return data, indices


def rms(data):
    samples = array.array('h', data)
    return math.sqrt(sum(n * n for n in samples) / len(samples))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path, help='Previous build with accepted C as profile 3')
    args = parser.parse_args()
    checks = 0
    for language, text in ((0x40000, 'Susi sieht sieben Segel. Sie sitzen zusammen am See.'),
                           (0x10000, 'Sue sees six ships. Susan visits the zoo on Thursday.')):
        paths = [args.library] + ([args.baseline] if args.baseline else [])
        engines = [DirectEciEngine(path.resolve(), dialect=language, encoding='utf8',
                                   experimental_generic=True) for path in paths]
        engine = engines[0]
        try:
            assert engine._dll.eciGetParam(engine._handle, HIGH_RATE_PROFILE) == 3
            checks += 1
            for rate in SAMPLE_RATES:
                for female in (0, 1):
                    for e in engines:
                        for parameter, value in ((SAMPLE_RATE, rate), (NATIVE_SPEED, 75),
                            (GENDER, female), (HEAD_SIZE, 29 if female else 50),
                            (BREATHINESS, 60 if female else 0)):
                            e.set_parameter(parameter, value)
                        e.set_parameter(HIGH_RATE_PROFILE, 3)
                    accepted = capture(engine, text)
                    if len(engines) > 1:
                        assert capture(engines[1], text) == accepted, (language, rate, female)
                        checks += 1
                    for old in (0, 1, 2, 3):
                        engine.set_parameter(HIGH_RATE_PROFILE, old)
                        # Preserve the old parameter's set/get contract for
                        # existing clients without reviving its sound variants.
                        assert engine._dll.eciGetParam(engine._handle, HIGH_RATE_PROFILE) == old
                        assert capture(engine, text) == accepted, (language, rate, female, old)
                        checks += 2
            for bad in (-1, 4):
                assert engine._dll.eciSetParam(engine._handle, HIGH_RATE_PROFILE, bad) == -1
                checks += 1
        finally:
            for e in engines: e.close()
    for language in LANGUAGES.values():
        engine = DirectEciEngine(args.library.resolve(), dialect=language.id, encoding='utf8',
                                 experimental_generic=True)
        try:
            for rate in (1, 6):
                engine.set_parameter(SAMPLE_RATE, rate)
                engine.set_parameter(NATIVE_VOLUME, 100); full = capture(engine, 'Hello world. 504.')
                engine.set_parameter(NATIVE_VOLUME, 30); quiet = capture(engine, 'Hello world. 504.')
                assert len(full[0]) == len(quiet[0]) and full[1] == quiet[1]
                assert abs(rms(quiet[0]) / rms(full[0]) - 181.907 / 2768.533) < .004, (language.name, rate)
                checks += 2
        finally:
            engine.close()
    print(f'Fixed tuning: {checks} legacy-parameter, real PCM/index, rate, model and volume checks; audio in RAM')


if __name__ == '__main__':
    main()
