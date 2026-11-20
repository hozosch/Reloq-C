"""Keep internal bracket contours from being spoken as comma names."""
from __future__ import annotations

import argparse
import ast
import codecs
import collections
from enum import IntEnum
from pathlib import Path
import re
import sys
import types
from typing import Generic, TypeVar

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'addon'), str(ROOT / 'nvda')]
from nvda_speech_test_api import execute
from _reloq_eci_direct import EciIndex, EciText, NATIVE_SPEED
from direct_fa_sequence import ScopedDirectPersianEngine
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, make_language_engine


def symbol_processor():
    ns = dict(IntEnum=IntEnum, re=re, codecs=codecs, collections=collections,
              Generic=Generic, TypeVar=TypeVar, _LocaleDataT=TypeVar('_LocaleDataT'),
              SYMPRES_NEVER=0, SYMPRES_ALWAYS=1, SYMPRES_NOREP=2,
              _getSpeechSymbolsForLocale=lambda _locale: None,
              log=types.SimpleNamespace(warning=lambda *a, **k: None, error=lambda *a, **k: None))
    source = ROOT / 'addon/testdata/nvda'
    execute(source / 'characterProcessing.py', lambda node: isinstance(node, ast.ClassDef) and node.name in (
        'LocaleDataMap', 'SymbolLevel', 'SpeechSymbol', 'SpeechSymbols', 'SpeechSymbolProcessor'), ns)
    fa, en = ns['SpeechSymbols'](), ns['SpeechSymbols']()
    fa.load(str(ROOT / 'experiments/persian/reference-nvda/fa_symbols_2026_3_1.dic'))
    en.load(str(source / 'locale/en/symbols.dic'))
    sources = {'fa': (fa, ns['SpeechSymbols']()), 'en': (en, ns['SpeechSymbols']())}
    ns['SpeechSymbolProcessor'].localeSymbols = ns['LocaleDataMap'](lambda locale: sources[locale])
    return ns['SpeechSymbolProcessor']('fa'), ns['SymbolLevel']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('library', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=True)
    args = parser.parse_args()
    processor, level = symbol_processor()
    original = ScopedDirectPersianEngine._speak_prepared
    projections = []
    def capture(engine, job, *args, **kwargs):
        projections.append(job.tagged)
        return original(engine, job, *args, **kwargs)
    ScopedDirectPersianEngine._speak_prepared = capture
    engine = make_language_engine(args.library, args.library, None, args.espeak_library, args.data_parent)
    count = 0
    try:
        for language in (PERSIAN_PROBE.id, PINGLISH_PROBE.id):
            engine.select_voice(language, '1')
            for index, text in enumerate(('UNIDENTIFIED ACTOR #1: (As character)',
                                          'Example #2: (Another voice)',
                                          'سلام NVDA (As character)',
                                          'NVDA شماره 1: (سلام)')):
                engine.set_parameter(NATIVE_SPEED, 65 if index % 2 == 0 else 110)
                for setting in (level.NONE, level.SOME, level.MOST, level.ALL):
                    processed = processor.processText(text, setting)
                    # Preserve the same bracket across one or more NVDA text
                    # and index fragments, including a mark immediately at
                    # the language switch before the enclosed words.
                    for split in (False, True):
                        cut = processed.find('(') + 1
                        parts = ((EciIndex(1), processed[:cut], EciIndex(2), processed[cut:], EciIndex(3))
                                 if split and cut else (EciIndex(1), processed, EciIndex(3)))
                        projections.clear()
                        audio, marks = bytearray(), []
                        assert engine.speak(parts, audio.extend, marks.append)
                        assert audio and marks == ([1, 2, 3] if split and cut else [1, 3]), (text, setting, marks)
                        assert not any('vir.1g' in value for value in projections), (text, setting, projections)
                        count += 1
            # Explicit character navigation and spelling still name a real
            # comma, despite the different treatment of generated punctuation.
            for char in (',', '،'):
                projections.clear()
                audio = bytearray()
                assert engine.speak((EciText(char, character_mode=True),), audio.extend, lambda _v: None)
                assert audio and any('vir.1g' in value for value in projections), (language, char, projections)
    finally:
        engine.close()
        ScopedDirectPersianEngine._speak_prepared = original
    print(f'Passed {count} mixed bracket and symbol-level checks, with explicit comma spelling retained.')


if __name__ == '__main__':
    main()
