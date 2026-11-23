"""Check productive suffixes on the older Persian word inventories."""
from __future__ import annotations

import argparse
import json
from pathlib import Path

from hybrid_fa_frontend import DirectFirstPersian
from test_classified_recovery import phones
from test_direct_frequency_words import entries as frequency_entries

SUFFIXES = ('ها', 'های', 'هایی', 'تر', 'ترین', 'هایم', 'هایت', 'هایش')


def main_entries():
    source = Path(__file__).with_name('direct_fa_lexicon.inc').read_text(encoding='utf8')
    for line in source.splitlines():
        value = line.strip()
        if value.startswith('{') and value.endswith('},'):
            yield tuple(json.loads('[' + value[1:-2] + ']'))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--exhaustive', action='store_true')
    args = parser.parse_args()
    # Different established stems exercise the same suffix rule, including
    # a final written heh and a word explicitly reported by a listener.
    cases = {'گلر': 'goler', 'مدرسه': 'mAdrese', 'بچه': 'bACCe',
             'خانه': 'Gane', 'آبجو': 'abJo'}
    checks = 0
    with DirectFirstPersian(args.library, None, None, None) as current, \
         DirectFirstPersian(args.previous, None, None, None) as old:
        for stem, base in cases.items():
            word = stem + 'های'
            assert old.analyze_direct(word) is None, word
            result = current.analyze_direct(word)
            assert result and len(result.parts) == 1, (word, result)
            assert phones(result.parts[0].text) == base + 'haje', (word, result)
            assert '.1ha.0je]' in result.parts[0].text, (word, result)
            for text in (word + ' آمد.', 'من ' + word + ' آمد.'):
                phrase = current.analyze_direct(text)
                assert phrase and base + 'haje' in phones(phrase.parts[0].text), (text, phrase)
            joined = current.analyze_direct(stem + '\u200cهای')
            assert joined and phones(joined.parts[0].text) == base + 'haje', (stem, joined)
            checks += 1
        # The simple plural was already correct, and explicit suffixes with
        # pre-existing readings still win over the generalized rule.
        for word in ('گلرها', 'گلرهایش', 'گلرهایشان', 'کشتیهای', 'نمی‌آراست'):
            assert current.analyze_direct(word) == old.analyze_direct(word), word
            assert current.project_fallback_word(word) == old.project_fallback_word(word), word
        if args.exhaustive:
            seen = set()
            for row in (*main_entries(), *frequency_entries()):
                stem = row[0]
                for suffix in SUFFIXES:
                    word = stem + suffix
                    if word in seen:
                        continue
                    seen.add(word)
                    previous = old.analyze_direct(word)
                    if previous is not None:
                        assert current.analyze_direct(word) == previous, word
                        checks += 1
            assert checks > 90000, checks
    print(f'Passed reported plural, other stems and {checks} existing analyses.')


if __name__ == '__main__':
    main()
