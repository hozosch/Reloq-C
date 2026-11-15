"""Generate Persian symbol names and digit spellings for character mode."""
from __future__ import annotations

import argparse
from pathlib import Path
import pprint

from espeak_ng_probe import EspeakPhonemeProbe
from espeak_worker import EspeakWorker
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
    '+': 'به علاوه', '-': 'خط تیره', '/': 'اسلش', '\\': 'بک اسلش',
    '=': 'مساوی', '<': 'کوچکتر', '>': 'بزرگتر', '_': 'زیر خط',
    '|': 'خط عمودی', '%': 'درصد', '٪': 'درصد', '#': 'شماره',
    '$': 'دلار', '€': 'یورو', '£': 'پوند', '«': 'گیومه باز',
    '»': 'گیومه بسته', '٫': 'ممیز', '٬': 'هزارگان', '﷼': 'ریال',
    '\u200c': 'نیم فاصله', '\u200d': 'نیم اتصال', 'ـ': 'کشیده',
    '…': 'سه نقطه', '&': 'و', "'": 'تک کوتیشن', '"': 'دابل کوتیشن',
    '@': 'علامت ات', '\u200e': 'نشانه چپ به راست',
    '\u200f': 'نشانه راست به چپ', '■': 'مربع', '●': 'دایره',
    '°': 'درجه', '×': 'ضرب در', '÷': 'تقسیم بر', '±': 'مثبت منفی',
    '^': 'علامت کرت', '`': 'علامت گراو', '~': 'تیلدا',
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=Path(__file__).with_name('persian_symbols.py'))
    args = parser.parse_args()
    set_confirmed_persian_probe()
    native = engine(args.probe, 0x90000)
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api, \
             EspeakWorker(args.worker, args.espeak_library, args.data_parent) as reference:
            if api.version != '1.53.0':
                raise RuntimeError('this table requires eSpeak NG 1.53.0')
            values = {}
            for char, name in NAMES.items():
                parts = translate_text(api, name)
                if len(parts) != 1 or parts[0].language != 'fa':
                    raise RuntimeError(f'unsupported symbol name {name!r}: {parts!r}')
                values[char] = parts[0].text
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
    args.output.write_text(output, encoding='utf8', newline='\n')
    print(f'Generated {len(values)} validated Persian symbol and digit names.')


if __name__ == '__main__':
    main()
