"""Check the accepted Salam A nucleus across contexts in the private probe."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from inspect_a_context import trace_a
from listening import render
from salam_samples import DONYA_ESPEAK, IRAN_ESPEAK, IRAN_PREFERRED, SALAM
from test_selected_probe import KEYS, set_persian_probe, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x90000,
                             encoding='utf8', native_voice=True)
    checks = 0
    try:
        engine.set_parameter(PROCESS_TAGS, 1)
        engine.set_parameter(SAMPLE_RATE, 3)
        engine.set_parameter(GENDER, 0)
        engine.set_parameter(NATIVE_SPEED, 50)
        common = dict(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1, s=1)
        set_probe(**common, a_target='656,1000,2620')
        reference = trace_a(engine, SALAM)
        assert len(reference) == 1, ('one Salam A expected', reference)
        reference_pcm = render(engine, SALAM)
        set_persian_probe(final=0)
        assert trace_a(engine, SALAM) == reference, 'Salam A targets changed'
        assert render(engine, SALAM) == reference_pcm, 'accepted Salam sound changed'
        checks += 1
        changed = 0
        texts = (IRAN_PREFERRED, IRAN_ESPEAK, DONYA_ESPEAK, '`[.1pa]', '`[.1sa]',
                 '`[.1la]', '`[.1ma]', '`[.1na]', '`[.1ta]', "`[.1t'a:']",
                 '`[.1fa.0na]')
        for gender in (0, 1):
            for speed in (40, 50, 98, 200):
                engine.set_parameter(GENDER, gender)
                engine.set_parameter(NATIVE_SPEED, speed)
                for text in texts:
                    assert text in engine._phonemes_for_probe(text), ('invalid tag', text)
                    set_probe(**common, a_target='656,1000,2620')
                    before = render(engine, text)
                    set_persian_probe(final=0)
                    actual = trace_a(engine, text)
                    after = render(engine, text)
                    assert actual and all(row == reference[0] for row in actual), ('A targets vary', text, actual)
                    assert len(after) == len(before), ('duration changed', text)
                    changed += after != before
                    checks += 1
                for text in ('`[.1pi]', '`[.1pe]', '`[.1sefr]', '`[.1pu]', 'viisi'):
                    set_probe(**common, a_target='656,1000,2620')
                    before = render(engine, text)
                    set_persian_probe(final=0)
                    assert render(engine, text) == before, ('other vowel or S changed', text)
                    checks += 1
        assert changed > 0, 'stabilization did not change any context'
        for dialect in (0x10000, 0x40000, 0x30000, 0x130000):
            other = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                other.set_parameter(PROCESS_TAGS, 1)
                other.set_parameter(SAMPLE_RATE, 3)
                set_probe(**common)
                before = render(other, 'Salam?')
                set_persian_probe(final=0)
                assert render(other, 'Salam?') == before, ('different language changed', dialect)
                checks += 1
            finally:
                other.close()
        print(f'Passed {checks} A nucleus, unchanged Salam and isolation checks.')
    finally:
        engine.close()
        for key in (*KEYS, 'RELOQ_FA_TRACE_A'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
