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
    dialects = (0x10000, 0x10001, 0x20000, 0x20001, 0x30000, 0x30001,
                0x40000, 0x50000, 0x60000, 0x70000, 0x80000, 0x90000,
                0xa0000, 0x120000, 0x130000)
    for dialect in dialects:
        prefix = {0x10000:'ENU', 0x10001:'ENG', 0x40000:'DEU'}.get(dialect)
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
        try:
            for joined, spaced in (('SAPI5', 'SAPI 5'), ('NASA3', 'NASA 3'),
                                   ('CAT23', 'CAT 23'), ('NVDA2026', 'NVDA 2026'),
                                   ('HTML5', 'HTML 5'),
                                   ('RICHEDIT50W', 'RICHEDIT 50 W'),
                                   ('RICHEDIT50TEST', 'RICHEDIT 50 TEST'),
                                   ('SAPI5API2W', 'SAPI 5 API 2 W')):
                for segmentation, camel in ((0, 0), (1, 0), (0, 1), (1, 1)):
                    engine.set_parameter(WORD_SEGMENTATION, segmentation)
                    engine.set_parameter(CAMEL_CASE, camel)
                    assert groups(engine._phonemes_for_probe(joined)) == groups(engine._phonemes_for_probe(spaced)), (
                        dialect, joined, segmentation, camel)
                    assert render(engine, [EciIndex(1), joined, EciIndex(2)]) == \
                           render(engine, [EciIndex(1), spaced, EciIndex(2)]), (dialect, joined)
                    count += 1
            # A caller's spelling and protected replacement remain under the
            # caller's control, even with both segmentation options enabled.
            for text in ('SAPI5', 'NVDA2026', 'RICHEDIT50W'):
                assert render(engine, [EciText(text, character_mode=True)]) == \
                       render(engine, [EciText(text, character_mode=True, segmentation_guarded=True)])
            # An index within a word remains an original event and must not
            # be moved into a synthetic word boundary.
            pcm, marks = render(engine, ['SAPI', EciIndex(71), '5'])
            assert pcm and [mark for mark, _ in marks] == [71]
            assert (pcm, marks) == render(engine, ['SAPI', EciIndex(71), ' 5'])
            assert render(engine, ['SA', EciIndex(72), 'PI5']) == \
                   render(engine, ['SA', EciIndex(72), 'PI 5'])
            assert render(engine, ['RICHEDIT50', EciIndex(73), 'W']) == \
                   render(engine, ['RICHEDIT 50', EciIndex(73), ' W'])
            if prefix:
                with tempfile.TemporaryDirectory() as temp:
                    folder = Path(temp)
                    (folder / (prefix + 'main.dic')).write_bytes(
                        b'SAPI5\t`[.1tEst]\nRICHEDIT50W\t`[.1tEst]\n')
                    engine.set_dictionary_path(folder)
                    for text in ('SAPI5', 'RICHEDIT50W'):
                        assert 'tEst' in engine._phonemes_for_probe(text), (dialect, text, 'dictionary')
        finally:
            engine.close()
    print(f'Passed {count} uppercase-number readings, spelling, indexes and dictionary cases.')


if __name__ == '__main__':
    main()
