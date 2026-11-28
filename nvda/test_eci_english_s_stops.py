"""English s+voiceless-stop release through the public ECI speech path."""
import argparse
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, EciIndex, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from test_eci_german_clock_native import phonemes, render


def engine(library, language):
    result = DirectEciEngine(library.resolve(), dialect=language,
                             encoding='utf8', native_voice=True)
    result.set_parameter(PROCESS_TAGS, 1)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--dictionary', type=Path)
    parser.add_argument('--shorter-s', action='store_true',
                        help='allow the experimental s duration to move later indices earlier')
    args = parser.parse_args()
    changed = ('scout', 'skill', 'screen', 'transcribe', 'spoon', 'spill', 'stop', 'still')
    unchanged = ('coat', 'paper', 'top', 'good dog', 'this car', 'ask',
                 '`[.1skhWt]', '`[.1trAnsh.2krIpt]')
    checks = 0
    for dialect in (0x10000, 0x10001):
        current, before = engine(args.library, dialect), engine(args.reference, dialect)
        try:
            for text in changed + unchanged:
                assert phonemes(current, text) == phonemes(before, text), (dialect, text)
                checks += 1
            for rate in (1, 3, 6):
                for speed in (50, 105, 220, 250):
                    for item in (current, before):
                        item.set_parameter(SAMPLE_RATE, rate)
                        item.set_parameter(NATIVE_SPEED, speed)
                    for text in changed:
                        actual = render(current, [text, EciIndex(7)])
                        old = render(before, [text, EciIndex(7)])
                        assert actual[0] != old[0], (hex(dialect), rate, speed, text)
                        if args.shorter_s:
                            assert len(actual[0]) <= len(old[0]), (hex(dialect), rate, speed, text)
                            assert [i for i, _ in actual[1]] == [i for i, _ in old[1]]
                            assert actual[1][-1][1] == len(actual[0])
                        else:
                            assert len(actual[0]) == len(old[0]) and actual[1] == old[1], \
                                (hex(dialect), rate, speed, text)
                        checks += 1
                    for text in unchanged:
                        assert render(current, [text, EciIndex(7)]) == \
                               render(before, [text, EciIndex(7)]), \
                               (hex(dialect), rate, speed, text)
                        checks += 1
            sentence = 'Please transcribe the scout, stop and spin.'
            expected = render(current, [sentence, EciIndex(11)])
            for cut in (1, 7, 17, 28, len(sentence)-1):
                assert render(current, [sentence[:cut], sentence[cut:], EciIndex(11)]) == expected
                checks += 1
        finally:
            current.close()
            before.close()
    if args.dictionary:
        current, before = engine(args.library, 0x10000), engine(args.reference, 0x10000)
        try:
            for item in (current, before):
                item.set_dictionary_path(args.dictionary.resolve())
            for word in ('transcript', 'transcranial', 'discretization'):
                assert render(current, [word]) == render(before, [word]), word
                checks += 1
            for word in ('transcribe', 'scout'):
                assert render(current, [word]) != render(before, [word]), word
                checks += 1
        finally:
            current.close()
            before.close()
    # Unrelated languages retain their own stop rules.
    for dialect, text in ((0x40000, 'Du wachst, wenn ihr wachst.'),
                          (0x80000, 'Sortez de cette situation.')):
        current, before = engine(args.library, dialect), engine(args.reference, dialect)
        try:
            assert render(current, [text]) == render(before, [text]), hex(dialect)
            checks += 1
        finally:
            current.close()
            before.close()
    print(f'English s+stop: {checks} phoneme, audio, duration, index and isolation checks')


if __name__ == '__main__':
    main()
