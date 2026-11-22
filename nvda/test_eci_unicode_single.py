"""Single UTF-8 characters and text context through the common ECI API."""
import argparse
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciText
from _reloq_languages import LANGUAGES
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    checks = 0
    for language in LANGUAGES.values():
        engine = DirectEciEngine(args.library.resolve(), dialect=language.id,
                                 encoding='utf8', native_voice=True)
        try:
            for char in ('⚭', 'Ж', '≪', 'α', '🙂', '€', '×', '→', '∑', '\u00a0', '\u200b',
                         '@', '+', '?', 'a', '1', ' ', '\t', '\n', '\r', '\x7f'):
                source = [EciIndex(11), char, EciIndex(12)]
                explicit = [EciIndex(11), EciText(char, character_mode=True), EciIndex(12)]
                actual = render(engine, source)
                assert actual[0] and actual == render(engine, explicit), (language.locale, char)
                assert [value for value, _ in actual[1]] == [11, 12]
                checks += 1
            protected = render(engine, [EciText('⚭', segmentation_guarded=True), EciIndex(15)])
            empty = render(engine, [EciText(' ', segmentation_guarded=True), EciIndex(15)])
            assert protected == empty, (language.locale, 'dictionary-protected text')
            checks += 1
            continued = render(engine, [EciText('⚭', source_in_context=True), EciIndex(15)])
            assert continued == protected, (language.locale, 'single run from longer source')
            spelled = [EciText('⚭', character_mode=True), EciIndex(15)]
            assert render(engine, [EciText('⚭', character_mode=True, source_in_context=True), EciIndex(15)]) == render(engine, spelled), (
                language.locale, 'explicit spelling in longer source')
            assert render(engine, [EciText('🙂', source_in_context=True), EciIndex(15)]) != empty, (
                language.locale, 'emoji in longer source')
            checks += 3
            reference = render(engine, ['Test ', EciIndex(13), ' ', EciIndex(14), ' Ende'])
            for char in (('⚭', 'α') if language.id != 0x120000 else ('⚭',)):
                assert render(engine, ['Test ', EciIndex(13), char, EciIndex(14), ' Ende']) == reference, (
                    language.locale, char, 'running text')
                checks += 1
            emoji = render(engine, ['Test ', EciIndex(13), '🙂', EciIndex(14), ' Ende'])
            assert emoji != reference, (language.locale, 'emoji in running text')
            checks += 1
        finally:
            engine.close()
    print(f'Passed {checks} UTF-8 isolated, running-text, emoji and source-index checks.')


if __name__ == '__main__':
    main()
