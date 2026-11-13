"""Separate ECI instances must not mix simultaneous phoneme or audio output."""
import argparse
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from threading import Barrier

from _reloq_eci_direct import DirectEciEngine
from test_eci_german_clock_native import phonemes, render


SAMPLES = (
    (0x130000, "d'os libros y dos amigos"),
    (0x40000, 'Diese Sprache ist sehr schoen.'),
    (0x10000, 'These are several examples for testing.'),
    (0x20000, 'Hola amigo, como estas?'),
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library.resolve()

    def make_engine(language):
        return DirectEciEngine(library, dialect=language, encoding='utf8', native_voice=True)

    references = []
    for language, text in SAMPLES:
        engine = make_engine(language)
        try:
            references.append((phonemes(engine, text), render(engine, [text])[0]))
        finally:
            engine.close()

    barrier = Barrier(len(SAMPLES))

    def check(item):
        index, (language, text) = item
        engine = make_engine(language)
        try:
            barrier.wait(timeout=30)
            expected_phonemes, expected_audio = references[index]
            for _ in range(100):
                actual = phonemes(engine, text)
                assert actual == expected_phonemes, (language, text, expected_phonemes, actual)
            for _ in range(20):
                assert render(engine, [text])[0] == expected_audio, (language, text)
        finally:
            engine.close()

    with ThreadPoolExecutor(max_workers=len(SAMPLES)) as pool:
        list(pool.map(check, enumerate(SAMPLES)))
    print('Parallel ECI instances: 400 phoneme and 80 audio comparisons passed')


if __name__ == '__main__':
    main()
