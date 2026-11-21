"""Generate Persian symbol names and digit spellings for character mode."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import pprint
import re
import unicodedata

from espeak_ng_probe import EspeakPhonemeProbe
from espeak_worker import EspeakWorker
from hybrid_fa_frontend import DirectFirstPersian
from pinglish_frontend import LATIN_NAMES
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import native_sequence_accepted, translate_text, word_tag

# Familiar Persian names follow the corresponding NVDA Persian symbol names
# where available. ASCII punctuation gets the same names as Persian variants.
NAMES = {
    ' ': 'فاصله', '\t': 'تب', '\n': 'سر خط', '\r': 'پایان خط',
    '.': 'نقطه', ',': 'ویرگول', '،': 'ویرگول', '?': 'علامت سوال',
    '؟': 'علامت سوال', '!': 'علامت تعجب', ':': 'دو نقطه',
    ';': 'نقطه ویرگول', '؛': 'نقطه ویرگول', '(': 'پرانتز باز',
    ')': 'پرانتز بسته', '[': 'کروشه باز', ']': 'کروشه بسته',
    '{': 'آکولاد باز', '}': 'آکولاد بسته', '*': 'ستاره',
    '+': 'به علاوه', '-': 'خطِ تیره', '/': 'اسلش', '\\': 'بک اسلش',
    '=': 'مساوی', '<': 'کوچکتر', '>': 'بزرگتر', '_': 'زیرخط',
    '|': 'خط عمودی', '%': 'درصد', '٪': 'درصد', '#': 'شماره',
    '$': 'دلار', '€': 'یورو', '£': 'پوند', '«': 'گیومه باز',
    '»': 'گیومه بسته', '٫': 'ممیز', '٬': 'هزارگان', '﷼': 'ریال',
    '\u200c': 'نیم فاصله', '\u200d': 'نیم اتصال', 'ـ': 'کشیده',
    '…': 'سه نقطه', '&': 'و', "'": 'تک کوتیشن', '"': 'دابل کوتیشن',
    '@': 'علامت ات', '\u200e': 'نشانه چپ به راست',
    '\u200f': 'نشانه راست به چپ', '■': 'مربع', '●': 'دایره',
    '°': 'درجه', '×': 'ضرب در', '÷': 'تقسیم بر', '±': 'مثبت منفی',
    '^': 'علامت کرت', '`': 'علامت گراو', '~': 'تیلدا',
    # The reported dictionary pronunciation uses IPA stress, schwa and
    # combining-dot marks. Keep their names useful in character navigation.
    'ˈ': 'تکیه اصلی', 'ˌ': 'تکیه فرعی', 'ə': 'شوا', 'Ə': 'شوای بزرگ',
    'ː': 'علامت کشش صدا', 'ˑ': 'علامت نیم کشش صدا',
    '\u0300': 'اکسان گراو', '\u0301': 'اکسان تیز',
    '\u0302': 'اکسان سیرکومفلکس', '\u0303': 'علامت تیلدا',
    '\u0304': 'خط بالا', '\u0306': 'علامت بریو', '\u0307': 'نقطه بالا',
    '\u0308': 'دو نقطه بالا', '\u030a': 'حلقه بالا', '\u030b': 'دو اکسان تیز',
    '\u030c': 'علامت کارون', '\u031b': 'شاخ', '\u0323': 'نقطه پایین',
    '\u0327': 'سدی زیر', '\u0328': 'اوگونک', '\u032f': 'علامت غیر هجایی',
    '\u0331': 'خط پایین', '\u0338': 'خط مورب روی حرف',
    '\u0655': 'همزه پایین',
}

# Known Persian labels use the native word rules below, so regenerating
# this table keeps their current pronunciation instead of importing a
# reference voice's duration or stress choices. The formatting and symbol
# labels also follow the pinned NVDA Persian symbol dictionary.
NATIVE_NAME_OVERRIDES = {
    '\x08': 'بک اسپیس',
    '\x0b': 'تب عمودی',
    '\x0c': 'پایان صفحه',
    '\x7f': 'حذف',
    '\x85': 'خط بعدی',
    '\u00a0': 'فاصله بدون شکست',
    '\u00ad': 'خط تیره نرم',
    '\u200b': 'فاصله بدون عرض',
    '\u202f': 'فاصله باریک بدون شکست',
    '\u202a': 'چپ به راستِ توکار',
    '\u202b': 'راست به چپِ توکار',
    '\u202c': 'پایانِ جهت',
    '\u202d': 'بازنویسیِ چپ به راست',
    '\u202e': 'بازنویسیِ راست به چپ',
    '✓': 'تیک',
    # NVDA normalizes the dictionary's ZWNJ characters to spaces before
    # forwarding the written name to a synthesizer.
    '➔': 'فِلِشِ راست نمای سر پهن',
}
NAMES.update(NATIVE_NAME_OVERRIDES)

GREEK = (
    ('Α', 'α', 'آلفا'), ('Β', 'β', 'بتا'), ('Γ', 'γ', 'گاما'),
    ('Δ', 'δ', 'دلتا'), ('Ε', 'ε', 'اپسیلون'), ('Ζ', 'ζ', 'زتا'),
    ('Η', 'η', 'اتا'), ('Θ', 'θ', 'تتا'), ('Ι', 'ι', 'یوتا'),
    ('Κ', 'κ', 'کاپا'), ('Λ', 'λ', 'لامبدا'), ('Μ', 'μ', 'مو'),
    ('Ν', 'ν', 'نو'), ('Ξ', 'ξ', 'کسی'), ('Ο', 'ο', 'امیکرون'),
    ('Π', 'π', 'پی'), ('Ρ', 'ρ', 'رو'), ('Σ', 'σ', 'سیگما'),
    ('Τ', 'τ', 'تاو'), ('Υ', 'υ', 'اوپسیلون'), ('Φ', 'φ', 'فی'),
    ('Χ', 'χ', 'خی'), ('Ψ', 'ψ', 'پسی'), ('Ω', 'ω', 'اُمگا'),
)

CLDR_OVERRIDES = {
    '∏': 'حاصل ضرب چندتایی', '∑': 'حاصل جمع چندتایی',
    '⋁': 'یا چندتایی', '⋂': 'اشتراک چندتایی', '⋃': 'اجتماع چندتایی',
    '🅰': 'دکمه گروه خونی ای', '🅱': 'دکمه گروه خونی بی',
    '🆎': 'دکمه گروه خونی ای بی', '🅾': 'دکمه گروه خونی او',
    '🆑': 'دکمه سی ال', '🆒': 'دکمه کول', '🆓': 'دکمه رایگان',
    '🆕': 'دکمه جدید', '🆖': 'دکمه ان جی', '🆗': 'دکمه اوکی',
    '🆙': 'دکمه بالا', '🆚': 'دکمه در برابر',
}


def named_symbols():
    """Prefer familiar explicit names; use pinned CLDR names for the rest."""
    source = Path(__file__).resolve().parents[2] / 'reference-cldr/persian/fa-annotations-single.json'
    data = json.loads(source.read_text(encoding='utf8'))
    if data.get('release') != '48' or data.get('license') != 'Unicode-3.0':
        raise RuntimeError('unexpected Persian CLDR source')
    names = {}
    for item in data['entries']:
        char = item['char']
        if len(char) != 1 or char in names:
            raise ValueError('invalid or duplicate single-character annotation')
        text = CLDR_OVERRIDES.get(char, item['name'])
        # Quotation/parenthesis marks in descriptive labels are not spoken
        # as literal symbols; they can also insert unsupported pause phones.
        names[char] = ' '.join(re.sub(r'[«»()!]', ' ', text).split())
    names.update(NAMES)
    for upper, lower, name in GREEK:
        names.setdefault(upper, 'حرف بزرگ یونانی ' + name)
        names.setdefault(lower, 'حرف کوچک یونانی ' + name)
    names.setdefault('ς', 'حرف کوچک یونانی سیگمای پایانی')
    return names


def restress_known_names(values, names, frontend):
    """Match symbol names to reliable native written-name pronunciations.

    Only one-to-one Persian word/tag alignments whose phone sequences match
    the native spelling are changed. Unclear labels and deaccented words are
    left intact instead of guessing their grammatical interpretation. For
    the two NVDA labels explicitly corrected here, also use the native phone
    sequence so the ezafe and compound boundaries match the written words.
    """
    tag_pattern = re.compile(r'`\[[^]]+\]')
    def unaccent(value):
        # Generated weak-word controls affect phrase prominence, not phones.
        # Include them in the result when the known phrase otherwise agrees.
        return re.sub(r'\.[012]', '.#', re.sub(r'`00\s*', '', value))
    output = dict(values)
    for char, original in values.items():
        name = names.get(char)
        if not name:
            continue
        # Prefer the same native phrase analysis used for an explicitly
        # written label, including weak conjunctions. Reuse it only when
        # every phone and boundary agrees with the existing symbol reading.
        phrase = frontend.analyze_direct(name)
        if (phrase is not None and len(phrase.parts) == 1 and
                (char in ('-', '_') or unaccent(phrase.parts[0].text) == unaccent(original))):
            output[char] = phrase.parts[0].text
            continue
        words = name.split()
        tags = tuple(tag_pattern.finditer(original))
        if len(words) != len(tags) or tag_pattern.sub('', original).strip():
            continue
        pieces, cursor = [], 0
        for word, match in zip(words, tags):
            pieces.append(original[cursor:match.start()])
            tag = match.group()
            native = frontend.analyze_direct(word)
            if (tag.count('.1') == 1 and native is not None and len(native.parts) == 1 and
                unaccent(native.parts[0].text) == unaccent(tag)):
                tag = native.parts[0].text
            pieces.append(tag)
            cursor = match.end()
        pieces.append(original[cursor:])
        output[char] = ''.join(pieces)
    return output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=Path(__file__).with_name('persian_symbols.py'))
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    set_confirmed_persian_probe()
    native = engine(args.probe, 0x90000)
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api, \
             EspeakWorker(args.worker, args.espeak_library, args.data_parent) as reference:
            if api.version != '1.53.0':
                raise RuntimeError('this table requires eSpeak NG 1.53.0')
            values = {}
            for char, name in named_symbols().items():
                if char in NATIVE_NAME_OVERRIDES:
                    continue  # generated independently from the native rules below
                parts = translate_text(api, name)
                if len(parts) != 1 or parts[0].language != 'fa':
                    raise RuntimeError(f'unsupported symbol name {name!r}: {parts!r}')
                values[char] = parts[0].text
            def fa_phrase(text):
                parts = translate_text(api, text)
                if len(parts) != 1 or parts[0].language != 'fa':
                    raise RuntimeError(f'unsupported Persian descriptor {text!r}')
                return parts[0].text
            prefixes = {True: fa_phrase('حرف بزرگ لاتین'), False: fa_phrase('حرف کوچک لاتین')}
            with_mark, conjunction = fa_phrase('با'), fa_phrase('و')
            accents = frozenset('\u0300\u0301\u0302\u0303\u0304\u0306\u0307\u0308'
                                '\u030a\u030b\u030c\u031b\u0323\u0327\u0328\u0331')
            # Canonical Unicode decomposition gives an accurate, distinct
            # description of accented Latin letters even without a CLDR label.
            for codepoint in range(1, 0x110000):
                char = chr(codepoint)
                name = unicodedata.name(char, '')
                if char in values or not name.startswith(('LATIN SMALL LETTER ', 'LATIN CAPITAL LETTER ')):
                    continue
                decomp = unicodedata.normalize('NFD', char)
                if (len(decomp) < 2 or not decomp[0].isascii() or not decomp[0].isalpha() or
                        any(mark not in accents for mark in decomp[1:])):
                    continue
                tags = [prefixes[name.startswith('LATIN CAPITAL')], LATIN_NAMES[decomp[0].lower()], with_mark]
                for i, mark in enumerate(decomp[1:]):
                    if i:
                        tags.append(conjunction)
                    tags.append(values[mark])
                values[char] = ' '.join(tags)
            for start in (ord('0'), 0x0660, 0x06f0):
                for code in range(start, start + 10):
                    char = chr(code)
                    rows = reference.translate('\x0118Y' + char + '\x01Y', voice='fa', separator='|')
                    words = [word for row in rows for word in row.phonemes.split()]
                    if len(words) != 1 or not words[0].endswith('|_!'):
                        raise RuntimeError(f'unsupported digit spelling {char!r}')
                    values[char] = word_tag(words[0][:-3])
            unicode_parts = translate_text(api, 'یونیکد')
            if len(unicode_parts) != 1 or unicode_parts[0].language != 'fa':
                raise RuntimeError('unsupported Unicode fallback name')
            unicode_name = unicode_parts[0].text
            names = named_symbols()
            for start in (ord('0'), 0x0660, 0x06f0):
                for i, word in enumerate(('صفر', 'یک', 'دو', 'سه', 'چهار',
                                           'پنج', 'شش', 'هفت', 'هشت', 'نه')):
                    names[chr(start + i)] = word
            with DirectFirstPersian(args.probe, None, None, None) as frontend:
                values = restress_known_names(values, names, frontend)
                for char, label in NATIVE_NAME_OVERRIDES.items():
                    analysis = frontend.analyze_direct(label)
                    if not analysis or len(analysis.parts) != 1 or analysis.parts[0].language != 'fa':
                        raise RuntimeError(f'unsupported native symbol name {label!r}')
                    values[char] = analysis.parts[0].text
            for name, tag in [*values.items(), ('unicode', unicode_name)]:
                native_text = tag.replace('~', ' ')
                if not native_sequence_accepted(native_text, native._phonemes_for_probe(native_text)):
                    raise RuntimeError(f'bad native symbol tag {name!r}: {tag!r}')
    finally:
        native.close()
    output = ('"""Generated Persian names for spelling symbols and digits.\n\n'
              'Text analysis follows the pinned eSpeak NG Persian data, '
              'GPL-3.0-or-later.\n"""\n\n'
              'SYMBOL_TAGS = ' + pprint.pformat(dict(sorted(values.items(), key=lambda x: ord(x[0]))),
                                                width=100, sort_dicts=False) + '\n'
              'UNICODE_TAG = ' + repr(unicode_name) + '\n')
    if args.check:
        if args.output.read_text(encoding='utf8') != output:
            raise SystemExit('Persian symbol names need regeneration')
    else:
        args.output.write_text(output, encoding='utf8', newline='\n')
    print(f'{"Checked" if args.check else "Generated"} {len(values)} validated Persian symbol and digit names.')


if __name__ == '__main__':
    main()
