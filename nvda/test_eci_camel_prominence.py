"""Check CamelCase stress and preserve ordinary speech and caller controls."""
import argparse
from pathlib import Path
import re
from tempfile import TemporaryDirectory

from _reloq_eci_direct import (CAMEL_CASE, PROCESS_TAGS, WORD_SEGMENTATION,
                               DirectEciEngine, EciIndex, EciText)
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    checks = 0
    for dialect in (0x10000, 0x10001):
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect,
                                 encoding='utf8', native_voice=True)
        try:
            for word_seg in (0, 1):
                engine.set_parameter(WORD_SEGMENTATION, word_seg)
                for tags in (0, 1):
                    engine.set_parameter(PROCESS_TAGS, tags)
                    for text in ('radio wave', 'help file', 'radio wave is ready',
                                 'my Window', 'HTTP Server'):
                        engine.set_parameter(CAMEL_CASE, 0)
                        normal = render(engine, [text])
                        engine.set_parameter(CAMEL_CASE, 1)
                        assert render(engine, [text]) == normal, (dialect, text, 'ordinary text changed')
                        checks += 1
                    engine.set_parameter(CAMEL_CASE, 1)
                    # The second word is no longer marked unaccented, while
                    # the following sentence retains its normal emphasis.
                    for text in ('radioWave', 'helpFile', 'radioWave is ready'):
                        output = engine._phonemes_for_probe(text)
                        groups = re.findall(r'(?:(`(?:00|[0-9]))\s+)?`\[([^]]+)\]', output)
                        assert len(groups) >= 2 and groups[1][0] not in ('`0', '`00'), (dialect, tags, text, output)
                        if text.endswith('ready'):
                            assert re.search(r'`2\s+`\[\.1rE', output), (dialect, output, 'later emphasis lost')
                        checks += 1
                    assert render(engine, ['radioWave'])[0] != render(engine, ['radio wave'])[0]
                    assert render(engine, ['helpFile'])[0] != render(engine, ['help file'])[0]
                    checks += 2
                    # Compare the actual audio with the documented ECI tag.
                    for joined, tagged in (('radioWave', 'radio `1 Wave'),
                                           ('helpFile', 'help `1 File'),
                                           ('radioWave is ready', 'radio `1 Wave is ready'),
                                           ('radioWaveTest', 'radio `1 Wave `1 Test')):
                        engine.set_parameter(CAMEL_CASE, 1)
                        engine.set_parameter(PROCESS_TAGS, tags)
                        actual = render(engine, [joined])[0]
                        engine.set_parameter(CAMEL_CASE, 0)
                        engine.set_parameter(PROCESS_TAGS, 1)
                        expected = render(engine, [tagged])[0]
                        assert actual == expected, (dialect, tags, joined, 'ECI tag differs')
                        checks += 1
                engine.set_parameter(CAMEL_CASE, 1)
                engine.set_parameter(PROCESS_TAGS, 1)
                for tag in ('`0', '`1', '`2'):
                    for separator in (' ', '\t', '\n'):
                        assert render(engine, [tag + separator + 'radioWave']) == render(engine, [tag + separator + 'radio Wave']), (dialect, tag, separator, 'caller prominence')
                        checks += 1
            engine.set_parameter(PROCESS_TAGS, 0)
            engine.set_parameter(CAMEL_CASE, 0)
            spelling = render(engine, [EciText('radioWave', character_mode=True)])
            guarded = render(engine, [EciText('radioWave', segmentation_guarded=True)])
            internal = render(engine, ['radio', EciIndex(71), 'Wave'])
            engine.set_parameter(CAMEL_CASE, 1)
            assert render(engine, [EciText('radioWave', character_mode=True)]) == spelling
            assert render(engine, [EciText('radioWave', segmentation_guarded=True)]) == guarded
            assert render(engine, ['radio', EciIndex(71), 'Wave']) == internal
            _, events = render(engine, [EciIndex(11), 'radioWave', EciIndex(12), ' now'])
            assert [event for event, _ in events] == [11, 12], events
            checks += 4
        finally:
            engine.close()

    # Other language modules do not all interpret `1 like English. In
    # particular, Japanese may speak the digit and Italian can de-emphasize
    # the word. Retain their original split behavior instead.
    for dialect in (0x40000, 0x20000, 0x20001, 0x30000, 0x30001,
                    0x50000, 0x70000, 0x90000, 0x60000, 0x80000,
                    0xa0000, 0x120000, 0x130000):
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect,
                                 encoding='utf8', native_voice=True)
        try:
            for tags in (0, 1):
                engine.set_parameter(PROCESS_TAGS, tags)
                engine.set_parameter(CAMEL_CASE, 0)
                expected = render(engine, ['radio Wave is ready'])
                engine.set_parameter(CAMEL_CASE, 1)
                assert render(engine, ['radioWave is ready']) == expected, (dialect, tags, 'non-English speech changed')
                checks += 1
        finally:
            engine.close()

    with TemporaryDirectory() as temporary:
        folder = Path(temporary)
        (folder / 'ENUmain.dic').write_bytes(b'radioWave\t`[.1tEst]\nhelp\t`[.1hElp]\n')
        (folder / 'ENURoot.dic').write_bytes(b'dectalk\t`[.1dEk.2tck]\n')
        engine = DirectEciEngine(args.library.resolve(), dialect=0x10000,
                                 encoding='utf8', native_voice=True)
        try:
            engine.set_dictionary_path(folder)
            for text in ('radioWave', 'decTalk'):
                engine.set_parameter(CAMEL_CASE, 0)
                normal = render(engine, [text])
                engine.set_parameter(CAMEL_CASE, 1)
                assert render(engine, [text]) == normal, (text, 'complete dictionary entry')
                checks += 1
            assert 'tEst' in engine._phonemes_for_probe('radioWave')
            engine.set_parameter(CAMEL_CASE, 1)
            engine.set_parameter(PROCESS_TAGS, 0)
            actual = render(engine, ['helpFile'])
            engine.set_parameter(CAMEL_CASE, 0)
            engine.set_parameter(PROCESS_TAGS, 1)
            assert actual == render(engine, ['help `1 File']), 'component dictionary pronunciation'
            checks += 2
        finally:
            engine.close()
    print(f'CamelCase prominence: {checks} phrase, dictionary, marker and character cases passed')


if __name__ == '__main__':
    main()
