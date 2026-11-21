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
    character = dll.eciPersianCharacterResolve
    character.argtypes = (ctypes.c_uint32, ctypes.c_void_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_int))
    character.restype = ctypes.c_int
    for codepoint, expected in ((0x064e, '`[.1A]'), (0x064f, '`[.1o]'), (0x0650, '`[.1e]')):
        result, kind = ctypes.create_string_buffer(64), ctypes.c_int(-1)
        assert character(codepoint, result, len(result), ctypes.byref(kind)) == 1
        assert result.value.decode('ascii') == expected and kind.value == 0, (codepoint, result.value, kind.value)

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
        'ایران': "`[.0i.1ran]",
        'خانه': "`[.0Ga.1ne]",
        'کتاب‌ها': '`[.0ke.0tab.1ha]',
        'کتابها': '`[.0ke.0tab.1ha]',
        'خانه‌ها': '`[.0Ga.0ne.1ha]',
        'افزونه‌ها': '`[.0Af.0zu.0ne.1ha]',
        'افزونه‌های': '`[.0Af.0zu.0ne.1ha.0je]',
        'افزونه‌هایی': "`[.0Af.0zu.0ne.1ha.0i]",
        'برنامه‌های': '`[.0bAr.0na.0me.1ha.0je]',
        'انتخاب‌های': '`[.0en.0te.0Gab.1ha.0je]',
        'کتابم': '`[.0ke.1ta.0bAm]',
        'کتابی': '`[.0ke.1ta.0bi]',
        # A dictionary-listed adjective with changed stem vowels is distinct
        # from an unaccented indefinite suffix on an otherwise identical stem.
        'مجازی': '`[.0mA.0Ja.1zi]',
        'آنتی': '`[.1an.0ti]',
        'سوینا': '`[.0se.0vi.1na]',
        'نازلی': '`[.1naz.0li]',
        # Alif-lam-alif forms with an accented /l/ syllable have a short,
        # unstressed initial vowel, including attached possessives.
        'الآن': '`[.0A.1lan]',
        'الان': '`[.0A.1lan]',
        'الآنش': '`[.0A.1la.0nAS]',
        'الانم': '`[.0A.1la.0nAm]',
        'خانهٔ': "`[.0Ga.1ne.0je]",
        'ولی': '`[.1vA.0li]',
        'می‌روم': '`[.1mi.0rA.0vAm]',
        'بروم': '`[.1be.0rA.0vAm]',
        'نوشتم': '`[.0ne.1veS.0tAm]',
        'نوشت': '`[.0ne.1veSt]',
        'آمد': "`[.0a.1mAd]",
        'می روم': '`[.1mi] `00 `[.0rA.0vAm]',
        'نمی روم': '`[.1ne.0mi] `00 `[.0rA.0vAm]',
        'اَ': '`[.1A]', 'اُ': '`[.1o]', 'اِ': '`[.1e]',
        'اَن': '`[.1An]', 'اُن': '`[.1on]', 'اِن': '`[.1en]',
        'و': '`[.1vA]',
        'و کتاب': '`00 `[.0vA] `[.0ke.1tab]',
        'کتاب و خانه': "`[.0ke.1tab] `00 `[.0vA] `[.0Ga.1ne]",
        'و، کتاب': '`[.1vA], `[.0ke.1tab]',
        'که': "`[.1ke]",
        'که کتاب': '`00 `[.0ke] `[.0ke.1tab]',
        'را': '`[.1ra]',
        'کتاب را': '`[.0ke.1tab] `00 `[.0ra]',
        'کتاب را، خانه': "`[.0ke.1tab] `00 `[.0ra], `[.0Ga.1ne]",
        'کتاب، را': '`[.0ke.1tab], `[.1ra]',
        'است': '`[.1Ast]',
        'خوب است': "`[.1Gub] `00 `[.0Ast]",
        'خوب، است': "`[.1Gub], `[.1Ast]",
        'این یک آزمایش است': '`[.1in] `[.1jek] `[.0az.0ma.1jeS] `00 `[.0Ast]',
        'من کتاب': '`[.1mAn] `[.0ke.1tab]',
        'ما کتاب': '`[.1ma] `[.0ke.1tab]',
        'از کتاب': '`00 `[.0Az] `[.0ke.1tab]',
        'اسلش': '`[.0es.1lAS]',
        'اسلش کتاب': '`[.0es.1lAS] `[.0ke.1tab]',
        'کتاب اسلش': '`[.0ke.1tab] `[.0es.1lAS]',
        'یونانی': '`[.0ju.0na.1ni]',
        'ژاپنی': '`[.0Za.0po.1ni]',
        'آلمانی': '`[.0al.0ma.1ni]',
        'انگلیسی': '`[.0en.0ge.0li.1si]',
        'حجازی': '`[.0he.0Ja.1zi]',
        'بردباری': '`[.0bord.0ba.1ri]',
        'سلامتی': '`[.0sA.0la.0mA.1ti]',
        'کثیریان': '`[.0kA.0si.0ri.1jan]',
        'عباسی': '`[.0Ab.0ba.1si]',
        'منچستریونایتد': '`[.0mAn.0Ces.0te.0rju.0naj.1ted]',
        'سایبری': '`[.0saj.0be.1ri]',
        'زمستانی': '`[.0ze.0mes.0ta.1ni]',
        'عنوان': '`[.0on.1van]',
        'عنوانِ': '`[.0on.1va.0ne]',
        'عمودی': '`[.0A.0mu.1di]',
        'عمودیِ': '`[.0A.0mu.1di.0je]',
        'عمل': '`[.0A.1mAl]',
        'عملِ': '`[.0A.1mA.0le]',
        'عملی': '`[.0A.0mA.1li]',
        'راه‌اندازی': '`[.0ra.0hAn.0da.1zi]',
        'راه‌اندازیِ': '`[.0ra.0hAn.0da.1zi.0e]',
        'آسیب‌یابی': '`[.0a.0si.0bja.1bi]',
        'آماده‌سازی': '`[.0a.0ma.0de.0sa.1zi]',
        'ذخیره‌سازیِ': '`[.0zA.0Gi.0re.0sa.1zi.0e]',
        'ذخیره‌شده': "`[.0zA.0Gi.0re.0So.1de]",
        'ذخیره‌شده‌ی': '`[.0zA.0Gi.0re.0So.1de.0je]',
        'ثبت‌شده': "`[.0sAbt.0So.1de]",
        'ثبت‌شده‌ی': '`[.0sAbt.0So.1de.0je]',
        'نصب‌شده‌ی': '`[.0nAsb.0So.1de.0je]',
        'معرفی': '`[.0mo.0wAr.0re.1fi]',
        'خواجه‌ئیان': "`[.1Ga.0Je.0i.1an]",
        'خواجهئیان': "`[.1Ga.0Je.0i.1an]",
        'خانه‌ئیان': "`[.1Ga.0ne.0i.1an]",
    }
    for word, expected in examples.items():
        assert project(word) == expected, (word, project(word), expected)
    assert project('می خانه').startswith('`[.1mi] '), 'an independent noun was mistaken for a verb'
    assert project('علامت‌دار') == project('علامت دار')
    for word in ('طور', 'سوی', 'طرف', 'خاطر', 'گفته', 'دیدن', 'دلیل', 'طورِ'):
        joined, spaced = 'به\u200c' + word, 'به ' + word
        assert project(joined) == project(spaced), (joined, project(joined), project(spaced))
    assert project('به‌روز') == "`[.0be.1ruz]"
    assert project('به‌روزرسانی') == "`[.0be.2ruz.0re.1sa.0ni]"
    assert project('به‌روزرسانی‌های') == "`[.0be.0ruz.0re.0sa.0ni.1ha.0je]"
    assert project('گزینه زمستانی ما او بود.').find('`[.0ze.0mes.0ta.1ni]') >= 0
    assert project('اِختیاراتِ مدیریتی').endswith('`[.0mo.0di.0ri.0jA.1ti]')
    assert project('اختیاراتِ مدیریتی').endswith('`[.0mo.0di.0ri.0jA.1ti]')

    # Neutral prepositions and conjunctions attach to the next word, but
    # citation forms, punctuation, and independent pronouns keep an accent.
    for word in ('و', 'یا', 'که', 'از', 'به', 'با', 'در', 'تا', 'جز'):
        tagged = project(word + ' خانه')
        assert tagged.startswith('`00 `[.0'), (word, tagged)
        assert project(word).startswith('`[.1'), (word, project(word))
        assert project(word + '، خانه').startswith('`[.1'), (word, project(word + '، خانه'))
    assert project('جز از ایران').count('`00 ') == 2
    assert project('تو خانه').startswith('`[.1to] '), 'independent pronoun was weakened'
    assert not project('بیست کتاب').startswith('`00 '), 'a content word was treated as a clitic'

    def fallback(word, tag):
        source = word.encode('utf8')
        target = ctypes.create_string_buffer(tag.encode('ascii'))
        assert stress(source, len(source), target, len(target)) == 0
        return target.value.decode('ascii')

    assert fallback('نمی‌روم', '`[.0nA.0mi.1rA.0vAm]') == '`[.1nA.0mi.0rA.0vAm]'
    assert fallback('کتابی', '`[.0ke.1ta.0bi]') == '`[.0ke.1ta.0bi]'
    assert fallback('یونانی', '`[.0ju.1na.0ni]') == '`[.0ju.0na.1ni]'
    for word in ('حجازی', 'بردباری', 'سلامتی', 'کثیریان', 'عباسی',
                 'منچستریونایتد', 'سایبری', 'زمستانی'):
        assert fallback(word, '`[.0sa.1ma.0ni]') == '`[.0sa.0ma.1ni]', word
    assert fallback('ولی', '`[.1vA.0li]') == '`[.1vA.0li]'
    # A suffix alone does not establish the analysis of an unknown fallback.
    assert fallback('ناشناخته‌سازی', '`[.1sa.0zi]') == '`[.1sa.0zi]'
    assert fallback('ناشناخته‌ها', '`[.1sa.1ha]') == '`[.1sa.1ha]'
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
