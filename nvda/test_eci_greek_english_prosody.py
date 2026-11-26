"""Greek voice: English sentence prominence and ordinary character names."""
import argparse
from pathlib import Path
import tempfile

from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciText, GENDER,
                               PROCESS_TAGS, SAMPLE_RATE)
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference', type=Path)
    args = parser.parse_args()
    greek = DirectEciEngine(args.library.resolve(), dialect=0x120000,
                            encoding='utf8', native_voice=True)
    old = (DirectEciEngine(args.reference.resolve(), dialect=0x120000,
                           encoding='utf8', native_voice=True)
           if args.reference else None)
    checks = 0
    try:
        for engine in (greek, old):
            if engine:
                engine.set_parameter(PROCESS_TAGS, 1)
        # These are the weak words and phrase marks selected by the native
        # US analyzer, mapped to the existing Greek phoneme inventory.
        expected = {
            'This is a test.': ('`00 `[.0iz]', '`00 `[.0a]', '`2 `[.1test]'),
            'I want to read the book.': ('`00 `[.0ay]', '`00 `[.0to]', '`00 `[.0de]', '`2 `[.1buk]'),
            'The reader is in the room.': ('`00 `[.0de]', '`00 `[.0iz]', '`00 `[.0in]', '`2 `[.1rum]'),
            'bread and butter': ('`00 `[.0and]', '`2 `[.1ba.0ter]'),
            "I've read the book": ('`00 `[.0ayb]', '`[.1red]', '`00 `[.0de]'),
            'I have 3 books.': ('`00 `[.0ay]', '`00 `[.0jab]', '`2 `[.1buks]'),
        }
        for text, fragments in expected.items():
            result = phonemes(greek, text)
            for fragment in fragments:
                assert fragment in result, (text, fragment, result)
                checks += 1
        assert '[.1da]' in phonemes(greek, 'the')
        assert '`00 `[.0de]' in phonemes(greek, 'the book')
        assert '`[.1en]' in phonemes(greek, 'NVDA is here')
        assert '`00 `[.0iz]' in phonemes(greek, 'NVDA is here')
        checks += 4

        characters = {
            '(': 'ανοιχτή παρένθεση', ')': 'κλειστή παρένθεση',
            '[': 'ανοιχτή αγκύλη', ']': 'κλειστή αγκύλη',
            '{': 'ανοιχτό άγκιστρο', '}': 'κλειστό άγκιστρο',
            '"': 'εισαγωγικά', '«': 'εισαγωγικά', '»': 'εισαγωγικά',
            '?': 'αγγλικό ερωτηματικό', ';': 'ερωτηματικό', ';': 'ερωτηματικό',
            '·': 'άνω τελεία', '·': 'άνω τελεία',
            '|': 'κάθετη γραμμή', '_': 'κάτω παύλα', '€': 'ευρώ',
            '$': 'δολάριο', '&': 'και', '%': 'τοις εκατό',
        }
        for rate, gender in ((0, 0), (1, 1), (3, 0), (6, 1)):
            greek.set_parameter(SAMPLE_RATE, rate)
            greek.set_parameter(GENDER, gender)
            assert render(greek, [EciText('?', True)]) != render(greek, [EciText(';', True)])
            checks += 1
            for char, name in characters.items():
                assert render(greek, [EciText(char, True), EciIndex(7)]) == render(greek, [name, EciIndex(7)]), (rate, char)
                checks += 1
            for char in ('|', '_', '€', '$', '&', '%'):
                a = 'δοκιμή ' + char + ' σύμβολο'
                b = 'δοκιμή ' + characters[char] + ' σύμβολο'
                assert render(greek, [a, EciIndex(7)]) == render(greek, [b, EciIndex(7)]), (rate, char)
                checks += 1
        for char, name in (('+', 'συν'), ('|', 'κάθετη γραμμή'),
                           ('_', 'κάτω παύλα'), ('/', 'κάθετος')):
            assert render(greek, ['α' + char + 'β']) == render(greek, ['α ' + name + ' β']), char
            checks += 1
        assert render(greek, ['α < β']) == render(greek, ['α μικρότερο από β'])
        assert render(greek, ['α>β']) == render(greek, ['α μεγαλύτερο από β'])
        checks += 2
        # Both Unicode spellings of the Greek question mark use the same
        # question contour as '?', including sentences with English words.
        for question in ('Πώς είσαι', 'Τι είναι το NVDA', 'Αθήνα'):
            expected = render(greek, [question + '?', EciIndex(7)])
            assert render(greek, [question + ';', EciIndex(7)]) == expected, question
            assert render(greek, [question + ';', EciIndex(7)]) == expected, question
            checks += 2
        assert render(greek, ['Αθήνα; Καλημέρα.', EciIndex(7)]) == \
               render(greek, ['Αθήνα? Καλημέρα.', EciIndex(7)])
        checks += 1
        assert render(greek, ['This is a test;', EciIndex(7)]) != \
               render(greek, ['This is a test?', EciIndex(7)])
        assert ';' in phonemes(greek, 'This costs $3; it is cheap.')
        assert render(greek, ['Πώς είναι το 2.1;', EciIndex(7)]) == \
               render(greek, ['Πώς είναι το 2.1?', EciIndex(7)])
        checks += 3
        # Malformed phonetic input is read as characters. Its bracket names
        # must use the same Greek sounds as normal text and character review.
        invalid = phonemes(greek, '`[xyz]')
        assert invalid.count('`[.0an.1gi.0li]') == 2, invalid
        assert '`[.0an.1ji.0li]' not in invalid, invalid
        indexed_bad = render(greek, ['`[', EciIndex(1), 'xyz', EciIndex(2), ']', EciIndex(3)])
        assert [mark[0] for mark in indexed_bad[1]] == [1, 2, 3]
        assert [mark[1] for mark in indexed_bad[1]] == sorted(mark[1] for mark in indexed_bad[1])
        checks += 4
        if old:
            for engine in (greek, old):
                engine.set_parameter(SAMPLE_RATE, 3)
                engine.set_parameter(GENDER, 0)
            for text in ('Αυτή είναι η επόμενη πρόταση.', 'Εγώ μιλάω από την Αθήνα.',
                         'Η (λέξη) είναι εδώ.', 'Είπε «γεια».', '<tag>', '`00 από την Αθήνα',
                         '`2 the book', 'Αθήνα `00 this is a test', '`1 the reader is here',
                         '`vfg40 Αθήνα', '`unknown Αθήνα'):
                assert render(greek, [text, EciIndex(7)]) == render(old, [text, EciIndex(7)]), text
                checks += 1
            for valid in ('`[.1a]', '`[.1ma]', '`[.1an.0gi]', '`[.1ab.0la]', '`[a]'):
                assert render(greek, [valid, EciIndex(7)]) == render(old, [valid, EciIndex(7)]), valid
                checks += 1
            with tempfile.TemporaryDirectory() as directory:
                Path(directory, 'ELLmain.dic').write_bytes(b'|\tTestwort\n')
                text = 'δοκιμή | σύμβολο'
                without = render(greek, [text])
                for engine in (greek, old):
                    engine.set_dictionary_path(Path(directory))
                assert render(greek, [text]) != without
                assert render(greek, [text]) == render(old, [text])
                for engine in (greek, old):
                    engine.set_dictionary_path(None)
                checks += 2
        text = 'Αθήνα This is a test. Αθήνα'
        combined = render(greek, [text, EciIndex(19)])
        for cut in range(1, len(text)):
            assert render(greek, [text[:cut], text[cut:], EciIndex(19)]) == combined, cut
            checks += 1
        indexed = render(greek, ['This ', EciIndex(1), 'is ', EciIndex(2),
                                 'a test.', EciIndex(3)])
        assert [x[0] for x in indexed[1]] == [1, 2, 3]
        assert [x[1] for x in indexed[1]] == sorted(x[1] for x in indexed[1])
        assert indexed[1][-1][1] == len(indexed[0])
        checks += 3
    finally:
        greek.close()
        if old:
            old.close()
    print(f'Greek English prominence and character names: {checks} native ECI checks passed')


if __name__ == '__main__':
    main()
