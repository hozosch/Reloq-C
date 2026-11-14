"""Focused checks for a German L rule adapted to the Finnish probe."""
from __future__ import annotations

import argparse
import ctypes
import os
from pathlib import Path
import sys
import tempfile

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES
from listening import render
from salam_samples import SALAM, QALAM, PERSIAN_PHRASE
from test_native_probe import IRAN
from test_question_rise import pitch_windows
from test_selected_probe import KEYS, set_probe


def trace_acoustic_values(engine: DirectEciEngine, text: str, prefix: str, **flags: int) -> str:
    """Capture the temporary native trace, which is written to stderr by C."""
    set_probe(**flags)
    os.environ['RELOQ_FA_TRACE_L'] = '1'
    saved = os.dup(2)
    with tempfile.TemporaryFile() as output:
        try:
            ctypes.CDLL(None).fflush(None)
            os.dup2(output.fileno(), 2)
            render(engine, text)
            ctypes.CDLL(None).fflush(None)
        finally:
            os.dup2(saved, 2)
            os.close(saved)
            os.environ.pop('RELOQ_FA_TRACE_L', None)
        output.seek(0)
        matches = [line.removeprefix(prefix + ' ')
                   for line in output.read().decode('utf8').splitlines()
                   if line.startswith(prefix + ' ')]
    assert len(matches) == 1, ('L trace count', text, matches)
    return matches[0]


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    count = 0
    try:
        for dialect in (0x90000, 0x40000, 0x10000, 0x130000):
            old = DirectEciEngine(args.baseline.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            new = DirectEciEngine(args.probe.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                for engine in (old, new):
                    engine.set_parameter(PROCESS_TAGS, 1)
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        for engine in (old, new):
                            engine.set_parameter(SAMPLE_RATE, rate)
                            engine.set_parameter(GENDER, gender)
                        cases = ((SALAM, QALAM, IRAN, '`[.1lA]', '`[.1Al]', 'noqte')
                                 if dialect == 0x90000 else ('Salam', 'Leben', 'test?'))
                        for text in cases:
                            set_probe()
                            expected = render(old, text)
                            assert render(new, text) == expected, ('disabled', dialect, rate, gender, text)
                            if dialect != 0x90000:
                                set_probe(a=1, q=5, x=1, h=5, l=2, rise=2)
                                assert render(new, text) == expected, ('other language', dialect, rate, gender, text)
                            count += 1
                        if dialect != 0x90000:
                            continue
                        for text in (SALAM, QALAM, '`[.1lA]', '`[.1Al]', '`[.1lu]', '`[.1ol]', '`[.1i.0li]'):
                            assert text in new._phonemes_for_probe(text), ('invalid tag', text)
                            flags = dict(a=1, q=6 if text == QALAM else 0)
                            set_probe(**flags)
                            finnish = render(new, text)
                            set_probe(l=1, **flags)
                            previous = render(new, text)
                            set_probe(l=2, **flags)
                            transferred = render(new, text)
                            samples = np.frombuffer(transferred, dtype='<i2').astype(np.int32)
                            assert finnish != transferred and previous != transferred, ('L did not change', rate, gender, text)
                            assert len(finnish) == len(transferred), ('L timing', rate, gender, text)
                            assert np.max(np.abs(samples)) < 32767 and np.sqrt(np.mean(samples.astype(float)**2)) > 200
                            count += 1
                        for text in (IRAN, '`[.1pA]', '`[.1si]', 'noqte'):
                            set_probe(a=1, q=5)
                            expected = render(new, text)
                            set_probe(a=1, q=5, l=2)
                            assert render(new, text) == expected, ('L leaked', rate, gender, text)
                            count += 1
                        set_probe()
                        front = render(new, '`[.1pA]')
                        set_probe(a=1)
                        assert render(new, '`[.1pA]') == front, ('short ä changed', rate, gender)
                        count += 1
                        for phone in 'ptk':
                            bare, explicit = f'`[.1{phone}A]', f'`[.1{phone}hA]'
                            set_probe(h=4)
                            h50 = render(new, bare)
                            set_probe(h=5)
                            h60 = render(new, bare)
                            set_probe(h=3)
                            h70 = render(new, bare)
                            gap1 = (len(h60)-len(h50))*500/SAMPLE_RATES[rate]
                            gap2 = (len(h70)-len(h60))*500/SAMPLE_RATES[rate]
                            assert 6 <= gap1 <= 10 and 6 <= gap2 <= 10, ('60% H', rate, gender, phone, gap1, gap2)
                            set_probe(h=5)
                            assert h60 == render(new, explicit), ('Finnish native H', rate, gender, phone)
                            count += 2
                        set_probe(a=1, q=6)
                        tagged_q = render(new, QALAM)
                        set_probe(a=1, q=6, h=5)
                        assert render(new, QALAM) == tagged_q, ('H modified Q', rate, gender)
                        count += 1
                if dialect == 0x90000:
                    new.set_parameter(SAMPLE_RATE, 3)
                    new.set_parameter(GENDER, 0)
                    set_probe(a=1, rise=2)
                    previous = pitch_windows(render(new, PERSIAN_PHRASE + '?'))
                    set_probe(a=1, l=2, rise=2)
                    transferred = pitch_windows(render(new, PERSIAN_PHRASE + '?'))
                    p1 = [v for t, v in previous if .65 <= t <= .72]
                    p2 = [v for t, v in transferred if .65 <= t <= .72]
                    assert len(p1) >= 3 and len(p2) >= 3
                    f1, f2 = float(np.median(p1)), float(np.median(p2))
                    assert .9 < f2/f1 < 1.1, ('question changed', f1, f2)
                    print(f'Question ending F0: Finnish L {f1:.0f} Hz, German-rule L {f2:.0f} Hz.')
                    count += 1
                    for speed in (40, 98, 200):
                        new.set_parameter(NATIVE_SPEED, speed)
                        for phone in 'ptk':
                            set_probe(h=5)
                            assert render(new, f'`[.1{phone}A]') == render(new, f'`[.1{phone}hA]'), ('H speed', speed, phone)
                            count += 1
            finally:
                old.close()
                new.close()
        # Compare corresponding real German and Finnish phone contexts. This
        # verifies the actual L targets, bandwidths and source amplitudes,
        # rather than merely asserting that two rendered waves are different.
        german = DirectEciEngine(args.probe.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
        finnish = DirectEciEngine(args.probe.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
        try:
            for engine in (german, finnish):
                engine.set_parameter(PROCESS_TAGS, 1)
                engine.set_parameter(SAMPLE_RATE, 3)
            pairs = (
                ('`[.1la]', '`[.1la]'),
                ('`[.0sE.1lam]', SALAM),
                ('`[.0kE.1lEm]', QALAM),
                ('`[.1lE]', '`[.1lA]'),
                ('`[.1lu]', '`[.1lu]'),
            )
            for german_text, finnish_text in pairs:
                original = trace_acoustic_values(german, german_text, 'DE_L')
                mapped = trace_acoustic_values(finnish, finnish_text, 'FA_L', a=1, l=2,
                                               q=6 if finnish_text == QALAM else 0)
                assert original == mapped, ('German L targets', german_text, finnish_text, original, mapped)
                count += 1
            print(f'German L target, bandwidth and voicing traces match for {len(pairs)} aligned contexts.')
        finally:
            german.close()
            finnish.close()
        print(f'Passed {count} German-L, 60% H, vowel, Q and language-isolation checks.')
    finally:
        for key in (*KEYS, 'RELOQ_FA_TRACE_L'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
