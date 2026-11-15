"""Verify direct ezafe forms against the native implementation and eSpeak."""
from __future__ import annotations
import argparse
import ctypes
import json
import random
import re
from pathlib import Path
from direct_fa_vocalized_rules import EXTRA, candidate
from test_direct_fa_suffixes import entries, reference_tags
from espeak_ng_probe import EspeakPhonemeProbe


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    rows = list(entries())
    source = Path(__file__).with_name('direct_fa_vocalized.inc').read_text(encoding='utf8')
    bitmap, exceptions = source.split('static const struct fa_direct_entry fa_vocalized_entries[] = {')
    body = bitmap.split('fa_vocalized_modes', 1)[1].split('= {', 1)[1]
    data = bytes(int(x, 16) for x in re.findall(r'0x([0-9a-f]{2})', body))
    stride = (len(rows) + 7) // 8
    assert len(data) == 2 * stride
    exact = dict(json.loads('[' + line.strip()[1:-2] + ']') for line in exceptions.splitlines()
                 if line.strip().startswith('{'))
    lookup = ctypes.CDLL(str(args.library.resolve())).reloq_fa_direct_context_word
    lookup.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int, ctypes.c_void_p, ctypes.c_size_t)
    lookup.restype = ctypes.c_int
    admitted = []
    for index, (stem, base) in enumerate(rows):
        modes = [mode for mode in range(2) if data[mode * stride + index // 8] & (1 << (index % 8))]
        assert len(modes) <= 1
        if modes:
            word = stem + '\u0650'
            assert word not in exact
            admitted.append((word, candidate(base, modes[0])))
    admitted.extend(exact.items())
    assert len(admitted) > 15000 and set(EXTRA) <= dict(admitted).keys()
    for word, expected in admitted:
        raw = word.encode('utf8')
        buffer = ctypes.create_string_buffer(2048)
        for follows in (0, 1):
            status = lookup(raw, len(raw), follows, buffer, len(buffer))
            assert status == 1 and buffer.value.decode('ascii') == expected, (word, status, expected, buffer.value)
    required = {'هنگامِ': '`[.0hen.1ga.0me]', 'ورود': "`[.1vo.0r'u:'d]",
                'اِختیاراتِ': "`[.1'e:'G.0tja.2ra.0te]"}
    assert all(dict(admitted)[word] == expected for word, expected in required.items())
    word, expected = admitted[0]
    raw = word.encode('utf8')
    assert lookup(raw, len(raw), 0, ctypes.create_string_buffer(len(expected)), len(expected)) == -2
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        for word, expected in random.Random(20261115).sample(admitted, 200) + list(required.items()):
            assert reference_tags(reference, word) == (expected,) * 4, word
    print(f'Passed {len(admitted)} direct vocalized forms, bounded output and eSpeak contexts.')

if __name__ == '__main__':
    main()
