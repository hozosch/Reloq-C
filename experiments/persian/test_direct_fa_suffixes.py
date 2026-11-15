"""Verify generated productive suffixes and their direct C implementation."""
from __future__ import annotations

import argparse
import ctypes
import json
import random
import re
from pathlib import Path

from direct_fa_suffix_rules import SUFFIXES, candidate
from espeak_ng_probe import EspeakPhonemeProbe
from text_frontend import word_tag


def entries():
    for line in Path(__file__).with_name('direct_fa_lexicon.inc').read_text(encoding='utf8').splitlines():
        line = line.strip()
        if line.startswith('{') and line.endswith('},'):
            yield tuple(json.loads('[' + line[1:-2] + ']'))


def reference_tags(api, word):
    result = []
    for text, count, index in ((word, 1, 0), (word + ' آمد.', 2, 0),
                               ('من ' + word + ' آمد.', 3, 1), ('من ' + word + '.', 2, 1)):
        clauses = api.translate(text, separator='|')
        assert len(clauses) == 1
        words = clauses[0].phonemes.strip().split()
        assert len(words) == count
        result.append(word_tag(words[index]))
    return tuple(result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--espeak-library', required=True, type=Path)
    parser.add_argument('--data-parent', required=True, type=Path)
    args = parser.parse_args()
    rows = list(entries())
    source = Path(__file__).with_name('direct_fa_suffixes.inc').read_text(encoding='utf8')
    body = source.split('fa_direct_suffix_bases', 1)[1].split('= {', 1)[1]
    values = bytes(int(x, 16) for x in re.findall(r'0x([0-9a-f]{2})', body))
    stride = (len(rows) + 7) // 8
    assert len(values) == stride * len(SUFFIXES)
    dll = ctypes.CDLL(str(args.library.resolve()))
    lookup = dll.reloq_fa_direct_context_word
    lookup.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int,
                       ctypes.c_void_p, ctypes.c_size_t)
    lookup.restype = ctypes.c_int
    admitted = []
    for index, (stem, base) in enumerate(rows):
        for kind, suffix in enumerate(SUFFIXES):
            if not values[kind * stride + index // 8] & (1 << (index % 8)):
                continue
            word, expected = stem + suffix, candidate(base, suffix)
            assert expected
            encoded = word.encode('utf8')
            output = ctypes.create_string_buffer(2048)
            for follows in (0, 1):
                status = lookup(encoded, len(encoded), follows, output, len(output))
                assert status == 1 and output.value.decode('ascii') == expected, (
                    word, follows, status, expected, output.value)
            admitted.append((word, expected))
    assert len(admitted) >= 60000
    for word in ('کتابم', 'کتابت', 'کتابش', 'کتابها', 'کاری', 'سلامم'):
        assert any(item[0] == word for item in admitted), word
    word, expected = admitted[0]
    encoded = word.encode('utf8')
    too_small = ctypes.create_string_buffer(len(expected))
    assert lookup(encoded, len(encoded), 0, too_small, len(too_small)) == -2
    # Re-check a deterministic sample directly against the pinned source.
    rng = random.Random(20261115)
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        for word, expected in rng.sample(admitted, 120):
            assert reference_tags(reference, word) == (expected,) * 4, word
    print(f'Passed {len(admitted)} direct suffix admissions, word contexts and buffer bounds.')


if __name__ == '__main__':
    main()
