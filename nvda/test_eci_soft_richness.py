"""Compare real unvoiced onsets of the B and gentle-richness C profiles.

Full speech naturally differs once periodic voice begins; these bounded
onsets isolate the separate AF path. Audio remains in memory.
"""
import argparse
from pathlib import Path
from _reloq_eci_direct import DirectEciEngine, GENDER, SAMPLE_RATE, SAMPLE_RATES
from test_eci_castilian_filter import capture, onset


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library', type=Path, required=True)
    args = p.parse_args()
    cases = ((0x10000, 'sister.', 60), (0x20000, 'seseo.', 60),
        (0x20001, 'seseo.', 60), (0x40000, 'Zehn.', 55),
        (0x40000, 'Zwei.', 55), (0x10000, 'five.', 40),
        (0x40000, 'fein.', 40), (0x20000, 'teta.', 8))
    checks = 0
    for dialect, text, ms in cases:
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect,
            encoding='utf8', experimental_generic=True)
        try:
            for gender in (0, 1):
                engine.set_parameter(GENDER, gender)
                for rate in (3, 2, 4, 5, 6):
                    hz = SAMPLE_RATES[rate]
                    engine.set_parameter(SAMPLE_RATE, rate)
                    b = capture(engine, text, 1)
                    c = capture(engine, text, 3)
                    prefix_b, _ = onset(b, hz, ms)
                    prefix_c, _ = onset(c, hz, ms)
                    assert prefix_b == prefix_c, (hex(dialect), text, gender, hz)
                    checks += 1
        finally:
            engine.close()
    print(f'Gentle richness: {checks} real S/TS/F/T onset identities; audio in RAM')


if __name__ == '__main__':
    main()
