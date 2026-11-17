"""Check every generated Persian compound admission against native lookup."""
from __future__ import annotations

import argparse
from pathlib import Path
import re

from direct_fa_compound_rules import SUFFIXES, candidate
from hybrid_fa_frontend import DirectFirstPersian
from test_direct_frequency_words import entries


def masks():
    text = Path(__file__).with_name('direct_fa_frequency_compounds.inc').read_text(encoding='utf8')
    array = text.split('fa_frequency_compound_masks[] = {', 1)[1].split('};', 1)[0]
    return [int(value, 16) for value in re.findall(r'0x([0-9a-f]{2})', array)]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', required=True, type=Path)
    parser.add_argument('--previous-library', required=True, type=Path)
    args = parser.parse_args()
    rows, bits = list(entries()), masks()
    assert len(rows) == len(bits) and all(mask < 1 << len(SUFFIXES) for mask in bits)
    checked = 0
    with DirectFirstPersian(args.native_library, None, None, None) as current, \
         DirectFirstPersian(args.previous_library, None, None, None) as previous:
        for (stem, base, nonfinal_base), mask in zip(rows, bits):
            if not mask:
                continue
            assert base == nonfinal_base and '\u200c' not in stem
            for kind, suffix in enumerate(SUFFIXES):
                if not mask & (1 << kind):
                    continue
                word = stem + suffix
                for follows in (False, True):
                    text = word + (' سلام' if follows else '')
                    newer, older = current.analyze_direct(text), previous.analyze_direct(text)
                    assert newer and newer.source_map is not None, text
                    end = len(word.encode('utf8'))
                    actual = newer.parts[0].text[:newer.source_map[end]]
                    if older is not None:
                        old = older.parts[0].text[:older.source_map[end]]
                        assert actual == old, (word, follows, actual, old)
                    else:
                        expected = candidate(base, kind, follows)
                        assert actual == expected, (word, follows, actual, expected)
                checked += 1
        assert current._worker is None and previous._worker is None
    assert checked >= 1000
    print(f'Passed {checked} native compound combinations in final and nonfinal positions.')


if __name__ == '__main__':
    main()
