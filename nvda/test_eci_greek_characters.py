"""Greek character labels use the same analysis as their spoken text."""
import argparse
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciText, GENDER, PROCESS_TAGS, SAMPLE_RATE
from test_eci_dash_pause import character_phonemes
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    names = {
        '.': 'τελεία', ',': 'κόμμα', '?': 'ερωτηματικό', '!': 'θαυμαστικό',
        ':': 'άνω και κάτω τελεία', '(': 'ανοιχτή παρένθεση',
        '[': 'ανοιχτή αγκύλη', ']': 'κλειστή αγκύλη', '@': 'παπάκι',
        '-': 'παύλα', '_': 'κάτω παύλα', '+': 'συν', '=': 'ίσον',
        '/': 'κάθετος', '\\': 'ανάποδη κάθετος', '%': 'τοις εκατό',
        '$': 'δολάριο', '&': 'και', '*': 'αστερίσκος', '#': 'δίεση',
        '|': 'κάθετη γραμμή', '^': 'περισπωμένη', '~': 'περισπωμένη',
        '`': 'βαρεία', '£': 'λίρα', '¥': 'γιέν',
        '§': 'παράγραφος', '°': 'βαθμός', '±': 'συν ή πλην', '×': 'επί',
        '÷': 'δια', '©': 'πνευματική ιδιοκτησία', '®': 'σήμα κατατεθέν',
        '™': 'εμπορικό σήμα', '‰': 'τοις χιλίοις',
        '†': 'σύμβολο σταυρού στίξης', '‡': 'σύμβολο διπλού σταυρού στίξης',
        '•': 'κουκίδα', '€': 'ευρώ', '«': 'εισαγωγικά',
        'α': 'άλφα', 'β': 'βήτα', 'γ': 'γάμμα', 'ά': 'άλφα τόνος',
        '0': 'μηδέν', '1': 'ένα', '2': 'δύο', '3': 'τρία', '4': 'τέσσερα',
        '5': 'πέντε', '6': 'έξι', '7': 'επτά', '8': 'οκτώ', '9': 'εννέα',
        '¡': 'ανάποδο θαυμαστικό', '¢': 'λεπτό', '¤': 'σύμβολο νομίσματος',
        '¦': 'διακεκομμένη κάθετος', '¨': 'διαλυτικά', 'ª': 'θηλυκό',
        '¬': 'άρνηση', '¯': 'μακρόν', '²': 'εκθέτης δύο', '³': 'εκθέτης τρία',
        '´': 'απόστροφος', '¶': 'σημείο παραγράφου', '¸': 'σεντίγια',
        '¹': 'εκθέτης ένα', 'º': 'αρσενικό', '¼': 'ένα τέταρτο',
        '½': 'ένα δεύτερο', '¾': 'τρία τέταρτα', '¿': 'ανάποδο ερωτηματικό',
        '˜': 'περισπωμένη',
    }
    engine = DirectEciEngine(args.library.resolve(), dialect=0x120000,
                             encoding='utf8', native_voice=True)
    try:
        for tags in (0, 1):
            engine.set_parameter(PROCESS_TAGS, tags)
            for character, name in names.items():
                if tags:
                    spoken = character_phonemes(engine, character)
                    expected = phonemes(engine, name)
                    assert spoken == expected, (tags, character, spoken, expected)
                result = render(engine, [EciText(character, True), EciIndex(7)])
                assert result == render(engine, [name, EciIndex(7)]), (tags, character)
                assert result[0] and result[1] == [(7, len(result[0]))], (tags, character)
        for rate, gender in ((0, 0), (1, 1), (3, 0), (6, 1)):
            engine.set_parameter(SAMPLE_RATE, rate)
            engine.set_parameter(GENDER, gender)
            for character in ('[', '_', ':', '€', 'ά'):
                assert render(engine, [EciText(character, True)]) == render(engine, [names[character]]), (rate, gender, character)
    finally:
        engine.close()
    print('Greek character names match connected text, including accents and punctuation')


if __name__ == '__main__':
    main()
