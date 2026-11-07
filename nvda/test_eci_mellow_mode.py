"""Mellow-mode switching through the public ECI interface, with PCM in RAM."""
from pathlib import Path
import argparse

from _reloq_eci_direct import DirectEciEngine, EciIndex, GENDER, MELLOW_MODE, SAMPLE_RATE
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library.resolve()
    cases = 0
    for dialect, text in ((0x40000, 'Am Morgen hören wir eine warme Stimme.'),
                          (0x10000, 'A clear voice in the morning.'),
                          (0x120000, 'Αυτή είναι μια δοκιμή της φωνής.')):
        engine = DirectEciEngine(library, dialect=dialect, encoding='utf8', native_voice=True)
        try:
            assert engine._dll.eciGetParam(engine._handle, MELLOW_MODE) == 0
            for gender in (0, 1):
                engine.set_parameter(GENDER, gender)
                for rate in (0, 1, 3, 2, 4, 5, 6):
                    engine.set_parameter(SAMPLE_RATE, rate)
                    engine.set_parameter(MELLOW_MODE, 0)
                    regular = render(engine, [text, EciIndex(7)])
                    engine.set_parameter(MELLOW_MODE, 1)
                    mellow = render(engine, [text, EciIndex(7)])
                    assert len(regular[0]) == len(mellow[0]) and regular[1] == mellow[1]
                    assert (regular[0] == mellow[0]) == (rate in (0, 1)), (hex(dialect), gender, rate)
                    engine.set_parameter(MELLOW_MODE, 0)
                    assert render(engine, [text, EciIndex(7)]) == regular, (hex(dialect), gender, rate)
                    cases += 1
            assert engine._dll.eciSetParam(engine._handle, MELLOW_MODE, 2) == -1
            assert engine._dll.eciGetParam(engine._handle, MELLOW_MODE) == 0
            engine.set_parameter(MELLOW_MODE, 1)
            assert engine._dll.eciReset(engine._handle) == 1
            assert engine._dll.eciGetParam(engine._handle, MELLOW_MODE) == 0
        finally:
            engine.close()
    print(f'Mellow mode: {cases} language/model/rate cases; low rates unchanged, '
          'high-rate output changes, exact switch-back, timing, index and reset passed')


if __name__ == '__main__':
    main()
