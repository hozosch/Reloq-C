"""Check the private Persian light-L, 50% H and eSpeak-informed A probes."""
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


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    checks = 0
    try:
        for dialect in (0x90000, 0x10000, 0x40000, 0x130000):
            base = DirectEciEngine(args.baseline.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            probe = DirectEciEngine(args.probe.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                for engine in (base, probe):
                    engine.set_parameter(PROCESS_TAGS, 1)
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        for engine in (base, probe):
                            engine.set_parameter(SAMPLE_RATE, rate)
                            engine.set_parameter(GENDER, gender)
                        cases = ((IRAN, SALAM, QALAM, '`[.1lA]', '`[.1Al]', '`[.1ta]', 'noqte')
                                 if dialect == 0x90000 else ('Leben', 'Salam', 'test?'))
                        for text in cases:
                            set_probe()
                            expected = render(base, text)
                            assert render(probe, text) == expected, ('disabled', dialect, rate, gender, text)
                            if dialect != 0x90000:
                                set_probe(a=7, q=5, x=1, h=4, l=1, rise=2)
                                assert render(probe, text) == expected, ('other language', dialect, rate, gender, text)
                            checks += 1
                        if dialect != 0x90000:
                            continue
                        for text in (SALAM, QALAM, '`[.1lA]', '`[.1Al]', '`[.1lu]', '`[.1ol]'):
                            assert text in probe._phonemes_for_probe(text)
                            set_probe(a=5, q=6 if text == QALAM else 0)
                            before = render(probe, text)
                            set_probe(a=5, q=6 if text == QALAM else 0, l=1)
                            after = render(probe, text)
                            assert before != after and len(before) == len(after), ('light L', rate, gender, text)
                            checks += 1
                        for text in ('`[.1pi]', '`[.1tA]', IRAN, 'noqte'):
                            set_probe(a=5, q=5)
                            before = render(probe, text)
                            set_probe(a=5, q=5, l=1)
                            assert render(probe, text) == before, ('L affected other phone', rate, gender, text)
                            checks += 1
                        set_probe(a=1)
                        three_peaks = render(probe, IRAN)
                        set_probe(a=7)
                        four_peaks = render(probe, IRAN)
                        assert three_peaks != four_peaks and len(three_peaks) == len(four_peaks), ('A fourth peak', rate, gender)
                        set_probe()
                        front = render(probe, '`[.1pA]')
                        set_probe(a=7)
                        assert render(probe, '`[.1pA]') == front, ('A touched Finnish ä', rate, gender)
                        checks += 2
                        for phone in 'ptk':
                            bare, explicit = f'`[.1{phone}A]', f'`[.1{phone}hA]'
                            set_probe(h=3)
                            before = render(probe, bare)
                            set_probe(h=4)
                            after = render(probe, bare)
                            gap_ms = (len(before) - len(after)) * 500 / SAMPLE_RATES[rate]
                            assert 14 <= gap_ms <= 18, ('50% H duration', rate, gender, phone, gap_ms)
                            assert after == render(probe, explicit), ('real Finnish H', rate, gender, phone)
                            checks += 2
                        set_probe(q=0)
                        regular_k = render(probe, QALAM)
                        set_probe(q=6)
                        tagged_q = render(probe, QALAM)
                        set_probe(q=6, h=4)
                        assert regular_k != tagged_q and render(probe, QALAM) == tagged_q, ('tagged Q/H', rate, gender)
                        set_probe(q=5)
                        ordinary_q = render(probe, 'qalam')
                        set_probe(q=6)
                        assert render(probe, 'qalam') == ordinary_q, ('Q=6 shape', rate, gender)
                        checks += 2
                if dialect == 0x90000:
                    probe.set_parameter(SAMPLE_RATE, 3)
                    probe.set_parameter(GENDER, 0)
                    set_probe(a=5, rise=2)
                    original = pitch_windows(render(probe, PERSIAN_PHRASE + '?'))
                    set_probe(a=5, l=1, rise=2)
                    lighter = pitch_windows(render(probe, PERSIAN_PHRASE + '?'))
                    end1 = [v for t, v in original if .65 <= t <= .72]
                    end2 = [v for t, v in lighter if .65 <= t <= .72]
                    assert len(end1) >= 3 and len(end2) >= 3
                    f01, f02 = float(np.median(end1)), float(np.median(end2))
                    assert .9 < f02/f01 < 1.1, ('question melody', f01, f02)
                    print(f'Accepted question ending: old {f01:.0f} Hz, lighter L {f02:.0f} Hz (approximate F0).')
                    checks += 1
                    for speed in (40, 98, 200):
                        probe.set_parameter(NATIVE_SPEED, speed)
                        for phone in 'ptk':
                            set_probe(h=4)
                            assert render(probe, f'`[.1{phone}A]') == render(probe, f'`[.1{phone}hA]'), ('H speed', speed, phone)
                            checks += 1
            finally:
                base.close()
                probe.close()
        print(f'Passed {checks} light-L, H, vowel, Q and language-isolation checks.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
