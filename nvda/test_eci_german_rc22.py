"""German final A, connected numbers and long-oe rhotics through ECI."""
import argparse
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER, PROCESS_TAGS,
                               SAMPLE_RATE, SAMPLE_RATES)
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                             encoding='utf8', experimental_generic=True)
    engine.set_parameter(PROCESS_TAGS, 1)
    checks = 0
    try:
        for gender in (0, 1):
            engine.set_parameter(GENDER, gender)
            for rate in SAMPLE_RATES:
                engine.set_parameter(SAMPLE_RATE, rate)
                # The text-only phrase-final A must also work after R, but
                # must not change an explicit phonetic A or a following word.
                sentence = 'Kamera.'
                assert render(engine, [sentence]) != render(engine, [phonemes(engine, sentence)])
                text = 'Kamera ist gut.'
                assert render(engine, [text]) == render(engine, [phonemes(engine, text)])
                final = render(engine, ['Kamera', EciIndex(7), '.'])[1][0][1]
                for tail in ('. Test.', '? Test.', '! Test.'):
                    assert render(engine, ['Kamera', EciIndex(7), tail])[1][0][1] == final
                    checks += 1
                checks += 2
                # Explicit SPR and text use the same connected onset and
                # glide in two-/three-and, with and without the tens part.
                for word in ('zweiund', 'dreiund', '22', '23',
                             'zweiundzwanzig', 'dreiundzwanzig'):
                    pronunciation = phonemes(engine, word)
                    assert render(engine, [word, EciIndex(9)]) == render(
                        engine, [pronunciation, EciIndex(9)]), (gender, rate, word, pronunciation)
                    checks += 1
                for word in ('herunter', 'höher', 'Hörbuch', 'Röhre'):
                    pronunciation = phonemes(engine, word)
                    assert render(engine, [word, EciIndex(9)]) == render(
                        engine, [pronunciation, EciIndex(9)]), (gender, rate, word, pronunciation)
                    checks += 1
    finally:
        engine.close()
    print(f'German rc22: {checks} final-A, number, R and phonetic-input checks')


if __name__ == '__main__':
    main()
