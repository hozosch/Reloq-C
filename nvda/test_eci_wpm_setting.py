"""A WPM setting follows the native tag without enabling text tags."""
import argparse
from pathlib import Path

from test_eci_inline_queue import Engine, TEXTS

NATIVE_WPM = 0x1017


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library
    checks = 0
    for language, parts in TEXTS.items():
        engine = Engine(library, language)
        try:
            engine.voice(6, 50)
            text = ''.join(parts)
            for wpm in (50, 150, 300):
                engine.tags(1)
                assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, 0) >= 0
                tagged = engine.speak(f'`vwpm{wpm} {text}')
                engine.tags(0)
                assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, wpm) >= 0
                configured = engine.speak(text)
                assert configured == tagged, (hex(language), wpm, len(configured), len(tagged))
                assert engine.lib.eciGetParam(engine.h, NATIVE_WPM) == wpm
                checks += 1
            assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, -1) == -1
            assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, 6001) == -1
            checks += 2
        finally:
            engine.close()

    engine = Engine(library, 0x40000)
    try:
        engine.voice(6, 50)
        assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, 150) >= 0
        engine.tags(0)
        literal = engine.speak('`vwpm300 Hallo Welt.')
        engine.tags(1)
        controlled = engine.speak('`vwpm300 Hallo Welt.')
        assert literal != controlled
        assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, 0) >= 0
        assert controlled == engine.speak('`vwpm300 Hallo Welt.')
        checks += 2
    finally:
        engine.close()
    print(f'WPM setting: {checks} language, tag isolation and bounds checks passed')


if __name__ == '__main__':
    main()
