"""Verify reference-admitted Persian compounds, boundaries and playback."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import ctypes
import hashlib
import json
from pathlib import Path
import random
import re

from direct_fa_compound_rules import SUFFIXES, candidate
from espeak_ng_probe import EspeakPhonemeProbe
from hybrid_fa_frontend import DirectFirstPersian
from test_direct_fa_suffixes import entries, reference_tags

HERE = Path(__file__).resolve().parent
EXAMPLES = ('پژوهشگر', 'پیش‌نمایش', 'اشتراک‌گذاری', 'پردازشگر')


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def table():
    content = (HERE / 'direct_fa_compounds.inc').read_text(encoding='utf8')
    metadata = json.loads(content.splitlines()[1].strip()[2:])
    assert metadata['lexicon_sha256'] == digest(HERE / 'direct_fa_lexicon.inc')
    assert metadata['nvda_words_sha256'] == digest(HERE / 'direct_fa_nvda_words.inc')
    primary, extra = content.split('static const struct fa_compound_extra fa_compound_extra_stems[] = {')
    primary = primary.split('fa_compound_main_masks[] = {', 1)[1]
    masks = bytes(int(value, 16) for value in re.findall(r'0x([0-9a-f]{2})', primary))
    rows = list(entries())
    assert len(rows) == len(masks) == metadata['bases']
    result = [(stem, tag, mask) for (stem, tag), mask in zip(rows, masks) if mask]
    extras = []
    for line in extra.splitlines():
        line = line.strip()
        if line.startswith('{') and line.endswith('},'):
            fields, mask = line[1:-2].rsplit(', ', 1)
            stem, tag = json.loads('[' + fields + ']')
            extras.append((stem, tag, int(mask, 16)))
    assert len(extras) > 100
    assert [word.encode('utf8') for word, _, _ in extras] == sorted(word.encode('utf8') for word, _, _ in extras)
    result.extend(extras)
    return result, metadata


def lookup_function(path):
    dll = ctypes.CDLL(str(path.resolve()))
    lookup = dll.reloq_fa_direct_context_word
    lookup.argtypes = (ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int, ctypes.c_void_p, ctypes.c_size_t)
    lookup.restype = ctypes.c_int
    return lookup


def lookup_word(lookup, word, follows):
    raw = word.encode('utf8')
    buffer = ctypes.create_string_buffer(2048)
    status = lookup(raw, len(raw), int(follows), buffer, len(buffer))
    return status, buffer.value.decode('ascii')


def compare_playback(args):
    if not args.previous:
        return
    if not args.scope_probe:
        raise ValueError('--previous requires --scope-probe')
    from _reloq_eci_direct import EciIndex, SAMPLE_RATE
    from nvda_driver_probe import make_language_engine
    voices = [make_language_engine(args.scope_probe, path, None, args.espeak_library, args.data_parent)
              for path in (args.previous, args.library)]
    try:
        for rate in (3, 6):
            for voice in voices:
                voice.set_parameter(SAMPLE_RATE, rate)
            for word in EXAMPLES:
                for before, after in (('', ''), ('سلام ', ' دنیا.'), ('NVDA ', ' سلام')):
                    mid = len(word) // 2
                    parts = (EciIndex(1), before + word[:mid], EciIndex(2), word[mid:] + after, EciIndex(3))
                    output = []
                    for voice in voices:
                        pcm, indexes = bytearray(), []
                        assert voice.speak(parts, pcm.extend, lambda value: indexes.append((value, len(pcm))))
                        output.append((bytes(pcm), indexes))
                    assert output[0] == output[1], (word, before, after, rate, output[0][1], output[1][1])
    finally:
        for voice in voices:
            voice.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    parser.add_argument('--scope-probe', type=Path)
    args = parser.parse_args()
    rows, metadata = table()
    lookup = lookup_function(args.library)
    old = lookup_function(args.previous) if args.previous else None
    admissions, by_kind = [], [0] * len(SUFFIXES)
    for stem, base, mask in rows:
        assert mask < 1 << len(SUFFIXES)
        for kind, suffix in enumerate(SUFFIXES):
            if not mask & (1 << kind):
                continue
            word = stem + suffix
            for follows in (False, True):
                expected = candidate(base, kind, follows)
                assert expected and lookup_word(lookup, word, follows) == (1, expected), (word, follows)
                if old:
                    status, previous = lookup_word(old, word, follows)
                    assert status == 0 or (status == 1 and previous == expected), (word, follows, previous, expected)
            admissions.append((word, base, kind))
            by_kind[kind] += 1
    assert by_kind == [metadata['admitted'][suffix] for suffix in SUFFIXES]
    assert all(count > 10000 for count in by_kind)
    for word in EXAMPLES:
        assert lookup_word(lookup, word, False)[0] == 1
        if old:
            assert lookup_word(old, word, False)[0] == 0
    assert lookup_word(lookup, 'ناموجودمطمئن‌نمایش', False)[0] == 0
    word = EXAMPLES[0]
    raw = word.encode('utf8')
    buffer = ctypes.create_string_buffer(b'keep', 5)
    assert lookup(raw, len(raw), 0, buffer, len(buffer)) == -2
    assert buffer.raw[1:] == b'eep\0', 'write escaped the available output buffer'
    with ThreadPoolExecutor(max_workers=4) as pool:
        assert list(pool.map(lambda word: lookup_word(lookup, word, False), EXAMPLES)) == [
            lookup_word(lookup, word, False) for word in EXAMPLES]
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        for word, base, kind in random.Random(49129).sample(admissions, 250):
            final, nonfinal = candidate(base, kind, False), candidate(base, kind, True)
            assert reference_tags(reference, word) == (final, nonfinal, nonfinal, final), word
    with DirectFirstPersian(args.library, None, None, None) as frontend:
        for word in EXAMPLES:
            analysis = frontend.analyze_direct(word)
            assert analysis and not analysis.used_fallback
            assert analysis.source_map == (0,) * len(word.encode('utf8')) + (len(analysis.parts[0].text),)
        assert frontend._worker is None
    compare_playback(args)
    print(f'Passed {len(admissions)} compound admissions, two word contexts, source maps and 250 reference comparisons.')


if __name__ == '__main__':
    main()
