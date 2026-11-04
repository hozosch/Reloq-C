"""The German E-I tuning is audible, scoped and repeatable on real PCM."""
import argparse
import ctypes
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER,
    GERMAN_GLIDE_BACKNESS, GERMAN_GLIDE_OPENING, GERMAN_GLIDE_SOFTNESS, PROCESS_TAGS, SAMPLE_RATE)
from test_eci_german_clock_native import phonemes, render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library.resolve()
    first = DirectEciEngine(library, dialect=0x40000, encoding='utf8', experimental_generic=True)
    other = DirectEciEngine(library, dialect=0x40000, encoding='utf8', experimental_generic=True)
    checks = 0
    try:
        defaults = {GERMAN_GLIDE_OPENING: 85, GERMAN_GLIDE_BACKNESS: 45,
                    GERMAN_GLIDE_SOFTNESS: 45}
        for param, default in defaults.items():
            assert first._dll.eciGetParam(first._handle, param) == default
            assert first._dll.eciSetParam(first._handle, param, -1) < 0
            assert first._dll.eciSetParam(first._handle, param, 101) < 0
            assert first._dll.eciGetParam(first._handle, param) == default
        for gender in (0, 1):
            for engine in (first, other):
                engine.set_parameter(GENDER, gender)
                engine.set_parameter(PROCESS_TAGS, 1)
            for rate in (0, 1, 3, 2, 4, 5, 6):
                for engine in (first, other):
                    engine.set_parameter(SAMPLE_RATE, rate)
                word = ['Homepage', EciIndex(7)]
                base = render(first, word)
                assert base == render(other, word)
                unaffected = {text: render(first, [text]) for text in ('Bein', 'drei', 'ja', 'Kamera')}
                transcription = phonemes(first, 'Homepage')
                for param, default in defaults.items():
                    for value in (0, 100):
                        first.set_parameter(param, value)
                        changed = render(first, word)
                        assert changed[0] != base[0] and changed[1][-1][0] == 7, (gender, rate, param, value)
                        assert render(first, ['Homepage']) == render(first, [transcription]), (gender, rate, param, value)
                        for text, before in unaffected.items():
                            assert render(first, [text]) == before, (gender, rate, param, value, text)
                        assert render(other, word) == base, (gender, rate, 'engine setting leaked')
                        checks += 1
                    first.set_parameter(param, default)
                    assert render(first, word) == base, (gender, rate, 'reset')
        first.set_parameter(GERMAN_GLIDE_OPENING, 73)
        first.set_parameter(GERMAN_GLIDE_BACKNESS, 82)
        first.set_parameter(GERMAN_GLIDE_SOFTNESS, 33)
        first._dll.eciReset.argtypes = (ctypes.c_void_p,)
        first._dll.eciReset.restype = ctypes.c_int
        assert first._dll.eciReset(first._handle) == 1
        for param, default in defaults.items():
            assert first._dll.eciGetParam(first._handle, param) == default
    finally:
        first.close()
        other.close()
    print(f'German glide: {checks} tuning, scope, phoneme and reset checks')


if __name__ == '__main__':
    main()
