"""Signal and isolation checks for the second private Persian experiment."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES
from listening import render

IRAN_US = '`[.1ir.0an]'
SETTINGS = ('RELOQ_FA_PROBE_A', 'RELOQ_FA_PROBE_Q', 'RELOQ_FA_PROBE_US_A')


def settings(a: int = 0, q: int = 0, us_a: int = 0) -> None:
    for key, value in zip(SETTINGS, (a, q, us_a)):
        os.environ[key] = str(value)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    count = 0
    try:
        for language in (0x10000, 0x90000, 0x40000, 0x130000):
            base = DirectEciEngine(args.baseline.resolve(), dialect=language, encoding='utf8', native_voice=True)
            probe = DirectEciEngine(args.probe.resolve(), dialect=language, encoding='utf8', native_voice=True)
            try:
                for e in (base, probe):
                    e.set_parameter(PROCESS_TAGS, 1)
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        for e in (base, probe):
                            e.set_parameter(SAMPLE_RATE, rate)
                            e.set_parameter(GENDER, gender)
                        if language == 0x10000:
                            cases = ('father', IRAN_US, '`[.1pi]', 'test')
                        elif language == 0x90000:
                            cases = ('qalam', 'noqte', 'aqa', 'Qalam', 'kalam', 'nokte', '`[.1pi]')
                        else:
                            cases = ('Ein kurzer Test.', 'qalam', 'noqte')
                        for text in cases:
                            settings()
                            expected = render(base, text)
                            assert render(probe, text) == expected, ('disabled', language, rate, gender, text)
                            count += 1
                            if language == 0x10000:
                                settings(us_a=1)
                                slight = render(probe, text)
                                settings(us_a=2)
                                closed = render(probe, text)
                                if text in ('father', IRAN_US):
                                    assert slight != expected and closed != expected and slight != closed, ('US A', rate, gender, text)
                                else:
                                    assert slight == expected and closed == expected, ('US A control', rate, gender, text)
                                settings(q=4)
                                assert render(probe, text) == expected, ('Q leaked into US', text)
                                count += 3
                            elif language == 0x90000:
                                settings(q=1)
                                short = render(probe, text)
                                settings(q=3)
                                softer = render(probe, text)
                                settings(q=4)
                                longer = render(probe, text)
                                if text in ('qalam', 'noqte', 'aqa', 'Qalam'):
                                    assert short != expected and softer != short and longer != softer, ('Q', rate, gender, text)
                                    add_1 = (len(softer) - len(short)) / (2 * SAMPLE_RATES[rate])
                                    add_2 = (len(longer) - len(short)) / (2 * SAMPLE_RATES[rate])
                                    assert 0.004 <= add_1 <= 0.020 and 0.012 <= add_2 <= 0.040, ('Q duration', rate, gender, text, add_1, add_2)
                                else:
                                    assert short == expected and softer == expected and longer == expected, ('Q control', rate, gender, text)
                                settings(us_a=2)
                                assert render(probe, text) == expected, ('US A leaked into Finnish', text)
                                count += 4
                            else:
                                settings(q=4, us_a=2)
                                assert render(probe, text) == expected, ('other language', language, rate, gender, text)
                                count += 1
            finally:
                base.close()
                probe.close()
        print(f'Passed {count} signal, duration and isolation checks.')
    finally:
        for key in SETTINGS:
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
