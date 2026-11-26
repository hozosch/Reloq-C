"""German e/r compound boundaries through the public ECI speech path."""
import argparse
from pathlib import Path
import re
import tempfile

from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciText, NATIVE_SPEED,
                               PROCESS_TAGS, SAMPLE_RATE)
from test_eci_german_clock_native import phonemes, render

ROOT = Path(__file__).resolve().parents[1]


def engine(library):
    return DirectEciEngine(library.resolve(), dialect=0x40000,
                           encoding='utf8', native_voice=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference', type=Path)
    args = parser.parse_args()
    actual = engine(args.library)
    old = engine(args.reference) if args.reference else None
    fixed = {
        'Schieberegler': 'Schiebe-Regler',
        'Leserichtung': 'Lese-Richtung',
        'Leserichtungen': 'Lese-Richtungen',
        'Schieberichtung': 'Schiebe-Richtung',
        'Lautstärkeregler': 'Lautstärke-Regler',
        'Lesereise': 'Lese-Reise',
        'Leseratte': 'Lese-Ratte',
        'Lesereihe': 'Lese-Reihe',
        'Schieberiegel': 'Schiebe-Riegel',
        'Leserechnung': 'Lese-Rechnung',
        'Leseregelung': 'Lese-Regelung',
        'Leserand': 'Lese-Rand',
        'Schieberand': 'Schiebe-Rand',
        'Leseregister': 'Lese-Register',
        'Leseröhre': 'Lese-Röhre',
        'Leserückmeldung': 'Lese-Rückmeldung',
        'Wärmeregler': 'Wärme-Regler',
        'Kälteregler': 'Kälte-Regler',
    }
    unchanged = ('Schieber', 'Leser', 'Leserbrief', 'Leseranzahl',
                 'Leserantwort', 'Schieberantrieb', 'Fahrerinnerung',
                 'Schieberegelung', 'Wärmeregelung', 'Hilferegler',
                 'Reiseroute', 'Lese-Richtung', 'Eingaberegler', 'Lautstärkeregelung')
    checks = 0
    try:
        actual.set_parameter(PROCESS_TAGS, 1)
        if old:
            old.set_parameter(PROCESS_TAGS, 1)
        for word, expected in fixed.items():
            original = re.findall(r'`\[[^]]*\]', phonemes(actual, word))
            separated = re.findall(r'`\[[^]]*\]', phonemes(actual, expected))
            assert original == separated, word
            checks += 1
        for rate in (0, 1, 3, 6):
            for speed in (50, 90):
                actual.set_parameter(SAMPLE_RATE, rate)
                actual.set_parameter(NATIVE_SPEED, speed)
                if old:
                    old.set_parameter(SAMPLE_RATE, rate)
                    old.set_parameter(NATIVE_SPEED, speed)
                for word, expected in fixed.items():
                    a = render(actual, [word, EciIndex(7)])
                    b = render(actual, [expected, EciIndex(7)])
                    assert a == b and a[0] and a[1] == [(7, len(a[0]))], (rate, speed, word)
                    checks += 1
                if old:
                    for word in unchanged:
                        assert render(actual, [word, EciIndex(7)]) == render(old, [word, EciIndex(7)]), (rate, speed, word)
                        checks += 1
        sentence = 'Der Schieberegler bestimmt die Leserichtung.'
        reference = render(actual, [sentence, EciIndex(19)])
        for cut in range(1, len(sentence)):
            assert render(actual, [sentence[:cut], sentence[cut:], EciIndex(19)]) == reference, cut
            checks += 1
        for word in ('Schieberegler', 'Leserichtung'):
            assert render(actual, [EciText(word, True)]) != render(actual, [word]), word
            checks += 1
        # The shipped Main/Root volumes contain short substrings such as
        # "IC"; these must not suppress a correction to the entire word.
        shipped = DirectEciEngine(args.library.resolve(), dialect=0x40000,
                    encoding='utf8', native_voice=True,
                    lexicon_path=ROOT / 'data/german_apple_observed.rlex')
        try:
            shipped.set_dictionary_path(ROOT / 'data/community')
            for word in ('Schieberegler', 'Leserichtung', 'Schieberand'):
                assert render(shipped, [word]) == render(shipped, [fixed[word]]), word
                checks += 1
        finally:
            shipped.close()
        # Exact custom pronunciations must win before compound correction.
        with tempfile.TemporaryDirectory() as directory:
            Path(directory, 'DEUmain.dic').write_bytes(
                b'Schieberegler\tHallo\nLeserichtung\tTestwort\n')
            custom = engine(args.library)
            try:
                custom.set_dictionary_path(Path(directory))
                custom.set_parameter(PROCESS_TAGS, 1)
                assert phonemes(custom, 'Schieberegler') == phonemes(custom, 'Hallo')
                assert phonemes(custom, 'Leserichtung') == phonemes(custom, 'Testwort')
                checks += 2
            finally:
                custom.close()
        with tempfile.TemporaryDirectory() as directory:
            Path(directory, 'DEURoot.dic').write_bytes(b'leserichtung\t`[.1tEst]\n')
            custom = engine(args.library)
            try:
                custom.set_dictionary_path(Path(directory))
                custom.set_parameter(PROCESS_TAGS, 1)
                assert '[.1tEst]' in phonemes(custom, 'Leserichtung')
                checks += 1
            finally:
                custom.close()
    finally:
        actual.close()
        if old:
            old.close()
    print(f'German e/r compounds: {checks} native pronunciation, audio, fragment and dictionary checks')


if __name__ == '__main__':
    main()
