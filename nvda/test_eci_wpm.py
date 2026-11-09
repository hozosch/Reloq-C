"""Native VWPM tags through the real ECI UTF-8 and language interfaces."""
import argparse
from pathlib import Path

from test_eci_inline_queue import Engine, TEXTS


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library
    checks = 0
    for language, (prefix, word, suffix) in TEXTS.items():
        e = Engine(library, language)
        try:
            e.voice(6, 50)
            text = prefix + word + suffix
            samples = {wpm: len(e.speak(f'`vwpm{wpm} {text}'))
                       for wpm in (50, 150, 178, 200, 300)}
            assert samples[50] > samples[150] > samples[178] > samples[200] > samples[300], (hex(language), samples)
            checks += len(samples)
            # The same setting applies in ordinary and UTF-8 ECI input.
            if language in (0x10000, 0x40000):
                e.add('`vwpm150 ' + text, legacy=True)
                assert len(e.finish()) == samples[150], hex(language)
                checks += 1
        finally:
            e.close()

    e = Engine(library, 0x40000)
    try:
        e.voice(6, 50)
        text = 'Hallo Welt.'
        base = e.speak(text)
        wpm = e.speak('`vwpm150 ' + text)
        assert e.speak('`vwpm178 ' + text) == base
        assert e.speak('`vs100 `vwpm150 ' + text) == wpm
        assert e.speak('`vwpm150 `vs100 ' + text) == e.speak('`vs100 ' + text)
        assert e.speak('`vwpm150 `v1 ' + text) == e.speak('`v1 ' + text)
        assert e.speak('`v1 `vwpm150 ' + text) == wpm
        assert e.speak('Hal`vwpm150lo Welt.') == wpm
        close = [len(e.speak(f'`vwpm{value} {text}')) for value in (149, 150, 151)]
        assert close[0] > close[1] > close[2], close
        checks += 7

        for invalid in ('vwpm0', 'vwpm6001', 'vwpm-1'):
            # Invalid controls must not reach the native WPM division.
            assert e.speak('`' + invalid + ' ' + text) == e.speak(' ' + invalid + ' ' + text), invalid
            checks += 1
        # A command survives a language switch; a later VS restores the
        # native scale instead of inheriting WPM into the next language.
        first = '`vwpm150 Hallo. `l1.0 Hello world.'
        explicit = '`vwpm150 Hallo. `l1.0 `vwpm150 Hello world.'
        assert e.speak(first) == e.speak(explicit)
        assert e.speak(first + ' `vs50 Again.') == e.speak(explicit + ' `vs50 Again.')
        checks += 2
        # Enabled and disabled tag modes remain separate.
        enabled = e.speak('`vwpm150 ' + text)
        e.tags(0)
        assert e.speak('`vwpm150 ' + text) != enabled
        checks += 1
    finally:
        e.close()
    print(f'VWPM: {checks} native duration, language, scope and tag checks passed')


if __name__ == '__main__':
    main()
