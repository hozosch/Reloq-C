"""Both Mandarin locales can initialize and alternate their English path."""
import argparse
from pathlib import Path
import subprocess
import sys
from _reloq_eci_direct import DirectEciEngine, EciIndex, SAMPLE_RATE, PROCESS_TAGS
from test_eci_german_clock_native import phonemes


def worker(library, order):
    engines = []
    try:
        for language in order:
            engine = DirectEciEngine(library.resolve(), dialect=language,
                                     encoding='utf8', native_voice=True)
            engines.append(engine)
            engine.set_parameter(SAMPLE_RATE, 1)
            engine.set_parameter(PROCESS_TAGS, 1)
        for _ in range(2):
            for engine in engines:
                for text in ('hello world', '你好 hello 世界', 'https://example.org/test'):
                    phones = phonemes(engine, text)
                    assert phones and ('hello' not in text or '[.2hE.1lo]' in phones), phones
                    audio = bytearray()
                    marks = []
                    assert engine.speak([text, EciIndex(9)], audio.extend, marks.append)
                    assert audio and marks == [9]
    finally:
        for engine in engines:
            engine.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--worker', choices=('cn-tw', 'tw-cn'))
    args = parser.parse_args()
    if args.worker:
        order = (0x60000, 0x60001) if args.worker == 'cn-tw' else (0x60001, 0x60000)
        worker(args.library, order)
    else:
        for order in ('cn-tw', 'tw-cn'):
            subprocess.run([sys.executable, '-B', __file__, '--library', str(args.library),
                            '--worker', order], check=True, timeout=90)
        print('Chinese locales: both initialization orders and alternating English speech passed')


if __name__ == '__main__':
    main()
