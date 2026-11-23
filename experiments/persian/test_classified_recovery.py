"""Check recovered Persian words and preserve previously native analyses."""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from compile_direct_classified_words import inventory
from hybrid_fa_frontend import DirectFirstPersian


def phones(tag):
    # The native Persian duration rule deliberately removes eSpeak's vowel
    # length markup while retaining vowel identity.
    tag = re.sub(r"'([Aaeiou]):'", r'\1', tag)
    return re.sub(r'\.[012]|[`\[\]~ ]', '', tag)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    args = parser.parse_args()
    examples = {
        'سرمربی': '`[.0sAr.0mo.1rAb.0bi]',
        'سرمربیان': '`[.0sAr.2mo.0rAb.1bi.0jan]',
        'پرسابقه': "`[.0por.1sa.0be.2R'e:']",
        'رحمتی': '`[.0rAh.1mA.0ti]',
        'باقری': '`[.1ba.0Re.2ri]',
        'پزشکیان': '`[.0pe.0zeS.0ki.1an]',
        'بیرانوند': '`[.0bej.0ran.1vAnd]',
    }
    with DirectFirstPersian(args.library, None, None, None) as current, \
         DirectFirstPersian(args.previous, None, None, None) as old:
        for word, reference in examples.items():
            analysis = current.analyze_direct(word)
            assert analysis and len(analysis.parts) == 1, (word, analysis)
            assert phones(analysis.parts[0].text) == phones(reference), (word, analysis, reference)
            for phrase in (f'{word} آمد.', f'من {word} آمد.'):
                result = current.analyze_direct(phrase)
                assert result and phones(reference) in phones(result.parts[0].text), (word, phrase, result)
        # A whole-word recovery must never swallow the established
        # half-space/prefix fallback route.
        for word in ('نمی‌آراست',):
            assert old.analyze_direct(word) is current.analyze_direct(word) is None
            assert old.project_fallback_word(word) == current.project_fallback_word(word), word
        checked = 0
        for word in inventory():
            previous = old.analyze_direct(word)
            if previous is None:
                continue
            current_result = current.analyze_direct(word)
            assert current_result == previous, (word, previous, current_result)
            checked += 1
        assert checked > 30000, checked
    print(f'Passed 7 reported words in context and {checked} existing native word analyses.')


if __name__ == '__main__':
    main()
