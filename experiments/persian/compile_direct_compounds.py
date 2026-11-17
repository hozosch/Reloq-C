"""Generate reference-checked admissions for bounded Persian compounds.

Only stems already present in the native lexicon or stable NVDA word table
are considered. Every admitted candidate agrees with eSpeak in four word
positions and is accepted by the native phone parser. The generated rules
run without an eSpeak instance; unadmitted forms retain the existing fallback.
"""
from __future__ import annotations

import argparse
from collections import Counter
import ctypes
import json
from pathlib import Path

from compile_direct_lexicon import EXPECTED_DICT, EXPECTED_SOURCES, SOURCE, digest, lexical_fingerprint, reference_tags
from compile_direct_plural import LEXICON, entries
from direct_fa_compound_rules import SUFFIXES, candidate
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import UnsupportedPronunciation, native_tag_accepted

HERE = Path(__file__).resolve().parent
NVDA_WORDS = HERE / 'direct_fa_nvda_words.inc'


def extra_entries(main_words, dll):
    result = []
    for line in NVDA_WORDS.read_text(encoding='utf8').splitlines():
        line = line.strip()
        if not line.startswith('{') or not line.endswith('},'):
            continue
        word, final, nonfinal = json.loads('[' + line[1:-2] + ']')
        if word in main_words or final != nonfinal:
            continue
        raw = word.encode('utf8')
        values = []
        for follows in (0, 1):
            output = ctypes.create_string_buffer(2048)
            status = dll.reloq_fa_direct_context_word(raw, len(raw), follows, output, len(output))
            values.append(output.value.decode('ascii') if status == 1 else None)
        if values == [final, nonfinal]:
            result.append((word, final))
    return sorted(result, key=lambda item: item[0].encode('utf8'))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('native-library', 'espeak-library', 'data-parent', 'probe'):
        parser.add_argument('--' + option, required=True, type=Path)
    parser.add_argument('--output', type=Path, default=HERE / 'direct_fa_compounds.inc')
    args = parser.parse_args()
    if any(digest(SOURCE / name) != expected for name, expected in EXPECTED_SOURCES.items()):
        raise RuntimeError('the pinned Persian reference sources changed')
    if digest(args.data_parent / 'espeak-ng-data/fa_dict') != EXPECTED_DICT:
        raise RuntimeError('the pinned Persian reference dictionary changed')
    rows = list(entries())
    dll = ctypes.CDLL(str(args.native_library.resolve()))
    dll.reloq_fa_direct_context_word.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int,
                                                  ctypes.c_void_p, ctypes.c_size_t)
    dll.reloq_fa_direct_context_word.restype = ctypes.c_int
    extras = extra_entries({word for word, _ in rows}, dll)
    masks, extra_masks, admitted = bytearray(len(rows)), [], Counter()
    voice = engine(args.probe, 0x90000)
    set_confirmed_persian_probe()
    parser_cache = {}

    def accepted(tag):
        if tag not in parser_cache:
            parser_cache[tag] = native_tag_accepted(tag, voice._phonemes_for_probe(tag))
        return parser_cache[tag]

    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            if api.version != '1.53.0':
                raise RuntimeError('compound admission requires eSpeak NG 1.53.0')
            for index, (stem, base) in enumerate((*rows, *extras)):
                mask = 0
                if '\u200c' not in stem:
                    for kind, suffix in enumerate(SUFFIXES):
                        final = candidate(base, kind, False)
                        nonfinal = candidate(base, kind, True)
                        if final is None or nonfinal is None:
                            continue
                        try:
                            if reference_tags(api, stem + suffix) != (final, nonfinal, nonfinal, final):
                                continue
                            if not all(accepted(tag) for tag in {final, nonfinal}):
                                continue
                        except (UnsupportedPronunciation, IndexError):
                            continue
                        mask |= 1 << kind
                        admitted[suffix] += 1
                if index < len(rows):
                    masks[index] = mask
                elif mask:
                    extra_masks.append((stem, base, mask))
                if index and index % 2000 == 0:
                    print(f'Checked {index}/{len(rows) + len(extras)} compound stems', flush=True)
    finally:
        voice.close()
    if any(admitted[suffix] < 50 for suffix in SUFFIXES):
        raise RuntimeError('unexpectedly few verified compounds; no table written')
    metadata = {'bases': len(rows), 'extra_bases': len(extras),
                'lexicon_sha256': digest(LEXICON), 'nvda_words_sha256': digest(NVDA_WORDS),
                'fa_dict_sha256': EXPECTED_DICT, 'fa_rules_sha256': EXPECTED_SOURCES['fa_rules'],
                'admitted': dict(admitted)}
    lines = ['/* Generated by compile_direct_compounds.py; GPL-3.0-or-later.\n',
             ' * ' + json.dumps(metadata, sort_keys=True, ensure_ascii=True) + '\n',
             ' */\n',
             '#if RELOQ_FA_LEXICON_FINGERPRINT != 0x' + lexical_fingerprint(rows) + 'ULL\n',
             '#error Persian compounds must be regenerated for this lexicon\n',
             '#endif\n', 'static const unsigned char fa_compound_main_masks[] = {\n']
    for offset in range(0, len(masks), 16):
        lines.append('    ' + ', '.join(f'0x{value:02x}' for value in masks[offset:offset + 16]) + ',\n')
    lines.append('};\nstatic const struct fa_compound_extra fa_compound_extra_stems[] = {\n')
    for word, tag, mask in extra_masks:
        lines.append('    {' + json.dumps(word, ensure_ascii=False) + ', ' + json.dumps(tag) +
                     f', 0x{mask:02x}' + '},\n')
    lines.append('};\n')
    temporary = args.output.with_name(args.output.name + '.tmp')
    temporary.write_text(''.join(lines), encoding='utf8', newline='\n')
    temporary.replace(args.output)
    print(json.dumps(metadata, sort_keys=True, ensure_ascii=True), flush=True)


if __name__ == '__main__':
    main()
