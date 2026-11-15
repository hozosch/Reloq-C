"""Inspect A timbre and context in the private Persian listening probe."""
from __future__ import annotations

import argparse
import ctypes
import os
from pathlib import Path
import re
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from listening import render
from salam_samples import DONYA_ESPEAK, IRAN_ESPEAK, SALAM
from test_native_probe import IRAN
from test_selected_probe import KEYS, set_probe

PATTERN = re.compile(r'FA_A F=([0-9,-]+) R=([0-9,-]+) B=([0-9,-]+)')


def trace_a(engine: DirectEciEngine, text: str) -> list[tuple[tuple[int, ...], ...]]:
    os.environ['RELOQ_FA_TRACE_A'] = '1'
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
            os.environ.pop('RELOQ_FA_TRACE_A', None)
        out.seek(0)
        rows = []
        for line in out.read().decode('utf8').splitlines():
            if match := PATTERN.fullmatch(line):
                rows.append(tuple(tuple(map(int, part.split(','))) for part in match.groups()))
        return rows


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--target', default='')
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x90000,
                             encoding='utf8', native_voice=True)
    try:
        engine.set_parameter(PROCESS_TAGS, 1)
        engine.set_parameter(SAMPLE_RATE, 3)
        set_probe(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1, s=1, a_target=args.target)
        for text in (SALAM, IRAN, IRAN_ESPEAK, DONYA_ESPEAK, '`[.1pa]',
                     '`[.1sa]', '`[.1la]', '`[.1ma]', '`[.1na]', '`[.1ta]',
                     "`[.1t'a:']", '`[.1fa.0na]'):
            print(f'{text}: {trace_a(engine, text)}')
    finally:
        engine.close()
        for key in (*KEYS, 'RELOQ_FA_TRACE_A'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
