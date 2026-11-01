"""Confirm that real ECI audio retains seconds and source indices, in RAM."""
from pathlib import Path
import argparse
from _reloq_eci_direct import DirectEciEngine, EciIndex, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES


def capture(engine, text):
    samples = 0
    indices = []
    def pcm(data):
        nonlocal samples
        samples += len(data)//2
    assert engine.speak([text, EciIndex(23)], pcm, lambda n: indices.append((n, samples)))
    assert samples and indices == [(23, samples)]
    return samples


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    cases = (('13:28', '13:28:25'), ('00:20', '00:20:48.526'),
             ('1:02', '1:02:03'), ('13:28', '13:28:00'))
    checks = 0
    for language in (0x10000, 0x10001):
        engine = DirectEciEngine(args.library.resolve(), dialect=language,
                                encoding='utf8', experimental_generic=True)
        try:
            for rate in (1, 3, 6):
                engine.set_parameter(SAMPLE_RATE, rate)
                for speed in (40, 75, 150):
                    engine.set_parameter(NATIVE_SPEED, speed)
                    for short, full in cases:
                        before, after = capture(engine, short), capture(engine, full)
                        assert after > before + SAMPLE_RATES[rate]*.015, (language, rate, speed, full, before, after)
                        checks += 1
        finally:
            engine.close()
    print(f'English seconds: {checks} actual PCM duration/end-index checks, both dialects, three rates and tempos; no audio files')


if __name__ == '__main__':
    main()
