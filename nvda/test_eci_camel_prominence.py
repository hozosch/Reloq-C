"""Check CamelCase prominence without changing ordinary phrase intonation."""
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
                for text in ('radio wave', 'help file', 'radio wave is ready',
                             'my Window', 'HTTP Server'):
                    engine.set_parameter(CAMEL_CASE, 0)
                    normal = render(engine, [text])
                    engine.set_parameter(CAMEL_CASE, 1)
                    assert render(engine, [text]) == normal, (dialect, text, 'ordinary text changed')
                    checks += 1
                for joined, spaced in (('radioWave', 'radio wave'), ('helpFile', 'help file')):
                    output = engine._phonemes_for_probe(joined)
                    groups = re.findall(r'`(00|[0-9])\s+`\[([^]]+)\]', output)
                    assert len(groups) == 2 and [item[0] for item in groups] == ['0', '2'], (dialect, joined, output)
                    assert render(engine, [joined])[0] != render(engine, [spaced])[0], (dialect, joined)
                    checks += 1
                assert render(engine, ['myWindow']) == render(engine, ['my Window']), (dialect, 'weak first word')
                assert render(engine, ['HTTPServer']) == render(engine, ['HTTP Server']), (dialect, 'acronym')
                phrase = engine._phonemes_for_probe('radioWave is ready')
                assert re.search(r'`2\s+`\[\.1rE', phrase), (dialect, phrase, 'later phrase emphasis lost')
                normal = engine._phonemes_for_probe('radio wave is ready')
                assert re.search(r'`2\s+`\[\.1rE', normal), (dialect, normal)
                checks += 4
            # The character-reading and protected dictionary paths retain
            # the original text. Index events inside a word stay in place.
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
            engine.set_parameter(PROCESS_TAGS, 1)
            assert render(engine, ['`2 radioWave']) == render(engine, ['`2 radio Wave']), 'caller prominence'
            checks += 5
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
            for text in ('radioWave', 'decTalk', 'helpFile'):
                engine.set_parameter(CAMEL_CASE, 0)
                normal = render(engine, [text])
                engine.set_parameter(CAMEL_CASE, 1)
                if text != 'helpFile':
                    assert render(engine, [text]) == normal, (text, 'complete dictionary entry')
                else:
                    assert render(engine, [text]) == render(engine, ['help File']), (text, 'component dictionary')
                checks += 1
            assert 'tEst' in engine._phonemes_for_probe('radioWave')
        finally:
            engine.close()
    print(f'CamelCase prominence: {checks} phrase, dictionary, marker and character cases passed')


if __name__ == '__main__':
    main()
