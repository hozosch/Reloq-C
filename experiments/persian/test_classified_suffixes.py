"""Preserve recovered Persian stem vowels through productive suffixes."""
from __future__ import annotations

import argparse
from pathlib import Path

from hybrid_fa_frontend import DirectFirstPersian
from test_classified_recovery import phones


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    # These are different stems, not a list of complete derived word fixes.
    stems = {'سرمربی': 'sArmorAbbi', 'زهرابه': 'zAhrabe',
             'فروزانی': 'foruzani', 'رحمتی': 'rAhmAti'}
    endings = {'ها': 'ha', 'های': 'haje', 'هایی': 'hai',
               'تر': 'tAr', 'ترین': 'tArin'}
    checks = 0
    with DirectFirstPersian(args.library, None, None, None) as current, \
         DirectFirstPersian(args.previous, None, None, None) as old:
        for stem, base in stems.items():
            for written, ending in endings.items():
                for join in ('', '\u200c'):
                    word = stem + join + written
                    before = old.analyze_direct(word)
                    result = current.analyze_direct(word)
                    assert before is None, (word, before)
                    assert result and len(result.parts) == 1, (word, result)
                    assert phones(result.parts[0].text) == base + ending, (word, result, base + ending)
                    for phrase in (word + ' آمد.', 'من ' + word + ' آمد.'):
                        spoken = current.analyze_direct(phrase)
                        assert spoken and base + ending in phones(spoken.parts[0].text), (phrase, spoken)
                    if written in ('های', 'هایی'):
                        assert '.1ha.0' in result.parts[0].text, (word, result)
                    checks += 1
        # An explicit verbal half-space and already known words must not be
        # swallowed by the new suffix recovery.
        for word in ('نمی‌آراست', 'کتاب‌ها', 'کتابهای', 'می‌آزمودند'):
            assert current.analyze_direct(word) == old.analyze_direct(word), word
            assert current.project_fallback_word(word) == old.project_fallback_word(word), word
    print(f'Passed {checks} joined and unspaced stem/suffix forms and existing-word isolation.')


if __name__ == '__main__':
    main()
