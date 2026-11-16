"""Compare saved native vowel targets with events from another eSpeak build.

This allows the Linux diagnostic to use the actual eSpeak DLL shipped with
Windows NVDA. Only identical corpus, word, vowel, stress and rate records
are paired. The input native measurements were already screened against the
local reference; this intentionally uses their conservative intersection.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import statistics

KEYS = ('word', 'vowel_index', 'phone', 'stress', 'espeak_wpm')


def compare(native, reference):
    if not native.get('corpus_sha256') or native['corpus_sha256'] != reference.get('corpus_sha256'):
        raise ValueError('the native and eSpeak reports use different corpora')
    by_key = {}
    for row in reference['rows']:
        key = tuple(row[field] for field in KEYS)
        if key in by_key:
            raise ValueError('duplicate eSpeak vowel record')
        by_key[key] = row
    paired, excluded = [], []
    for row in native['rows']:
        key = tuple(row[field] for field in KEYS)
        found = by_key.get(key)
        if found is None:
            excluded.append(dict(zip(KEYS, key)))
            continue
        pair = dict(row)
        for field in ('espeak_alone_ms', 'espeak_before_word_ms', 'espeak_ratio'):
            pair[field] = found[field]
        paired.append(pair)
    groups = {}
    for row in paired:
        groups.setdefault((row['word'], row['vowel_index'], row['phone'], row['stress']), []).append(row)
    candidates = []
    for (word, index, phone, stress), group in groups.items():
        if len(group) != 3 or len({row['espeak_wpm'] for row in group}) != 3:
            continue
        gaps = [row['native_ratio'] - row['espeak_ratio'] for row in group]
        if min(gaps) > 0.15 or max(gaps) < -0.15:
            candidates.append({'word': word, 'vowel_index': index, 'phone': phone, 'stress': stress,
                               'median_ratio_gap': round(statistics.median(gaps), 4),
                               'ratio_gaps': [round(value, 4) for value in gaps]})
    candidates.sort(key=lambda row: abs(row['median_ratio_gap']), reverse=True)
    return {'method': 'Within-engine before-following-word / isolated-word ratios; '
                      'native targets and eSpeak events use different boundaries. '
                      'Native results are the conservative intersection with the local reference.',
            'native_screen_espeak_version': native['espeak_version'],
            'espeak_version': reference['espeak_version'], 'corpus_sha256': native['corpus_sha256'],
            'review_candidates': candidates, 'rows': paired, 'excluded': excluded}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native', type=Path, required=True)
    parser.add_argument('--espeak', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    result = compare(json.loads(args.native.read_text(encoding='utf8')),
                     json.loads(args.espeak.read_text(encoding='utf8')))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2) + '\n', encoding='utf8')
    print(f"eSpeak {result['espeak_version']}: {len(result['rows'])} matched vowel observations, "
          f"{len(result['excluded'])} excluded; {args.output}")


if __name__ == '__main__':
    main()
