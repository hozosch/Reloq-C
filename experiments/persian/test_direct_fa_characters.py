"""Compare shared native character names with the previous NVDA frontend."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes
from pathlib import Path
import random
import re
import string
import unicodedata

from hybrid_fa_frontend import DirectFirstPersian
from persian_symbols import SYMBOL_TAGS, UNICODE_TAG

HERE = Path(__file__).resolve().parent


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    frontend = DirectFirstPersian(args.library, None, None, None)
    native = frontend._character
    assert native is not None, 'the shared native character API is missing'
    values = set(map(ord, SYMBOL_TAGS))
    values.update(map(ord, string.ascii_letters))
    values.update((0x0621, 0x0622, 0x0627, 0x0634, 0x0643, 0x0649, 0x064a,
                   0x06a9, 0x06cc, 0x200c, 0x200d, 0x02c8, 0x02cc, 0xfe0f,
                   0x0654, 0x0623, 0x0307, 0x0259, 0x03b1, 0x00e9, 0x0416,
                   0x12550, 0x13460, 0x18cff, 0x18e00, 0x2b81e, 0x3d000,
                   0x1f642, 0x1f1e9, 0x10ffff, 0xfb56, 0xfe8e))
    aliases = (HERE / 'direct_fa_characters.inc').read_text(encoding='ascii')
    values.update(int(cp, 16) for cp in re.findall(r'\{0x([0-9a-f]+)u, 0x[0-9a-f]+u\}', aliases))
    letters = (HERE / 'direct_fa_letters.inc').read_text(encoding='ascii')
    values.update(int(cp, 16) for cp in re.findall(r'\{0x([0-9a-f]+)u,', letters))
    rng = random.Random(49129)
    values.update(cp for _ in range(6000) if (cp := rng.randrange(1, 0x110000)) < 0xd800 or cp > 0xdfff)

    def native_text(codepoint):
        output = ctypes.create_string_buffer(2048)
        assert native(codepoint, output, len(output)) == 1, hex(codepoint)
        return output.value.decode('ascii')

    try:
        assert native.__name__ in ('eciPersianCharacter', 'reloq_fa_direct_character')
        for cp in sorted(values):
            char = chr(cp)
            frontend._character = None
            expected = frontend.analyze_character(char)
            frontend._character = native
            actual = frontend.analyze_character(char)
            assert actual == expected, (hex(cp), expected, actual)
            assert native_text(cp) == expected.parts[0].text, hex(cp)
            assert actual.source_map == (0,) * len(char.encode('utf8')) + (len(actual.parts[0].text),)
        for invalid in (0, 0xd800, 0xdfff, 0x110000, 0xffffffff):
            untouched = ctypes.create_string_buffer(b'keep', 5)
            assert native(invalid, untouched, len(untouched)) == 0
            assert untouched.value == b'keep'
        for char in 'ˈˌə\u0307αé⚽🙂':
            assert native_text(ord(char)) == SYMBOL_TAGS[char], char
        assert native_text(ord('Ж')) == '`l1.0 cyrillic capital letter zhe `l9.0'
        assert native_text(0x12550) == '`l1.0 cuneiform numeric sign one n01 `l9.0'
        assert native_text(0x18cff) == '`l1.0 khitan small script character 18cff `l9.0'
        assert native_text(0x3d000) == '`l1.0 small seal character 3d000 `l9.0'
        assert native_text(0x0378).startswith(UNICODE_TAG) and not unicodedata.name('\u0378', '')
        untouched = ctypes.create_string_buffer(b'keep', 5)
        assert native(0x1f642, untouched, 3) == -1 and untouched.value == b'keep'
        assert native(0x1f642, None, 0) == -1
        with ThreadPoolExecutor(max_workers=4) as pool:
            sample = sorted(values)[:300]
            assert list(pool.map(native_text, sample)) == [native_text(cp) for cp in sample]
        assert frontend._worker is None, 'standalone characters must not need eSpeak'
    finally:
        frontend._character = native
        frontend.close()
    print(f'Passed {len(values)} native Persian character parity, buffer, and concurrency checks.')


if __name__ == '__main__':
    main()
