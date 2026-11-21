"""Check direct Persian numbers against eSpeak and script-equivalent forms."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import random
import re

from espeak_ng_probe import EspeakPhonemeProbe
from text_frontend import word_tag


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.native_library.resolve()))
    convert_text = getattr(dll, 'eciPersianTranslateUtf8', None) or dll.reloq_fa_direct_text
    release = getattr(dll, 'eciPersianRelease', None) or dll.reloq_fa_direct_free
    convert_text.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                             ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)), ctypes.POINTER(ctypes.c_size_t))
    convert_text.restype = ctypes.c_int
    release.argtypes = (ctypes.c_void_p,)

    def convert(text):
        encoded = text.encode('utf8')
        output, mapping, offset = ctypes.c_void_p(), ctypes.POINTER(ctypes.c_size_t)(), ctypes.c_size_t()
        status = convert_text(encoded, len(encoded), ctypes.byref(output),
                              ctypes.byref(mapping), ctypes.byref(offset))
        if status:
            assert not output.value and not bool(mapping)
            return status, None
        try:
            result = ctypes.string_at(output).decode('ascii')
            indexes = tuple(mapping[i] for i in range(len(encoded) + 1))
            assert indexes == tuple(sorted(indexes)) and indexes[-1] == len(result)
            return status, result
        finally:
            release(output)
            release(mapping)

    fa_digits = str.maketrans('0123456789,.', '۰۱۲۳۴۵۶۷۸۹٬٫')
    arabic_digits = str.maketrans('0123456789,.', '٠١٢٣٤٥٦٧٨٩٬٫')
    def phones(value):
        # Persian lexical stress is now assigned by our own rules rather
        # than copied from eSpeak; compare the underlying phones separately.
        return re.sub(r'\.[012]', '.0', re.sub(r"'([Aaeiou]):'", r'\1', value))
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        for number in range(1000):
            source = str(number)
            clauses = reference.translate(source, separator='|')
            expected = ' '.join(word_tag(token) for token in clauses[0].phonemes.strip().split())
            if 100 <= number < 200:
                # Our number grammar deliberately uses short /æ/ for صد,
                # including the connected first syllable of 101–199.
                assert "s'A:'" in expected, number
                expected = expected.replace("s'A:'", 'sA', 1)
            for script in (source, source.translate(fa_digits), source.translate(arabic_digits)):
                status, tagged = convert(script)
                assert status == 0 and phones(tagged) == phones(expected), (number, script, tagged, expected)
            if number % 17 == 0:
                text = 'من ' + source + ' آمد.'
                # Surrounding words now follow our own contextual stress
                # rules and need not match eSpeak's sentence intonation.
                # The number itself must retain its checked cardinal phones.
                status, tagged = convert(text)
                assert status == 0 and phones(expected) in phones(tagged), (text, tagged, expected)
    for text in ('4', '12', '101', '999'):
        status, tagged = convert(text)
        accents = re.findall(r'\.([012])', tagged)
        assert status == 0 and accents and accents[-1] == '1', (text, tagged)
    examples = {
        '1001': "`[.2jek] `[.0he.2za.0ro] `[.1jek]",
        '003': "`[.0sefr] `[.0sefr] `[.1se]",
        '۳٫۱۴': '`[.2se] `[.0noR.2te] `[.0jek] `[.0Ca.1har]',
        '−۳': "`[.0mAn.1fi] `[.1se]",
        '+۳': "`[.0mos.1bAt] `[.1se]",
    }
    for text, expected in examples.items():
        assert convert(text) == (0, expected), text
    for text in ('3.1.4', '۳٫۱٫۴', '٣٫١٫٤'):
        status, version = convert(text)
        assert status == 0 and version.count('noR') == 2, text
    for text in ('3,14', '3.1.4,5', '۳٫', '۱٬۲۳', '۱٬۲۳۴٬۵۶', '1234567890123',
                 'سلام ٫ دنیا', 'سلام ٬ دنیا', '۱۲-۳'):
        assert convert(text)[0] == 2, text
    randomizer = random.Random(71899)
    for _ in range(1000):
        number = randomizer.randrange(10**12)
        plain, grouped = str(number), f'{number:,}'
        baseline = convert(plain)
        assert baseline[0] == 0
        for other in (grouped, grouped.translate(fa_digits), grouped.translate(arabic_digits)):
            assert convert(other) == baseline, (number, other)
        fraction = ''.join(str(randomizer.randrange(10)) for _ in range(4))
        decimal = grouped + '.' + fraction
        assert convert(decimal) == convert(decimal.translate(fa_digits)) == convert(decimal.translate(arabic_digits))
    print('Passed 1,000 eSpeak cardinal comparisons in three scripts and 1,000 grouping/decimal cases.')


if __name__ == '__main__':
    main()
