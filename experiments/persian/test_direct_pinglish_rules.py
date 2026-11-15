"""Compare native Pinglish rules and English exceptions with eSpeak."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import random

from espeak_ng_probe import EspeakPhonemeProbe
from pinglish_frontend import english_exception
from text_frontend import UnsupportedPronunciation, word_tag


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--espeak-library', required=True, type=Path)
    parser.add_argument('--data-parent', required=True, type=Path)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    native = dll.reloq_fa_pinglish_word
    native.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t)
    native.restype = ctypes.c_int

    def run(word, capacity=2048):
        encoded = word.encode('ascii')
        output = ctypes.create_string_buffer(capacity)
        status = native(encoded, len(encoded), output, capacity)
        return status, output.value.decode('ascii')

    words = ['khane', 'chetori', 'kheili', 'khodahafez', 'computer', 'shomaa', 'shahr', 'dost']
    randomizer = random.Random(20261115)
    pieces = ('s', 'm', 'b', 'd', 'r', 'sh', 'kh', 't', 'n', 'p', 'l', 'gh',
              'a', 'e', 'i', 'o', 'u', 'aa', 'oo')
    words += [''.join(randomizer.choice(pieces) for _ in range(randomizer.randrange(2, 8)))
              for _ in range(2000)]
    checked = 0
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        for word in words:
            status, result = run(word)
            if status == 0:
                continue
            assert status == 1
            phones = reference.translate(word, voice='fa-latn', separator='|')[0].phonemes
            try:
                expected = word_tag(phones)
            except UnsupportedPronunciation:
                continue
            assert result == expected, (word, result, expected)
            assert run(word.upper()) == (1, result)
            checked += 1
        for word in ('father', 'phone', 'python', 'math', 'the', 'this', 'with', 'PHOTO'):
            assert english_exception(word) and run(word)[0] == 0
            assert '(en)' in reference.translate(word, voice='fa-latn', separator='|')[0].phonemes
        for word in ('salam', 'khane', 'chetori', 'internet', 'computer'):
            assert not english_exception(word)
            assert '(en)' not in reference.translate(word, voice='fa-latn', separator='|')[0].phonemes
    assert checked >= 1500, checked
    assert run('khane', 3)[0] == -2
    assert run('zooon')[0] == 0
    assert run('khane\0')[0] == -1
    assert run('world')[0] == 1  # /w/ uses the closest available Persian /v/.
    print(f'Passed {checked} direct Pinglish words, English exceptions and output bounds.')


if __name__ == '__main__':
    main()
