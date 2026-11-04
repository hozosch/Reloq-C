"""Sentence-final A, connected seven and i-to-e regressions through ECI."""
import argparse
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER, NATIVE_PITCH,
    NATIVE_INFLECTION, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES)
from test_eci_german_clock_native import phonemes, render


def open_engine(path):
    e = DirectEciEngine(path.resolve(), dialect=0x40000, encoding='utf8',
                        native_voice=True)
    e.set_parameter(PROCESS_TAGS, 1)
    return e


def first_index(engine, ending):
    return render(engine, ['Pizza', EciIndex(1), ending])[1][0][1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    engine = open_engine(args.library)
    baseline = open_engine(args.baseline) if args.baseline else None
    checks = 0
    try:
        for gender in (0, 1):
            engine.set_parameter(GENDER, gender)
            for rate in SAMPLE_RATES:
                engine.set_parameter(SAMPLE_RATE, rate)
                # A new sentence should use the same preceding A duration
                # as an isolated sentence, including intervening ECI indices.
                final = first_index(engine, '.')
                for ending in ('. Test', '? Test', '! Test', '. Mama.'):
                    assert first_index(engine, ending) == final, (gender, rate, ending)
                    checks += 1
                # A comma with a following word is still the short text A;
                # explicit phonetic input keeps its own pronunciation.
                text = 'Pizza, Test'
                assert render(engine, [text]) == render(engine, [phonemes(engine, text)])
                checks += 1
                text = 'Pizza. Test'
                assert render(engine, [text]) != render(engine, [phonemes(engine, text)])
                checks += 1
                for number in ('27', '47', '67', '97'):
                    p = phonemes(engine, number)
                    assert render(engine, [number, EciIndex(7)]) == render(engine, [p, EciIndex(7)]), (gender, rate, number)
                    checks += 1
                for pitch in (90, 98, 100):
                    engine.set_parameter(NATIVE_PITCH, pitch)
                    engine.set_parameter(NATIVE_INFLECTION, 100)
                    audio, indices = render(engine, ['Barriere. Wie eben?', EciIndex(5)])
                    assert audio and indices == [(5, len(audio))]
                    checks += 1
                engine.set_parameter(NATIVE_PITCH, 65)
                engine.set_parameter(NATIVE_INFLECTION, 30)
        if baseline is not None:
            for e in (engine, baseline):
                e.set_parameter(GENDER, 0)
                e.set_parameter(SAMPLE_RATE, 1)
            for text in ('Rot', 'vierund', 'Barriere', 'wie eben', 'Pizza', 'Pizza. Test'):
                assert render(engine, [text]) != render(baseline, [text]), text
                checks += 1
            for text in ('sieben', '1 Test', 'plus', 'Pizza ist gut', 'Pizza, Test'):
                assert render(engine, [text]) == render(baseline, [text]), text
                checks += 1
            for text in ('27', '47', '67', '97', 'siebenund'):
                new = render(engine, [text])[0]
                old = render(baseline, [text])[0]
                assert len(new) < len(old), text
                checks += 1
    finally:
        engine.close()
        if baseline is not None:
            baseline.close()
    print(f'German rc17: {checks} sentence-end, i/e, seven, pitch and baseline checks')


if __name__ == '__main__':
    main()
