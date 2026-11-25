"""Exercise the precedence switch using real ECI dictionaries and audio."""
import argparse
from pathlib import Path
from tempfile import TemporaryDirectory

from _reloq_eci_direct import (CAMEL_CASE, CAMEL_IGNORE_DICTIONARY, PROCESS_TAGS,
                               WORD_SEGMENTATION, DirectEciEngine, EciIndex, EciText)
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    checks = 0
    with TemporaryDirectory() as temporary:
        folder = Path(temporary)
        (folder / 'ENUmain.dic').write_bytes(b'radioWave\t`[.1tEst]\nradio\t`[.1fUn]\n')
        (folder / 'ENURoot.dic').write_bytes(b'dectalk\t`[.1dEk.2tck]\n')
        (folder / 'DEUmain.dic').write_bytes(b'MeinWort\t`[.1tEst]\n')
        for language, source, explicit in ((0x10000, 'radioWave', 'radio `1 Wave'),
                                            (0x10000, 'decTalk', 'dec `1 Talk'),
                                            (0x40000, 'MeinWort', 'Mein Wort')):
            engine = DirectEciEngine(args.library.resolve(), dialect=language,
                                     encoding='utf8', native_voice=True)
            try:
                engine.set_dictionary_path(folder)
                assert engine._dll.eciGetParam(engine._handle, CAMEL_IGNORE_DICTIONARY) == 0
                for word_seg in (0, 1):
                    engine.set_parameter(WORD_SEGMENTATION, word_seg)
                    engine.set_parameter(CAMEL_CASE, 0)
                    entry = render(engine, [source])
                    engine.set_parameter(CAMEL_CASE, 1)
                    assert render(engine, [source]) == entry, (source, 'default')
                    # Even if the preference is retained, it has no effect
                    # until CamelCase itself is enabled.
                    engine.set_parameter(CAMEL_IGNORE_DICTIONARY, 1)
                    engine.set_parameter(CAMEL_CASE, 0)
                    assert render(engine, [source]) == entry, (source, 'disabled')
                    engine.set_parameter(CAMEL_CASE, 1)
                    split = render(engine, [source])
                    assert split != entry, (source, 'whole-word entry still won')
                    engine.set_parameter(CAMEL_CASE, 0)
                    engine.set_parameter(PROCESS_TAGS, 1)
                    assert split == render(engine, [explicit]), (source, 'component lookup')
                    engine.set_parameter(PROCESS_TAGS, 0)
                    engine.set_parameter(CAMEL_CASE, 1)
                    guarded = render(engine, [EciText(source, segmentation_guarded=True)])
                    assert guarded == entry, (source, 'explicit guard')
                    events = render(engine, [EciIndex(41), source, EciIndex(42), ' next'])[1]
                    assert [event for event, _ in events] == [41, 42], events
                    engine.set_parameter(CAMEL_IGNORE_DICTIONARY, 0)
                    assert render(engine, [source]) == entry, (source, 'restore')
                    checks += 7
            finally:
                engine.close()
    print(f'CamelCase dictionary priority: {checks} native audio, guard and source-index checks passed')


if __name__ == '__main__':
    main()
