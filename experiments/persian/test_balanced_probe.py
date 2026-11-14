"""Focused checks of Finnish/off vowel mixtures and the shorter H probe."""
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
from salam_samples import SALAM, SALAM_PREVIOUS, PERSIAN_PHRASE
from test_native_probe import IRAN
from test_question_rise import PHRASE, pitch_windows
from test_selected_probe import KEYS, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    checks = 0
    try:
        for language in (0x90000, 0x10000, 0x40000, 0x130000):
            base = DirectEciEngine(args.baseline.resolve(), dialect=language, encoding='utf8', native_voice=True)
            probe = DirectEciEngine(args.probe.resolve(), dialect=language, encoding='utf8', native_voice=True)
            try:
                for engine in (base, probe):
                    engine.set_parameter(PROCESS_TAGS, 1)
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        for engine in (base, probe):
                            engine.set_parameter(SAMPLE_RATE, rate)
                            engine.set_parameter(GENDER, gender)
                        cases = ((IRAN, SALAM, SALAM_PREVIOUS, 'noqte', '`[.1Ga]', '`[.1ta]')
                                 if language == 0x90000 else ('father', 'qalam', 'test?'))
                        for text in cases:
                            set_probe()
                            expected = render(base, text)
                            assert render(probe, text) == expected, ('disabled mode', language, rate, gender, text)
                            if language != 0x90000:
                                set_probe(a=5, q=5, x=1, h=3, rise=2)
                                assert render(probe, text) == expected, ('language isolation', language, rate, gender, text)
                            checks += 1
                        if language != 0x90000:
                            continue
                        set_probe(a=0)
                        finnish_a = render(probe, IRAN)
                        set_probe(a=4)
                        off_a = render(probe, IRAN)
                        set_probe(a=5)
                        a5050 = render(probe, IRAN)
                        set_probe(a=6)
                        a2575 = render(probe, IRAN)
                        assert len({finnish_a, off_a, a5050, a2575}) == 4
                        assert len(finnish_a) == len(off_a) == len(a5050) == len(a2575)
                        checks += 2
                        # The Finnish ä phone must not be retuned as long /A/.
                        set_probe()
                        front = render(probe, '`[.1pA]')
                        set_probe(a=5)
                        assert render(probe, '`[.1pA]') == front, ('front vowel leaked', rate, gender)
                        assert SALAM in probe._phonemes_for_probe(SALAM)
                        assert render(probe, SALAM) != render(probe, SALAM_PREVIOUS), ('Salam vowels', rate, gender)
                        checks += 3
                        for phone in 'ptk':
                            bare, explicit = f'`[.1{phone}a]', f'`[.1{phone}ha]'
                            set_probe(a=5, h=2)
                            previous_h = render(probe, bare)
                            set_probe(a=5, h=3)
                            shorter_h = render(probe, bare)
                            gap_ms = (len(previous_h) - len(shorter_h)) * 500 / SAMPLE_RATES[rate]
                            assert 6 <= gap_ms <= 10 and shorter_h != previous_h, ('H duration', rate, gender, phone, gap_ms)
                            assert render(probe, explicit) == shorter_h, ('native H', rate, gender, phone)
                            checks += 2
                        for word in ('noqte', 'qalam', '`[.1spa]', '`[.1akta]', '`[.1ha]'):
                            set_probe(a=5, q=5)
                            expected = render(probe, word)
                            set_probe(a=5, q=5, h=3)
                            assert render(probe, word) == expected, ('H control', rate, gender, word)
                            checks += 1
                        set_probe(a=5, rise=1)
                        earlier = render(probe, PERSIAN_PHRASE + '?')
                        set_probe(a=5, rise=2)
                        accepted = render(probe, PERSIAN_PHRASE + '?')
                        assert earlier != accepted and len(earlier) == len(accepted), ('accepted question contour', rate, gender)
                        checks += 1
                if language == 0x90000:
                    probe.set_parameter(SAMPLE_RATE, 3)
                    probe.set_parameter(GENDER, 0)
                    set_probe(a=3, rise=2)
                    old_points = pitch_windows(render(probe, PHRASE + '?'))
                    set_probe(a=5, rise=2)
                    new_points = pitch_windows(render(probe, PERSIAN_PHRASE + '?'))
                    old_late = [p for t, p in old_points if .64 <= t <= .70]
                    new_late = [p for t, p in new_points if .65 <= t <= .72]
                    assert len(old_late) >= 3 and len(new_late) >= 3, ('question F0', old_points, new_points)
                    old_hz, new_hz = float(np.median(old_late)), float(np.median(new_late))
                    assert .88 < new_hz/old_hz < 1.12, ('question F0 changed', old_hz, new_hz)
                    print(f'Accepted question ending: earlier {old_hz:.0f} Hz, corrected Salam {new_hz:.0f} Hz (approximate F0).')
                    checks += 1
                    for speed in (40, 98, 200):
                        probe.set_parameter(NATIVE_SPEED, speed)
                        for phone in 'ptk':
                            set_probe(a=5, h=3)
                            automatic = render(probe, f'`[.1{phone}a]')
                            explicit = render(probe, f'`[.1{phone}ha]')
                            assert automatic == explicit, ('H at speed', speed, phone)
                            checks += 1
                    for text, flags in (
                        ('noqte', dict(a=5, q=5)),
                        ('`[.1Ga]', dict(a=5, x=1)),
                        ('`[.1ta]', dict(a=5, h=3)),
                        (PERSIAN_PHRASE + '?', dict(a=5, rise=2)),
                    ):
                        set_probe(**flags)
                        expected = render(probe, text)
                        set_probe(a=5, q=5, x=1, h=3, rise=2)
                        assert render(probe, text) == expected, ('combined probes', text)
                        checks += 1
            finally:
                base.close()
                probe.close()
        print(f'Passed {checks} vowel, H, Q, question and language-isolation checks.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
