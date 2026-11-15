"""Verify common NVDA readings in the C frontend and against eSpeak."""
from __future__ import annotations

import argparse
import ctypes
import json
from pathlib import Path
import random

from espeak_ng_probe import EspeakPhonemeProbe
from test_direct_fa_suffixes import reference_tags


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    rows = []
    for line in Path(__file__).with_name('direct_fa_nvda_words.inc').read_text(encoding='utf8').splitlines():
        line = line.strip()
        if line.startswith('{') and line.endswith('},'):
            rows.append(tuple(json.loads('[' + line[1:-2] + ']')))
    assert len(rows) >= 1700
    assert len({word for word, _, _ in rows}) == len(rows)
    assert [word for word, _, _ in rows] == sorted(word for word, _, _ in rows)
    assert 'متوقف' in {word for word, _, _ in rows}
    dll = ctypes.CDLL(str(args.library.resolve()))
    lookup = dll.reloq_fa_direct_context_word
    lookup.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int, ctypes.c_void_p, ctypes.c_size_t)
    lookup.restype = ctypes.c_int
    for word, final, nonfinal in rows:
        encoded = word.encode('utf8')
        for follows, expected in ((0, final), (1, nonfinal)):
            result = ctypes.create_string_buffer(2048)
            status = lookup(encoded, len(encoded), follows, result, len(result))
            assert status == 1 and result.value.decode('ascii') == expected, (word, follows, status, result.value)
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        sample = random.Random(20261115).sample(rows, 200)
        sample.append(next(row for row in rows if row[0] == 'متوقف'))
        for word, final, nonfinal in sample:
            assert reference_tags(reference, word) == (final, nonfinal, nonfinal, final), word
        assert reference_tags(reference, 'دار') == ('`[.1dar]',) * 4
    print(f'Passed {len(rows)} direct NVDA word readings and 201 four-context reference comparisons.')


if __name__ == '__main__':
    main()
