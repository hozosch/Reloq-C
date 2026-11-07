"""Regressions for Greek prominence, question contour and short PCM peaks."""
import argparse
from array import array
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER, NATIVE_SPEED,
                               NATIVE_VOLUME, ORIGINAL_QUESTIONS, PROCESS_TAGS, SAMPLE_RATE)
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    library = args.library.resolve()
    greek = DirectEciEngine(library, dialect=0x120000, encoding='utf8', native_voice=True)
    try:
        greek.set_parameter(PROCESS_TAGS, 1)
        # Text from NVDA's Greek interface: the planner used to insert `0
        # ahead of the explicitly stressed content words in these phrases.
        cases = [('Γλώσσα Ελληνικά αναδυόμενο κουτί', '[.0a.0na.0di.1o.0me.0no]'),
                 ('Αυτόματη εναλλαγή γλώσσας ενεργή', '[.1glo.0sas]')]
        for text, stressed in cases:
            phones = phonemes(greek, text)
            assert stressed in phones and '`0 `' + stressed not in phones, phones
            whole = render(greek, [text, EciIndex(7)])
            assert whole[0] and whole[1] == [(7, len(whole[0]))]
            for cut in range(len(text) + 1):
                assert render(greek, [text[:cut], text[cut:], EciIndex(7)]) == whole, (text, cut)
        assert '`[.0a] `[.0po]' in phonemes(greek, 'από Αθήνα')
        assert '[.0a.1po]' in phonemes(greek, 'από')
        assert render(greek, ['`0 Αθήνα και Θεσσαλονίκη']) != render(greek, ['`1 Αθήνα και Θεσσαλονίκη'])
    finally:
        greek.close()

    for language, question, statement in (
        (0x10000, 'What would you like to do?', 'This is a sentence.'),
        (0x40000, 'Was möchtest du machen?', 'Das ist ein Satz.'),
    ):
        engine = DirectEciEngine(library, dialect=language, encoding='utf8', native_voice=True)
        try:
            for rate in (1, 6):
                engine.set_parameter(SAMPLE_RATE, rate)
                engine.set_parameter(ORIGINAL_QUESTIONS, 0)
                rising = render(engine, [question, EciIndex(1)])
                plain = render(engine, [statement, EciIndex(2)])
                engine.set_parameter(ORIGINAL_QUESTIONS, 1)
                original = render(engine, [question, EciIndex(1)])
                assert original[0] != rising[0] and len(original[0]) == len(rising[0])
                assert original[1] == rising[1]
                assert render(engine, [statement, EciIndex(2)]) == plain
                engine.set_parameter(ORIGINAL_QUESTIONS, 0)
                assert render(engine, [question, EciIndex(1)]) == rising
        finally:
            engine.close()

    german = DirectEciEngine(library, dialect=0x40000, encoding='utf8', native_voice=True)
    try:
        german.set_parameter(NATIVE_VOLUME, 100)
        german.set_parameter(NATIVE_SPEED, 98)
        for gender in (0, 1):
            german.set_parameter(GENDER, gender)
            for rate in range(7):
                german.set_parameter(SAMPLE_RATE, rate)
                data, marks = render(german, ['48', EciIndex(5)])
                pcm = array('h');pcm.frombytes(data)
                assert pcm and 0 < max(abs(s) for s in pcm) < 28001, (gender, rate)
                assert marks == [(5, len(data))], (gender, rate, marks)
    finally:
        german.close()
    print('Greek sentence prominence, selectable question contour and short peaks passed')


if __name__ == '__main__':
    main()
