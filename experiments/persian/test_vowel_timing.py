"""Verify the optional, eSpeak-informed Persian back-A timing candidate."""
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
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from listening import render
from salam_samples import (SALAM, DONYA_FINAL_STRESS, DONYA_FINAL_STRESS_LONG_A,
                           IRAN_FINAL_STRESS, IRAN_FINAL_STRESS_LONG_A)
from test_selected_probe import KEYS, set_probe

PATTERN = re.compile(r'FA_NUC phone=(-?\d+) class=(-?\d+) stress=(-?\d+) '
                     r'length=(-?\d+) before=(-?\d+) duration=(-?\d+)')


def capture(engine, text: str, vowel: int):
    set_probe(a=1, q=5, h=5, l=4, rise=2, vowel=vowel)
    os.environ['RELOQ_FA_TRACE_NUC'] = '1'
    saved = os.dup(2)
    with tempfile.TemporaryFile() as f:
        try:
            ctypes.CDLL(None).fflush(None)
            os.dup2(f.fileno(), 2)
            pcm = render(engine, text)
            ctypes.CDLL(None).fflush(None)
        finally:
            os.dup2(saved, 2)
            os.close(saved)
            os.environ.pop('RELOQ_FA_TRACE_NUC', None)
        f.seek(0)
        rows = [tuple(map(int, m.groups())) for line in f.read().decode().splitlines()
                if (m := PATTERN.search(line))]
    assert rows, ('missing duration trace', text)
    return pcm, rows


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--baseline', required=True, type=Path)
    ap.add_argument('--probe', required=True, type=Path)
    args = ap.parse_args()
    count = 0
    test = DirectEciEngine(args.probe.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    try:
        test.set_parameter(PROCESS_TAGS, 1)
        test.set_parameter(SAMPLE_RATE, 3)
        test.set_parameter(GENDER, 0)
        cases = ((SALAM, True), (DONYA_FINAL_STRESS, True),
                 (IRAN_FINAL_STRESS, True), (IRAN_FINAL_STRESS_LONG_A, False),
                 (DONYA_FINAL_STRESS_LONG_A, False), ('`[.1pA]', False))
        for speed in (40, 98, 200):
            test.set_parameter(NATIVE_SPEED, speed)
            for text, has_short_back_a in cases:
                assert text in test._phonemes_for_probe(text), ('invalid phoneme tag', text)
                old_pcm, old = capture(test, text, 0)
                new_pcm, new = capture(test, text, 1)
                assert len(old) == len(new), ('nucleus count changed', speed, text, old, new)
                changed = 0
                for a, b in zip(old, new):
                    assert a[:4] == b[:4] and a[4] == b[4] and a[4] == a[5], (
                        'nucleus context changed', speed, text, a, b)
                    expected = (b[4]*5+2)//4 if b[0] == 34 and b[1] == 1 and b[3] == 1 and b[4] > 0 else b[4]
                    assert b[5] == expected, ('unexpected duration', speed, text, a, b)
                    changed += b[5] != a[5]
                assert (changed > 0) == has_short_back_a, ('wrong vowel selection', speed, text, old, new)
                if has_short_back_a:
                    assert new_pcm != old_pcm and len(new_pcm) > len(old_pcm), ('A did not lengthen', speed, text)
                else:
                    assert new_pcm == old_pcm, ('other phone altered', speed, text)
                count += 1
    finally:
        test.close()

    try:
        for dialect in (0x10000, 0x40000, 0x30000, 0x130000):
            baseline = DirectEciEngine(args.baseline.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            probe = DirectEciEngine(args.probe.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                for e in (baseline, probe):
                    e.set_parameter(PROCESS_TAGS, 1)
                    e.set_parameter(SAMPLE_RATE, 3)
                set_probe()
                wanted = render(baseline, 'Salam?')
                set_probe(a=1, q=5, h=5, l=4, rise=2, vowel=1)
                assert render(probe, 'Salam?') == wanted, ('vowel leaked to other dialect', dialect)
                count += 1
            finally:
                baseline.close()
                probe.close()
        print(f'Passed {count} vowel-timing, context, speed and dialect-isolation checks.')
    finally:
        for key in (*KEYS, 'RELOQ_FA_TRACE_NUC'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
