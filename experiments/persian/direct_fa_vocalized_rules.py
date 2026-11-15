"""Reference-verified vowel-mark templates used by the native frontend."""
import re

EXTRA = ('ورود', 'وُرود', 'اختیارات', 'اختیاراتِ', 'اِختیارات', 'اِختیاراتِ', 'زن', 'علی',
         'بسته', 'تیره', 'خطِ', 'خط', 'پرانتز', 'علامتِ', 'تعجب', 'کوچکتر', 'بزرگتر',
         'ویرگول', 'نقطه', 'کروشه', 'آکولاد', 'زیرخط', 'فاصله', 'درصد', 'نیم', 'اتصال', 'صفحه',
         'نشده', 'شده', 'انتخاب', 'کادر', 'تیک', 'تیکِ', 'انتخاب‌شده')


def candidate(base, mode):
    if not base.startswith('`[') or not base.endswith(']') or '~' in base:
        return None
    syllables = re.findall(r'\.([012])([^.]+)', base[2:-1])
    if not syllables or ''.join('.' + s + b for s, b in syllables) != base[2:-1]:
        return None
    body = syllables[-1][1]
    last = body[-1:]
    if last in 'bdfghjklmnprstvzSZGRCJ':
        syllables[-1] = (syllables[-1][0], body[:-1])
        tail = last + 'e'
    else:
        tail = 'e'
    primary = next((i for i, (s, _b) in enumerate(syllables) if s == '1'), 0)
    if mode:
        primary = len(syllables) - 1
    result = []
    for i, (stress, value) in enumerate(syllables + [(None, tail)]):
        if mode:
            stress = '1' if i == primary else '2' if i == 0 and primary >= 2 else '0'
        elif stress is None:
            stress = '2' if i >= primary + 2 and (i - primary) % 2 == 0 else '0'
        result.append('.' + stress + value)
    return '`[' + ''.join(result) + ']'
