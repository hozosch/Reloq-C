"""Check that the A-centre candidate is derived from Salam, not an offset guess."""
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
from salam_samples import DONYA_ESPEAK, IRAN_ESPEAK, SALAM
from test_native_probe import IRAN
from test_selected_probe import KEYS, set_probe


def centre(start: int, end: int) -> int:
    if start > 0 and end > 0:
        return (start + end + 1) // 2
    return start if start > 0 else end


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x90000,
                             encoding='utf8', native_voice=True)
    count = 0
    try:
        engine.set_parameter(PROCESS_TAGS, 1)
        engine.set_parameter(SAMPLE_RATE, 3)
        engine.set_parameter(GENDER, 0)
        # Match the default speed in the listening comparison.
        engine.set_parameter(NATIVE_SPEED, 50)
        common = dict(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1, s=1)
        set_probe(**common)
        rows = trace_a(engine, SALAM)
        assert len(rows) == 1, ('one Salam A expected', rows)
        ref_f, ref_r, _ = rows[0]
        targets = tuple(centre(ref_f[i], ref_r[i]) for i in range(3))
        target = ','.join(map(str, targets))
        pcm_reference = render(engine, SALAM)
        set_probe(**common, a_target=target)
        assert render(engine, SALAM) == pcm_reference, 'accepted Salam A changed'
        count += 1
        changed = 0
        contexts = (IRAN, IRAN_ESPEAK, DONYA_ESPEAK, '`[.1pa]', '`[.1sa]',
                    '`[.1la]', '`[.1ma]', '`[.1na]', '`[.1ta]', "`[.1t'a:']")
        for gender in (0, 1):
            for speed in (40, 50, 98, 200):
                engine.set_parameter(GENDER, gender)
                engine.set_parameter(NATIVE_SPEED, speed)
                for text in contexts:
                    assert text in engine._phonemes_for_probe(text), ('invalid A tag', text)
                    set_probe(**common)
                    unanchored = trace_a(engine, text)
                    original_pcm = render(engine, text)
                    set_probe(**common, a_target=target)
                    anchored = trace_a(engine, text)
                    actual_pcm = render(engine, text)
                    assert len(unanchored) == len(anchored) == 1, (text, unanchored, anchored)
                    old_f, old_r, old_b = unanchored[0]
                    new_f, new_r, new_b = anchored[0]
                    assert old_b == new_b and old_f[3:] == new_f[3:] and old_r[3:] == new_r[3:], (
                        'unrelated resonances or bandwidth changed', text)
                    for i in range(3):
                        assert (old_f[i] <= 0) == (new_f[i] <= 0), ('start inheritance', text, i)
                        assert (old_r[i] <= 0) == (new_r[i] <= 0), ('end inheritance', text, i)
                        if old_f[i] > 0 and old_r[i] > 0:
                            assert new_r[i] - new_f[i] == old_r[i] - old_f[i], ('transition', text, i)
                        if centre(old_f[i], old_r[i]) > 0:
                            assert centre(new_f[i], new_r[i]) == targets[i], ('A centre', text, i, anchored, targets)
                    assert len(original_pcm) == len(actual_pcm), ('A duration changed', text)
                    changed += original_pcm != actual_pcm
                    count += 1
                for text in ('`[.1pi]', '`[.1pe]', '`[.1sefr]', '`[.1pu]'):
                    set_probe(**common)
                    unchanged = render(engine, text)
                    set_probe(**common, a_target=target)
                    assert render(engine, text) == unchanged, ('other vowel changed', text)
                    count += 1
        assert changed > 0, 'A anchor did not affect any context'
        for dialect in (0x10000, 0x40000, 0x30000, 0x130000):
            other = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
            try:
                other.set_parameter(PROCESS_TAGS, 1)
                other.set_parameter(SAMPLE_RATE, 3)
                set_probe(**common)
                ordinary = render(other, 'Salam?')
                set_probe(**common, a_target=target)
                assert render(other, 'Salam?') == ordinary, ('other language changed', dialect)
                count += 1
            finally:
                other.close()
        print(f'Passed {count} A-centre, transition and isolation checks. Salam target: {target}.')
    finally:
        engine.close()
        for key in (*KEYS, 'RELOQ_FA_TRACE_A'):
            os.environ.pop(key, None)


if __name__ == '__main__':
    main()
