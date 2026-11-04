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
            'video-encoder', 'macro-encoder', '„audio-encoder“',
            'vorne-audio-encoder-hinten',
        )),
        (0x10000, 'Please ', ' continue.', (
            "bad'hhesiron", "badh'hhesa", 'badhhesa', 'caesure', 'Nietzsche',
        )),
        (0x10001, 'Please ', ' continue.', ('caesure', 'Nietzsche')),
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
    print(f'Crash regressions: {checks} audible and index checks')


if __name__ == '__main__':
    main()
