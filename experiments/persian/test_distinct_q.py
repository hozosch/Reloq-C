"""Check that private Q=7 preserves ordinary Finnish K and the chosen Q."""
from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np

from listening import render
from test_selected_probe import engine, set_probe, SAMPLE_RATE, GENDER


def select(q: int, h: int = 5) -> None:
    set_probe(a=1, q=q, x=1, rise=2, h=h, l=4, vowel=1, s=1,
              a_target='salam-fixed', final=1)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    args = parser.parse_args()
    count = 0
    finnish = engine(args.probe, 0x90000)
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                finnish.set_parameter(SAMPLE_RATE, rate)
                finnish.set_parameter(GENDER, gender)
                for tag in ('`[.1ka]', '`[.1kA]', '`[.1ra]', '`[.0sA.1lam]',
                            '`[.1do.0nja]'):
                    select(0)
                    plain = render(finnish, tag)
                    select(7)
                    assert render(finnish, tag) == plain, ('ordinary phones changed', rate, gender, tag)
                    count += 1
                for tag in ('`[.1RA]', '`[.0RA.1lAm]', '`[.1ka.0RAz]',
                            '`[.1noR.0te]', '`[.1aR.0ki]', '`[.1aR.0pa]'):
                    select(0)
                    old = render(finnish, tag)
                    select(7)
                    new = render(finnish, tag)
                    assert new and new != old, ('Q marker ignored', rate, gender, tag)
                    assert new == render(finnish, tag), ('non-deterministic Q', rate, gender, tag)
                    count += 2
                select(7, h=0)
                q_without_h = render(finnish, '`[.1RA]')
                k_without_h = render(finnish, '`[.1kA]')
                select(7, h=5)
                assert render(finnish, '`[.1RA]') == q_without_h, ('Q aspirated', rate, gender)
                assert render(finnish, '`[.1kA]') != k_without_h, ('K lost aspiration', rate, gender)
                count += 2

        finnish.set_parameter(SAMPLE_RATE, 3)  # 16 kHz
        finnish.set_parameter(GENDER, 0)
        select(5, h=0)
        original = np.frombuffer(render(finnish, 'noqte'), dtype='<i2').astype(float)
        select(7, h=0)
        marked = np.frombuffer(render(finnish, '`[.1noR.0te]'), dtype='<i2').astype(float)
        # The established Q5 and the marked Q7 both produce release noise
        # before T in the previously silent reference window.
        window = slice(200 * 16, 225 * 16)
        rms5, rms7 = [np.sqrt(np.mean(a[window] ** 2)) for a in (original, marked)]
        assert rms5 > 70 and rms7 > 70 and .8 < rms7 / rms5 < 1.2, (rms5, rms7)
        count += 1
    finally:
        finnish.close()
    for language in (0x10000, 0x40000, 0x130000):
        other = engine(args.probe, language)
        try:
            for phrase in ('test qalam', 'Iran', 'hello'):
                select(0)
                baseline = render(other, phrase)
                select(7)
                assert render(other, phrase) == baseline, ('language isolation', language, phrase)
                count += 1
        finally:
            other.close()
    print(f'Passed {count} Q/K, aspiration, cluster-release and language-isolation checks.')


if __name__ == '__main__':
    main()
