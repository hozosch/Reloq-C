"""Check the Persian text rules embedded in the actual acoustic DLL."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    for name in ('eciPersianProjectUtf8', 'eciPersianRelease', 'reloq_fa_direct_text',
                 'reloq_fa_direct_project', 'reloq_fa_direct_number', 'reloq_fa_direct_letter',
                 'eciPersianCharacter', 'reloq_fa_direct_character',
                 'reloq_fa_direct_free', 'reloq_fa_pinglish_word'):
        getattr(dll, name)
    direct, free = dll.reloq_fa_direct_text, dll.reloq_fa_direct_free
    direct.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                       ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)), ctypes.POINTER(ctypes.c_size_t))
    direct.restype = ctypes.c_int
    free.argtypes, free.restype = (ctypes.c_void_p,), None
    examples = {
        'به علاوه': "`00 `[.0be] `[.0A.0la.1v'e:']",
        'بعلاوه': "`[.0be.0A.0la.1v'e:']",
        'علامت': '`[.0A.0la.1mAt]',
        'بدبخت': '`[.0bAd.1bAGt]',
        'سوال': '`[.1so]~`[.0al]',
    }
    for text, expected in examples.items():
        source = text.encode('utf8')
        result, offsets, error = ctypes.c_void_p(), ctypes.POINTER(ctypes.c_size_t)(), ctypes.c_size_t()
        assert direct(source, len(source), ctypes.byref(result), ctypes.byref(offsets), ctypes.byref(error)) == 0, text
        try:
            output = ctypes.string_at(result).decode('ascii')
            positions = tuple(offsets[i] for i in range(len(source) + 1))
            assert output == expected, (text, output)
            assert positions[0] == 0 and positions[-1] == len(output)
            assert all(a <= b for a, b in zip(positions, positions[1:])), text
        finally:
            free(result)
            free(offsets)
    print(f'Passed {len(examples)} embedded Persian readings and source maps.')


if __name__ == '__main__':
    main()
