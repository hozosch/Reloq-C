"""Exercise packaged Persian Unicode names through NVDA's actual speech code."""
from __future__ import annotations

import argparse
from contextlib import nullcontext
import ctypes
from pathlib import Path
import re
import sys
import threading
from unittest import mock

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'addon'))
from nvda_speech_test_api import speech_api
from test_synth import main as test_package


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--addon', type=Path, required=True)
    args = parser.parse_args()
    library, addon = args.library.resolve(), args.addon.resolve()
    original_sizeof = ctypes.sizeof
    original_windll = getattr(ctypes, 'WinDLL', None)
    checks = []

    def inspect(module):
        probe = sys.modules[module.__name__ + '._persian.nvda_driver_probe']
        sequence = sys.modules[module.__name__ + '._persian.direct_fa_sequence']
        symbols = sys.modules[module.__name__ + '._persian.persian_symbols'].SYMBOL_TAGS
        done, players, errors, received, projected, native_text = threading.Event(), [], [], [], [], []
        original_prepared = sequence.ScopedDirectPersianEngine._speak_prepared
        original_native = probe.DirectEciEngine.speak

        def observe_prepared(instance, prepared, *args, **kwargs):
            projected.append((prepared.source, prepared.tagged))
            return original_prepared(instance, prepared, *args, **kwargs)

        def observe_native(instance, parts, *args, **kwargs):
            native_text.extend(p if isinstance(p, str) else p.text
                               for p in parts if isinstance(p, str) or hasattr(p, 'text'))
            return original_native(instance, parts, *args, **kwargs)

        class Player:
            def __init__(self, hz):
                self.buffers = []
                players.append(self)
            def feed(self, pcm, onDone=None):
                self.buffers.append(pcm)
                if onDone:
                    onDone()
            def idle(self): pass
            def pause(self, switch): pass
            def stop(self): pass
            def close(self): pass

        class Driver(module.SynthDriver):
            @staticmethod
            def _make_player(hz): return Player(hz)
            def _on_done(self, job): done.set()
            def _on_error(self, error): errors.append(error); done.set()

        module.add_profiles = lambda languages, _: {
            **languages, probe.PERSIAN_PROBE.id: probe.PERSIAN_PROBE,
            probe.PINGLISH_PROBE.id: probe.PINGLISH_PROBE}
        module.make_engine = lambda _, *, languages, language, voice: probe.make_language_engine(
            library, library, None, Path('/unavailable-espeak'), Path('/unavailable'),
            languages=languages, language=language, voice=voice)
        sequence.ScopedDirectPersianEngine._speak_prepared = observe_prepared
        probe.DirectEciEngine.speak = observe_native
        patches = [mock.patch.object(ctypes, 'sizeof', original_sizeof)]
        if original_windll is not None:
            patches.append(mock.patch.object(ctypes, 'WinDLL', original_windll))
        with patches[0], (patches[1] if len(patches) > 1 else nullcontext()):
            driver = Driver()
            with mock.patch.object(sys.modules['synthDriverHandler'], 'getSynth', return_value=driver):
                api = speech_api(driver, lambda value: (received.append(value), driver.speak(value)))
                speech_config = sys.modules['config'].conf['speech']
                try:
                    spelling_config = speech_config[driver.name]
                except KeyError:
                    speech_config[driver.name] = {}
                    spelling_config = speech_config[driver.name]
                for key, value in dict(useSpellingFunctionality=True, capPitchChange=0,
                                       sayCapForCapitals=False, beepForCapitals=False).items():
                    spelling_config[key] = value
                dictionary = api.speak.__globals__['speechDictHandler']

                def run(items, *, level=0, direct=False):
                    done.clear(); projected.clear(); native_text.clear()
                    before = {id(player): len(player.buffers) for player in players}
                    if direct:
                        driver.speak(items)
                    else:
                        api.speak(items, symbolLevel=level)
                    assert done.wait(30), (items, errors)
                    if errors:
                        raise errors[-1]
                    audio = b''.join(chunk for player in players
                                     for chunk in player.buffers[before.get(id(player), 0):])
                    assert audio, (items, received[-1], projected, native_text)
                    return audio, tuple(projected), tuple(native_text)

                def spelling(char, locale, enabled):
                    spelling_config['useSpellingFunctionality'] = enabled
                    return list(api.getSpellingSpeech(char, locale=locale, endsUtterance=False))

                try:
                    driver.sampleRate = '3'
                    with mock.patch.dict(sys.modules, {'speechDictHandler': dictionary,
                                                       'characterProcessing': api.symbols}):
                        for locale in ('fa_IR', 'fa_Latn'):
                            driver.language = locale
                            character_mode = sys.modules['speech.commands'].CharacterModeCommand
                            index = sys.modules['speech.commands'].IndexCommand
                            if locale == 'fa_IR':
                                for text, level in (('سلام:دنیا', api.symbols.SymbolLevel.MOST),
                                                    ('سلام دونقطه. دنیا', api.symbols.SymbolLevel.NONE)):
                                    _, projected_colon, _ = run([text], level=level)
                                    tags = ' '.join(tag for _, tag in projected_colon)
                                    assert '`[.0do.1noR.0te]' in tags, (text, tags)
                                    checks.append((locale, 'connected colon name', text))
                            single_character_audio = {}
                            for char in ('⚭', 'Ж', 'α', '⚽', '≪', '«', '»', '€', '×', '→', '∑', '\u00a0', '\u200b',
                                         '\u202a', '\u202b', '\u202c', '\u202d', '\u202e', '✓', '➔',
                                         '@', '+', '?', ':', 'a', '1', ' ', '\t', '\n', '\r', '\x7f'):
                                try:
                                    expected, _, _ = run([character_mode(True), module.NativeDictionaryWord(char),
                                                          character_mode(False)])
                                    single_character_audio[char] = expected
                                except BaseException as error:
                                    raise AssertionError((locale, repr(char), 'explicit character')) from error
                                for enabled in (False, True):
                                    spelling_config['useSpellingFunctionality'] = enabled
                                    actual, _, _ = run([index(71), char, index(72)])
                                    assert actual == expected, (locale, char, enabled)
                                    checks.append((locale, 'plain single character', char, enabled))
                            # With NVDA leaving a raw symbol untouched,
                            # even a recognized Persian name belongs only to
                            # an isolated character, not a sentence fragment.
                            before, after = ('سلام ', ' دنیا') if locale == 'fa_IR' else ('salam ', ' donya')
                            with mock.patch.object(api.symbols, 'processSpeechSymbols',
                                                   side_effect=lambda loc, text, level: text):
                                reference, _, _ = run([before, index(73), ' ', index(74), after])
                                for char in ('⚭', 'α', '≪', '\u202e', '✓', '➔'):
                                    actual, _, _ = run([before, index(73), char, index(74), after])
                                    assert actual == reference, (locale, char, 'running text')
                                    checks.append((locale, 'raw running text', char))
                                for char in ('⚽', '🙂', '🌍'):
                                    actual, _, _ = run([before, index(73), char, index(74), after])
                                    assert actual != reference, (locale, char, 'emoji in running text')
                                    checks.append((locale, 'emoji in running text', char))
                                # Dashes still separate languages and affect
                                # timing; do not turn them into spaces when
                                # suppressing their Unicode names in prose.
                                middle = 'NVDA' if locale == 'fa_IR' else 'phone'
                                left = 'سلام' if locale == 'fa_IR' else 'salam'
                                right = 'خوب' if locale == 'fa_IR' else 'khub'
                                expected, _, _ = run([left, index(75), '-', middle,
                                                      index(76), '-', right])
                                for dash in ('‐', '–', '—'):
                                    actual, _, _ = run([left, index(75), dash, middle,
                                                        index(76), dash, right])
                                    assert actual == expected, (locale, dash, 'running dash')
                                    checks.append((locale, 'running dash', dash))
                                quoted_word = 'دنیا' if locale == 'fa_IR' else 'donya'
                                quoted, _, _ = run([left + ' «', index(77), quoted_word,
                                                    index(78), '» ' + right])
                                bracketed, _, _ = run([left + ' (', index(77), quoted_word,
                                                       index(78), ') ' + right])
                                assert quoted == bracketed, (locale, 'running quotes')
                                checks.append((locale, 'running quotes'))
                            for char in ('⚽', '🙂', 'α', 'Ж', '⚭', '≪', '\x7f', '\u00a0', '\u200b',
                                         '\u202a', '\u202b', '\u202c', '\u202d', '\u202e', '✓', '➔'):
                                on, projects, native = run(spelling(char, locale, True))
                                off, _, _ = run(spelling(char, locale, False))
                                assert on == off, (locale, char, len(on), len(off))
                                if char in ('⚽', '🙂', 'α'):
                                    expected = re.sub(r"'([Aaeiou]):'", r"\1", symbols[char])
                                    assert expected in ' '.join(tag for _, tag in projects), (locale, char, projects)
                                if char in ('Ж', '⚭'):
                                    expected = {'Ж': 'cyrillic capital letter zhe',
                                                '⚭': 'marriage symbol'}[char]
                                    assert expected in ' '.join(native).lower(), (locale, char, native)
                                if char == '\x7f':
                                    assert '`[.1hAzf]' in ' '.join(tag for _, tag in projects), (locale, char, projects)
                                    assert 'delete' not in ' '.join(native).lower(), (locale, char, native)
                                if char in ('\u202a', '\u202b', '\u202c', '\u202d', '\u202e', '✓', '➔'):
                                    expected = re.sub(r"'([Aaeiou]):'", r"\1", symbols[char])
                                    assert expected in ' '.join(tag for _, tag in projects), (locale, char, projects)
                                    assert on == single_character_audio[char], (locale, char, 'NVDA versus direct name')
                                    english = {'\u202a': 'left to right embedding',
                                               '\u202b': 'right to left embedding',
                                               '\u202c': 'pop directional formatting',
                                               '\u202d': 'left to right override',
                                               '\u202e': 'right to left override',
                                               '✓': 'check mark',
                                               '➔': 'heavy wide headed rightwards arrow'}[char]
                                    assert english not in ' '.join(native).lower(), (locale, char, native)
                                checks.append((locale, char))

                            # The same unlisted Unicode character in a running
                            # sentence must not inject an English Unicode name.
                            text = 'سلام ⚭ دنیا' if locale == 'fa_IR' else 'salam ⚭ donya'
                            audio, _, native = run([text])
                            assert 'marriage symbol' not in ' '.join(native).lower(), (locale, native)
                            without_name, _, _ = run([text.replace('⚭', ' ')])
                            assert audio == without_name, locale
                            checks.append((locale, 'running text'))

                            # Indexes do not turn a single fragment inside a
                            # larger sentence into a character-navigation job.
                            index = sys.modules['speech.commands'].IndexCommand
                            before, after = ('سلام ', ' دنیا') if locale == 'fa_IR' else ('salam ', ' donya')
                            _, _, native = run([before, index(17), '⚭', index(18), after])
                            assert 'marriage symbol' not in ' '.join(native).lower(), (locale, native)
                            checks.append((locale, 'indexed running text'))

                            # NVDA may put just a symbol between language
                            # commands. Its new ECI handle must receive the
                            # context of the complete original speech request.
                            language = sys.modules['speech.commands'].LangChangeCommand
                            before, after = ('سلام ', ' دنیا') if locale == 'fa_IR' else ('salam ', ' donya')
                            def switched(char, *, explicit=False):
                                marker = ([character_mode(True), char, character_mode(False)]
                                          if explicit else [char])
                                return [before, index(31), language('en_US'), *marker, index(32),
                                        language(locale), after]
                            with mock.patch.object(api.symbols, 'processSpeechSymbols',
                                                   side_effect=lambda loc, text, level: text):
                                reference, _, _ = run(switched(' '))
                                for char in ('⚭', 'Ж', 'α', '\u200b'):
                                    actual, _, _ = run(switched(char))
                                    assert actual == reference, (locale, char, 'language boundary')
                                    spelled, _, _ = run(switched(char, explicit=True))
                                    assert spelled != reference, (locale, char, 'explicit character mode')
                                    checks.append((locale, 'language boundary', char))
                                emoji, _, _ = run(switched('🙂'))
                                assert emoji != reference, (locale, 'emoji at language boundary')
                                checks.append((locale, 'emoji at language boundary'))
                            # Exercise the same command sequence after NVDA's
                            # preprocessing as well, as the real synth sees it.
                            direct_reference, _, _ = run(switched(' '), direct=True)
                            direct_symbol, _, _ = run(switched('⚭'), direct=True)
                            direct_emoji, _, _ = run(switched('🙂'), direct=True)
                            assert direct_symbol == direct_reference and direct_emoji != direct_reference, locale
                            single, _, _ = run([language('en_US'), '⚭'], direct=True)
                            spelled_single, _, _ = run([language('en_US'), character_mode(True),
                                                        module.NativeDictionaryWord('⚭'),
                                                        character_mode(False)], direct=True)
                            assert single == spelled_single, (locale, 'isolated switched character')
                            checks.append((locale, 'direct language boundary'))

                            # Keep the user's explicit NVDA dictionary mapping.
                            original_dict = dictionary.processText
                            dictionary.processText = lambda text: 'واژه' if text == '⚭' else text
                            try:
                                actual, _, _ = run(spelling('⚭', locale, False))
                                expected, _, _ = run(['واژه'])
                                assert actual == expected, locale
                                actual, _, _ = run(['⚭'])
                                assert actual == expected, (locale, 'plain character dictionary')
                            finally:
                                dictionary.processText = original_dict
                            checks.append((locale, 'speech dictionary'))

                            # NVDA can suppress even a character-mode symbol
                            # during its symbol-level preprocessing.
                            spelling_config['useSpellingFunctionality'] = True
                            expected, _, _ = run(spelling('⚭', locale, True))
                            original_symbols = api.symbols.processSpeechSymbols
                            def hide_symbol(loc, text, level):
                                return '' if text == '⚭' else original_symbols(loc, text, level)
                            with mock.patch.object(api.symbols, 'processSpeechSymbols', side_effect=hide_symbol):
                                for enabled in (True, False):
                                    actual, _, _ = run(spelling('⚭', locale, enabled))
                                    assert actual == expected, (locale, enabled)
                                    checks.append((locale, 'suppressed spelling', enabled))
                finally:
                    driver.terminate()
                    sequence.ScopedDirectPersianEngine._speak_prepared = original_prepared
                    probe.DirectEciEngine.speak = original_native

    test_package(inspect, inspect_only=True, package_path=addon, persian_package=True)
    print(f'Passed {len(checks)} packaged Persian/Pinglish NVDA Unicode checks.')


if __name__ == '__main__':
    main()
