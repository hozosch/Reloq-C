"""Admit additional exact Persian words after native display equivalence checks."""
from __future__ import annotations

import argparse
from collections import Counter
import ctypes
import json
from pathlib import Path
import re

from compile_direct_lexicon import (CORPUS, EXPECTED_DICT, EXPECTED_SOURCES, SOURCE,
                                   candidates, digest, reference_words)
from compile_direct_pause_words import normalized
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import UnsupportedPronunciation, native_sequence_accepted, word_tag

HERE = Path(__file__).resolve().parent

# The words form Persian diacritic names reported in issue 49 and NVDA's
# existing symbol replacement. They also need to work without the reader.
EXTRA_SYMBOL_NAME_WORDS = {'همزه‌ی', 'همزه', 'مقصوره', 'تشدید'}


def existing_words():
    words = set()
    for name in ('direct_fa_lexicon.inc', 'direct_fa_context.inc', 'direct_fa_pause_words.inc'):
        for line in (HERE / name).read_text(encoding='utf8').splitlines():
            value = line.strip()
            if value.startswith('{') and value.endswith('},'):
                words.add(json.loads('[' + value[1:-2] + ']')[0])
    return words


def generate(api, native, dll):
    existing = existing_words()
    rows, rejected = [], Counter()
    for word in sorted(set(candidates(SOURCE / 'fa_list', CORPUS)) | EXTRA_SYMBOL_NAME_WORDS,
                       key=lambda value: value.encode('utf8')):
        if word in existing:
            continue
        if normalized(dll, word) != word:
            rejected['noncanonical'] += 1
            continue
        try:
            reference = reference_words(api, word)
            phone_lists = [raw.strip('|').split('|') for raw in reference]
            if any(phones[0] == '_' or phones[-1] == '_' for phones in phone_lists):
                # A reference pause at a word edge can select a native
                # glottal onset on the following word. Preserve that context
                # through the reference route until direct rules encode it.
                rejected['word-edge pause'] += 1
                continue
            tags = tuple(word_tag(raw) for raw in reference)
            if any(tag.startswith('~') or tag.endswith('~') for tag in tags):
                rejected['word-edge pause'] += 1
                continue
            if (tags[0] != tags[3] or tags[1] != tags[2] or
                    re.sub(r'\.[012]', '.', tags[0]) != re.sub(r'\.[012]', '.', tags[1])):
                rejected['context-dependent'] += 1
                continue
            if not all(native_sequence_accepted(tag.replace('~', ' '),
                       native._phonemes_for_probe(tag.replace('~', ' '))) for tag in set(tags)):
                rejected['native display'] += 1
                continue
            rows.append((word, tags[0], tags[1]))
        except UnsupportedPronunciation:
            rejected['unsupported'] += 1
    if len(rows) < 20:
        raise RuntimeError('unexpectedly few verified recovered words')
    if not EXTRA_SYMBOL_NAME_WORDS <= existing | {word for word, _, _ in rows}:
        raise RuntimeError('a required symbol name failed reference validation')
    meta = {'fa_list_sha256': EXPECTED_SOURCES['fa_list'], 'fa_rules_sha256': EXPECTED_SOURCES['fa_rules'],
            'fa_dict_sha256': EXPECTED_DICT, 'admitted': len(rows), 'rejected': dict(sorted(rejected.items()))}
    lines = ['/* Generated from eSpeak NG Persian sources, GPL-3.0-or-later.\n',
             ' * ' + json.dumps(meta, sort_keys=True, ensure_ascii=True) + '\n',
             ' */\n', 'static const struct fa_context_entry fa_recovered_entries[] = {\n']
    for word, final, nonfinal in rows:
        lines.append('    {' + ', '.join(json.dumps(value, ensure_ascii=False)
                                        for value in (word, final, nonfinal)) + '},\n')
    lines.append('};\n')
    return ''.join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('native-library', 'espeak-library', 'data-parent', 'probe'):
        parser.add_argument('--' + option, type=Path, required=True)
    parser.add_argument('--output', type=Path, default=HERE / 'direct_fa_recovered_words.inc')
    args = parser.parse_args()
    if any(digest(SOURCE / name) != expected for name, expected in EXPECTED_SOURCES.items()):
        raise RuntimeError('unexpected Persian reference source')
    if digest(args.data_parent / 'espeak-ng-data/fa_dict') != EXPECTED_DICT:
        raise RuntimeError('unexpected eSpeak Persian dictionary')
    dll = ctypes.CDLL(str(args.native_library.resolve()))
    dll.reloq_fa_direct_normalize.argtypes = (ctypes.c_char_p, ctypes.c_size_t,
        ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)),
        ctypes.POINTER(ctypes.c_size_t))
    dll.reloq_fa_direct_normalize.restype = ctypes.c_int
    dll.reloq_fa_direct_free.argtypes = (ctypes.c_void_p,)
    set_confirmed_persian_probe()
    native = engine(args.probe, 0x90000)
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            if api.version != '1.53.0':
                raise RuntimeError('this table requires eSpeak NG 1.53.0')
            content = generate(api, native, dll)
    finally:
        native.close()
    args.output.write_text(content, encoding='utf8', newline='\n')
    print('Generated recovered Persian direct words.')


if __name__ == '__main__':
    main()
