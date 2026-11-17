"""Check Persian word stress, inflections, numbers and fallback tags."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import re


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    translate = dll.eciPersianTranslateUtf8
    translate.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                          ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)), ctypes.POINTER(ctypes.c_size_t))
    translate.restype = ctypes.c_int
    dll.eciPersianRelease.argtypes = (ctypes.c_void_p,)
    stress = dll.eciPersianStressUtf8
    stress.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    stress.restype = ctypes.c_int

    def project(source):
        raw = source.encode('utf8')
        out, mapping, error = ctypes.c_void_p(), ctypes.POINTER(ctypes.c_size_t)(), ctypes.c_size_t()
        code = translate(raw, len(raw), ctypes.byref(out), ctypes.byref(mapping), ctypes.byref(error))
        assert code == 0, (source, code, error.value)
        try:
            result = ctypes.string_at(out).decode('ascii')
            indexes = tuple(mapping[i] for i in range(len(raw) + 1))
            assert indexes == tuple(sorted(indexes)) and indexes[-1] == len(result)
            return result
        finally:
            dll.eciPersianRelease(out)
            dll.eciPersianRelease(mapping)

    examples = {
        'کتاب': '`[.0ke.1tab]',
        'دنیا': '`[.0do.1nja]',
        'ایران': "`[.0'i:'.1r'a:'n]",
        'خانه': "`[.0Ga.1n'e:']",
        'کتاب‌ها': '`[.0ke.0tab.1ha]',
        'کتابها': '`[.0ke.0tab.1ha]',
        'خانه‌ها': "`[.0Ga.0n'e:'.1ha]",
        'کتابم': '`[.0ke.1ta.0bAm]',
        'کتابی': '`[.0ke.1ta.0bi]',
        'خانهٔ': "`[.0Ga.1n'e:'.0je]",
        'ولی': '`[.1vA.0li]',
        'می‌روم': '`[.1mi.0rA.0vAm]',
        'بروم': '`[.1be.0rA.0vAm]',
        'نوشتم': '`[.0ne.1veS.0tAm]',
        'نوشت': '`[.0ne.1veSt]',
        'آمد': "`[.0'a:'.1mAd]",
        'می روم': '`[.1mi] `[.0rA.0vAm]',
        'نمی روم': '`[.1ne.0mi] `[.0rA.0vAm]',
        'اَ': "`[.1'A:']", 'اُ': "`[.1'o:']", 'اِ': "`[.1'e:']",
        'اَن': '`[.1An]', 'اُن': '`[.1on]', 'اِن': '`[.1en]',
    }
    for word, expected in examples.items():
        assert project(word) == expected, (word, project(word), expected)
    assert project('می خانه').startswith('`[.0mi] '), 'a noun was mistaken for a verb'
    assert project('علامت‌دار') == project('علامت دار')

    def fallback(word, tag):
        source = word.encode('utf8')
        target = ctypes.create_string_buffer(tag.encode('ascii'))
        assert stress(source, len(source), target, len(target)) == 0
        return target.value.decode('ascii')

    assert fallback('نمی‌روم', '`[.0nA.0mi.1rA.0vAm]') == '`[.1nA.0mi.0rA.0vAm]'
    assert fallback('کتابی', '`[.0ke.1ta.0bi]') == '`[.0ke.1ta.0bi]'
    assert fallback('ولی', '`[.1vA.0li]') == '`[.1vA.0li]'
    # Multi-tag fallback output has no safe one-word alignment.
    multi = '`[.1ke.0tab] `[.1Ga.0ne]'
    assert fallback('کتاب', multi) == multi
    assert stress(b'word', 4, None, 0) == -1

    for number in ('4', '۱۲', '۲۴', '101', '۳٫۱۴'):
        tagged = project(number)
        marks = list(re.finditer(r'\.[012]', tagged))
        assert marks and marks[-1].group() == '.1', (number, tagged)
    assert project('+۳').startswith('`[.0mos.1bAt] ')
    assert project('−۳').startswith('`[.0mAn.1fi] ')
    print(f'Passed {len(examples)} Persian word-stress examples, fallback, numbers and source maps.')


if __name__ == '__main__':
    main()
