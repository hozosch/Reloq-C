"""Keep an uppercase word's normal reading before an attached number."""
import argparse
from pathlib import Path
import re
import tempfile

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciText, WORD_SEGMENTATION, CAMEL_CASE
from test_eci_german_clock_native import render


def groups(value):
    return re.findall(r'`\[([^]]+)\]', value)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    count = 0
    for dialect, prefix in ((0x10000, 'ENU'), (0x10001, 'ENG'), (0x40000, 'DEU')):
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
        try:
            for word, digits in (('SAPI', '5'), ('NASA', '3'), ('CAT', '23'),
                                 ('NVDA', '2026'), ('HTML', '5')):
                for segmentation, camel in ((0, 0), (1, 0), (0, 1), (1, 1)):
                    engine.set_parameter(WORD_SEGMENTATION, segmentation)
                    engine.set_parameter(CAMEL_CASE, camel)
                    joined, spaced = word + digits, word + ' ' + digits
                    assert groups(engine._phonemes_for_probe(joined)) == groups(engine._phonemes_for_probe(spaced)), (
                        dialect, joined, segmentation, camel)
                    assert render(engine, [EciIndex(1), joined, EciIndex(2)]) == \
                           render(engine, [EciIndex(1), spaced, EciIndex(2)]), (dialect, joined)
                    count += 1
            # A caller's spelling and protected replacement remain under the
            # caller's control, even with both segmentation options enabled.
            for text in ('SAPI5', 'NVDA2026'):
                assert render(engine, [EciText(text, character_mode=True)]) == \
                       render(engine, [EciText(text, character_mode=True, segmentation_guarded=True)])
            # An index within a word remains an original event and must not
            # be moved into a synthetic word boundary.
            pcm, marks = render(engine, ['SAPI', EciIndex(71), '5'])
            assert pcm and [mark for mark, _ in marks] == [71]
            assert (pcm, marks) == render(engine, ['SAPI', EciIndex(71), ' 5'])
            assert render(engine, ['SA', EciIndex(72), 'PI5']) == \
                   render(engine, ['SA', EciIndex(72), 'PI 5'])
            with tempfile.TemporaryDirectory() as temp:
                folder = Path(temp)
                (folder / (prefix + 'main.dic')).write_bytes(b'SAPI5\t`[.1tEst]\n')
                engine.set_dictionary_path(folder)
                assert 'tEst' in engine._phonemes_for_probe('SAPI5'), (dialect, 'dictionary')
        finally:
            engine.close()
    print(f'Passed {count} uppercase-number readings, spelling, indexes and dictionary cases.')


if __name__ == '__main__':
    main()
