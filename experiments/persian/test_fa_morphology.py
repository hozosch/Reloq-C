"""Exercise productive Persian morphology, accent and safe ambiguity guards."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',required=True,type=Path)
    args=parser.parse_args()
    dll=ctypes.CDLL(str(args.library.resolve()))
    translate=dll.eciPersianTranslateUtf8
    translate.argtypes=(ctypes.c_char_p,ctypes.c_size_t,ctypes.POINTER(ctypes.c_void_p),
                        ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)),ctypes.POINTER(ctypes.c_size_t))
    translate.restype=ctypes.c_int
    dll.eciPersianRelease.argtypes=(ctypes.c_void_p,)
    checks=0

    def project(text,covered=True):
        nonlocal checks
        raw=text.encode('utf8');out=ctypes.c_void_p()
        mapping=ctypes.POINTER(ctypes.c_size_t)();error=ctypes.c_size_t()
        status=translate(raw,len(raw),ctypes.byref(out),ctypes.byref(mapping),ctypes.byref(error))
        if not covered:
            assert status==1 and not out.value,(text,status,error.value)
            checks+=1;return ''
        assert status==0,(text,status,error.value)
        try:
            result=ctypes.string_at(out).decode('ascii')
            positions=tuple(mapping[i] for i in range(len(raw)+1))
            assert positions==tuple(sorted(positions)) and positions[-1]==len(result),(text,positions)
            checks+=1;return result
        finally:
            dll.eciPersianRelease(out);dll.eciPersianRelease(mapping)

    # Previously reported and held-out phrases use the same connected weak
    # preposition with an ordinary or a half-space. Existing independent
    # lexemes and their derivatives retain their old pronunciations.
    for word in ('طور','سوی','طرف','خاطر','گفته','دیدن','دلیل','شدت',
                 'شرط','صورت','عنوان','راحتی','علاوه','نظر'):
        joined=project('به‌'+word);spaced=project('به '+word)
        assert joined==spaced and joined.startswith('`00 `[.0be] '),(word,joined,spaced)
    assert project('به‌روز')=="`[.0b'e:'.1r'u:'z]"
    assert project('به‌روزرسانی')=="`[.0b'e:'.2r'u:'z.0re.1sa.0ni]"
    assert project('به‌روزرسانی‌های')=="`[.0b'e:'.0r'u:'z.0re.0sa.0ni.1ha.0je]"

    # Suffixes are rules over independently spoken stems, including stems
    # absent from the old plural bitmap and additional held-out nouns.
    for stem in ('کانال','پاتیل','هتل','پاستیل','سبیل','کتاب','ماشین',
                 'کودک','قلم','لباس','دوست','خانه','پرده','پنجره','جعبه'):
        for ending,tail in (('ها','.1ha]'),('های','.1ha.0je]'),('هایی',".1ha.0'i:']")):
            tag=project(stem+'‌'+ending)
            assert tag.endswith(tail) and tag.count('.1')==1,(stem,ending,tag)
    assert project('خانه‌ها')=='`[.0Ga.0ne.1ha]'
    assert project('دیوانه‌ها')=='`[.0di.0va.0ne.1ha]'
    assert project('نیمه‌راه')=='`[.0ni.0me.1rahw]'
    assert project('آماده‌سازی')=='`[.0a.0ma.0de.0sa.1zi]'
    assert project('عجیب‌ترین').endswith('.0tA.1rin]')
    assert project('نیم‌فاصله').endswith(".0fa.0se.1l'e:']")
    assert project('نیم‌اتصال').endswith('.0et.0te.1sal]')
    assert project('بک‌اسلش')=='`[.0bAk.0es.1lAS]'
    assert project('نمی‌روم')=='`[.1ne.0mi.0rA.0vAm]'
    assert project('نمی‌خورم')=='`[.1ne.0mi.0Go.0rAm]'

    # Apply productive accent patterns, but protect possessives, verbs,
    # independent pronouns and a known indefinite -i form.
    expected={
        'فارسی':'`[.0far.1si]', 'صندلی':'`[.0sAn.0dA.1li]',
        'قربانی':'`[.0Ror.0ba.1ni]', 'انسانی':'`[.0en.0sa.1ni]',
        'تهرانی':'`[.0teh.0ra.1ni]',
        'درباره':"`[.0dAr.0ba.1r'e:']", 'درمورد':"`[.0dAr.0m'o:'.1red]",
        'اگرچه':'`[.0A.0gAr.1Ce]', 'همچنین':'`[.0hAm.0Ce.1nin]',
        'مربیان':'`[.0mo.0rAb.0bi.1jan]', 'بازهم':'`[.1baz.0hAm]',
        'ازهم':'`[.0Az.1hAm]', 'اوهم':'`[.1o.0hAm]', 'وهم':'`[.1vAhm]',
        'دارد':'`[.1da.0rAd]',
        'کتابی':'`[.0ke.1ta.0bi]', 'دربارت':'`[.0dAr.1ba.0rAt]',
        'درگیرش':'`[.0dAr.1gi.0rAS]', 'باباتون':'`[.1ba.1ba.0tun]',
        'آنچه':'`[.1an.0Ce]',
    }
    for word,tag in expected.items():assert project(word)==tag,(word,project(word),tag)
    assert project('خدمات تخصصی').endswith('`[.0tA.0GAs.0so.1si]')
    assert project('خدماتِ تخصصی').endswith('`[.0tA.0GAs.0so.1si]')
    assert project('خدمات اختصاصی').endswith('`[.0eG.0te.0sa.1si]')
    assert project('خدمات انضباطی').endswith('`[.0en.0ze.0ba.1ti]')
    unchanged=project('تخصصی')
    assert project('یک تخصصی').endswith(unchanged)
    assert project('کتاب، تخصصی').endswith(unchanged)

    # All vowels in these synthetic words are explicitly marked. An
    # unmarked internal consonant or shadda is not guessed.
    for word,tag in (('کُنَد','`[.0ko.1nAd]'),('پَرَپ','`[.0pA.1rAp]'),
                     ('پَرْدَس','`[.0pAr.1dAs]')):
        assert project(word)==tag,(word,project(word),tag)
    project('پَرپَپ',covered=False);project('پَرّ',covered=False)

    # A deliberately deep sequence must fail safely instead of recursing
    # without bounds or producing truncated output.
    project('کتاب'+'‌ها'*12,covered=False)
    print(f'Passed {checks} Persian morphology, stress, boundary and ambiguity checks.')


if __name__=='__main__':main()
