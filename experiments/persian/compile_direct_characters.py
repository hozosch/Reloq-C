"""Generate native Persian symbol, Latin-letter and compatibility names.

The data is the same pinned, validated table used by the Persian NVDA
prototype. Compatibility aliases are limited to single-codepoint NFKC
forms of already supported Persian letters, matching its character path.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import unicodedata

from persian_symbols import SYMBOL_TAGS, UNICODE_TAG
from pinglish_frontend import LATIN_NAMES

HERE = Path(__file__).resolve().parent


def generate() -> str:
    text = (HERE / 'direct_fa_letters.inc').read_text(encoding='utf8')
    letters = {int(value, 16) for value in re.findall(r'\{0x([0-9a-fA-F]+)u,', text)}
    if not letters:
        raise ValueError('The Persian letter-name table is empty')
    normalized_arabic = {0x0643: 0x06a9, 0x064a: 0x06cc, 0x0649: 0x06cc}
    aliases = {}
    for codepoint in range(1, 0x110000):
        if 0xd800 <= codepoint <= 0xdfff:
            continue
        normalized = unicodedata.normalize('NFKC', chr(codepoint))
        if len(normalized) != 1:
            continue
        mapped = normalized_arabic.get(ord(normalized), ord(normalized))
        if mapped in letters and mapped != codepoint:
            aliases[codepoint] = mapped
    symbols = {ord(char): tag for char, tag in SYMBOL_TAGS.items()}
    lines = [
        '/* Generated from the pinned Persian NVDA symbol/letter names;',
        ' * GPL-3.0-or-later. Compatibility aliases were checked with Unicode 15.0.0.',
        ' * Regenerate with compile_direct_characters.py. */',
        'static const struct fa_character_name fa_symbol_names[] = {',
    ]
    lines.extend('    {0x%04xu, %s},' % (cp, json.dumps(tag)) for cp, tag in sorted(symbols.items()))
    lines.extend(('};', 'static const char *const fa_latin_names[26] = {'))
    lines.extend('    %s,' % json.dumps(LATIN_NAMES[chr(cp)]) for cp in range(ord('a'), ord('z') + 1))
    lines.extend(('};', 'static const struct fa_character_alias fa_letter_aliases[] = {'))
    lines.extend('    {0x%04xu, 0x%04xu},' % (cp, mapped) for cp, mapped in sorted(aliases.items()))
    lines.extend(('};', 'static const char fa_unicode_name[] = ' + json.dumps(UNICODE_TAG) + ';', ''))
    return '\n'.join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=HERE / 'direct_fa_characters.inc')
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    value = generate()
    if args.check:
        if args.output.read_text(encoding='utf8') != value:
            raise SystemExit('Native character names need regeneration')
    else:
        args.output.write_text(value, encoding='utf8', newline='\n')
    print('Checked native Persian symbols and character compatibility.' if args.check else
          'Generated native Persian symbols and character compatibility.')


if __name__ == '__main__':
    main()
