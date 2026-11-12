"""Regression tests for the isolated Aragonese/Spanish experiment."""
import argparse
from pathlib import Path

from probe import spanish_probe_input


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path)
    args = parser.parse_args()
    cases = {'sinyal': 'siñal', 'cabanyera': 'cabañera', 'nyapir': 'ñapir',
             'bony': 'boñ', 'Espanya': 'España', 'cigüenya': 'cigüeña',
             'NYAPIR': 'ÑAPIR', 'Nyapir': 'Ñapir', 'choven': 'choven',
             'aixina': 'aixina', 'puent': 'puent'}
    for source, expected in cases.items():
        assert spanish_probe_input(source) == expected, source
    assert spanish_probe_input('cigu\u0308enya') == 'cigüeña'
    print(f'Aragonese experiment: {len(cases) + 1} isolated orthography checks passed')
    if args.library:
        from _reloq_eci_direct import DirectEciEngine
        engine = DirectEciEngine(args.library.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
        try:
            for source, expected in cases.items():
                if 'ny' not in source.lower():
                    continue
                adapted = engine._phonemes_for_probe(spanish_probe_input(source))
                assert adapted and adapted == engine._phonemes_for_probe(expected), source
            assert engine._phonemes_for_probe('Espanya') != engine._phonemes_for_probe('España')
        finally:
            engine.close()
        print('Aragonese experiment: Spanish native phoneme comparisons passed')


if __name__ == '__main__':
    main()
