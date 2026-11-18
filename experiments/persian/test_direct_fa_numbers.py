"""Check direct Persian numbers against eSpeak and script-equivalent forms."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import random

from espeak_ng_probe import EspeakPhonemeProbe
from text_frontend import translate_text, word_tag


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.native_library.resolve()))
    dll.reloq_fa_direct_text.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
                                         ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)), ctypes.POINTER(ctypes.c_size_t))
    dll.reloq_fa_direct_text.restype = ctypes.c_int
    dll.reloq_fa_direct_free.argtypes = (ctypes.c_void_p,)

    def convert(text):
        encoded = text.encode('utf8')
        output, mapping, offset = ctypes.c_void_p(), ctypes.POINTER(ctypes.c_size_t)(), ctypes.c_size_t()
        status = dll.reloq_fa_direct_text(encoded, len(encoded), ctypes.byref(output),
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
            dll.reloq_fa_direct_free(output)
            dll.reloq_fa_direct_free(mapping)

    fa_digits = str.maketrans('0123456789,.', '۰۱۲۳۴۵۶۷۸۹٬٫')
    arabic_digits = str.maketrans('0123456789,.', '٠١٢٣٤٥٦٧٨٩٬٫')
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        for number in range(1000):
            source = str(number)
            clauses = reference.translate(source, separator='|')
            expected = ' '.join(word_tag(token) for token in clauses[0].phonemes.strip().split())
            for script in (source, source.translate(fa_digits), source.translate(arabic_digits)):
                assert convert(script) == (0, expected), (number, script)
            if number % 17 == 0:
                text = 'من ' + source + ' آمد.'
                assert convert(text) == (0, translate_text(reference, text)[0].text), text
    examples = {
        '1001': "`[.2jek] `[.0he.2za.0ro] `[.1jek]",
        '003': "`[.0sefr] `[.0sefr] `[.1se]",
        '۳٫۱۴': "`[.2se] `[.0noR.2t'e:'] `[.0jek] `[.1Ca.0har]",
        '−۳': "`[.1mAn.0fi] `[.1se]",
        '+۳': "`[.1mos.0bAt] `[.1se]",
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
