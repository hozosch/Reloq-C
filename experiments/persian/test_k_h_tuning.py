"""Check the isolated Persian K-noise and reduced final-H candidates."""
from __future__ import annotations

import argparse
import os
from pathlib import Path

from listening import render
from test_selected_probe import (GENDER, KEYS, SAMPLE_RATE, engine, set_candidate_persian_probe,
                                 set_confirmed_persian_probe, set_probe)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    args = parser.parse_args()
    persian = engine(args.probe, 0x90000)
    english = engine(args.probe, 0x10000)
    german = engine(args.probe, 0x40000)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for e in (persian, english, german):
                    e.set_parameter(SAMPLE_RATE, rate)
                    e.set_parameter(GENDER, gender)

                for text in ('`[.1ka.0RAz]', '`[.0RA.1lAm] `[.1ka.0RAz]'):
                    set_confirmed_persian_probe(k_shape=0)
                    old = render(persian, text)
                    set_confirmed_persian_probe(k_shape=1)
                    variant1 = render(persian, text)
                    set_confirmed_persian_probe(k_shape=2)
                    variant2 = render(persian, text)
                    assert len(old) == len(variant1) == len(variant2) and len({old, variant1, variant2}) == 3, (
                        'K candidates', rate, gender, text)
                    checks += 1

                for text in ('`[.1ki]', '`[.1ke]', '`[.1kA]', '`[.1ko]', '`[.1ku]',
                             '`[.1Ra]', '`[.1sa.0wAt]', '`[.1mahw]'):
                    set_confirmed_persian_probe(k_shape=0)
                    old = render(persian, text)
                    for shape in (1, 2):
                        set_confirmed_persian_probe(k_shape=shape)
                        assert render(persian, text) == old, ('K isolation', rate, gender, shape, text)
                        checks += 1

                for text in ('`[.1dAhw] `[.1sefr]', '`[.0dA.1vaz.0dAhw] `[.1sefr]'):
                    set_confirmed_persian_probe(end_h_db=0)
                    old = render(persian, text)
                    set_confirmed_persian_probe(end_h_db=2)
                    quieter = render(persian, text)
                    set_confirmed_persian_probe(end_h_db=3)
                    quietest = render(persian, text)
                    assert len(old) == len(quieter) == len(quietest) and len({old, quieter, quietest}) == 3, (
                        'final H candidates', rate, gender, text)
                    checks += 1

                text = '`[.1dAhw] `[.1sefr] `[.0RA.1lAm] `[.1ka.0RAz]'
                set_confirmed_persian_probe(end_h_db=0, k_shape=0)
                previous = render(persian, text)
                set_candidate_persian_probe()
                candidate = render(persian, text)
                assert candidate != previous and len(candidate) == len(previous), (
                    'combined candidate', rate, gender)
                checks += 1

                for text in ('`[.1mahw] `[.1sefr]', "`[.1m'e:'hr]", '`[.1ka]',
                             '`[.1sa.0wAt]', '`[.1Ra]', '`[.1sA.0lAm]'):
                    set_confirmed_persian_probe(end_h_db=0)
                    old = render(persian, text)
                    for level in (2, 3):
                        set_confirmed_persian_probe(end_h_db=level)
                        assert render(persian, text) == old, ('H isolation', rate, gender, level, text)
                        checks += 1

                for e, text in ((english, 'The quiet cat.'), (german, 'Das kalte Papier.')):
                    set_confirmed_persian_probe()
                    old = render(e, text)
                    set_confirmed_persian_probe(k_shape=1, end_h_db=2)
                    assert render(e, text) == old, ('other language changed', rate, gender, text)
                    checks += 1

                options = dict(a=1, q=7, x=1, rise=2, l=4, vowel=1, s=1,
                               a_target='salam-fixed', final=1, glottal=2, end_h=1, nasal=1)
                for shape in (1, 2):
                    set_probe(h=0, k_shape=shape, **options)
                    bare = render(persian, '`[.1ka.0RAz]')
                    set_probe(h=5, k_shape=shape, **options)
                    aspirated = render(persian, '`[.1ka.0RAz]')
                    assert len(aspirated) > len(bare), ('aspiration lost', rate, gender, shape)
                    checks += 1
    finally:
        for voice in (persian, english, german):
            voice.close()
        for key in KEYS:
            os.environ.pop(key, None)
    print(f'Passed {checks} Persian K, H, aspiration and language-isolation checks.')


if __name__ == '__main__':
    main()
