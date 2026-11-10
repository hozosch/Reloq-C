"""Brazilian number words retain the requested open and closed vowels."""
import argparse
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, EciIndex, PROCESS_TAGS, SAMPLE_RATE
from test_eci_german_clock_native import phonemes, render
from test_eci_inline_queue import Engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library.resolve()
    engine = DirectEciEngine(library, dialect=0x70000, encoding='utf8', native_voice=True)
    try:
        engine.set_parameter(PROCESS_TAGS, 1)
        closed, opened = phonemes(engine, 'dôze'), phonemes(engine, 'dezenóve')
        assert phonemes(engine, '12') == closed
        assert phonemes(engine, '19') == opened
        assert phonemes(engine, 'dóze') != closed
        assert phonemes(engine, 'dezenôve') != opened
        for number, vowel in (('112', '.1do.0zi'), ('119', '.1nc.0vi'),
                              ('4312', '.1do.0zi'), ('7819', '.1nc.0vi'),
                              ('12.019', '.1do.0zi'), ('12.019', '.1nc.0vi'),
                              ('-12', '.1do.0zi'), ('19%', '.1nc.0vi')):
            assert vowel in phonemes(engine, number), number
        for rate in (0, 1, 3, 2, 4, 5, 6):
            engine.set_parameter(SAMPLE_RATE, rate)
            for number, expected in (('12', 'dôze'), ('19', 'dezenóve')):
                actual = render(engine, [number, EciIndex(7)])
                expanded = render(engine, [expected, EciIndex(7)])
                assert actual == expanded, (rate, number)
    finally:
        engine.close()
    # ECI's byte and UTF-8 entry points must reach the same language rules.
    engine = Engine(library, 0x70000)
    try:
        for number in ('12', '19', '4312', '7819'):
            engine.add(number, legacy=True)
            legacy = engine.finish()
            assert engine.speak(number) == legacy, number
    finally:
        engine.close()
    print('Brazilian Portuguese: 12 and 19, larger numbers, all sample rates and both ECI inputs passed')


if __name__ == '__main__':
    main()
