"""Check recovered direct words against four eSpeak contexts."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from espeak_worker import EspeakWorker
from hybrid_fa_frontend import DirectFirstPersian
from text_frontend import word_tag


def entries():
    path = Path(__file__).with_name('direct_fa_recovered_words.inc')
    for line in path.read_text(encoding='utf8').splitlines():
        value = line.strip()
        if value.startswith('{') and value.endswith('},'):
            yield tuple(json.loads('[' + value[1:-2] + ']'))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=True)
    args = parser.parse_args()
    rows = tuple(entries())
    assert len(rows) >= 20
    assert [word.encode('utf8') for word, _, _ in rows] == sorted({word.encode('utf8') for word, _, _ in rows})
    with DirectFirstPersian(args.native_library, args.worker, args.espeak_library, args.data_parent) as frontend:
        with EspeakWorker(args.worker, args.espeak_library, args.data_parent) as reference:
            for word, final, nonfinal in rows:
                for prefix, suffix, index, count, expected in (
                        ('', '', 0, 1, final), ('', ' آمد.', 0, 2, nonfinal),
                        ('من ', ' آمد.', 1, 3, nonfinal), ('من ', '.', 1, 2, final)):
                    text = prefix + word + suffix
                    clauses = reference.translate(text, separator='|')
                    words = [token for clause in clauses for token in clause.phonemes.split()]
                    assert len(words) == count and word_tag(words[index]) == expected, (word, text, words)
                    actual = frontend.analyze_direct(text)
                    assert actual and not actual.used_fallback and actual.source_map is not None, text
                    start = len(prefix.encode('utf8'))
                    end = start + len(word.encode('utf8'))
                    at = actual.source_map[start]
                    assert actual.parts[0].text[at:at + len(expected)] == expected, (word, text)
                    assert all(position == at for position in actual.source_map[start:end]), text
            assert frontend._worker is None, 'recovered direct words started eSpeak'
    print(f'Passed {len(rows)} recovered direct words in four reference contexts.')


if __name__ == '__main__':
    main()
