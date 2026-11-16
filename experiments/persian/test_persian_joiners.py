"""Keep Persian half-spaces intact through NVDA without bypassing dictionaries."""
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
import unicodedata

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'addon'))
from nvda_speech_test_api import execute


class SynthCommand:
    pass


class CharacterModeCommand(SynthCommand):
    def __init__(self, state):
        self.state = state


class LangChangeCommand(SynthCommand):
    def __init__(self, lang):
        self.lang = lang


def main():
    speech = types.ModuleType('speech')
    speech.__path__ = []
    commands = types.ModuleType('speech.commands')
    commands.SynthCommand = SynthCommand
    commands.CharacterModeCommand = CharacterModeCommand
    commands.LangChangeCommand = LangChangeCommand
    speech.commands = commands
    sys.modules['speech'], sys.modules['speech.commands'] = speech, commands
    package = types.ModuleType('probe_driver')
    package.__path__ = [str(ROOT / 'nvda')]
    sys.modules['probe_driver'] = package
    joiners = importlib.import_module('probe_driver._reloq_persian_joiners')
    assert commands.ReloqPersianJoinerGuard is joiners.ReloqPersianJoinerGuard

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
    processor = ns['SpeechSymbolProcessor']('fa')
    level = ns['SymbolLevel']
    sys.modules['characterProcessing'] = types.SimpleNamespace(
        SymbolLevel=level, processSpeechSymbols=lambda locale, text, kind: processor.processText(text, kind))
    sys.modules['config'] = types.SimpleNamespace(conf={'speech': {'symbolLevel': level.NONE}})
    dictionaries = types.SimpleNamespace(dictionaries={'builtin': []})
    sys.modules['speechDictHandler'] = dictionaries
    fa_id, en_id = 0x7f0000, 0x10000
    profiles = {fa_id: types.SimpleNamespace(id=fa_id, locale='fa_IR'),
                en_id: types.SimpleNamespace(id=en_id, locale='en_US')}
    driver = types.SimpleNamespace(_selectedLanguage=fa_id, _languages=profiles, _submit_lock=threading.Lock())

    word = 'علامت‌دار'
    assert processor.processText(word, level.NONE) == 'علامت دار'
    for kind in (level.NONE, level.SOME, level.MOST, level.ALL):
        guarded = joiners.protect_joiners(driver, [word], symbol_level=kind)
        assert len(guarded) == 3 and guarded[0].enabled and not guarded[2].enabled
        processed = processor.processText(guarded[1], kind)
        assert joiners.restore_joiners(processed) == word
        assert joiners.restore_joiners(unicodedata.normalize('NFKC', processed)) == word
    assert joiners.protect_joiners(driver, [word], symbol_level=level.CHAR) == [word]
    spelled = [CharacterModeCommand(True), word, CharacterModeCommand(False)]
    assert joiners.protect_joiners(driver, spelled) == spelled
    original_pua = word + '\ue070'
    assert joiners.protect_joiners(driver, [original_pua]) == [original_pua]
    dictionaries.dictionaries['user'] = [types.SimpleNamespace(compiled=re.compile(word))]
    assert joiners.protect_joiners(driver, [word]) == [word]
    dictionaries.dictionaries['user'] = [types.SimpleNamespace(compiled=re.compile('\ue070'))]
    assert joiners.protect_joiners(driver, [word]) == [word]
    dictionaries.dictionaries['user'] = []
    driver._selectedLanguage = en_id
    assert joiners.protect_joiners(driver, [word]) == [word]
    switched = joiners.protect_joiners(driver, [LangChangeCommand('fa_IR'), word])
    assert len(switched) == 4 and joiners.restore_joiners(processor.processText(switched[2], level.NONE)) == word
    # Execute the actual generated driver's _parts implementation, including
    # its restoration guards, rather than only testing the helper in isolation.
    from build_experimental_nvda import PREFIX, add_persian
    base = {'manifest.ini': (ROOT / 'addon/synth/manifest.ini').read_bytes(),
            PREFIX + '__init__.py': (ROOT / 'addon/synth/synthDrivers/reloqC/__init__.py').read_bytes()}
    source = add_persian(base, b'dll')[PREFIX + '__init__.py'].decode('utf8')
    tree = ast.parse(source)
    part_function = next(node for node in tree.body if isinstance(node, ast.FunctionDef) and node.name == '_parts')
    eci = importlib.import_module('probe_driver._reloq_eci_direct')
    namespace = {'GERMAN': profiles[fa_id], 'LANGUAGES': profiles, 'EciText': eci.EciText,
                 'CharacterModeCommand': CharacterModeCommand, 'LangChangeCommand': LangChangeCommand,
                 'ReloqPersianJoinerGuard': joiners.ReloqPersianJoinerGuard,
                 'restore_joiners': joiners.restore_joiners}
    for name in ('IndexCommand', 'PitchCommand', 'RateCommand', 'VolumeCommand', 'EciTagCommand',
                 'SegmentationGuard', 'NativeDictionaryWord', 'CapitalWordCommand', 'BreakCommand', 'PhonemeCommand'):
        namespace[name] = type(name, (), {})
    module = ast.fix_missing_locations(ast.Module(body=[part_function], type_ignores=[]))
    exec(compile(module, 'generated-nvda-driver', 'exec'), namespace)
    guarded = joiners.protect_joiners(types.SimpleNamespace(_selectedLanguage=fa_id, _languages=profiles,
                                     _submit_lock=threading.Lock()), [word])
    guarded[1] = processor.processText(guarded[1], level.NONE) + '  '
    parts = namespace['_parts'](guarded, language=profiles[fa_id], languages=profiles)
    assert parts == [eci.EciText(word + '  ')]
    parts = namespace['_parts']([original_pua], language=profiles[fa_id], languages=profiles)
    assert parts == [eci.EciText(original_pua)]
    print('Passed real NVDA symbol levels, dictionaries, spelling, literal PUA and language-switch checks.')


if __name__ == '__main__':
    main()
