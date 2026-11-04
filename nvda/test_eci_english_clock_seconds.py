"""Confirm that real ECI audio retains seconds and source indices, in RAM."""
from pathlib import Path
import argparse
from _reloq_eci_direct import DirectEciEngine, EciIndex, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES


def render(engine, parts):
    data = bytearray()
    indices = []
    assert engine.speak(parts, data.extend,
                        lambda n: indices.append((n, len(data)//2)))
    assert data
    return bytes(data), indices


def capture(engine, text):
    data, indices = render(engine, [text, EciIndex(23)])
    samples = len(data)//2
    assert indices == [(23, samples)]
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
                                encoding='utf8', native_voice=True)
        try:
            for rate in (1, 3, 6):
                engine.set_parameter(SAMPLE_RATE, rate)
                for speed in (40, 75, 150):
                    engine.set_parameter(NATIVE_SPEED, speed)
                    for short, full in cases:
                        before, after = capture(engine, short), capture(engine, full)
                        assert after > before + SAMPLE_RATES[rate]*.015, (language, rate, speed, full, before, after)
                        checks += 1
            engine.set_parameter(SAMPLE_RATE, 1)
            engine.set_parameter(NATIVE_SPEED, 75)
            original = render(engine, ['13:28:', EciIndex(1), '25',
                                      EciIndex(2), ' next.', EciIndex(3)])
            expanded = render(engine, ['13:28', EciIndex(1), ' and 25 seconds',
                                      EciIndex(2), ' next.', EciIndex(3)])
            assert original == expanded, (language, 'fragment PCM/index mapping')
        finally:
            engine.close()
    print(f'English seconds: {checks} actual PCM duration/end-index checks and two fragment PCM/index identities; both dialects, three rates and tempos; no audio files')


if __name__ == '__main__':
    main()
