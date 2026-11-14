"""Check the private German-informed stop aspiration in the Finnish path."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES
from listening import render
from measure_t_onset import rough_onsets
from test_selected_probe import KEYS, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    checks = 0
    try:
        for language in (0x90000, 0x40000, 0x10000, 0x130000):
            e = DirectEciEngine(args.library.resolve(), dialect=language, encoding='utf8', native_voice=True)
            try:
                e.set_parameter(PROCESS_TAGS, 1)
                for rate in (1, 3, 6):
                    for gender in (0, 1):
                        e.set_parameter(SAMPLE_RATE, rate)
                        e.set_parameter(GENDER, gender)
                        if language == 0x90000:
                            changed = ('`[.1ta]', '`[.1pa]', '`[.1ka]', '`[.1ata]', '`[.1apa]', '`[.1aka]')
                            controls = ('`[.1spa]', '`[.1akta]', '`[.1si]', 'qalam', 'noqte')
                        else:
                            changed = ()
                            controls = ('`[.1ta]', 'qalam', 'test')
                        for text in changed + controls:
                            set_probe(q=5)
                            original = render(e, text)
                            set_probe(q=5, stops=1)
                            candidate = render(e, text)
                            if text in changed:
                                delta_ms = (len(candidate)-len(original))*500/SAMPLE_RATES[rate]
                                assert candidate != original and 35 <= delta_ms <= 45, \
                                    ('aspiration timing', rate, gender, text, delta_ms)
                            else:
                                assert candidate == original, ('aspiration control', language, rate, gender, text)
                            checks += 1
                if language == 0x90000:
                    e.set_parameter(SAMPLE_RATE, 3)
                    e.set_parameter(GENDER, 0)
                    for phone in ('p', 't', 'k'):
                        tag = f'`[.1{phone}a]'
                        set_probe()
                        release0, voice0 = rough_onsets(render(e, tag))
                        set_probe(stops=1)
                        release1, voice1 = rough_onsets(render(e, tag))
                        added = (voice1-release1) - (voice0-release0)
                        assert 25 <= added <= 50, ('onset gap', phone, added)
                        print(f'{phone}: estimated release-to-voicing gap '
                              f'{voice0-release0} -> {voice1-release1} ms.')
                        checks += 1
                    for speed in (40, 98, 200):
                        e.set_parameter(NATIVE_SPEED, speed)
                        set_probe(q=5)
                        original = render(e, '`[.1ta]')
                        q = render(e, 'qalam')
                        set_probe(q=5, stops=1)
                        assert render(e, '`[.1ta]') != original
                        assert render(e, 'qalam') == q, ('Q modified by stop aspiration', speed)
                        checks += 2
            finally:
                e.close()
        print(f'Passed {checks} aspiration, context, Q and language-isolation checks.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
