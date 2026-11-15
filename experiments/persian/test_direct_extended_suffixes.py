"""Validate every admitted direct Persian ending and a reference sample."""
from __future__ import annotations

import argparse
import ctypes
import random
import re
from pathlib import Path

from direct_fa_extended_suffix_rules import SUFFIXES, candidate
from espeak_ng_probe import EspeakPhonemeProbe
from test_direct_fa_suffixes import entries, reference_tags


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--espeak-library', required=True, type=Path)
    parser.add_argument('--data-parent', required=True, type=Path)
    args = parser.parse_args()
    rows = list(entries())
    source = Path(__file__).with_name('direct_fa_extended_suffixes.inc').read_text(encoding='utf8')
    body = source.split('fa_extended_suffix_bases', 1)[1].split('= {', 1)[1]
    values = bytes(int(x, 16) for x in re.findall(r'0x([0-9a-f]{2})', body))
    stride = (len(rows) + 7) // 8
    assert len(values) == stride * len(SUFFIXES) * 2
    dll = ctypes.CDLL(str(args.library.resolve()))
    lookup = dll.reloq_fa_direct_context_word
    lookup.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int, ctypes.c_void_p, ctypes.c_size_t)
    lookup.restype = ctypes.c_int
    admitted = []
    for index, (stem, base) in enumerate(rows):
        for kind, suffix in enumerate(SUFFIXES):
            modes = [mode for mode in range(2)
                     if values[(kind * 2 + mode) * stride + index // 8] & (1 << (index % 8))]
            assert len(modes) <= 1
            if not modes:
                continue
            expected = candidate(base, kind, modes[0])
            assert expected
            word = stem + suffix
            encoded = word.encode('utf8')
            buffer = ctypes.create_string_buffer(2048)
            for follows in (0, 1):
                status = lookup(encoded, len(encoded), follows, buffer, len(buffer))
                assert status == 1 and buffer.value.decode('ascii') == expected, (
                    word, follows, status, expected, buffer.value)
            admitted.append((word, expected))
    assert len(admitted) > 50000, len(admitted)
    for word in ('کتابمان', 'کتابهایم', 'کتابهایشان', 'کارترین'):
        assert any(candidate_word == word for candidate_word, _ in admitted), word
    word, expected = admitted[0]
    raw = word.encode('utf8')
    assert lookup(raw, len(raw), 0, ctypes.create_string_buffer(len(expected)), len(expected)) == -2
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        for word, expected in random.Random(20261115).sample(admitted, 200):
            assert reference_tags(reference, word) == (expected,) * 4, word
    print(f'Passed {len(admitted)} extended Persian suffix admissions and source contexts.')


if __name__ == '__main__':
    main()
