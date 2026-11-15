"""Check Persian S noise against viisi without changing its formants."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from inspect_s_context import trace_s
from listening import render
from salam_samples import SALAM
from test_selected_probe import KEYS, set_probe

PROFILE = re.compile(r'F=([0-9,]+) R=([0-9,]+) AF=(\d+) BANDS=([0-9,]+)')


def profile(engine: DirectEciEngine, text: str) -> tuple:
    rows = trace_s(engine, text)
    assert len(rows) == 1, ('expected one S', text, rows)
    match = PROFILE.fullmatch(rows[0])
    assert match, ('bad S trace', rows[0])
    f, r, af, bands = match.groups()
    return (tuple(map(int, f.split(','))), tuple(map(int, r.split(','))),
            int(af), tuple(map(int, bands.split(','))))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    args = parser.parse_args()
    count = 0
    previous = DirectEciEngine(args.previous.resolve(), dialect=0x90000,
                               encoding='utf8', native_voice=True)
    probe = DirectEciEngine(args.probe.resolve(), dialect=0x90000,
                            encoding='utf8', native_voice=True)
    try:
        for engine in (previous, probe):
            engine.set_parameter(PROCESS_TAGS, 1)
            engine.set_parameter(SAMPLE_RATE, 3)
        cases = (SALAM, '`[.1sa]', '`[.1as]', '`[.1sA]', '`[.1As]',
                 '`[.1su]', '`[.1us]', '`[.1is]', '`[.1si]')
        for gender in (0, 1):
            for speed in (40, 98, 200):
                for engine in (previous, probe):
                    engine.set_parameter(GENDER, gender)
                    engine.set_parameter(NATIVE_SPEED, speed)
                set_probe(a=1, q=5, h=5, l=4, rise=2, vowel=1, s=0)
                reference = profile(probe, 'viisi')
                baseline_viisi = render(probe, 'viisi')
                for text in cases:
                    assert text in probe._phonemes_for_probe(text), ('invalid tag', text)
                    # The previous probe predates the back-A duration update;
                    # disable all vowel timing when comparing library builds.
                    set_probe(a=1, q=5, h=5, l=4, rise=2, vowel=0, s=0)
                    assert render(probe, text) == render(previous, text), ('disabled changed', text)
                    set_probe(a=1, q=5, h=5, l=4, rise=2, vowel=1, s=0)
                    original = profile(probe, text)
                    original_pcm = render(probe, text)
                    set_probe(a=1, q=5, h=5, l=4, rise=2, vowel=1, s=1)
                    changed = profile(probe, text)
                    assert changed[:2] == original[:2], ('formants changed', text, changed, original)
                    assert changed[2:] == reference[2:], ('noise differs from viisi', text, changed, reference)
                    adjusted_pcm = render(probe, text)
                    assert len(adjusted_pcm) == len(original_pcm), ('timing changed', text)
                    if changed != original:
                        assert adjusted_pcm != original_pcm, ('S change inaudible to engine', text)
                    count += 1
                assert render(probe, 'viisi') == baseline_viisi, ('reference changed', gender, speed)
                for text in ('`[.1pA]', '`[.1pa]', '`[.1fa]', '`[.1Sa]', '`[.1Ga]', '`[.1Za]'):
                    assert text in probe._phonemes_for_probe(text), ('invalid control tag', text)
                    set_probe(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1, s=0)
                    ordinary = render(probe, text)
                    set_probe(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1, s=1)
                    assert render(probe, text) == ordinary, ('non-S changed', text)
                    count += 1
        for dialect in (0x10000, 0x40000, 0x30000, 0x130000):
            old = DirectEciEngine(args.previous.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            new = DirectEciEngine(args.probe.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                for engine in (old, new):
                    engine.set_parameter(PROCESS_TAGS, 1)
                    engine.set_parameter(SAMPLE_RATE, 3)
                set_probe(a=1, q=5, h=5, l=4, rise=2, vowel=1, s=1)
                assert render(old, 'Salam?') == render(new, 'Salam?'), ('language isolation', dialect)
                count += 1
            finally:
                old.close()
                new.close()
        print(f'Passed {count} S context, reference and language-isolation checks.')
    finally:
        previous.close()
        probe.close()
        for key in (*KEYS, 'RELOQ_FA_TRACE_S'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
