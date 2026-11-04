"""Words previously dropped by the native planner must remain audible."""
import argparse
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER, PROCESS_TAGS,
                               SAMPLE_RATE, SAMPLE_RATES)
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    cases = (
        (0x40000, 'Bitte ', ' weiter.', (
            'dane-ben', 'Dane-Ben', 'dage-gen', 'audio-encoder',
            'video-encoder', 'macro-encoder', 'audio-encode',
            'video-encode', 'macro-encode', '„audio-encoder“',
            'vorne-audio-encoder-hinten',
        )),
        (0x10000, 'Please ', ' continue.', (
            "bad'hhesiron", "badh'hhesa", 'badhhesa', 'caesure', 'cæsure',
            "h're", "h've", '2:30th', "you're'd", 'juarabcdefghij',
            'Mc Name', '03 Marble', 'example.com', 'test@example.com',
            'EURUSD10', 'Nietzsche', 'tzsche', 'abtzsche', 'recosp',
            'uncosp', 'noncosp', 'anticosp',
        )),
        (0x10001, 'Please ', ' continue.', (
            'caesure', 'cæsure', "h're", "h've", '2:30th', "you're'd",
            'juarabcdefghij', 'Mc Name', '03 Marble', 'example.com',
            'test@example.com', 'EURUSD10', 'Nietzsche', 'tzsche',
            'abtzsche', 'recosp', 'uncosp', 'noncosp', 'anticosp',
        )),
        (0x30000, 'Avant ', ' ensuite.', (
            'quil', 'quil.', 'quils', 'tranquil', 'tranquille', 'quille',
        )),
        (0x30001, 'Avant ', ' ensuite.', (
            'quil', 'quil.', 'quils', 'tranquil', 'tranquille', 'quille',
        )),
        (0x20000, 'Total ', ' gracias.', (
            '€123 456.78', '$12 345.67', '€1 234 567.89',
            '1234567890121ª', '1234567890122ª',
        )),
        (0x20001, 'Total ', ' gracias.', (
            '€123 456.78', '$12 345.67', '€1 234 567.89',
            '1234567890121ª', '1234567890122ª',
        )),
    )
    checks = 0
    for language, before, after, words in cases:
        engine = DirectEciEngine(args.library.resolve(), dialect=language,
                                 encoding='utf8', experimental_generic=True)
        try:
            engine.set_parameter(PROCESS_TAGS, 1)
            for word in words:
                standalone, _ = render(engine, [word])
                assert standalone and any(standalone), (language, word, 'silent')
                assert phonemes(engine, word).strip(), (language, word, 'no phonemes')
                audio, marks = render(engine, [before, EciIndex(1), word,
                                              EciIndex(2), after, EciIndex(3)])
                assert [mark for mark, _ in marks] == [1, 2, 3], (language, word, marks)
                p1, p2, p3 = (offset for _, offset in marks)
                # The word and following text must both reach the callback;
                # an index alone can be delivered after aborted synthesis.
                assert p1 < p2 < p3 == len(audio), (language, word, marks)
                assert any(audio[p1:p2]) and any(audio[p2:p3]), (language, word)
                checks += 3
            recovered, _ = render(engine, ['Test.'])
            assert recovered and any(recovered), (language, 'recovery')
            checks += 1
            if language in (0x30000, 0x30001):
                # French qu has a silent u here: the i is the syllabic
                # vowel and the single final l is not the glide in quille.
                for word, reference in (('quil', 'kil'), ('quils', 'kil'),
                                        ('tranquil', 'tranquille')):
                    assert phonemes(engine, word) == phonemes(engine, reference), (language, word)
                    checks += 1
                assert phonemes(engine, 'quille') != phonemes(engine, 'quil')
                checks += 1
            if language in (0x20000, 0x20001):
                # Compare the entire number, including the cents. Merely
                # checking for audio would miss dropped thousands groups.
                for symbol in ('€', '$'):
                    for digits in ('1 234', '12 345', '123 456', '1 234 567'):
                        for decimal in ('.78', ',78', '.00'):
                            amount = symbol + digits + decimal
                            reference = symbol + digits.replace(' ', '') + decimal
                            assert phonemes(engine, amount) == phonemes(engine, reference), (language, amount)
                            checks += 1
                for number in ('1234567890121', '1234567890122'):
                    sentence = 'Total ' + number + 'ª gracias.'
                    reference = 'Total ' + number + ' ª gracias.'
                    assert phonemes(engine, sentence) == phonemes(engine, reference), (language, number)
                    assert render(engine, [sentence, EciIndex(7)]) == render(
                        engine, [reference, EciIndex(7)]), (language, number)
                    checks += 2
            if language == 0x40000:
                # Both spellings of these acoustic targets must use the same
                # native plan in each model and at every supported rate.
                for gender in (0, 1):
                    engine.set_parameter(GENDER, gender)
                    for phrase in ('Homepage', 'Kann er', 'Kann er das?',
                                   'Smiley', 'Snob', 'ein Smiley', 'ein Snob'):
                        transcription = phonemes(engine, phrase)
                        for rate in SAMPLE_RATES:
                            engine.set_parameter(SAMPLE_RATE, rate)
                            assert render(engine, [phrase]) == render(
                                engine, [transcription]), (gender, rate, phrase)
                            checks += 1
        finally:
            engine.close()
    # Mandarin's English bank reuses the donor phonetics but has its own
    # pitch planner. It currently reports some interior indices at the end,
    # so check the final marker and the actual length of speech on each side.
    engine = DirectEciEngine(args.library.resolve(), dialect=0x60000,
                             encoding='utf8', experimental_generic=True)
    try:
        for word in ('recosp', 'uncosp', 'noncosp', 'tzsche'):
            standalone, marks = render(engine, [word, EciIndex(7)])
            assert len(standalone) > 10000 and marks == [(7, len(standalone))], (word, marks)
            for sentence in (word + ' test', 'test ' + word, '你好 ' + word + ' 世界'):
                audio, marks = render(engine, [sentence, EciIndex(7)])
                assert len(audio) > len(standalone) and marks == [(7, len(audio))], (word, sentence, marks)
                checks += 1
            checks += 1
    finally:
        engine.close()
    # Korean has its own handling of Latin words and likewise reports some
    # interior marks at the end. Verify the word and surrounding speech.
    engine = DirectEciEngine(args.library.resolve(), dialect=0xa0000,
                             encoding='utf8', experimental_generic=True)
    try:
        for word in ('caesure', "bad'hhesiron", 'recosp', 'uncosp', 'tzsche'):
            standalone, marks = render(engine, [word, EciIndex(7)])
            assert len(standalone) > 10000 and marks == [(7, len(standalone))], (word, marks)
            for sentence in (word + ' test', 'test ' + word):
                audio, marks = render(engine, [sentence, EciIndex(7)])
                assert len(audio) > len(standalone) and marks == [(7, len(audio))], (word, sentence, marks)
                checks += 1
            checks += 1
    finally:
        engine.close()
    print(f'Crash regressions: {checks} audible and index checks')


if __name__ == '__main__':
    main()
