"""Focused checks for French L and its transitions in the Finnish probe."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES
from listening import render
from salam_samples import SALAM, QALAM, PERSIAN_PHRASE
from test_native_probe import IRAN
from test_question_rise import pitch_windows
from test_selected_probe import KEYS, set_probe
from test_german_l_transfer import trace_acoustic_values


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True, type=Path)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    count = 0
    try:
        for dialect in (0x90000, 0x30000, 0x30001, 0x40000, 0x10000, 0x130000):
            baseline = DirectEciEngine(args.baseline.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            probe = DirectEciEngine(args.probe.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                for engine in (baseline, probe):
                    engine.set_parameter(PROCESS_TAGS, 1)
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        for engine in (baseline, probe):
                            engine.set_parameter(SAMPLE_RATE, rate)
                            engine.set_parameter(GENDER, gender)
                        cases = ((SALAM, QALAM, IRAN, '`[.1lA]', '`[.1Al]', 'noqte')
                                 if dialect == 0x90000 else ('Salam', 'La lumière', 'test?'))
                        for text in cases:
                            set_probe()
                            expected = render(baseline, text)
                            assert render(probe, text) == expected, ('disabled', dialect, rate, gender, text)
                            if dialect != 0x90000:
                                set_probe(a=1, q=5, x=1, h=5, l=3, rise=2)
                                assert render(probe, text) == expected, ('other language', dialect, rate, gender, text)
                            count += 1
                        if dialect != 0x90000:
                            continue
                        for text in (SALAM, QALAM, '`[.1lA]', '`[.1Al]', '`[.1lu]', '`[.1ol]', '`[.1i.0li]'):
                            assert text in probe._phonemes_for_probe(text), ('invalid tag', text)
                            flags = dict(a=1, q=6 if text == QALAM else 0)
                            set_probe(**flags)
                            finnish = render(probe, text)
                            set_probe(l=2, **flags)
                            german = render(probe, text)
                            set_probe(l=3, **flags)
                            french = render(probe, text)
                            pcm = np.frombuffer(french, dtype='<i2').astype(np.int32)
                            assert french != finnish and french != german, ('L did not change', rate, gender, text)
                            assert len(finnish) == len(french), ('L timing', rate, gender, text)
                            assert np.max(np.abs(pcm)) < 32767 and np.sqrt(np.mean(pcm.astype(float)**2)) > 200
                            count += 1
                        for text in (IRAN, '`[.1pA]', '`[.1si]', 'noqte'):
                            set_probe(a=1, q=5)
                            expected = render(probe, text)
                            set_probe(a=1, q=5, l=3)
                            assert render(probe, text) == expected, ('L leaked', rate, gender, text)
                            count += 1
                        for phone in 'ptk':
                            bare, explicit = f'`[.1{phone}A]', f'`[.1{phone}hA]'
                            set_probe(h=4)
                            h50 = render(probe, bare)
                            set_probe(h=5)
                            h60 = render(probe, bare)
                            gap_ms = (len(h60)-len(h50))*500/SAMPLE_RATES[rate]
                            assert 6 <= gap_ms <= 10 and h60 == render(probe, explicit), ('60% H', rate, gender, phone, gap_ms)
                            count += 1
                        set_probe(q=6)
                        q = render(probe, QALAM)
                        set_probe(q=6, h=5)
                        assert render(probe, QALAM) == q, ('H modified Q', rate, gender)
                        count += 1
                if dialect == 0x90000:
                    probe.set_parameter(SAMPLE_RATE, 3)
                    probe.set_parameter(GENDER, 0)
                    set_probe(a=1, rise=2)
                    old = pitch_windows(render(probe, PERSIAN_PHRASE + '?'))
                    set_probe(a=1, l=3, rise=2)
                    new = pitch_windows(render(probe, PERSIAN_PHRASE + '?'))
                    a = [v for t, v in old if .65 <= t <= .72]
                    b = [v for t, v in new if .65 <= t <= .72]
                    assert len(a) >= 3 and len(b) >= 3 and .9 < np.median(b)/np.median(a) < 1.1
                    print(f'Question ending F0: Finnish L {np.median(a):.0f} Hz, French L {np.median(b):.0f} Hz.')
                    count += 1
                    for text in ('`[.1al.0rA]', '`[.1Al.0ma]', '`[.1al.0Ga]', '`[.1aG.0lA]'):
                        set_probe(a=1, q=5, x=1, h=5, l=3, rise=2)
                        assert text in probe._phonemes_for_probe(text), ('cluster tag', text)
                        samples = np.frombuffer(render(probe, text), dtype='<i2').astype(np.int32)
                        assert len(samples) > 100 and 200 < np.max(np.abs(samples)) < 32767, ('cluster', text)
                        count += 1
                    for speed in (40, 98, 200):
                        probe.set_parameter(NATIVE_SPEED, speed)
                        for phone in 'ptk':
                            set_probe(h=5)
                            assert render(probe, f'`[.1{phone}A]') == render(probe, f'`[.1{phone}hA]')
                            count += 1
            finally:
                baseline.close()
                probe.close()

        # Historical German L must be unchanged in the new experiment.
        previous = DirectEciEngine(args.previous.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
        probe = DirectEciEngine(args.probe.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
        try:
            for engine in (previous, probe):
                engine.set_parameter(PROCESS_TAGS, 1)
            for text in (SALAM, QALAM, '`[.1lA]', '`[.1lu]'):
                set_probe(a=1, q=6 if text == QALAM else 0, h=5, l=2)
                assert render(previous, text) == render(probe, text), ('German L changed', text)
                count += 1
        finally:
            previous.close()
            probe.close()

        # Trace the original French rule and its translated Finnish version.
        french = DirectEciEngine(args.probe.resolve(), dialect=0x30000, encoding='utf8', native_voice=True)
        finnish = DirectEciEngine(args.probe.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
        try:
            for engine in (french, finnish):
                engine.set_parameter(PROCESS_TAGS, 1)
                engine.set_parameter(SAMPLE_RATE, 3)
            pairs = (
                ('`[.l1a]', '`[.1la]'),
                ('`[.s0E.l1am]', SALAM),
                ('`[.k0E.l1Em]', QALAM),
                ('`[.l1E]', '`[.1lA]'),
                ('`[.l1u]', '`[.1lu]'),
                ('`[.l1i]', '`[.1li]'),
                ('`[.1El]', '`[.1Al]'),
            )
            for fr_text, fi_text in pairs:
                assert fr_text in french._phonemes_for_probe(fr_text), ('French tag', fr_text)
                original = trace_acoustic_values(french, fr_text, 'FR_L')
                mapped = trace_acoustic_values(finnish, fi_text, 'FA_L', a=1, l=3,
                                               q=6 if fi_text == QALAM else 0)
                assert original == mapped, ('French L values', fr_text, fi_text, original, mapped)
                count += 1
            print(f'French L targets and transitions match in {len(pairs)} matched contexts.')
        finally:
            french.close()
            finnish.close()
        print(f'Passed {count} French-L, 60% H, Q and language-isolation checks.')
    finally:
        for key in (*KEYS, 'RELOQ_FA_TRACE_L'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
