"""Inline vf and the ECI/NVDA inflection setting use the same native plan."""
import argparse
from pathlib import Path

from test_eci_inline_queue import Engine, TEXTS


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library.resolve()
    checks = 0
    for language, parts in TEXTS.items():
        engine = Engine(library, language)
        try:
            text = ''.join(parts)
            for target in (0, 15, 30, 70, 100):
                engine.voice(3, target)
                expected = engine.speak(text)
                for previous in (0, 30, 100):
                    engine.voice(3, previous)
                    assert engine.speak(f'`vf{target} {text}') == expected, (hex(language), previous, target)
                    checks += 1
            for previous in (0, 30, 100):
                engine.voice(3, previous)
                expected = engine.speak(f'{parts[0]}`vf70 {parts[1]}{parts[2]}')
                engine.voice(3, previous)
                engine.tags(0)
                engine.add(parts[0])
                engine.voice(3, 70)
                engine.add(parts[1] + parts[2])
                assert engine.finish() == expected, (hex(language), previous, 'queued inflection')
                engine.tags(1)
                checks += 1
            # Presets set inflection too, including after another inline vf.
            if language >> 16 not in (6, 8, 10):
                for previous in (0, 30, 100):
                    engine.voice(3, previous)
                    for tag, inflection in (('v1', 30), ('v3', 35), ('v4', 47)):
                        expected = engine.speak(f'`{tag} {text}')
                        assert engine.speak(f'`{tag} `vf{inflection} {text}') == expected, (hex(language), previous, tag)
                        checks += 1
        finally:
            engine.close()
    print(f'Inflection: {checks} native tag/slider and preset comparisons passed')


if __name__ == '__main__':
    main()
