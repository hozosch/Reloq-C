"""Check the short initial front vowel in alāmat and its NVDA forms."""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex
from nvda_driver_probe import PERSIAN_PROBE, make_language_engine
from persian_symbols import SYMBOL_TAGS
from text_frontend import word_tag


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=option != 'worker')
    args = parser.parse_args()
    engine = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                  args.espeak_library, args.data_parent)
    try:
        frontend = engine._engines[PERSIAN_PROBE.id]._frontend
        phones = lambda value: re.sub(r'\.[012]', '.#', re.sub(r'`00\s*', '', value))
        for word in ('علامت', 'علامتِ', 'علامتهای', 'علامت‌دار', 'علامت‌زدنی', 'علامت‌گذاری'):
            analysis = frontend.analyze_direct(word)
            assert analysis and phones(analysis.parts[0].text).startswith('`[.#A.#la.#mA'), (word, analysis)
        assert word_tag("a:|l|'A|m|a|t") == '`[.0A.1la.0mAt]'
        assert word_tag("a:|l|'A|m|a|t|d|'A|R") == '`[.0A.1la.0mAt.1dar]'
        # Reference phoneme parsing remains intact; the number grammar fixes
        # the vowel of صد separately without changing arbitrary long /æ/.
        assert word_tag("s|,a:|d|o") == "`[.2s'A:'.0do]"
        assert word_tag("'i:|R|A|n") == "`[.1'i:'.0r'a:'n]"
        for symbol in ('!', '?', '@', '^', '`', '؟'):
            assert phones(SYMBOL_TAGS[symbol]).startswith('`[.#A.#la.#mAt]'), symbol

        def speak(*items):
            pcm, marks = bytearray(), []
            assert engine.speak(items, pcm.extend, lambda value: marks.append((value, len(pcm))))
            return bytes(pcm), marks
        whole, marks = speak(EciIndex(1), 'بدونِ علامت', EciIndex(2))
        assert whole and marks == [(1, 0), (2, len(whole))]
        joined, _ = speak('کادرِ علامت‌زدنی علامت‌دار')
        split, _ = speak('کادرِ علامت‌زدنی علامت دار')
        assert joined == split
        assert frontend._worker is None, 'the covered UI forms must use the direct path'
    finally:
        engine.close()
    print('Passed alāmat front-vowel, spelling symbols, direct forms and source-index checks.')


if __name__ == '__main__':
    main()
