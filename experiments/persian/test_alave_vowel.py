"""Verify the short front vowel in alāve across Persian input paths."""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex, EciText
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
        expected = {
            'علاوه': "`[.0A.1la.0v'e:']",
            'علاوه‌ی': "`[.0A.1la.0ve.1j'e:']",
            'بعلاوه': "`[.0be.0A.1la.0v'e:']",
            'بعلاوه‌ی': "`[.0be.0A.1la.0ve.1j'e:']",
            'به علاوه': "`[.0be] `[.0A.1la.0v'e:']",
        }
        for word, tag in expected.items():
            analysis = frontend.analyze_direct(word)
            assert analysis and phones(analysis.parts[0].text) == phones(tag), (word, analysis)
        assert word_tag("a:|l|'A|v|e:") == expected['علاوه']
        assert word_tag("b|e|a:|l|'A|v|e:") == expected['بعلاوه']
        # Existing symbol names retain their own preposition pronunciation;
        # both paths must still preserve the intended vowel in alāve.
        symbol = frontend.analyze_character('+')
        assert symbol and phones(symbol.parts[0].text) == phones(SYMBOL_TAGS['+'])
        assert phones(SYMBOL_TAGS['+']).endswith(phones(expected['علاوه']))

        for part in ('به علاوه', 'بعلاوه', EciText('+', character_mode=True)):
            pcm, marks = bytearray(), []
            assert engine.speak((EciIndex(1), part, EciIndex(2)), pcm.extend,
                                lambda value: marks.append((value, len(pcm))))
            assert pcm and marks == [(1, 0), (2, len(pcm))], (part, marks)
        assert frontend._worker is None, 'the common words must use the direct text path'
    finally:
        engine.close()
    print('Passed alāve vowel, direct spellings, plus symbol and source-index checks.')


if __name__ == '__main__':
    main()
