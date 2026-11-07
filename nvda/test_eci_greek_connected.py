"""Greek English phones, weak words, and unchanged native Greek digraphs."""
import argparse
from pathlib import Path
import unicodedata

from _reloq_eci_direct import DirectEciEngine, EciIndex, GENDER, PROCESS_TAGS, SAMPLE_RATE
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    greek = DirectEciEngine(args.library.resolve(), dialect=0x120000,
                            encoding='utf8', native_voice=True)
    old = (DirectEciEngine(args.baseline.resolve(), dialect=0x120000,
                           encoding='utf8', native_voice=True)
           if args.baseline else None)
    try:
        greek.set_parameter(PROCESS_TAGS, 1)
        if old:
            old.set_parameter(PROCESS_TAGS, 1)
        english = {
            'Manhattan': '[.0man.1ja.0tan]',
            'button': '[.1ba.0ton]',
            'kitten': '[.1ki.0ten]',
            'work': '[.1work]',
            'working': '[.1wor.0kin]',
            'word': '[.1word]',
            'world': '[.1world]',
            'worker': '[.1wor.0ker]',
            'her': '[.1jer]',
            'doctor': '[.1dok.0ter]',
            'ongoing': '[.1on.0go.0win]',
            'finger': '[.1fin.0ger]',
            'singer': '[.1si.0ner]',
            'language': '[.1lan.0gwidz]',
        }
        for word, expected in english.items():
            result = phonemes(greek, word)
            assert expected in result, (word, result)
            assert render(greek, [word, EciIndex(7)])[0]

        weak_apo = '`[.0a] `[.0po]'
        for text in ('από Αθήνα', 'Από την Αθήνα', 'από London', 'από 5'):
            result = phonemes(greek, text)
            assert weak_apo in result and '[.1po]' not in result, (text, result)
            assert result == phonemes(greek, unicodedata.normalize('NFD', text))
            pcm, marks = render(greek, [text, EciIndex(7)])
            assert pcm and marks == [(7, len(pcm))], (text, marks)
            for cut in range(1, len(text)):
                assert render(greek, [text[:cut], text[cut:], EciIndex(7)]) == (pcm, marks), (text, cut)
        for text in ('από', 'από.', 'από, Αθήνα', 'λέω από'):
            assert '[.0a.1po]' in phonemes(greek, text), text
        assert '[.1a.0po]' in phonemes(greek, 'άπο Αθήνα')

        # κατά is commonly proclitic before its complement, but not in
        # isolation or when the orthography marks another syllable.
        weak_kata = '`[.0ka] `[.0ta]'
        for text in ('κατά Αθήνα', 'Κατά τη διάρκεια', 'κατά London', 'κατά 5'):
            result = phonemes(greek, text)
            assert weak_kata in result and '[.1ta]' not in result, (text, result)
            assert result == phonemes(greek, unicodedata.normalize('NFD', text))
            pcm, marks = render(greek, [text, EciIndex(7)])
            assert pcm and marks == [(7, len(pcm))], (text, marks)
            for cut in range(1, len(text)):
                assert render(greek, [text[:cut], text[cut:], EciIndex(7)]) == (pcm, marks), (text, cut)
        for text in ('κατά', 'κατά.', 'κατά, Αθήνα', 'είμαι κατά'):
            assert '[.0ka.1ta]' in phonemes(greek, text), text
        assert '[.1ka.0ta]' in phonemes(greek, 'κάτα Αθήνα')

        # These unaccented function words weaken in a sentence, while a
        # written accent and a word spoken on its own keep their stress.
        for word in ('κι', 'τη', 'δε', 'πως', 'αν', 'σαν', 'ως', 'στου', 'στης', 'στων'):
            alone = phonemes(greek, word)
            context = phonemes(greek, word + ' σπίτι')
            assert '[.1' in alone and '[.0' in context, (word, alone, context)
        for word in ('πώς', 'πού', 'ή'):
            assert '[.1' in phonemes(greek, word + ' σπίτι'), word
        # Full Greek negatives are normally stress-bearing in speech even
        # though the written monosyllables do not carry an accent mark.
        for word, expected in (('δεν', '[.1Den]'), ('μην', '[.1min]')):
            assert expected in phonemes(greek, word + ' θέλω'), word

        # Medial γγ normally uses the same nasal and stop as γκ. Synthetic
        # spelling pairs isolate that choice without changing the vowels.
        for normal, same_sounds in (('άγγελος', 'άγκελος'),
                                    ('αγγούρι', 'αγκούρι'),
                                    ('αγγλικά', 'αγκλικά'),
                                    ('εγγόνι', 'εγκόνι'),
                                    ('συγγενής', 'συγκενής')):
            assert phonemes(greek, normal) == phonemes(greek, same_sounds), normal
            assert render(greek, [normal, EciIndex(7)]) == render(greek, [same_sounds, EciIndex(7)]), normal
        # Some learned compounds retain a nasal followed by fricative γ.
        for word, different in (('έγγραφο', 'έγκραφο'),
                                ('συγγραφέας', 'συγκραφέας'),
                                ('εγγεγραμμένος', 'εγκεγραμμένος'),
                                ('εγγενής', 'εγκενής'),
                                ('έγγαμος', 'έγκαμος'),
                                ('συγγνώμη', 'συγκνώμη')):
            assert render(greek, [word, EciIndex(7)]) != render(greek, [different, EciIndex(7)]), word

        # The native γκ rules are unchanged, including at the start of a word.
        for word in ('άγκυρα', 'αγκάθι', 'γκολ', 'εγκάρδιο'):
            if old:
                assert phonemes(greek, word) == phonemes(old, word), word
        for rate in range(7):
            greek.set_parameter(SAMPLE_RATE, rate)
            if old:
                old.set_parameter(SAMPLE_RATE, rate)
            for gender in (0, 1):
                greek.set_parameter(GENDER, gender)
                if old:
                    old.set_parameter(GENDER, gender)
                for text in ('από την Αθήνα', 'Manhattan working ongoing', 'άγκυρα και αγκάθι'):
                    pcm, marks = render(greek, [text, EciIndex(7)])
                    assert pcm and marks == [(7, len(pcm))], (rate, gender, text)
                    if old and text == 'άγκυρα και αγκάθι':
                        assert (pcm, marks) == render(old, [text, EciIndex(7)]), (rate, gender, text)
    finally:
        greek.close()
        if old:
            old.close()
    print('Greek connected speech, English nasals, syllabic n and native gamma passed')


if __name__ == '__main__':
    main()
