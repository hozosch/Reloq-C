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
        # Preserve the existing sentence analysis rather than emphasizing
        # every lexical content word in ordinary connected Greek.
        cases = [('Γλώσσα Ελληνικά αναδυόμενο κουτί', '[.0a.0na.0di.1o.0me.0no]'),
                 ('Αυτόματη εναλλαγή γλώσσας ενεργή', '[.1glo.0sas]')]
        for text, stressed in cases:
            phones = phonemes(greek, text)
            assert '`0 `' + stressed in phones, phones
            whole = render(greek, [text, EciIndex(7)])
            assert whole[0] and whole[1] == [(7, len(whole[0]))]
            for cut in range(len(text) + 1):
                assert render(greek, [text[:cut], text[cut:], EciIndex(7)]) == whole, (text, cut)
        # NVDA can join a control label and a new sentence with a wide gap.
        # The following sentence should keep its own native prominence.
        title = 'Αλλαγή Διαμόρφωσης Γλώσσας  παράθυρο διαλόγου'
        body = 'Πρέπει να γίνει επανεκκίνηση του NVDA, για να τεθεί σε ισχύ η νέα γλώσσα.'
        first = '`[.1pre.0pi]'
        isolated = phonemes(greek, body)
        combined = phonemes(greek, title + '  ' + body)
        assert combined[combined.index(first):] == isolated[isolated.index(first):], (combined, isolated)
        assert phonemes(greek, title + '  ' + body) == phonemes(greek, title + ', ' + body)
        mixed_title = 'Αλλαγή γλώσσας για το NVDA'
        assert phonemes(greek, mixed_title + '  ' + body) == phonemes(greek, mixed_title + ', ' + body)
        whole = render(greek, [title + '  ' + body, EciIndex(11)])
        assert whole[0] and whole[1] == [(11, len(whole[0]))]
        assert whole == render(greek, [title + ', ' + body, EciIndex(11)])
        for cut in range(len(title) - 1, len(title) + 4):
            text = title + '  ' + body
            assert render(greek, [text[:cut], text[cut:], EciIndex(11)]) == whole
        # A double space after an ordinary preposition is not a new phrase.
        assert phonemes(greek, 'Αυτός έρχεται από  Αθήνα.') == phonemes(greek, 'Αυτός έρχεται από Αθήνα.')
        assert '`[.0a] `[.0po]' in phonemes(greek, 'από Αθήνα')
        assert '[.0a.1po]' in phonemes(greek, 'από')
        assert render(greek, ['`0 Αθήνα και Θεσσαλονίκη']) != render(greek, ['`1 Αθήνα και Θεσσαλονίκη'])
        # Injecting a prominence tag directly after a hyphen caused the
        # Castilian core to read a spurious punctuation name between words.
        for joiner in ('-', '‐', '‑', '−'):
            assert '[.1pa.0bla]' not in phonemes(greek, 'Καλός καιρός' + joiner + 'ήρθε η καταιγίδα.')
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
