"""Keep English contractions whole in Persian voice speech and source marks."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciText, GENDER, SAMPLE_RATE
from nvda_driver_probe import make_language_engine


def render(engine, parts):
    audio, marks = bytearray(), []
    assert engine.speak(parts, audio.extend, lambda value: marks.append((value, len(audio))))
    assert audio
    return bytes(audio), marks


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = args.library.resolve()
    engine = make_language_engine(dll, dll, None, Path('/unavailable'), Path('/unavailable'))
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                engine.set_parameter(SAMPLE_RATE, rate)
                engine.set_parameter(GENDER, gender)
                for before, after in (
                    ("Some developers say they", "ve stopped opening it."),
                    ("سلام we", "re ready دنیا"),
                    ("I", "m here."),
                    ("It isn", "t finished."),
                    ("Amir", "s test."),
                ):
                    for marks in (False, True):
                        def parts(apostrophe):
                            if marks:
                                return (EciIndex(10), before, EciIndex(11), apostrophe,
                                        EciIndex(12), after, EciIndex(13))
                            return (before + apostrophe + after,)
                        standard = render(engine, parts("'"))
                        for quote in ('\u2018', '\u2019'):
                            assert render(engine, parts(quote)) == standard, (rate, gender, before, quote, marks)
                            checks += 1
                # Standalone character reading remains a character name.
                assert render(engine, (EciText('\u2019', character_mode=True),))
                checks += 1
    finally:
        engine.close()
    print(f'Passed {checks} English contraction, mixed-sentence and source-mark comparisons.')


if __name__ == '__main__':
    main()
