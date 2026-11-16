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
    tree = ast.parse(add_persian(base, b'dll')[PREFIX + '__init__.py'].decode('utf8'))
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
    driver._selectedLanguage = pinglish_id
    assert any(isinstance(item, native_word) for item in protected(['phone 3.5']))
    assert protected(['salam 3.5']) == ['salam 3.5']
    driver._selectedLanguage = en_id
    assert protected(['version 3.5']) == ['version 3.5']
    print('Passed English decimal points through Persian NVDA symbols, indexes, spelling and dictionaries.')


if __name__ == '__main__':
    main()
