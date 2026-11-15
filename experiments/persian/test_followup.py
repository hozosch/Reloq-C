"""Focused checks for Persian H, glottal, A and word-boundary aspiration."""
from __future__ import annotations

import argparse
import os
from pathlib import Path

from inspect_a_context import trace_a
from listening import render
from _reloq_eci_direct import SAMPLE_RATES
from salam_samples import IRAN_PREFERRED, SALAM
from test_selected_probe import GENDER, KEYS, NATIVE_SPEED, SAMPLE_RATE, engine, set_probe


def choose(**changes) -> None:
    values = dict(a=1, q=7, x=1, rise=2, h=5, l=4, vowel=1, s=1,
                  a_target='salam-fixed', final=1, glottal=2, end_h=1, nasal=0, glot_onset=1)
    values.update(changes)
    set_probe(**values)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    prev = engine(args.previous, 0x90000)
    cur = engine(args.probe, 0x90000)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for e in (prev, cur):
                    e.set_parameter(SAMPLE_RATE, rate)
                    e.set_parameter(GENDER, gender)
                for text in ('`[.1ka.0RAz]', '`[.0RA.1lAm] `[.1ka.0RAz]',
                             '`[.1sefr] `[.1ka.0RAz]', '`[.1sa.0ka]'):
                    choose(h=0)
                    bare = render(cur, text)
                    choose(h=5)
                    aspirated = render(cur, text)
                    delta_ms = (len(aspirated) - len(bare)) * 500 / SAMPLE_RATES[rate]
                    assert aspirated != bare and 35 <= delta_ms <= 60, (
                        'initial/intervocalic aspiration', rate, gender, text, delta_ms)
                    checks += 1
                for text in ('`[.1skA]', '`[.0RA.1lAm]', '`[.1noR.0te]'):
                    choose(h=0)
                    bare = render(cur, text)
                    choose(h=5)
                    assert render(cur, text) == bare, ('unwanted aspiration', rate, gender, text)
                    checks += 1
                text = '`[.1dAhw] `[.1sefr]'
                choose(end_h=0)
                old_h = render(cur, text)
                choose(end_h=1)
                new_h = render(cur, text)
                assert new_h != old_h and len(new_h) == len(old_h), ('H timbre', rate, gender)
                choose(end_h=0)
                old_other = render(cur, '`[.1sA.0lAm]')
                choose(end_h=1)
                assert render(cur, '`[.1sA.0lAm]') == old_other, ('H affected non-H', rate, gender)
                checks += 2
                for text in (IRAN_PREFERRED, SALAM, '`[.1pa]', '`[.1sa]'):
                    choose(nasal=0)
                    bare = render(cur, text)
                    choose(nasal=1)
                    light = render(cur, text)
                    choose(nasal=2)
                    stronger = render(cur, text)
                    assert bare != light and light != stronger and len(bare) == len(light) == len(stronger), (
                        'nasal A', rate, gender, text)
                    checks += 1
                for text in ('`[.1pi]', '`[.1pe]', '`[.1pA]', '`[.1sefr]'):
                    choose(nasal=0)
                    bare = render(cur, text)
                    choose(nasal=1)
                    assert render(cur, text) == bare, ('other vowel affected', rate, gender, text)
                    checks += 1
                for text in ('`[.1sa.0wAt]', '`[.0so.1wal]', '`[.1mAs.0w\'u:\'l]'):
                    choose(glot_onset=0)
                    no_onset = render(cur, text)
                    choose(glot_onset=1)
                    onset = render(cur, text)
                    assert onset != no_onset, ('native glottal onset missing', rate, gender, text)
                    checks += 1
                for text in (SALAM, IRAN_PREFERRED, '`[.1pe]', '`[.1ka]'):
                    choose(glot_onset=0)
                    ordinary = render(cur, text)
                    choose(glot_onset=1)
                    assert render(cur, text) == ordinary, ('glottal onset leaked', rate, gender, text)
                    checks += 1
                for text in (SALAM, IRAN_PREFERRED, '`[.1sefr]', '`[.1ka]', '`[.1noR.0te]'):
                    choose(glottal=1, end_h=0, nasal=0)
                    assert render(cur, text) == render(prev, text), ('baseline changed', rate, gender, text)
                    checks += 1

        cur.set_parameter(SAMPLE_RATE, 3)
        cur.set_parameter(GENDER, 0)
        choose(nasal=0)
        stable = trace_a(cur, IRAN_PREFERRED)
        choose(nasal=1)
        assert trace_a(cur, IRAN_PREFERRED) == stable and stable, 'A formants or bandwidths moved'
        checks += 1
        for speed in (40, 98, 200):
            cur.set_parameter(NATIVE_SPEED, speed)
            choose(h=0)
            bare = render(cur, '`[.0RA.1lAm] `[.1ka.0RAz]')
            choose(h=5)
            assert render(cur, '`[.0RA.1lAm] `[.1ka.0RAz]') != bare, ('aspiration speed', speed)
            choose(glot_onset=0)
            bare = render(cur, '`[.1sa.0wAt]')
            choose(glot_onset=1)
            assert render(cur, '`[.1sa.0wAt]') != bare, ('glottal speed', speed)
            checks += 2
    finally:
        prev.close()
        cur.close()
        for key in (*KEYS, 'RELOQ_FA_TRACE_A', 'RELOQ_FA_TRACE_H_START'):
            os.environ.pop(key, None)
    print(f'Passed {checks} acoustic, word-boundary and preserved-voice checks.')


if __name__ == '__main__':
    main()
