"""Acoustic checks for private glottal marker and Finnish coda-H."""
from __future__ import annotations

import argparse
from pathlib import Path

import numpy as np
from scipy.signal import periodogram

from listening import render
from test_selected_probe import engine, set_probe, SAMPLE_RATE, GENDER, NATIVE_SPEED
from _reloq_eci_direct import SAMPLE_RATES


def select(*, glottal: int = 1, h: int = 5) -> None:
    set_probe(a=1, q=7, x=1, rise=2, h=h, l=4, vowel=1, s=1,
              a_target='salam-fixed', final=1, glottal=glottal)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    args = parser.parse_args()
    before = engine(args.previous, 0x90000)
    current = engine(args.probe, 0x90000)
    checks = 0
    try:
        for rate in (1, 3, 6):
            hz = SAMPLE_RATES[rate]
            for gender in (0, 1):
                for e in (before, current):
                    e.set_parameter(SAMPLE_RATE, rate)
                    e.set_parameter(GENDER, gender)
                for tag in ('`[.1kA]', '`[.1RA]', '`[.0sA.1lam]',
                            '`[.1noR.0te]', '`[.1dAh]', '`[.1sA.0wAt]'):
                    select(glottal=0)
                    original = render(before, tag)
                    assert render(current, tag) == original, ('disabled probe changed', rate, gender, tag)
                    if 'w' not in tag:
                        select(glottal=1)
                        assert render(current, tag) == original, ('unrelated phone changed', rate, gender, tag)
                        checks += 1
                    checks += 1
                for tag in ('`[.1sa.0wAt]', '`[.0wA.1li]', '`[.1ka.0wA.0RA]'):
                    select(glottal=0)
                    glide = render(current, tag)
                    select(glottal=1)
                    closure = render(current, tag)
                    assert closure and closure != glide, ('glottal marker ignored', rate, gender, tag)
                    assert closure == render(current, tag), ('glottal not deterministic', rate, gender, tag)
                    checks += 2
                select(glottal=1, h=0)
                without_h = render(current, '`[.1sa.0wAt]')
                select(glottal=1, h=5)
                assert render(current, '`[.1sa.0wAt]') == without_h, ('glottal aspirated', rate, gender)
                checks += 1

                # Phoneme-display output omits this final H, but the actual
                # PCM contains an additional high-frequency noise tail.
                bare = np.frombuffer(render(current, '`[.1dA]'), dtype='<i2').astype(float)
                breath = np.frombuffer(render(current, '`[.1dAhw]'), dtype='<i2').astype(float)
                end = np.flatnonzero(np.abs(bare) > 25)[-1]
                start = int(end + hz * .010)
                window = breath[start:start + int(hz * .025)]
                rms = float(np.sqrt(np.mean(window * window)))
                frequencies, power = periodogram(window, hz)
                high = float(power[frequencies > 3000].sum())
                low = float(power[frequencies < 2000].sum())
                assert rms > 20 and high > low * .07, ('missing coda H noise', rate, gender, rms, high, low)
                assert render(current, "`[.1m'e:'hr]") != render(current, "`[.1m'e:'r]"), (
                    'H before final consonant missing', rate, gender)
                with_h = render(current, '`[.1dAhw] `[.1sefr]')
                bare_carrier = render(current, '`[.1dAw] `[.1sefr]')
                with_h = np.frombuffer(with_h, dtype='<i2').astype(float)
                bare_carrier = np.frombuffer(bare_carrier, dtype='<i2').astype(float)
                # Seek H-like noise before the following S, where the control
                # carrier is quiet. It is not enough merely to compare the
                # differently timed byte streams.
                found_h = False
                for millis in range(100, 270, 10):
                    start, count = int(millis * hz / 1000), int(hz * .010)
                    candidate = with_h[start:start + count]
                    control = bare_carrier[start:start + count]
                    level = np.sqrt(np.mean(candidate * candidate))
                    reference_level = np.sqrt(np.mean(control * control))
                    frequencies, power = periodogram(candidate, hz)
                    high = power[frequencies > 3000].sum()
                    low = power[frequencies < 2000].sum()
                    if level > 70 and reference_level < max(65, level * .2) and high > low * .2:
                        found_h = True
                        break
                assert found_h, ('H before next word missing', rate, gender)
                checks += 3
        current.set_parameter(SAMPLE_RATE, 3)
        current.set_parameter(GENDER, 0)
        for speed in (40, 98, 200):
            current.set_parameter(NATIVE_SPEED, speed)
            select(glottal=0)
            glide = render(current, '`[.1sa.0wAt]')
            select(glottal=1)
            assert render(current, '`[.1sa.0wAt]') != glide, ('glottal at speed', speed)
            assert render(current, '`[.1dAhw] `[.1sefr]') != render(current, '`[.1dAw] `[.1sefr]'), (
                'H before next word at speed', speed)
            checks += 2
        print(f'Passed {checks} glottal, H noise, Q/K and isolation checks.')
    finally:
        before.close()
        current.close()


if __name__ == '__main__':
    main()
