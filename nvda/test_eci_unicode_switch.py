"""Unicode context must survive a real per-language ECI synthesis boundary."""
import argparse
from pathlib import Path

from _reloq_eci_direct import EciIndex, EciLanguage, EciText
from _reloq_languages import GERMAN, LANGUAGES, LanguageEngine
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    engine = LanguageEngine(args.library.resolve(), language=GERMAN.id)
    checks = 0
    try:
        native = engine._engines[GERMAN.id]
        assert native._dll.eciGetParam(native._handle, 0x101e) & 4
        assert not native._dll.eciAddTextUtf8(native._handle, b'X', 1, 8)
        for language in LANGUAGES.values():
            to = EciLanguage(language.id, '1')
            back = EciLanguage(GERMAN.id, '1')
            for char in ('⚭', 'α', 'Ж', '×', '\u200b', ':', 'a', '1', ' ', '🙂'):
                original = ['Test ', EciIndex(10), to, char, EciIndex(11), back, ' Ende']
                ordinary = ['Test ', EciIndex(10), to,
                            EciText(char, segmentation_guarded=True), EciIndex(11), back, ' Ende']
                assert render(engine, original) == render(engine, ordinary), (language.locale, char)
                checks += 1
            for char in ('⚭', 'α', '🙂'):
                implicit = [to, EciIndex(20), char, EciIndex(21)]
                explicit = [to, EciIndex(20), EciText(char, character_mode=True), EciIndex(21)]
                assert render(engine, implicit) == render(engine, explicit), (language.locale, char)
                checks += 1
                spelled = ['Test ', EciIndex(22), to,
                           EciText(char, character_mode=True), EciIndex(23), back, ' Ende']
                assert render(engine, spelled) != render(engine, [
                    'Test ', EciIndex(22), to,
                    EciText(char, segmentation_guarded=True), EciIndex(23), back, ' Ende'
                ]) or char in ('🙂', 'α'), (language.locale, char, 'explicit character mode')
                checks += 1
    finally:
        engine.close()
    print(f'Passed {checks} cross-language Unicode context and spelling checks.')


if __name__ == '__main__':
    main()
