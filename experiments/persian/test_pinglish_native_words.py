"""Verify the native Pinglish map against the shared Persian word pipeline."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import re


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    native = getattr(dll, 'eciPersianPinglishWord', None) or dll.reloq_fa_pinglish_word
    native.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    native.restype = ctypes.c_int
    translate = dll.eciPersianTranslateUtf8
    translate.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                          ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)), ctypes.POINTER(ctypes.c_size_t))
    translate.restype = ctypes.c_int
    dll.eciPersianRelease.argtypes = (ctypes.c_void_p,)

    def pinglish(word):
        source = word.encode('utf8')
        output = ctypes.create_string_buffer(4096)
        status = native(source, len(source), output, len(output))
        return status, output.value.decode('ascii')

    def persian(word):
        source = word.encode('utf8')
        output, mapping, error = ctypes.c_void_p(), ctypes.POINTER(ctypes.c_size_t)(), ctypes.c_size_t()
        status = translate(source, len(source), ctypes.byref(output), ctypes.byref(mapping), ctypes.byref(error))
        assert status == 0, (word, status, error.value)
        try:
            return ctypes.string_at(output).decode('ascii')
        finally:
            dll.eciPersianRelease(output)
            dll.eciPersianRelease(mapping)

    table = Path(__file__).with_name('direct_fa_pinglish_lexicon.inc').read_text(encoding='utf8')
    entries = re.findall(r'^\s*\{"([a-z]+)", "([^"\\]+)"\},', table, re.M)
    assert len(entries) > 25000
    checked = 0
    for roman, script in entries:
        if 'ph' in roman or 'th' in roman or 'ooo' in roman:
            continue  # Existing explicit English/fallback routing.
        assert pinglish(roman) == (1, persian(script)), (roman, script, pinglish(roman), persian(script))
        checked += 1
    for roman, script in (('khane', 'خانه'), ('khaneh', 'خانه'), ('chetori', 'چطوری'),
                          ('parantez', 'پرانتز'), ('rayane', 'رایانه'),
                          ('mikhaham', 'می‌خواهم'), ('miravam', 'می‌روم'),
                          ('nemiravam', 'نمی‌روم'), ('tehrani', 'تهرانی'),
                          ('futbal', 'فوتبال'), ('footbal', 'فوتبال'),
                          ('ketabam', 'کتابم'), ('ketabha', 'کتاب‌ها'),
                          ('khubtar', 'خوبتر'), ('donyaye', 'دنیای'), ('majazi', 'مجازی')):
        assert pinglish(roman) == (1, persian(script)), (roman, script, pinglish(roman), persian(script))
        assert pinglish(roman.upper()) == pinglish(roman), roman
    for word in ('phone', 'this', 'zooon'):
        assert pinglish(word)[0] == 0, word
    # Unknown but pronounceable Pinglish defaults to the normal Persian
    # final stress; recognized lexical exceptions continue to win above.
    for word in ('alan', 'zavarexi', 'kavarejo'):
        status,tag=pinglish(word)
        stresses=re.findall(r'\.([012])',tag)
        assert status==1 and stresses[-1]=='1' and stresses.count('1')==1,(word,tag)
    print(f'Passed {checked} native Pinglish/Persian word comparisons and morphology checks.')


if __name__ == '__main__':
    main()
