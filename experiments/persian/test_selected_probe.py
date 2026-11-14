"""Check the selected private Finnish-based Persian acoustic experiment."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from listening import render
from test_native_probe import IRAN

KEYS = ('RELOQ_FA_PROBE_A', 'RELOQ_FA_PROBE_Q', 'RELOQ_FA_PROBE_X',
        'RELOQ_FA_PROBE_US_A', 'RELOQ_FA_PROBE_RISE', 'RELOQ_FA_PROBE_STOPS',
        'RELOQ_FA_PROBE_H', 'RELOQ_FA_PROBE_L', 'RELOQ_FA_PROBE_VOWEL')


def set_probe(a: int = 0, q: int = 0, x: int = 0, rise: int = 0,
              stops: int = 0, h: int = 0, l: int = 0, vowel: int = 0) -> None:
    for key, value in zip(KEYS, (a, q, x, 0, rise, stops, h, l, vowel)):
        os.environ[key] = str(value)


def engine(path: Path, language: int) -> DirectEciEngine:
    e = DirectEciEngine(path.resolve(), dialect=language, encoding='utf8', native_voice=True)
    e.set_parameter(PROCESS_TAGS, 1)
    return e


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    checks = 0
    try:
        for language in (0x10000, 0x90000, 0x40000, 0x130000):
            base, probe = engine(args.baseline, language), engine(args.probe, language)
            try:
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        for e in (base, probe):
                            e.set_parameter(SAMPLE_RATE, rate)
                            e.set_parameter(GENDER, gender)
                        cases = ((IRAN, '`[.1pi]', '`[.1Gi]', '`[.1Si]',
                                  'qalam', 'noqte', 'kalam', 'aqa')
                                 if language == 0x90000 else ('father', 'qalam', 'test'))
                        for text in cases:
                            set_probe()
                            original = render(base, text)
                            assert render(probe, text) == original, ('disabled', language, text)
                            set_probe(a=2, q=5, x=1)
                            selected = render(probe, text)
                            if language != 0x90000:
                                assert selected == original, ('language isolation', language, text)
                            elif text in (IRAN, '`[.1Gi]', 'qalam', 'noqte', 'kalam', 'aqa'):
                                assert selected != original, ('expected change', rate, gender, text)
                            else:
                                assert selected == original, ('Finnish control', rate, gender, text)
                            checks += 2
                        if language == 0x90000:
                            set_probe()
                            k_base = render(probe, 'kalam')
                            s_base = render(probe, '`[.1Si]')
                            set_probe(q=5, x=1)
                            assert render(probe, 'kalam') == k_base, ('ordinary K changed', rate, gender)
                            assert render(probe, '`[.1Si]') == s_base, ('ordinary S changed', rate, gender)
                            checks += 2
                            # The selected release is 3 dB quieter, but keeps the
                            # chosen duration. The vowel must not be attenuated.
                            set_probe(q=4)
                            loud = render(probe, 'qalam')
                            set_probe(q=5)
                            quiet = render(probe, 'qalam')
                            assert loud != quiet and len(loud) == len(quiet)
                            assert loud[-len(loud)//3:] == quiet[-len(quiet)//3:]
                            checks += 2
                            # The native Finnish scheduler suppresses this release
                            # before another stop. The private probe must supply it.
                            for word in ('noqte', 'noQte', 'qta', 'aqki', 'aqpa'):
                                set_probe(q=4)
                                cluster4 = render(probe, word)
                                set_probe(q=5)
                                cluster5 = render(probe, word)
                                assert cluster4 != cluster5 and len(cluster4) == len(cluster5), \
                                    ('Q release in cluster', rate, gender, word)
                                checks += 1
                if language == 0x90000:
                    probe.set_parameter(SAMPLE_RATE, 3)
                    probe.set_parameter(GENDER, 0)
                    set_probe(q=4)
                    loud = np.frombuffer(render(probe, 'qalam'), dtype='<i2')[:320].astype(float)
                    set_probe(q=5)
                    quiet = np.frombuffer(render(probe, 'qalam'), dtype='<i2')[:320].astype(float)
                    ratio = np.sqrt(np.mean(quiet * quiet) / np.mean(loud * loud))
                    assert 0.69 < ratio < 0.73, ('q initial RMS ratio', ratio)
                    print(f'Selected initial Q release: {20*np.log10(ratio):.2f} dB versus Q4.')
                    set_probe(q=4)
                    q4 = np.frombuffer(render(probe, 'noqte'), dtype='<i2').astype(float)
                    set_probe(q=5)
                    q5 = np.frombuffer(render(probe, 'noqte'), dtype='<i2').astype(float)
                    # In this reference context the inserted frication falls in
                    # the previously silent 195-220 ms cluster window.
                    window = slice(195 * 16, 220 * 16)
                    rms4 = np.sqrt(np.mean(q4[window] ** 2))
                    rms5 = np.sqrt(np.mean(q5[window] ** 2))
                    assert rms4 > 100 and rms5 > 70, ('inaudible release', rms4, rms5)
                    assert rms5 < rms4, ('Q5 not quieter in cluster', rms4, rms5)
                    print(f'Q before T in the 195–220 ms window: Q4 RMS {rms4:.0f}, Q5 RMS {rms5:.0f}.')
                    checks += 3
                    for speed in (40, 98, 200):
                        probe.set_parameter(NATIVE_SPEED, speed)
                        for word in ('noqte', 'qta', 'aqki', 'aqpa'):
                            set_probe(q=4)
                            a = render(probe, word)
                            set_probe(q=5)
                            b = render(probe, word)
                            assert a != b and len(a) == len(b), ('cluster at speed', speed, word)
                            checks += 1
                    probe.set_parameter(NATIVE_SPEED, 98)
                    for text, only in (
                        ('noqte', dict(a=2, q=5)),
                        ('`[.1pa] `[.1ta] `[.1ka]', dict(a=2, stops=1)),
                        ('`[.0sa.1lam] `[.1do.0nja]?', dict(a=2, rise=1)),
                        ('`[.1Ga]', dict(a=2, x=1)),
                    ):
                        set_probe(**only)
                        expected = render(probe, text)
                        set_probe(a=2, q=5, x=1, rise=1, stops=1)
                        assert render(probe, text) == expected, ('combined modes', text)
                        checks += 1
            finally:
                base.close()
                probe.close()
        print(f'Passed {checks} acoustic and isolation checks.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
