"""Exercise reported Persian stress, diacritics and hamza in the shared C API."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import re
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = (ctypes.WinDLL if sys.platform == 'win32' else ctypes.CDLL)(str(args.library.resolve()))
    translate = dll.eciPersianTranslateUtf8
    translate.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                          ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_size_t))
    translate.restype = ctypes.c_int
    adapt = dll.eciPersianStressUtf8
    adapt.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    adapt.restype = ctypes.c_int
    complete = dll.eciPersianProjectTextCompleteUtf8
    complete.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int,
                         ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t,
                         ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.c_size_t),
                         ctypes.POINTER(ctypes.c_size_t))
    complete.restype = ctypes.c_int
    dll.eciPersianRelease.argtypes = (ctypes.c_void_p,)

    def project(text):
        raw = text.encode('utf8'); output = ctypes.c_void_p(); mapping = ctypes.c_void_p()
        error = ctypes.c_size_t()
        status = translate(raw, len(raw), ctypes.byref(output), ctypes.byref(mapping), ctypes.byref(error))
        try:
            assert status == 0, (text, status, error.value)
            return ctypes.string_at(output).decode('ascii')
        finally:
            dll.eciPersianRelease(output); dll.eciPersianRelease(mapping)

    def adjust(word, tag):
        raw = word.encode('utf8'); output = ctypes.create_string_buffer(tag.encode('ascii'), 2048)
        assert adapt(raw, len(raw), output, len(output)) == 0
        return output.value.decode('ascii')

    def final_stress(tag):
        marks = re.findall(r'\.([012])', tag)
        return len(marks) > 1 and marks[-1] == '1' and marks.count('1') == 1

    for word in ('تتر', 'کرجی', 'رشتی', 'بوشهری', 'کریپتو', 'رمزارز', 'سرمایه‌گذاری'):
        tag = project(word)
        assert final_stress(tag), (word, tag)
    # The pattern is productive across places, and a true indefinite -i or
    # a verbal prefix retains its own grammatical stress.
    for word in ('تهرانی', 'کرمانی', 'شیرازی'):
        tag = project(word)
        assert final_stress(tag), (word, tag)
    for word, tag in (('یزدی', '`[.1jAz.0di]'), ('اراکی', '`[.0A.1ra.0ki]')):
        assert final_stress(adjust(word, tag)), (word, adjust(word, tag))
    assert project('کتابی') == '`[.0ke.1ta.0bi]'
    assert project('می رشتی').startswith('`[.1mi] `00 `[.0rAS.0ti]')
    assert project('می‌روم').startswith('`[.1mi')
    for word, tag in (('بیضائی', '`[.1bi.0za.0wi]'),
                      ('مینائی', '`[.1mi.0na.0wi]'),
                      ('ماهینی', '`[.1ma.0hi.0ni]')):
        assert final_stress(adjust(word, tag)), (word, adjust(word, tag))
    assert adjust('جائی', '`[.1Ja.0wi]') == '`[.1Ja.0wi]'
    assert project('اساسی').startswith("`[.0'A:'"), project('اساسی')
    for mark, expected in (('َ', '`[.1A]'), ('ُ', '`[.1o]'), ('ِ', '`[.1e]')):
        assert project(mark) == expected
        assert project('ا' + mark) == expected
    assert project('َُِ') == '`[.1A] `[.1o] `[.1e]'

    assert project('سوئد') == "`[.0s'u:'.1wed]"
    assert project('سوئدی') == "`[.0s'u:'.0we.1di]"
    assert adjust('سوئد', "`[.0s'u:'.1wed]") == "`[.0s'u:'.1wed]"
    assert adjust('مسئول', "`[.0mAs.1w'u:'l]") == "`[.0mAs.1w'u:'l]"
    assert project('معلوم') == "`[.0mAw.1l'u:'m]"

    # Unknown Persian in Pinglish mode reaches the same complete C path;
    # all source byte boundaries remain monotonic.
    text = 'salam ژژژژژ donya'; raw = text.encode('utf8'); n = len(raw)
    offsets = (ctypes.c_size_t * (n + 1))(*range(n + 1))
    indexes = (ctypes.c_ubyte * (n + 1))(*([1] * (n + 1)))
    targets = (ctypes.c_size_t * (n + 1))(); output = ctypes.c_void_p(); error = ctypes.c_size_t()
    status = complete(raw, n, 1, offsets, indexes, n + 1, ctypes.byref(output), targets, ctypes.byref(error))
    try:
        assert status == 0, (status, error.value)
        assert ctypes.string_at(output).decode('ascii').count('Z') == 5
        assert list(targets) == sorted(targets)
    finally:
        dll.eciPersianRelease(output)
    print('Passed Persian stress, vowel marks, contextual hamza and shared complete Pinglish projection.')


if __name__ == '__main__':
    main()
