"""Check every direct Persian internal-pause word against eSpeak context."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from espeak_worker import EspeakWorker
from hybrid_fa_frontend import DirectFirstPersian
from text_frontend import word_tag


def entries():
    rows = []
    path = Path(__file__).with_name('direct_fa_pause_words.inc')
    for line in path.read_text(encoding='utf8').splitlines():
        item = line.strip()
        if item.startswith('{') and item.endswith('},'):
            rows.append(tuple(json.loads('[' + item[1:-2] + ']')))
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    rows = entries()
    assert len(rows) >= 50
    assert [word.encode('utf8') for word, _ in rows] == sorted({word.encode('utf8') for word, _ in rows})
    with DirectFirstPersian(args.native_library, args.worker, args.espeak_library, args.data_parent) as frontend:
        with EspeakWorker(args.worker, args.espeak_library, args.data_parent) as reference:
            for word, tag in rows:
                assert ',' in tag and not tag.startswith(',') and not tag.endswith(',')
                for prefix, suffix, index, count in (
                        ('', '', 0, 1), ('', ' آمد.', 0, 2),
                        ('من ', ' آمد.', 1, 3), ('من ', '.', 1, 2)):
                    text = prefix + word + suffix
                    clauses = reference.translate(text, separator='|')
                    words = [token for clause in clauses for token in clause.phonemes.split()]
                    assert len(words) == count and word_tag(words[index]) == tag, (word, text)
                    actual = frontend.analyze_direct(text)
                    assert actual and not actual.used_fallback and actual.source_map is not None, text
                    start = len(prefix.encode('utf8'))
                    end = start + len(word.encode('utf8'))
                    at = actual.source_map[start]
                    assert actual.parts[0].text[at:at + len(tag)] == tag, text
                    assert all(value == at for value in actual.source_map[start:end]), text
            assert frontend._worker is None, 'a direct pause word started the reference helper'
    print(f'Passed {len(rows)} direct pause words in four reference contexts and position maps.')


if __name__ == '__main__':
    main()
