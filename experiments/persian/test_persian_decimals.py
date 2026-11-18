"""Test English decimals before real NVDA Persian symbol processing."""
from __future__ import annotations

import ast
import codecs
import collections
from enum import IntEnum
import importlib
from pathlib import Path
import re
import sys
import threading
import types
from typing import Generic, TypeVar

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'addon'))
from nvda_speech_test_api import execute


class SynthCommand: pass
class CharacterModeCommand(SynthCommand):
    def __init__(self, state): self.state = state
class IndexCommand(SynthCommand):
    def __init__(self, index): self.index = index
class LangChangeCommand(SynthCommand):
    def __init__(self, lang): self.lang = lang


def main():
    speech = types.ModuleType('speech')
    speech.__path__ = []
    commands = types.ModuleType('speech.commands')
    for item in (SynthCommand, CharacterModeCommand, IndexCommand, LangChangeCommand):
        setattr(commands, item.__name__, item)
    speech.commands = commands
    sys.modules['speech'], sys.modules['speech.commands'] = speech, commands
    package = types.ModuleType('probe_driver')
    package.__path__ = [str(ROOT / 'nvda'), str(ROOT / 'addon/synth/synthDrivers/reloqC')]
    sys.modules['probe_driver'] = package
    persian = types.ModuleType('probe_driver._persian')
    persian.__path__ = [str(ROOT / 'experiments/persian')]
    sys.modules['probe_driver._persian'] = persian
    decimals = importlib.import_module('probe_driver._reloq_persian_decimals')
    native_word = importlib.import_module('probe_driver._speech_tags').NativeDictionaryWord

    ns = dict(IntEnum=IntEnum, re=re, codecs=codecs, collections=collections,
              Generic=Generic, TypeVar=TypeVar, _LocaleDataT=TypeVar('_LocaleDataT'),
              SYMPRES_NEVER=0, SYMPRES_ALWAYS=1, SYMPRES_NOREP=2,
              _getSpeechSymbolsForLocale=lambda locale: None,
              log=types.SimpleNamespace(warning=lambda *a, **k: None, error=lambda *a, **k: None))
    nvda = ROOT / 'addon/testdata/nvda'
    execute(nvda / 'characterProcessing.py', lambda node: isinstance(node, ast.ClassDef) and node.name in (
        'LocaleDataMap', 'SymbolLevel', 'SpeechSymbol', 'SpeechSymbols', 'SpeechSymbolProcessor'), ns)
    fa, en = ns['SpeechSymbols'](), ns['SpeechSymbols']()
    fa.load(str(ROOT / 'experiments/persian/reference-nvda/fa_symbols_2026_3_1.dic'))
    en.load(str(nvda / 'locale/en/symbols.dic'))
    sources = {'fa': (fa, ns['SpeechSymbols']()), 'en': (en, ns['SpeechSymbols']())}
    ns['SpeechSymbolProcessor'].localeSymbols = ns['LocaleDataMap'](lambda locale: sources[locale])
    processors = {locale: ns['SpeechSymbolProcessor'](locale) for locale in ('fa', 'en')}
    level = ns['SymbolLevel']
    sys.modules['characterProcessing'] = types.SimpleNamespace(
        SymbolLevel=level,
        processSpeechSymbols=lambda locale, text, kind: processors[locale[:2]].processText(text, kind))
    sys.modules['config'] = types.SimpleNamespace(conf={'speech': {'symbolLevel': level.NONE}})
    dictionaries = types.SimpleNamespace(dictionaries={'builtin': []})
    sys.modules['speechDictHandler'] = dictionaries
    fa_id, pinglish_id, en_id = 0x7f0000, 0x7f0001, 0x10000
    profiles = {fa_id: types.SimpleNamespace(id=fa_id, locale='fa_IR'),
                pinglish_id: types.SimpleNamespace(id=pinglish_id, locale='fa_Latn'),
                en_id: types.SimpleNamespace(id=en_id, locale='en_US')}
    driver = types.SimpleNamespace(_selectedLanguage=fa_id, _languages=profiles, _submit_lock=threading.Lock())
    assert processors['fa'].processText('3.5', level.NONE) != '3.5'
    assert processors['en'].processText('3.5', level.NONE) == '3.5'

    def protected(sequence):
        return decimals.protect_decimals(driver, sequence, symbol_level=level.NONE)
    def pieces(sequence):
        return [(type(item).__name__, item.text if isinstance(item, native_word) else item)
                for item in sequence]
    for text in ('version 3.5 is available.', 'سلام version 3.5', '3.5 version', 'version 1,234.5',
                 'NVDA 2026.3.1', 'version v1.5'):
        assert any(isinstance(item, native_word) and '.' in item.text for item in protected([text])), text
    # Amir's actual version strings must survive every ordinary symbol level,
    # including when NVDA places an index on either side of a decimal point.
    for whole, version in (('Google Chrome, version 157.0.8103.79 (AMD64)', '157.0.8103.79'),
                           ('BlindRadio, version 0.2.100', '0.2.100')):
        start = whole.index(version)
        for kind in (level.NONE, level.SOME, level.MOST, level.ALL):
            for cut in range(1, len(version)):
                sequence = [whole[:start] + version[:cut], IndexCommand(100 + cut),
                            version[cut:] + whole[start + len(version):]]
                guarded = decimals.protect_decimals(driver, sequence, symbol_level=kind)
                actual = ''.join(item.text for item in guarded if isinstance(item, native_word))
                assert actual == version, (kind, cut, whole, actual)
                assert [item.index for item in guarded if isinstance(item, IndexCommand)] == [100 + cut]
    negative = protected(['version -3.5'])
    assert [item.text for item in negative if isinstance(item, native_word)] == [' minus ', '3.5']
    positive = protected(['version +3.5'])
    assert [item.text for item in positive if isinstance(item, native_word)] == [' ', '3.5']
    hyphenated = protected(['version v-1.5'])
    assert [item.text for item in hyphenated if isinstance(item, native_word)] == ['-', '1.5'], pieces(hyphenated)
    signed_split = protected(['version -', IndexCommand(9), '3.5'])
    assert [item.text for item in signed_split if isinstance(item, native_word)] == [' minus ', '3.5']
    for text in ('3.5', 'نسخه 3.5', 'version 3,5', 'نسخه ۳٫۵'):
        assert protected([text]) == [text], text
    sequence = ['version ', '3', IndexCommand(1), '.', IndexCommand(2), '5']
    marked = protected(sequence)
    assert [item.text for item in marked if isinstance(item, native_word)] == ['3', '.', '5'], pieces(marked)
    assert [item.index for item in marked if isinstance(item, IndexCommand)] == [1, 2]

    # Execute the generated driver's actual conversion after the same NVDA
    # symbol processor and chunk separators used by speech.speak.
    from build_experimental_nvda import PREFIX, add_persian
    base = {'manifest.ini': (ROOT / 'addon/synth/manifest.ini').read_bytes(),
            PREFIX + '__init__.py': (ROOT / 'addon/synth/synthDrivers/reloqC/__init__.py').read_bytes()}
    tree = ast.parse(add_persian(base)[PREFIX + '__init__.py'].decode('utf8'))
    function = next(node for node in tree.body if isinstance(node, ast.FunctionDef) and node.name == '_parts')
    eci = importlib.import_module('probe_driver._reloq_eci_direct')
    namespace = {'GERMAN': profiles[fa_id], 'LANGUAGES': profiles, 'EciText': eci.EciText,
                 'EciIndex': eci.EciIndex, 'IndexCommand': IndexCommand,
                 'CharacterModeCommand': CharacterModeCommand, 'LangChangeCommand': LangChangeCommand,
                 'NativeDictionaryWord': native_word,
                 'ReloqPersianJoinerGuard': importlib.import_module('probe_driver._reloq_persian_joiners').ReloqPersianJoinerGuard,
                 'restore_joiners': lambda value: value}
    for name in ('PitchCommand', 'RateCommand', 'VolumeCommand', 'EciTagCommand', 'SegmentationGuard',
                 'CapitalWordCommand', 'BreakCommand', 'PhonemeCommand'):
        namespace[name] = type(name, (), {})
    module = ast.fix_missing_locations(ast.Module(body=[function], type_ignores=[]))
    exec(compile(module, 'generated-nvda-driver', 'exec'), namespace)
    nvda_sequence = [processors['fa'].processText(item, level.NONE) + '  ' if isinstance(item, str) else item
                     for item in marked]
    parts = namespace['_parts'](nvda_sequence, language=profiles[fa_id], languages=profiles)
    assert ''.join(part.text for part in parts if isinstance(part, eci.EciText)) == 'version   3.5'
    assert [part.value for part in parts if isinstance(part, eci.EciIndex)] == [1, 2]
    explicit = ['version ', LangChangeCommand('fa_IR'), '3.5']
    assert protected(explicit) == explicit
    spelled = [CharacterModeCommand(True), 'version 3.5', CharacterModeCommand(False)]
    assert protected(spelled) == spelled
    assert decimals.protect_decimals(driver, ['version 3.5'], symbol_level=level.CHAR) == ['version 3.5']
    dictionaries.dictionaries['user'] = [types.SimpleNamespace(compiled=re.compile(r'3\.5'))]
    assert protected(['version 3.5']) == ['version 3.5']
    dictionaries.dictionaries['user'] = []
    # NVDA's mandatory built-in dictionary can match a CamelCase word far
    # from the version. It must still run on that word without preventing
    # safe protection of the unrelated number.
    class BuiltinEntry:
        def __init__(self, pattern):
            self.pattern, self.replacement, self.type, self.caseSensitive = pattern, ' ', 1, True
            self.compiled = re.compile(pattern)
        def sub(self, value):
            return self.compiled.sub(' ', value)
    class BuiltinDictionary(list):
        def sub(self, value):
            for entry in self:
                value = entry.sub(value)
            return value
    builtin = BuiltinDictionary((BuiltinEntry(r'(?=[A-Z])(?<=[a-z])'),
                                 BuiltinEntry(r'(?=[A-Z][a-z])(?<=[A-Z])'),
                                 BuiltinEntry(r'(?=\d)(?<=[^\W\d])')))
    dictionaries.dictionaries['builtin'] = builtin
    whole = 'BlindRadio, version 0.2.100'
    guarded = protected([whole])
    assert any(isinstance(item, native_word) and item.text == '0.2.100' for item in guarded)
    processed = ''.join(item.text if isinstance(item, native_word) else builtin.sub(item)
                        for item in guarded if isinstance(item, (str, native_word)))
    assert processed == builtin.sub(whole)
    for kind in (level.NONE, level.SOME, level.MOST, level.ALL):
        split = ['BlindRadio, version 0', IndexCommand(100), '.2', IndexCommand(101), '.100']
        output = decimals.protect_decimals(driver, split, symbol_level=kind)
        assert ''.join(item.text for item in output if isinstance(item, native_word)) == '0.2.100'
        assert [item.index for item in output if isinstance(item, IndexCommand)] == [100, 101]
    # The stock builtin's letter-to-number separator must be retained while
    # the following full decimal survives the Persian symbol processor.
    for adjacent, number in (('version v1.5.0', '1.5.0'),
                             ('سلام v1.5.0 خوب', '1.5.0'),
                             ('NVDA1.5.0', '1.5.0'),
                             ('version a1.2 and b3.4.5', None)):
        guarded = protected([adjacent])
        rebuilt = ''.join(item.text if isinstance(item, native_word) else builtin.sub(item)
                          for item in guarded if isinstance(item, (str, native_word)))
        assert rebuilt == builtin.sub(adjacent), (adjacent, pieces(guarded), rebuilt)
        assert any(isinstance(item, native_word) and '.' in item.text for item in guarded), adjacent
        if number is not None:
            assert any(isinstance(item, native_word) and item.text.endswith(number) for item in guarded), adjacent
    adjacent = 'version v1.5.0'
    for kind in (level.NONE, level.SOME, level.MOST, level.ALL):
        for cut in range(1, len(adjacent)):
            sequence = [adjacent[:cut], IndexCommand(cut), adjacent[cut:]]
            guarded = decimals.protect_decimals(driver, sequence, symbol_level=kind)
            output = ''.join(item.text if isinstance(item, native_word) else builtin.sub(item)
                             for item in guarded if isinstance(item, (str, native_word)))
            expected = builtin.sub(adjacent[:cut]) + builtin.sub(adjacent[cut:])
            assert output == expected, (kind, cut, pieces(guarded), output, expected)
            assert [item.index for item in guarded if isinstance(item, IndexCommand)] == [cut]
            assert ''.join(item.text for item in guarded if isinstance(item, native_word)).replace(' ', '') == '1.5.0'
    # A user dictionary spanning the version is always left to NVDA.
    dictionaries.dictionaries['user'] = [types.SimpleNamespace(compiled=re.compile(r'0\.2\.100'))]
    assert protected([whole]) == [whole]
    dictionaries.dictionaries['user'] = []
    # A customized builtin rule that changes the number must retain control.
    class NumberReplacement(BuiltinEntry):
        def sub(self, value):
            return self.compiled.sub('custom', value)
    builtin.append(NumberReplacement(r'1\.5\.0'))
    assert protected(['version v1.5.0']) == ['version v1.5.0']
    builtin.pop()
    # NVDA 2026.3 exposes dictionaries through enabled definitions instead
    # of the legacy dictionary map.
    builtin_definition = types.SimpleNamespace(enabled=True, source=types.SimpleNamespace(value='builtin'),
                                               dictionary=builtin)
    user_definition = types.SimpleNamespace(enabled=True, source=types.SimpleNamespace(value='user'),
                                            dictionary=[])
    dictionaries.listAvailableSpeechDictDefinitions = lambda: [builtin_definition, user_definition]
    assert any(isinstance(item, native_word) for item in protected([whole]))
    assert any(isinstance(item, native_word) and item.text.endswith('1.5.0') for item in protected([adjacent]))
    # An add-on dictionary after builtin can match the new separated spelling
    # even though it did not match the original text. It must retain priority.
    after_builtin = NumberReplacement(r'v 1\.5\.0')
    user_definition.dictionary = [after_builtin]
    assert protected([adjacent]) == [adjacent]
    user_definition.dictionary = [types.SimpleNamespace(compiled=re.compile(r'0\.2\.100'))]
    assert protected([whole]) == [whole]
    del dictionaries.listAvailableSpeechDictDefinitions
    dictionaries.dictionaries['builtin'] = []
    driver._selectedLanguage = pinglish_id
    assert any(isinstance(item, native_word) for item in protected(['phone 3.5']))
    assert protected(['salam 3.5']) == ['salam 3.5']
    dictionaries.dictionaries['builtin'] = builtin
    assert any(isinstance(item, native_word) and item.text.endswith('1.5.0')
               for item in protected(['v1.5.0']))
    assert any(isinstance(item, native_word) and item.text.endswith('1.5.0')
               for item in protected(['NVDA1.5.0']))
    assert protected(['salam1.5']) == ['salam1.5']
    driver._selectedLanguage = en_id
    assert protected(['version 3.5']) == ['version 3.5']
    # Run the normal driver's earlier acronym guard too: an initialism
    # directly followed by a version must not lose the numeric point before
    # the Persian decimal guard sees it.
    from bisect import bisect_left
    wordseg = importlib.import_module('probe_driver._reloq_wordseg')
    tags = importlib.import_module('probe_driver._speech_tags')
    normal_source = tree  # The driver generated for the Persian test add-on.
    normal_class = next(node for node in normal_source.body if isinstance(node, ast.ClassDef) and node.name == 'SynthDriver')
    methods = [node for node in normal_class.body if isinstance(node, ast.FunctionDef) and node.name in (
        '_protect_tags', '_native_following_boundary', '_nvda_builtin_splits_acronym',
        '_nvda_dictionary_matches', '_nvda_personal_word_ranges')]
    namespace = dict(bisect_left=bisect_left, re=re, config=sys.modules['config'],
                     LangChangeCommand=LangChangeCommand, CharacterModeCommand=CharacterModeCommand,
                     NativeDictionaryWord=native_word, SegmentationGuard=tags.SegmentationGuard,
                     protect_sequence=tags.protect_sequence, dotted_version_word=decimals.dotted_version_word,
                     language_for=importlib.import_module('probe_driver._reloq_languages').language_for,
                     _BUILTIN_BOUNDARY=re.compile(r'(?=[A-Z])(?<=[a-z])|(?=[A-Z][a-z])(?<=[A-Z])|(?=\d)(?<=[^\W\d])'))
    for name in ('protected_native_spans', 'word_runs', 'camel_word', 'builtin_acronym'):
        namespace[name] = getattr(wordseg, name)
    module = ast.Module(body=[ast.ClassDef(name='SynthDriver', bases=[], keywords=[], body=methods, decorator_list=[])],
                        type_ignores=[])
    exec(compile(ast.fix_missing_locations(module), 'real-reloq-protection', 'exec'), namespace)
    normal = namespace['SynthDriver']()
    normal._terminating, normal._voiceTags = False, False
    normal._selectedLanguage, normal._voice = fa_id, '1'
    normal._languages, normal._submit_lock = profiles, threading.Lock()
    normal._wordSegmentation = normal._camelCase = False
    normal._wordseg_keys = None
    normal._imported_dictionaries = types.SimpleNamespace(matches=lambda *_: False, process=lambda text, *_: text)
    sys.modules['synthDriverHandler'] = types.SimpleNamespace(getSynth=lambda: normal)
    sys.modules['globalVars'] = types.SimpleNamespace(speechDictionaryProcessing=True)
    dictionaries.dictionaries['builtin'] = builtin
    for text in ('v1.5.0', 'NVDA1.5.0'):
        after_acronyms = normal._protect_tags([text], symbolLevel=level.NONE)
        after_decimals = decimals.protect_decimals(normal, after_acronyms, symbol_level=level.NONE)
        guarded = ''.join(item.text for item in after_decimals if isinstance(item, native_word))
        assert '1.5.0' in guarded, (text, pieces(after_acronyms), pieces(after_decimals))
    for text in ('v1.5.0', 'NVDA1.5.0'):
        for cut in range(1, len(text)):
            sequence = [text[:cut], IndexCommand(100 + cut), text[cut:]]
            after_acronyms = normal._protect_tags(sequence, symbolLevel=level.NONE)
            after_decimals = decimals.protect_decimals(normal, after_acronyms, symbol_level=level.NONE)
            guarded = ''.join(item.text for item in after_decimals if isinstance(item, native_word)).replace(' ', '')
            assert guarded == '1.5.0', (text, cut, pieces(after_acronyms), pieces(after_decimals))
            assert [item.index for item in after_decimals if isinstance(item, IndexCommand)] == [100 + cut]
    split = ['NVDA1', IndexCommand(1), '.', IndexCommand(2), '5.0']
    result = decimals.protect_decimals(normal, normal._protect_tags(split, symbolLevel=level.NONE),
                                       symbol_level=level.NONE)
    assert ''.join(item.text for item in result if isinstance(item, native_word)).replace(' ', '') == '1.5.0'
    dictionaries.dictionaries['user'] = [types.SimpleNamespace(compiled=re.compile(r'NVDA1\.5\.0'),
                                                              pattern='NVDA1.5.0', replacement='custom', type=0)]
    original = ['NVDA1.5.0']
    assert decimals.protect_decimals(normal, normal._protect_tags(original, symbolLevel=level.NONE),
                                    symbol_level=level.NONE) == original
    print('Passed English decimal points through Persian NVDA symbols, indexes, spelling and dictionaries.')


if __name__ == '__main__':
    main()
