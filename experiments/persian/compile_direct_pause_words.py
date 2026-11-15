"""Generate direct Persian words with verified short-pause behavior."""
from __future__ import annotations

import argparse
from collections import Counter
import ctypes
import json
from pathlib import Path

from compile_direct_lexicon import EXPECTED_DICT, EXPECTED_SOURCES, SOURCE, digest, is_persian_word, reference_words
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import UnsupportedPronunciation, native_sequence_accepted, word_tag

HERE = Path(__file__).resolve().parent


def source_words() -> tuple[str, ...]:
    words = set()
    for raw in (SOURCE / 'fa_list').read_text(encoding='utf8').splitlines():
        fields = raw.split('//', 1)[0].split()
        if len(fields) >= 2 and is_persian_word(fields[0]) and '_' in fields[1]:
            words.add(fields[0])
    return tuple(sorted(words, key=lambda word: word.encode('utf8')))


def normalized(dll, word: str) -> str:
    raw = word.encode('utf8')
    output = ctypes.c_void_p()
    mapping = ctypes.POINTER(ctypes.c_size_t)()
    offset = ctypes.c_size_t()
    status = dll.reloq_fa_direct_normalize(raw, len(raw), ctypes.byref(output),
                                           ctypes.byref(mapping), ctypes.byref(offset))
    if status:
        raise RuntimeError(f'normalization failed for {word!r}')
    try:
        return ctypes.string_at(output).decode('utf8')
    finally:
        dll.reloq_fa_direct_free(output)
        dll.reloq_fa_direct_free(mapping)


def generate(api, native, dll) -> str:
    rows = []
    rejected = Counter()
    for word in source_words():
        if normalized(dll, word) != word:
            rejected['noncanonical spelling'] += 1
            continue
        try:
            reference = reference_words(api, word)
            phone_lists = [raw.strip('|').split('|') for raw in reference]
            tags = tuple(word_tag(raw) for raw in reference)
            tag = tags[0]
            if any(value != tag for value in tags[1:]):
                rejected['context-dependent'] += 1
            elif any('_' not in phones for phones in phone_lists):
                rejected['no short pause'] += 1
            elif any(phones[0] == '_' or phones[-1] == '_' for phones in phone_lists):
                # Word-edge pauses need surrounding word and punctuation
                # context; leave them to the verified reference fallback.
                rejected['word-edge boundary'] += 1
            elif not native_sequence_accepted(tag, native._phonemes_for_probe(tag)):
                rejected['native parser'] += 1
            else:
                rows.append((word, tag))
        except UnsupportedPronunciation:
            rejected['unsupported'] += 1
    if len(rows) < 50:
        raise RuntimeError('unexpectedly few verified internal-pause words')
    existing = set()
    for name in ('direct_fa_lexicon.inc', 'direct_fa_context.inc'):
        for line in (HERE / name).read_text(encoding='utf8').splitlines():
            item = line.strip()
            if item.startswith('{') and item.endswith('},'):
                existing.add(json.loads('[' + item[1:-2] + ']')[0])
    if any(word in existing for word, _ in rows):
        raise RuntimeError('an internal-pause word overlaps an existing direct entry')
    metadata = {'fa_list_sha256': digest(SOURCE / 'fa_list'),
                'fa_dict_sha256': EXPECTED_DICT,
                'candidates': len(source_words()), 'admitted': len(rows),
                'rejected': dict(sorted(rejected.items()))}
    output = [
        '/* Generated from eSpeak NG Persian dictionary; GPL-3.0-or-later.\n',
        ' * Internal short pauses use a glottal onset before vowels and no comma.\n',
        ' * ' + json.dumps(metadata, sort_keys=True, ensure_ascii=True) + '\n',
        ' */\n',
        'static const struct fa_direct_entry fa_pause_entries[] = {\n',
    ]
    for word, tag in rows:
        output.append('    {' + json.dumps(word, ensure_ascii=False) + ', ' +
                      json.dumps(tag, ensure_ascii=True) + '},\n')
    output.append('};\n')
    return ''.join(output)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--output', type=Path, default=HERE / 'direct_fa_pause_words.inc')
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    if any(digest(SOURCE / name) != expected for name, expected in EXPECTED_SOURCES.items()):
        raise RuntimeError('unexpected Persian reference source')
    if digest(args.data_parent / 'espeak-ng-data/fa_dict') != EXPECTED_DICT:
        raise RuntimeError('unexpected eSpeak Persian dictionary')
    dll = ctypes.CDLL(str(args.native_library.resolve()))
    dll.reloq_fa_direct_normalize.argtypes = (
        ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_void_p),
        ctypes.POINTER(ctypes.POINTER(ctypes.c_size_t)), ctypes.POINTER(ctypes.c_size_t))
    dll.reloq_fa_direct_normalize.restype = ctypes.c_int
    dll.reloq_fa_direct_free.argtypes = (ctypes.c_void_p,)
    set_confirmed_persian_probe()
    native = engine(args.probe, 0x90000)
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            if api.version != '1.53.0':
                raise RuntimeError('this table requires eSpeak NG 1.53.0')
            value = generate(api, native, dll)
    finally:
        native.close()
    if args.check:
        if args.output.read_text(encoding='utf8') != value:
            raise RuntimeError('generated Persian internal-pause table differs')
    else:
        args.output.write_text(value, encoding='utf8', newline='\n')
    print('Checked direct Persian internal-pause words.')


if __name__ == '__main__':
    main()
