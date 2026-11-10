"""Preserve Eloquence's actual all-capitals spelling while splitting other words."""
import argparse
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, EciCapitalWord, EciIndex, EciText
from _reloq_languages import LanguageEngine
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library.resolve()
    for language in (0x10000, 0x10001):
        direct = DirectEciEngine(library, dialect=language, encoding='utf8', native_voice=True)
        try:
            before = render(direct, ['Hello world.'])[0]
            for word, expected in (('CGROUP', True), ('FDOPEN', True), ('USRDCT', True),
                                   ('TMPFILES', False), ('GETHOSTBYADDR', False)):
                assert direct.spells_uppercase(word) is expected, (language, word)
            assert render(direct, ['Hello world.'])[0] == before, 'probe changed later audio'
        finally:
            direct.close()
        engine = LanguageEngine(library, language=language, voice='1')
        try:
            for original, segmented, chosen in (('CGROUP', 'C group', 'CGROUP'),
                                                ('FDOPEN', 'F D open', 'FDOPEN'),
                                                ('TMPFILES', 'T M P files', 'T M P files'),
                                                ('GETHOSTBYADDR', 'get host by addr', 'get host by addr')):
                candidate = [EciText('Before '), EciCapitalWord(original, segmented), EciText(' after'), EciIndex(7)]
                expected = [EciText('Before '), EciText(chosen), EciText(' after'), EciIndex(7)]
                assert render(engine, candidate) == render(engine, expected), (language, original)
            cached = len(engine._uppercase_cache)
            render(engine, [EciCapitalWord('CGROUP', 'C group')])
            assert len(engine._uppercase_cache) == cached
            engine._uppercase_cache.clear()
            direct = engine._engines[language]
            original_probe = direct.spells_uppercase
            direct.spells_uppercase = lambda word: None
            try:
                assert render(engine, [EciCapitalWord('TMPFILES', 'T M P files')]) == render(engine, [EciText('TMPFILES')])
            finally:
                direct.spells_uppercase = original_probe
        finally:
            engine.close()
    print('Capital words: native spelling, segmentation, callback restoration, cache and fallback passed')


if __name__ == '__main__':
    main()
