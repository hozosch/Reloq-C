"""Check native frequency-table entries against their reference contexts."""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

from compile_direct_lexicon import reference_words
from espeak_ng_probe import EspeakPhonemeProbe
from hybrid_fa_frontend import DirectFirstPersian
from text_frontend import word_tag


def entries():
    path = Path(__file__).with_name('direct_fa_frequency_words.inc')
    for line in path.read_text(encoding='utf8').splitlines():
        value = line.strip()
        if value.startswith('{') and value.endswith('},'):
            yield tuple(json.loads('[' + value[1:-2] + ']'))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('native-library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=True)
    args = parser.parse_args()
    rows = tuple(entries())
    keys = [word.encode('utf8') for word, _, _ in rows]
    assert len(rows) >= 1000 and keys == sorted(set(keys))
    with DirectFirstPersian(args.native_library, None, None, None) as frontend, \
         EspeakPhonemeProbe(args.espeak_library, args.data_parent) as reference:
        for number, (word, final, nonfinal) in enumerate(rows, 1):
            expected = (final, nonfinal, nonfinal, final)
            assert tuple(word_tag(raw) for raw in reference_words(reference, word)) == expected, word
            for prefix, suffix, tag in (('', '', final), ('', ' آمد.', nonfinal),
                                        ('من ', ' آمد.', nonfinal), ('من ', '.', final)):
                text = prefix + word + suffix
                actual = frontend.analyze_direct(text)
                assert actual and not actual.used_fallback and actual.source_map is not None, text
                start = len(prefix.encode('utf8'))
                end = start + len(word.encode('utf8'))
                at = actual.source_map[start]
                phones = lambda value: re.sub(r'\.[012]', '.#', value)
                assert phones(actual.parts[0].text[at:at + len(tag)]) == phones(tag), (word, text, actual.parts)
                assert all(position == at for position in actual.source_map[start:end]), text
            if number % 5000 == 0:
                print(f'Checked {number}/{len(rows)} words', flush=True)
        assert frontend._worker is None, 'frequency words unexpectedly started eSpeak'
    print(f'Passed {len(rows)} direct words in four reference contexts.')


if __name__ == '__main__':
    main()
