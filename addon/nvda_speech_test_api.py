"""Execute pinned NVDA speech/symbol methods with external services stubbed.

The native integration test uses upstream speak/processText, dictionary
parsers and the complete symbol processor. No reimplementation of backtick
or punctuation handling is involved. Audio devices and the UI are external.
"""
import ast
import codecs
import collections
from enum import IntEnum
import itertools
import re
import sys
import types
import unicodedata
from typing import Generic, TypeVar

from nvda_test_api import SOURCE


def execute(path, select, namespace):
    tree = ast.parse(path.read_text(encoding="utf-8"))
    nodes = [node for node in tree.body if select(node)]
    future = ast.ImportFrom(module="__future__", names=[ast.alias(name="annotations")], level=0)
    tree = ast.fix_missing_locations(ast.Module(body=[future, *nodes], type_ignores=[]))
    exec(compile(tree, str(path), "exec"), namespace)


def speech_api(driver, on_sequence):
    commands = sys.modules["speech.commands"]
    ns = dict(vars(commands), IntEnum=IntEnum, re=re, codecs=codecs,
              collections=collections, Generic=Generic, TypeVar=TypeVar,
              _LocaleDataT=TypeVar("_LocaleDataT"),
              SYMPRES_NEVER=0, SYMPRES_ALWAYS=1, SYMPRES_NOREP=2,
              _getSpeechSymbolsForLocale=lambda locale: None,
              log=types.SimpleNamespace(warning=lambda *a, **kw: None,
                  error=lambda *a, **kw: None, io=lambda *a: None))
    execute(SOURCE / "characterProcessing.py",
        lambda n: isinstance(n, ast.ClassDef) and n.name in (
            "LocaleDataMap", "SymbolLevel", "SpeechSymbol", "SpeechSymbols", "SpeechSymbolProcessor"), ns)
    sources = {}
    for locale in ("en", "de", "es", "fr"):
        data = ns["SpeechSymbols"]()
        data.load(str(SOURCE / "locale" / locale / "symbols.dic"))
        sources[locale] = (data, ns["SpeechSymbols"]())
    ns["SpeechSymbolProcessor"].localeSymbols = ns["LocaleDataMap"](lambda locale: sources[locale])
    processors = {locale: ns["SpeechSymbolProcessor"](locale) for locale in sources}
    symbols = types.SimpleNamespace(SymbolLevel=ns["SymbolLevel"],
        processSpeechSymbols=lambda locale, text, level:
            processors.get(locale.split("_")[0], processors["en"]).processText(text, level))
    config = sys.modules["config"]
    for key, value in dict(autoDialectSwitching=True, unicodeNormalization=False, symbolLevel=0).items():
        config.conf["speech"][key] = value

    class Spri(IntEnum):
        NORMAL = 0

    class SpeechMode(IntEnum):
        off = 0
        beeps = 1
        onDemand = 2
        talk = 3

    sys.modules["speechViewer"] = types.SimpleNamespace(isActive=False)
    sys.modules["inputCore"] = types.SimpleNamespace(logTimeSinceInput=lambda: None)
    ns.update(characterProcessing=symbols, config=config, Spri=Spri, SpeechMode=SpeechMode,
        _speechState=types.SimpleNamespace(speechMode=SpeechMode.talk, isPaused=False),
        filter_speechSequence=sys.modules["speech.extensions"].filter_speechSequence,
        pre_speech=sys.modules["speech.extensions"].pre_speech,
        getCurrentLanguage=lambda: driver._languages[driver._selectedLanguage].locale,
        languageHandling=types.SimpleNamespace(shouldMakeLangChangeCommand=lambda: True),
        logBadSequenceTypes=lambda sequence: None, CHUNK_SEPARATOR="  ",
        RE_CONVERT_WHITESPACE=re.compile("[\0\r\n]"),
        speechDictHandler=types.SimpleNamespace(processText=lambda text: text),
        unicodeNormalize=lambda text: unicodedata.normalize("NFKC", text),
        shouldSwitchVoice=lambda: True)
    execute(SOURCE / "speech/manager.py",
        lambda n: isinstance(n, ast.ClassDef) and n.name == "ParamChangeTracker", ns)
    manager_source = ast.parse((SOURCE / "speech/manager.py").read_text(encoding="utf-8"))
    manager_class = next(n for n in manager_source.body if isinstance(n, ast.ClassDef) and n.name == "SpeechManager")
    methods = [n for n in manager_class.body if isinstance(n, ast.FunctionDef)
               and n.name in ("_ensureEndUtterance", "_processSpeechSequence")]
    method_module = ast.Module(body=[ast.ImportFrom(module="__future__",
        names=[ast.alias(name="annotations")], level=0), *methods], type_ignores=[])
    exec(compile(ast.fix_missing_locations(method_module), str(SOURCE / "speech/manager.py"), "exec"), ns)
    manager = type("Manager", (), {n.name: ns[n.name] for n in methods})()
    manager._indexCounter = itertools.count(10000)
    manager._indexesToCallbacks = {}

    def queue(sequence, priority):
        # Execute the upstream command handling; replace only scheduling and
        # priority/idle callbacks around the resulting complete utterances.
        utterance = []
        for part in manager._processSpeechSequence(sequence):
            if isinstance(part[0], commands.EndUtteranceCommand):
                if utterance:
                    on_sequence(utterance)
                    utterance = []
            else:
                utterance.extend(part)
        if utterance:
            on_sequence(utterance)

    ns["_manager"] = types.SimpleNamespace(speak=queue)
    execute(SOURCE / "speech/speech.py",
        lambda n: isinstance(n, ast.FunctionDef) and n.name in ("processText", "speak"), ns)
    return types.SimpleNamespace(speak=ns["speak"], processText=ns["processText"], symbols=symbols)
