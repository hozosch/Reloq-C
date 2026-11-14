"""Focused checks for the newer A, Finnish H and Spanish question contour."""
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
from test_native_probe import IRAN
from test_question_rise import PHRASE, pitch_windows
from test_selected_probe import KEYS, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    checks = 0
    try:
        for language in (0x90000, 0x10000, 0x40000, 0x130000):
            previous = DirectEciEngine(args.previous.resolve(), dialect=language, encoding='utf8', native_voice=True)
            probe = DirectEciEngine(args.probe.resolve(), dialect=language, encoding='utf8', native_voice=True)
            try:
                for engine in (previous, probe):
                    engine.set_parameter(PROCESS_TAGS, 1)
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        for engine in (previous, probe):
                            engine.set_parameter(SAMPLE_RATE, rate)
                            engine.set_parameter(GENDER, gender)
                        cases = ((IRAN, 'qalam', 'noqte', '`[.1Ga]', '`[.1tha]', PHRASE + '?')
                                 if language == 0x90000 else ('father', 'qalam', 'test'))
                        for text in cases:
                            set_probe(a=2, q=5, x=1, rise=1, stops=1)
                            assert render(previous, text) == render(probe, text), ('previous settings changed', language, rate, gender, text)
                            checks += 1
                        if language != 0x90000:
                            for text in ('father', 'qalam', 'test?'):
                                set_probe()
                                expected = render(probe, text)
                                set_probe(a=3, q=5, x=1, rise=2, h=1)
                                assert render(probe, text) == expected, ('other language', language, rate, gender, text)
                                checks += 1
                            continue
                        set_probe(a=2)
                        old_a = render(probe, IRAN)
                        set_probe(a=3)
                        off80 = render(probe, IRAN)
                        set_probe(a=4)
                        off100 = render(probe, IRAN)
                        assert old_a != off80 and off80 != off100 and len(old_a) == len(off80) == len(off100)
                        set_probe()
                        neutral_i = render(probe, '`[.1pi]')
                        set_probe(a=3)
                        assert render(probe, '`[.1pi]') == neutral_i, ('other vowel', rate, gender)
                        checks += 4
                        for word in ('`[.1pha]', '`[.1tha]', '`[.1kha]', '`[.1apha]'):
                            if word not in probe._phonemes_for_probe(word):
                                # The engine may resyllabify the intervocalic
                                # example, but the actual phonemes must remain.
                                if word != '`[.1apha]':
                                    raise AssertionError(('phone tag rejected', word))
                            set_probe(a=3)
                            full = render(probe, word)
                            set_probe(a=3, h=1)
                            short = render(probe, word)
                            delta_ms = (len(full)-len(short))*500/SAMPLE_RATES[rate]
                            assert full != short and 12 <= delta_ms <= 20, ('short H', rate, gender, word, delta_ms)
                            checks += 1
                        for word in ('`[.1ha]', '`[.1spa]', '`[.1ta]', 'qalam', 'noqte'):
                            set_probe(a=3, q=5)
                            ordinary = render(probe, word)
                            set_probe(a=3, q=5, h=1)
                            assert render(probe, word) == ordinary, ('H control', rate, gender, word)
                            checks += 1
                        for text in (PHRASE + '.', PHRASE + '?', 'salam donya.', 'salam donya?', 'Iran?'):
                            set_probe(a=3, rise=1)
                            old = render(probe, text)
                            set_probe(a=3, rise=2)
                            new = render(probe, text)
                            assert ((old != new) == text.endswith('?')) and len(old) == len(new), \
                                ('question contour', rate, gender, text)
                            checks += 1
                if language == 0x90000:
                    probe.set_parameter(SAMPLE_RATE, 3)
                    probe.set_parameter(GENDER, 0)
                    set_probe(a=2, rise=1)
                    old_pts = pitch_windows(render(probe, PHRASE + '?'))
                    set_probe(a=2, rise=2)
                    new_pts = pitch_windows(render(probe, PHRASE + '?'))
                    old_late = [p for t, p in old_pts if .64 <= t <= .69]
                    new_late = [p for t, p in new_pts if .64 <= t <= .69]
                    assert len(old_late) >= 3 and len(new_late) >= 3
                    old_pitch, new_pitch = np.median(old_late), np.median(new_late)
                    assert new_pitch > old_pitch * 1.1, ('rise is not higher', old_pitch, new_pitch)
                    spanish = DirectEciEngine(args.probe.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
                    try:
                        spanish.set_parameter(PROCESS_TAGS, 1)
                        spanish.set_parameter(SAMPLE_RATE, 3)
                        spanish.set_parameter(GENDER, 0)
                        set_probe()
                        spanish_pts = pitch_windows(render(spanish, PHRASE + '?'))
                        span_late = [p for t, p in spanish_pts if .70 <= t <= .75]
                        assert len(span_late) >= 3
                        spanish_pitch = np.median(span_late)
                        assert .85 < new_pitch / spanish_pitch < 1.15, ('Spanish comparison', new_pitch, spanish_pitch)
                    finally:
                        spanish.close()
                    print(f'Example ending, old Finnish {old_pitch:.0f} Hz, tuned {new_pitch:.0f} Hz, Spanish {spanish_pitch:.0f} Hz (approximate F0).')
                    checks += 2
                    for speed in (40, 98, 200):
                        probe.set_parameter(NATIVE_SPEED, speed)
                        set_probe(a=3)
                        full = render(probe, '`[.1tha]')
                        statement = render(probe, PHRASE + '.')
                        set_probe(a=3, h=1, rise=2)
                        shortened = render(probe, '`[.1tha]')
                        assert shortened != full and len(shortened) < len(full), ('H at speed', speed)
                        assert render(probe, PHRASE + '.') == statement, ('statement at speed', speed)
                        checks += 2
                    probe.set_parameter(NATIVE_SPEED, 98)
                    for text, individually in (
                        ('noqte', dict(a=3, q=5)),
                        ('`[.1Ga]', dict(a=3, x=1)),
                        ('`[.1tha]', dict(a=3, h=1)),
                        (PHRASE + '?', dict(a=3, rise=2)),
                    ):
                        set_probe(**individually)
                        expected = render(probe, text)
                        set_probe(a=3, q=5, x=1, h=1, rise=2)
                        assert render(probe, text) == expected, ('combined probes', text)
                        checks += 1
            finally:
                previous.close()
                probe.close()
        print(f'Passed {checks} A, H, Q, question and language-isolation checks.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
