"""Check the private probe, including isolation and audible signal changes."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, PROCESS_TAGS, SAMPLE_RATE
from listening import render

IRAN = "`[.1'i:'.0r'a:'n]"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    pairs = []
    count = 0
    try:
        for dialect in (0x90000, 0x40000, 0x10000, 0x130000):
            old = DirectEciEngine(args.baseline.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            new = DirectEciEngine(args.probe.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            pairs.append((old, new))
            old.set_parameter(PROCESS_TAGS, 1)
            new.set_parameter(PROCESS_TAGS, 1)
            for rate in (1, 3, 6):
                for gender in (0, 1):
                    for e in (old, new):
                        e.set_parameter(SAMPLE_RATE, rate)
                        e.set_parameter(GENDER, gender)
                    cases = (('qalam', 'noqte', 'aqa', 'Qalam', 'kalam', 'nokte', IRAN, 'Sini')
                             if dialect == 0x90000 else
                             ('Hier ist ein kurzer Test.', 'qalam', 'noqte'))
                    for text in cases:
                        os.environ.pop('RELOQ_FA_PROBE_A', None)
                        os.environ.pop('RELOQ_FA_PROBE_Q', None)
                        baseline = render(old, text)
                        disabled = render(new, text)
                        assert baseline == disabled, ('disabled', dialect, rate, gender, text)
                        count += 1
                        if dialect != 0x90000:
                            os.environ['RELOQ_FA_PROBE_A'] = '1'
                            os.environ['RELOQ_FA_PROBE_Q'] = '2'
                            assert render(new, text) == baseline, ('other language', dialect, text)
                            count += 1
                            continue
                        os.environ['RELOQ_FA_PROBE_Q'] = '1'
                        q1 = render(new, text)
                        os.environ['RELOQ_FA_PROBE_Q'] = '2'
                        q2 = render(new, text)
                        if text in ('qalam', 'noqte', 'aqa', 'Qalam'):
                            assert q1 != baseline and q2 != baseline and q1 != q2, ('q failed', rate, gender, text)
                        else:
                            assert q1 == baseline and q2 == baseline, ('q leaked', rate, gender, text)
                        count += 2
                        if text in (IRAN, 'Sini'):
                            os.environ['RELOQ_FA_PROBE_Q'] = '0'
                            os.environ['RELOQ_FA_PROBE_A'] = '1'
                            a = render(new, text)
                            assert (a != baseline) == (text == IRAN), ('a failed or leaked', rate, gender, text)
                            count += 1
        print(f'Passed {count} PCM isolation and difference checks.')
    finally:
        os.environ.pop('RELOQ_FA_PROBE_A', None)
        os.environ.pop('RELOQ_FA_PROBE_Q', None)
        for old, new in pairs:
            old.close()
            new.close()


if __name__ == '__main__':
    main()
