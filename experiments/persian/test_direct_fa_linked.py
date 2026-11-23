"""Check reviewed Persian readings and linked-compound rules.

Covers the hand-checked table, the linking /o/ of half-space compounds, the
Arabic article with sun-letter assimilation, joined known words, final tanwin
in guessed words and the reading of 1000-1999. Needs only the direct text
library; no eSpeak or acoustic provider is loaded.
"""
from __future__ import annotations

import argparse
import ctypes as c
from pathlib import Path
import sys

ZWNJ = '‌'

# (text, expected reading, True if every word must be covered without guessing)
CASES = (
    # Linking /o/ in coordinated compounds, both half-space spellings.
    (f'گفت{ZWNJ}وگو', '`[.0gof.0to.1gu]', True),
    (f'گفت{ZWNJ}و{ZWNJ}گو', '`[.0gof.0to.1gu]', True),
    (f'گفت{ZWNJ}وگوی', '`[.0gof.0to.1gu.0je]', True),
    (f'کسب{ZWNJ}وکار', '`[.0kAs.0bo.1kar]', True),
    (f'رفت{ZWNJ}وآمد', '`[.0rAf.0to.0a.1mAd]', True),
    (f'جست{ZWNJ}وجو', '`[.0Jos.0to.1Ju]', True),
    # A tail that is a word of its own keeps its consonantal /v/.
    (f'هم{ZWNJ}وطن', '`[.0hAm.0vA.1tAn]', True),
    (f'بی{ZWNJ}وفا', '`[.0bi.0vA.1fa]', True),
    (f'نخست{ZWNJ}وزیر', '`[.0no.0Gost.0vA.1zir]', True),
    # Arabic article: /ol/, sun-letter assimilation, shortened final vowel.
    (f'حق{ZWNJ}الزحمه', '`[.0hA.0Roz.0zAh.1me]', True),
    (f'فی{ZWNJ}البداهه', '`[.0fel.0be.0da.1he]', True),
    (f'فارغ{ZWNJ}التحصیل', '`[.0fa.0re.0Rot.0tAh.1sil]', True),
    (f'لازم{ZWNJ}الاجرا', '`[.0la.0ze.0mo.0leJ.1ra]', True),
    (f'حتی{ZWNJ}المقدور', '`[.0hAt.0tAl.0mAR.1dur]', True),
    (f'روح{ZWNJ}الله', '`[.0ru.0hol.1lahw]', True),
    (f'بین{ZWNJ}المللی', '`[.0bej.0nol.0me.0lA.1li]', True),
    (f'فوق{ZWNJ}العاده', '`[.0fo.0Ro.0la.1de]', True),
    ('دارالفنون', '`[.0da.0rol.0fo.1nun]', True),
    ('الوصل', '`[.0Al.1vAsl]', False),
    # Existing readings that these rules must not disturb.
    ('عبدالله', '`[.0Ab.0dol.1lahw]', True),
    ('ابوالفضل', '`[.0A.0bol.1fAzl]', True),
    # Known words written together.
    ('سیدمهدی', '`[.0se.0jjed.0mAh.1di]', False),
    # Reviewed words that the generated tables missed or misread.
    ('نخست', '`[.0no.1Gost]', True),
    ('عادت', '`[.0a.1dAt]', True),
    ('عمر', '`[.1omr]', True),
    ('عملیات', '`[.0A.0mA.0li.1jat]', True),
    ('عملیاتِ', '`[.0A.0mA.0li.1ja.0te]', True),
    ('سرتاسر', '`[.0sAr.0ta.1sAr]', True),
    ('عملاً', '`[.0A.0mA.1lAn]', True),
    # Final tanwin in a word that has to be guessed must not fail.
    ('نسبتًا', '`[.0nAsb.1tAn]', False),
    # 1000-1999 are read without a leading "yek".
    ('1000', '`[.0he.1zar]', True),
    ('۱۴۰۳', '`[.0he.2za.0ro] `[.2Ca.0har.0sA.0do] `[.1se]', True),
    ('2000', '`[.0do] `[.0he.1zar]', True),
    ('1000000', '`[.0jek] `[.0mi.1ljun]', True),
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    args = parser.parse_args()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.native_library.resolve()))
    signature = (c.c_char_p, c.c_size_t, c.POINTER(c.c_void_p), c.POINTER(c.c_void_p),
                 c.POINTER(c.c_size_t))
    for name in ('reloq_fa_direct_text', 'reloq_fa_direct_text_complete'):
        getattr(dll, name).argtypes = signature
        getattr(dll, name).restype = c.c_int
    dll.reloq_fa_direct_free.argtypes = (c.c_void_p,)
    dll.reloq_fa_direct_free.restype = None

    def convert(name, text):
        data = text.encode('utf8')
        output, mapping, error = c.c_void_p(), c.c_void_p(), c.c_size_t()
        status = getattr(dll, name)(data, len(data), c.byref(output), c.byref(mapping),
                                    c.byref(error))
        try:
            return status, c.string_at(output).decode('utf8') if status == 0 else ''
        finally:
            dll.reloq_fa_direct_free(output)
            dll.reloq_fa_direct_free(mapping)

    failures = []
    for text, expected, covered in CASES:
        complete = convert('reloq_fa_direct_text_complete', text)
        if complete != (0, expected):
            failures.append((text, expected, complete))
        strict = convert('reloq_fa_direct_text', text)
        if covered and strict != (0, expected):
            failures.append((text, expected, strict))
        if not covered and strict[0] == 0:
            failures.append((text, 'expected the strict lookup to decline', strict))
    for text, expected, actual in failures:
        print(f'FAIL {text}: expected {expected}, got {actual}', file=sys.stderr)
    if failures:
        raise SystemExit(1)
    print(f'{len(CASES)} linked-compound and reviewed-reading cases passed')


if __name__ == '__main__':
    main()
