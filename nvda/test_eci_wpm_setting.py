"""A WPM setting follows the native tag without enabling text tags."""
import argparse
from pathlib import Path

from test_eci_inline_queue import Engine, TEXTS
from _reloq_eci_direct import DirectEciEngine, EciProsody, NATIVE_WPM as WPM, PROCESS_TAGS, ORIGINAL_ACOUSTIC_CLOCK
from test_eci_german_clock_native import render

NATIVE_WPM = 0x1017


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--original-clock', action='store_true')
    args = parser.parse_args()
    library = args.library
    checks = 0
    for language, parts in TEXTS.items():
        engine = Engine(library, language)
        try:
            assert engine.lib.eciSetParam(engine.h,ORIGINAL_ACOUSTIC_CLOCK,int(args.original_clock)) >= 0
            engine.voice(6, 50)
            text = ''.join(parts)
            for wpm in (50, 150, 300, 350, 400, 434, 435, 450, 500, 550, 600):
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
            # Queued WPM changes preserve one connected sentence even with
            # ordinary caller-supplied tags disabled.
            engine.tags(1)
            assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, 0) >= 0
            expected = engine.speak(f'`vwpm150 {parts[0]}`vwpm300 {parts[1]}`vwpm150 {parts[2]}')
            engine.tags(0)
            assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, 150) >= 0
            engine.add(parts[0])
            assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, 300) >= 0
            engine.add(parts[1])
            assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, 150) >= 0
            engine.add(parts[2])
            assert engine.finish() == expected, (hex(language), 'queued WPM')
            assert engine.lib.eciGetParam(engine.h, NATIVE_WPM) == 150
            checks += 2
            for first, second in ((434, 435), (435, 434)):
                engine.tags(1)
                assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, 0) >= 0
                tagged = engine.speak(f'`vwpm{first} {parts[0]}`vwpm{second} {parts[1]}{parts[2]}')
                engine.tags(0)
                assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, first) >= 0
                engine.add(parts[0])
                assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, second) >= 0
                engine.add(parts[1] + parts[2])
                assert engine.finish() == tagged, (hex(language), first, second, 'adjacent WPM')
                checks += 1
        finally:
            engine.close()

    for language in (0x10000, 0x40000, 0x60000, 0x80000, 0x120000):
        engine = Engine(library, language)
        try:
            assert engine.lib.eciSetParam(engine.h,ORIGINAL_ACOUSTIC_CLOCK,int(args.original_clock)) >= 0
            engine.voice(6, 50)
            text = TEXTS[language][1] + '.'
            for wpm in (1, 4):
                engine.tags(1)
                assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, 0) >= 0
                tagged = engine.speak(f'`vwpm{wpm} {text}')
                engine.tags(0)
                assert engine.lib.eciSetParam(engine.h, NATIVE_WPM, wpm) >= 0
                assert engine.speak(text) == tagged, (hex(language), wpm)
                checks += 1
        finally:
            engine.close()

    engine = Engine(library, 0x40000)
    try:
        assert engine.lib.eciSetParam(engine.h,ORIGINAL_ACOUSTIC_CLOCK,int(args.original_clock)) >= 0
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
    for language in (0x10000, 0x40000, 0x60000):
        engine = DirectEciEngine(library.resolve(), dialect=language, encoding='utf8', native_voice=True)
        try:
            engine.set_parameter(ORIGINAL_ACOUSTIC_CLOCK,int(args.original_clock))
            engine.set_parameter(PROCESS_TAGS, 0)
            engine.set_parameter(WPM, 150)
            text = TEXTS[language]
            changed = render(engine, [text[0], EciProsody(8, 300), text[1], EciProsody(8, 150), text[2]])
            engine.set_parameter(WPM, 0)
            engine.set_parameter(PROCESS_TAGS, 1)
            tagged = render(engine, [f'`vwpm150 {text[0]}`vwpm300 {text[1]}`vwpm150 {text[2]}'])
            assert changed == tagged, hex(language)
            checks += 1
        finally:
            engine.close()
    print(f'WPM setting: {checks} language, tag isolation and bounds checks passed')


if __name__ == '__main__':
    main()
