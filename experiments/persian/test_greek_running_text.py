"""Keep Greek alphabet names in running Persian and Pinglish speech."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciLanguage
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH, make_language_engine


def render(engine, parts):
    audio, marks = bytearray(), []
    assert engine.speak(parts, audio.extend, lambda value: marks.append((value, len(audio))))
    assert audio
    return bytes(audio), marks


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    library = args.library.resolve()
    dll = (ctypes.WinDLL if sys.platform == 'win32' else ctypes.CDLL)(str(library))
    greek = dll.eciUnicodeGreekName
    greek.argtypes = (ctypes.c_uint32, ctypes.c_char_p, ctypes.c_size_t)
    greek.restype = ctypes.c_int
    buffer = ctypes.create_string_buffer(128)
    for scalar, expected in (('α', b'alpha'), ('β', b'beta'), ('Ω', b'omega')):
        assert greek(ord(scalar), buffer, len(buffer)) == 1 and buffer.value == expected
    assert greek(ord('Ж'), buffer, len(buffer)) == 0
    engine = make_language_engine(library, library, None, Path('/unavailable'), Path('/unavailable'))
    try:
        for profile, before, after in ((PERSIAN_PROBE, 'سلام ', ' دنیا'),
                                       (PINGLISH_PROBE, 'salam ', ' donya')):
            engine.select_voice(profile.id, '1')
            actual = (EciIndex(10), before, EciIndex(11), 'αβ', EciIndex(12), after, EciIndex(13))
            named = (EciIndex(10), before, EciLanguage(US_ENGLISH.id, '1'), EciIndex(11),
                     'alpha beta', EciIndex(12), EciLanguage(profile.id, '1'), after, EciIndex(13))
            assert render(engine, actual) == render(engine, named), profile.locale
            # A decomposed accent can be separated from its letter by source
            # marks; it must not become a separately spoken Unicode name.
            plain = (EciIndex(10), before + 'α', EciIndex(11), EciIndex(12), after, EciIndex(13))
            accented = (EciIndex(10), before + 'α', EciIndex(11), '\u0301', EciIndex(12), after, EciIndex(13))
            assert render(engine, accented) == render(engine, plain), profile.locale
    finally:
        engine.close()
    print('Passed native Greek alphabet names and mixed Persian/Pinglish source-mark routing.')


if __name__ == '__main__':
    main()
