"""Keep st/sp release planning when another s occurs later in the text."""
import argparse
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, EciIndex, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from test_eci_german_clock_native import phonemes, render


def engine(path, dialect):
    result = DirectEciEngine(path.resolve(), dialect=dialect,
                             encoding='utf8', native_voice=True)
    result.set_parameter(PROCESS_TAGS, 1)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--dictionary', type=Path)
    args = parser.parse_args()
    changed = ('space', 'spaced', 'spice', 'states', 'steps', 'stamps',
               'stops', 'sports', 'spikes', 'spokes', 'spokesman',
               '`[.1spes]', '`[.1stets]', 'space station')
    controls = ('pace', 'tate', 'spill', 'still', 'speak', 'stay', 'stop',
                'spices', 'special', 'cat', 'cry', 'scat', 'scry', 'skates',
                'scouts', 'discuss', 'school', 'misrate', 'Israeli', 'zero',
                'shrink', 'ask', 'this car')
    checks = 0
    for dialect in (0x10000, 0x10001):
        current, before = engine(args.library, dialect), engine(args.reference, dialect)
        try:
            for text in changed + controls:
                assert phonemes(current, text) == phonemes(before, text), (hex(dialect), text)
                checks += 1
            for rate in (1, 3, 6):
                for speed in (50, 105, 220, 250):
                    for item in (current, before):
                        item.set_parameter(SAMPLE_RATE, rate)
                        item.set_parameter(NATIVE_SPEED, speed)
                    for text in changed + controls:
                        actual = render(current, [text, EciIndex(7)])
                        old = render(before, [text, EciIndex(7)])
                        assert len(actual[0]) == len(old[0]) and actual[1] == old[1], \
                            (hex(dialect), rate, speed, text, 'timing')
                        if text in controls:
                            assert actual == old, (hex(dialect), rate, speed, text, 'isolation')
                        elif speed <= 105:
                            assert actual[0] != old[0], (hex(dialect), rate, speed, text, 'missing change')
                        checks += 1
            sentence = 'A space station stops beside the skates.'
            expected = render(current, [sentence, EciIndex(11)])
            for cut in (1, 8, 19, 31, len(sentence)-1):
                assert render(current, [sentence[:cut], sentence[cut:], EciIndex(11)]) == expected
                checks += 1
            if args.dictionary:
                for item in (current, before):
                    item.set_dictionary_path(args.dictionary.resolve())
                    item.set_parameter(NATIVE_SPEED, 50)
                for word in ('space', 'states', 'spice', 'steps'):
                    actual, old = render(current, [word, EciIndex(7)]), render(before, [word, EciIndex(7)])
                    assert actual[0] != old[0] and len(actual[0]) == len(old[0]) and actual[1] == old[1], word
                    checks += 1
                for word in ('skates', 'scouts', 'misrate', 'Israeli'):
                    assert render(current, [word]) == render(before, [word]), word
                    checks += 1
        finally:
            current.close()
            before.close()
    for dialect, text in ((0x40000, 'Du wachst, wenn ihr wachst.'),
                          (0x120000, 'Καλημέρα σας!')):
        current, before = engine(args.library, dialect), engine(args.reference, dialect)
        try:
            assert render(current, [text]) == render(before, [text]), hex(dialect)
            checks += 1
        finally:
            current.close()
            before.close()
    print(f'English st/sp context: {checks} phoneme, timing, dictionary and isolation checks')


if __name__ == '__main__':
    main()
