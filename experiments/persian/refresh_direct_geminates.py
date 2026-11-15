"""Refresh affected direct words after the Persian affricate correction.

This targeted update validates every changed word and affected plural against
the same four reference contexts and native parser as the full generators.
The ordered keys and admission bitmap must stay unchanged.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re

from compile_direct_lexicon import (EXPECTED_DICT, EXPECTED_SOURCES, SOURCE, checked_tag,
                                    digest, lexical_fingerprint, reference_tags)
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import UnsupportedPronunciation, native_tag_accepted

HERE = Path(__file__).resolve().parent
LEXICON = HERE / 'direct_fa_lexicon.inc'
PLURAL = HERE / 'direct_fa_plural.inc'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    args = parser.parse_args()
    if any(digest(SOURCE / name) != value for name, value in EXPECTED_SOURCES.items()):
        raise RuntimeError('unexpected reference sources')
    if digest(args.data_parent / 'espeak-ng-data/fa_dict') != EXPECTED_DICT:
        raise RuntimeError('unexpected reference dictionary')
    lex_lines = LEXICON.read_text(encoding='utf8').splitlines(keepends=True)
    plural = PLURAL.read_text(encoding='utf8')
    bits = bytes(int(part, 16) for line in plural.splitlines()
                 if line.strip().startswith('0x') for part in line.strip().rstrip(',').split(', '))
    rows, changes = [], []
    for line_index, line in enumerate(lex_lines):
        item = line.strip()
        if not (item.startswith('{') and item.endswith('},')):
            continue
        word, old = json.loads('[' + item[1:-2] + ']')
        new = re.sub(r'J(?=(?:\.[012])?J)', 'd', old)
        if new != old:
            changes.append((len(rows), line_index, word, old, new))
        rows.append((word, new))
    if not changes or len(bits) != (len(rows) + 7) // 8:
        raise RuntimeError('unexpected lexicon or plural bitmap')
    if any(re.search(r'J(?:\.[012])?J', (HERE / name).read_text(encoding='utf8'))
           for name in ('direct_fa_context.inc', 'direct_fa_pause_words.inc')):
        raise RuntimeError('regenerate the affected context or pause table too')
    native = engine(args.probe, 0x90000)
    try:
        set_confirmed_persian_probe()
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            if api.version != '1.53.0':
                raise RuntimeError('unexpected reference version')
            for index, _, word, old, new in changes:
                if checked_tag(api, word) != new or not native_tag_accepted(new, native._phonemes_for_probe(new)):
                    raise RuntimeError(f'unverified geminate {word!r}: {old!r} -> {new!r}')
                admitted = False
                if '\u200c' not in word and new.endswith(']'):
                    final, nonfinal = new[:-1] + '.1ha]', new[:-1] + '.0ha]'
                    try:
                        tags = reference_tags(api, word + '\u200cها')
                        admitted = (tags == (final, nonfinal, nonfinal, final) and
                                    all(native_tag_accepted(t, native._phonemes_for_probe(t))
                                        for t in (final, nonfinal)))
                    except UnsupportedPronunciation:
                        pass
                if admitted != bool(bits[index // 8] & (1 << (index % 8))):
                    raise RuntimeError(f'plural admission changed for {word!r}')
    finally:
        native.close()
    for _, line_index, word, _, new in changes:
        lex_lines[line_index] = '    {' + json.dumps(word, ensure_ascii=False) + ', ' + json.dumps(new) + '},\n'
    fingerprint = lexical_fingerprint(rows)
    updated = ''.join(lex_lines)
    updated, count = re.subn(r'(?m)^(#define RELOQ_FA_LEXICON_FINGERPRINT 0x)[0-9a-f]+(ULL)$',
                             r'\g<1>' + fingerprint + r'\2', updated)
    if count != 1:
        raise RuntimeError('missing lexicon fingerprint')
    lex_sha = hashlib.sha256(updated.encode('utf8')).hexdigest()
    match = re.search(r'(?m)^ \* (\{.*\})$', plural)
    if match is None:
        raise RuntimeError('missing plural metadata')
    metadata = json.loads(match.group(1))
    metadata['lexicon_sha256'] = lex_sha
    plural = plural[:match.start(1)] + json.dumps(metadata, sort_keys=True) + plural[match.end(1):]
    plural, count = re.subn(r'(?m)^(#if RELOQ_FA_LEXICON_FINGERPRINT != 0x)[0-9a-f]+(ULL)$',
                            r'\g<1>' + fingerprint + r'\2', plural)
    if count != 1:
        raise RuntimeError('missing plural fingerprint')
    LEXICON.write_text(updated, encoding='utf8', newline='\n')
    PLURAL.write_text(plural, encoding='utf8', newline='\n')
    print(f'Updated {len(changes)} verified geminate words and their plural fingerprint.')


if __name__ == '__main__':
    main()
