"""Compile verified direct Persian readings for common NVDA interface text.

The source word inventory derives from NVDA's GPL translations. Every added
reading is checked against eSpeak in four positions and the Eloquence phone
parser; word-edge reference pauses still use the general fallback.
"""
from __future__ import annotations
import argparse
from collections import Counter
import ctypes
import hashlib
import json
from pathlib import Path
import re

from compile_direct_lexicon import (EXPECTED_DICT, EXPECTED_SOURCES, SOURCE, digest,
                                   is_persian_word, reference_words)
from compile_direct_pause_words import normalized
from espeak_ng_probe import EspeakPhonemeProbe
from hybrid_fa_frontend import DirectFirstPersian
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import UnsupportedPronunciation, native_sequence_accepted, word_tag

HERE = Path(__file__).resolve().parent
INVENTORY = HERE / 'reference-nvda/fa_words_2026_3_1.tsv'


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('native-library', 'espeak-library', 'data-parent', 'probe'):
        p.add_argument('--' + name, required=True, type=Path)
    p.add_argument('--output', type=Path, default=HERE / 'direct_fa_nvda_words.inc')
    args = p.parse_args()
    if any(digest(SOURCE / name) != value for name, value in EXPECTED_SOURCES.items()):
        raise RuntimeError('Persian reference sources changed')
    if digest(args.data_parent / 'espeak-ng-data/fa_dict') != EXPECTED_DICT:
        raise RuntimeError('Persian reference dictionary changed')
    entries = []
    for line in INVENTORY.read_text(encoding='utf8').splitlines():
        if line and not line.startswith('#'):
            word, count = line.split('\t')
            if is_persian_word(word):
                entries.append((word, int(count)))
    # The active DLL may already include the previous generated table. Keep
    # those words in the candidates so rebuilding never silently removes it.
    existing = set()
    previous_table = HERE / 'direct_fa_nvda_words.inc'
    if previous_table.exists():
        for line in previous_table.read_text(encoding='utf8').splitlines():
            line = line.strip()
            if line.startswith('{') and line.endswith('},'):
                existing.add(json.loads('[' + line[1:-2] + ']')[0])
    dll = ctypes.CDLL(str(args.native_library.resolve()))
    dll.reloq_fa_direct_normalize.argtypes = (ctypes.c_char_p, ctypes.c_size_t,
        ctypes.POINTER(ctypes.c_void_p), ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)),
        ctypes.POINTER(ctypes.c_size_t))
    dll.reloq_fa_direct_normalize.restype = ctypes.c_int
    dll.reloq_fa_direct_free.argtypes = (ctypes.c_void_p,)
    missing_worker = Path('/nonexistent/reference-worker')
    frontend = DirectFirstPersian(args.native_library, missing_worker, args.espeak_library, args.data_parent)
    voice = engine(args.probe, 0x90000)
    set_confirmed_persian_probe()
    rows, rejected, coverage = [], Counter(), Counter()
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as ref:
            if ref.version != '1.53.0':
                raise RuntimeError('generated text requires eSpeak NG 1.53.0')
            for index, (word, count) in enumerate(entries):
                if word not in existing and frontend.analyze_direct(word) is not None:
                    coverage['already_direct'] += count
                    continue
                if normalized(dll, word) != word:
                    rejected['noncanonical'] += 1
                    continue
                try:
                    raw = reference_words(ref, word)
                    phones = [value.strip('|').split('|') for value in raw]
                    if any(value[0] == '_' or value[-1] == '_' for value in phones):
                        rejected['edge_pause'] += 1
                        continue
                    tags = tuple(word_tag(value) for value in raw)
                    if (tags[0] != tags[3] or tags[1] != tags[2] or
                        re.sub(r'\.[012]', '.', tags[0]) != re.sub(r'\.[012]', '.', tags[1])):
                        rejected['context_dependent'] += 1
                        continue
                    if not all(native_sequence_accepted(tag.replace('~', ' '),
                               voice._phonemes_for_probe(tag.replace('~', ' '))) for tag in set(tags)):
                        rejected['native_display'] += 1
                        continue
                    rows.append((word, tags[0], tags[1]))
                    coverage['new_direct'] += count
                except UnsupportedPronunciation as error:
                    rejected[str(error).split(':', 1)[0]] += 1
                if index and index % 500 == 0:
                    print('Checked',index,'of',len(entries),'NVDA words',flush=True)
    finally:
        voice.close();frontend.close()
    if len(rows) < 200 or 'متوقف' not in {r[0] for r in rows}:
        raise RuntimeError('unexpectedly low NVDA direct coverage')
    metadata = {'nvda_po_git_blob': '3376c75d89fca3cd61a214ed2ace5eb80540444c',
                'inventory_sha256': digest(INVENTORY), 'fa_dict_sha256': EXPECTED_DICT,
                'fa_rules_sha256': EXPECTED_SOURCES['fa_rules'],
                'candidates': len(entries), 'admitted': len(rows),
                'weighted_coverage': dict(coverage), 'rejected': dict(sorted(rejected.items()))}
    out = ['/* Generated from NVDA Persian translations and eSpeak NG rules.\n',
           ' * NVDA: GPL-2.0-or-later; eSpeak NG: GPL-3.0-or-later.\n',
           ' * '+json.dumps(metadata,sort_keys=True,ensure_ascii=True)+'\n */\n',
           'static const struct fa_context_entry fa_nvda_entries[] = {\n']
    for word,final,nonfinal in rows:
        out.append('    {'+', '.join(json.dumps(s,ensure_ascii=False) for s in (word,final,nonfinal))+'},\n')
    out.append('};\n')
    temp=args.output.with_name(args.output.name+'.tmp')
    temp.write_text(''.join(out),encoding='utf8',newline='\n');temp.replace(args.output)
    print(json.dumps(metadata,sort_keys=True),flush=True)

if __name__ == '__main__':
    main()
