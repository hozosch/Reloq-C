"""Trace the Finnish S rule in viisi and representative Persian contexts."""
from __future__ import annotations

import argparse
import ctypes
import os
from pathlib import Path
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from listening import render
from salam_samples import SALAM
from test_selected_probe import KEYS, set_probe


def trace_s(engine: DirectEciEngine, text: str) -> list[str]:
    os.environ['RELOQ_FA_TRACE_S'] = '1'
    saved = os.dup(2)
    with tempfile.TemporaryFile() as out:
        try:
            ctypes.CDLL(None).fflush(None)
            os.dup2(out.fileno(), 2)
            render(engine, text)
            ctypes.CDLL(None).fflush(None)
        finally:
            os.dup2(saved, 2)
            os.close(saved)
            os.environ.pop('RELOQ_FA_TRACE_S', None)
        out.seek(0)
        return [line.removeprefix('FA_S ') for line in out.read().decode('utf8').splitlines()
                if line.startswith('FA_S ')]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x90000,
                             encoding='utf8', native_voice=True)
    try:
        engine.set_parameter(PROCESS_TAGS, 1)
        engine.set_parameter(SAMPLE_RATE, 3)
        set_probe(a=1, q=5, h=5, l=4, rise=2, vowel=1)
        for text in ('viisi', "`[.1v'i:'.0si]", SALAM,
                     '`[.1sa]', '`[.1as]', '`[.1sA]', '`[.1As]',
                     '`[.1su]', '`[.1us]', '`[.1is]', '`[.1si]'):
            print(f'{text}: {trace_s(engine, text)}')
    finally:
        engine.close()
        for key in (*KEYS, 'RELOQ_FA_TRACE_S'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
