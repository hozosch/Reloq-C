"""Test mixed German and French L rules in the private Finnish probe."""
from __future__ import annotations
import argparse
import ctypes
import os
from pathlib import Path
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, PROCESS_TAGS, SAMPLE_RATE
from listening import render
from salam_samples import SALAM, QALAM, PERSIAN_PHRASE, IRAN_FINAL_STRESS
from test_native_probe import IRAN
from test_selected_probe import KEYS, set_probe


def traces(engine, text: str, mode: int) -> list[str]:
    set_probe(a=1, h=5, l=mode, rise=2)
    os.environ['RELOQ_FA_TRACE_L'] = '1'
    saved = os.dup(2)
    with tempfile.TemporaryFile() as f:
        try:
            ctypes.CDLL(None).fflush(None)
            os.dup2(f.fileno(), 2)
            render(engine, text)
            ctypes.CDLL(None).fflush(None)
        finally:
            os.dup2(saved, 2)
            os.close(saved)
            os.environ.pop('RELOQ_FA_TRACE_L', None)
        f.seek(0)
        return [line.removeprefix('FA_L ') for line in f.read().decode().splitlines()
                if line.startswith('FA_L ')]


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--baseline', required=True, type=Path)
    ap.add_argument('--probe', required=True, type=Path)
    args = ap.parse_args()
    cases = (
        (SALAM, 2, 5), (QALAM, 2, 6), ('`[.1Al]', 3, 5),
        ('`[.1Al] `[.1sa]', 3, 5), ('`[.1Al.0ta]', 2, 5),
        ('sala', 2, 5), ('sal', 3, 5), ('sal sana', 3, 5),
        ('salta', 2, 5), ('sal?', 3, 5),
    )
    count = 0
    try:
        for dialect in (0x90000, 0x40000, 0x30000, 0x10000, 0x130000):
            base = DirectEciEngine(args.baseline.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            test = DirectEciEngine(args.probe.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                for e in (base, test):
                    e.set_parameter(PROCESS_TAGS, 1)
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        for e in (base, test):
                            e.set_parameter(SAMPLE_RATE, rate)
                            e.set_parameter(GENDER, gender)
                        inputs = (SALAM, IRAN, 'sal sana') if dialect == 0x90000 else ('Salam', 'test?')
                        for text in inputs:
                            set_probe()
                            old = render(base, text)
                            assert render(test, text) == old, ('probe disabled', dialect, rate, gender, text)
                            if dialect != 0x90000:
                                set_probe(a=1, q=5, x=1, h=5, l=4, rise=2)
                                assert render(test, text) == old, ('other dialect', dialect, rate, gender, text)
                            count += 1
                        if dialect != 0x90000:
                            continue
                        different = 0
                        for text, selected, q in cases:
                            flags = dict(a=1, q=q, h=5, rise=2)
                            set_probe(l=selected, **flags)
                            wanted = render(test, text)
                            set_probe(l=4, **flags)
                            actual = render(test, text)
                            set_probe(l=3 if selected == 2 else 2, **flags)
                            alternate = render(test, text)
                            assert actual == wanted, ('L selection', rate, gender, text, selected)
                            different += wanted != alternate
                            count += 1
                        assert different >= 4, ('L cases lack an audible difference', rate, gender)
                        for phrase in (PERSIAN_PHRASE, SALAM + ' `[.0do.1nja]'):
                            set_probe(a=1, h=5, l=2, rise=2)
                            expected = render(test, phrase + '?')
                            set_probe(a=1, h=5, l=4, rise=2)
                            assert render(test, phrase + '?') == expected, ('question contour', phrase)
                            count += 1
                if dialect == 0x90000:
                    assert IRAN_FINAL_STRESS in test._phonemes_for_probe(IRAN_FINAL_STRESS), ('invalid Iran tag', IRAN_FINAL_STRESS)
                    test.set_parameter(SAMPLE_RATE, 3)
                    test.set_parameter(GENDER, 0)
                    for text, choices in (('sal sala', (3, 2)), ('sala sal', (2, 3))):
                        de, fr, mixed = traces(test, text, 2), traces(test, text, 3), traces(test, text, 4)
                        assert len(de) == len(fr) == len(mixed) == 2, (text, de, fr, mixed)
                        assert mixed == [fr[i] if c == 3 else de[i] for i, c in enumerate(choices)], (
                            'multiple L choices', text, de, fr, mixed)
                        count += 1
            finally:
                base.close()
                test.close()
        print(f'Passed {count} focused L-position, question-contour and language-isolation checks.')
    finally:
        for key in (*KEYS, 'RELOQ_FA_TRACE_L'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
