"""Check Persian front-K-derived alternatives and quieter final-H trials."""
from __future__ import annotations

import argparse
import os
from pathlib import Path

import numpy as np

from inspect_a_context import trace_a
from listening import render
from test_selected_probe import GENDER, KEYS, SAMPLE_RATE, engine, set_confirmed_persian_probe, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    args = parser.parse_args()
    persian = engine(args.probe, 0x90000)
    english = engine(args.probe, 0x10000)
    german = engine(args.probe, 0x40000)
    checks = 0
    try:
        set_confirmed_persian_probe()
        assert os.environ['RELOQ_FA_PROBE_K_SHAPE'] == '3'
        assert os.environ['RELOQ_FA_PROBE_END_H_DB'] == '9'
        checks += 2
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for e in (persian, english, german):
                    e.set_parameter(SAMPLE_RATE, rate)
                    e.set_parameter(GENDER, gender)

                for text in ('`[.1kaS]', '`[.1ka.0RAz]', '`[.1sefr] `[.1ka.0RAz]'):
                    candidates = []
                    for shape in (1, 3, 4, 5):
                        set_confirmed_persian_probe(k_shape=shape)
                        candidates.append(render(persian, text))
                    assert len(set(map(len, candidates))) == 1 and len(set(candidates)) == 4, (
                        'K candidates indistinguishable', rate, gender, text)
                    checks += 1

                # The liked K before /ae/, other vowels, the uvular phone,
                # and the separate glottal marker are controls.
                for text in ('`[.1kAf]', '`[.1ke]', '`[.1ki]', '`[.1ko]', '`[.1ku]',
                             '`[.1Ra]', '`[.1noR.0te]', '`[.1sa.0wAt]', '`[.1mahw]'):
                    set_confirmed_persian_probe(k_shape=0)
                    unchanged = render(persian, text)
                    for shape in (3, 4, 5):
                        set_confirmed_persian_probe(k_shape=shape)
                        assert render(persian, text) == unchanged, ('K leaked', rate, gender, shape, text)
                        checks += 1

                for text in ('`[.1dAhw] `[.1sefr]', '`[.0dA.1vaz.0dAhw] `[.1sefr]'):
                    candidates = []
                    for level in (3, 6, 9, 12):
                        set_confirmed_persian_probe(end_h_db=level)
                        candidates.append(render(persian, text))
                    assert len(set(map(len, candidates))) == 1 and len(set(candidates)) == 4, (
                        'H levels did not change sound', rate, gender, text)
                    arrays = [np.frombuffer(pcm, dtype='<i2').astype(np.float64) for pcm in candidates]
                    mask = np.abs(arrays[0] - arrays[-1]) > 3
                    assert mask.any(), ('empty H change', rate, gender, text)
                    rms = [float(np.sqrt(np.mean(a[mask] ** 2))) for a in arrays]
                    assert all(a > b for a, b in zip(rms, rms[1:])), ('H not quieter', rate, gender, rms)
                    checks += 2

                for text in ('`[.1mahw] `[.1sefr]', "`[.1m'e:'hr]", '`[.1ka]',
                             '`[.1sa.0wAt]', '`[.1Ra]', '`[.1sA.0lAm]'):
                    set_confirmed_persian_probe(end_h_db=0)
                    unchanged = render(persian, text)
                    set_confirmed_persian_probe(end_h_db=12)
                    assert render(persian, text) == unchanged, ('H reduction leaked', rate, gender, text)
                    checks += 1

                for e, text in ((english, 'The quiet cat.'), (german, 'Das kalte Papier.')):
                    set_confirmed_persian_probe()
                    unchanged = render(e, text)
                    set_confirmed_persian_probe(k_shape=5, end_h_db=12)
                    assert render(e, text) == unchanged, ('other language changed', rate, gender, text)
                    checks += 1

                opts = dict(a=1, q=7, x=1, rise=2, l=4, vowel=1, s=1,
                            a_target='salam-fixed', final=1, glottal=2, end_h=1, nasal=1)
                for shape in (3, 4, 5):
                    set_probe(h=0, k_shape=shape, **opts)
                    bare = render(persian, '`[.1ka.0RAz]')
                    set_probe(h=5, k_shape=shape, **opts)
                    aspirated = render(persian, '`[.1ka.0RAz]')
                    assert len(aspirated) > len(bare), ('aspiration lost', rate, gender, shape)
                    checks += 1

        # The dark vowel's own fixed formant target must remain as accepted.
        persian.set_parameter(SAMPLE_RATE, 3)
        persian.set_parameter(GENDER, 0)
        for text in ('`[.1kaS]', '`[.1ka.0RAz]'):
            set_confirmed_persian_probe()
            original = trace_a(persian, text)
            assert original, ('missing A traces', text)
            for shape in (3, 4, 5):
                set_confirmed_persian_probe(k_shape=shape)
                assert trace_a(persian, text) == original, ('A target changed', shape, text)
                checks += 1
    finally:
        for voice in (persian, english, german):
            voice.close()
        for key in (*KEYS, 'RELOQ_FA_TRACE_A'):
            os.environ.pop(key, None)
    print(f'Passed {checks} front-K, final-H, vowel and language-isolation checks.')


if __name__ == '__main__':
    main()
