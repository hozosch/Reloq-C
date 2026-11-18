"""Check scoped Japanese short-R articulation in the Persian provider."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import threading

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def first_voiced(rows):
    return next(row.controls for row in rows if row.controls[2] > 10)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    barrier = threading.Barrier(4)
    def cold_start(_):
        engine, scope = voice(args.candidate, 0x90000)
        try:
            barrier.wait(timeout=15)
            return capture(engine, scope, '`[.1ra] `[.1ara]', True)[0].pcm
        finally:
            engine.close()
    with ThreadPoolExecutor(max_workers=4) as pool:
        outputs = list(pool.map(cold_start, range(4)))
    assert outputs and all(x == outputs[0] for x in outputs), 'concurrent R initialization differs'

    fa, scope = voice(args.candidate, 0x90000)
    ja, japanese_scope = voice(args.candidate, 0x80000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        for gender in (0, 1):
            for rate in (1, 3, 6):
                for speed in (40, 98, 170):
                    for engine in (fa, ja, old):
                        if engine:
                            for key, value in ((GENDER, gender), (SAMPLE_RATE, rate), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    current, rows = capture(fa, scope, '`[.1ra]', True)
                    _, reference = capture(ja, japanese_scope, '`[.1ra]', False)
                    a, b = first_voiced(rows), first_voiced(reference)
                    # Same short-R voicing level and four main resonances as
                    # the Japanese rule. The ordinary male breathiness differs
                    # between the two language providers and stays native.
                    for col in (2, 9, 13, 15, 17):
                        assert abs(a[col] - b[col]) <= 2, (gender, rate, speed, col, a[col], b[col])
                    if old:
                        before, baseline = capture(old, old_scope, '`[.1ra]', True)
                        assert current.pcm != before.pcm and len(current.pcm) == len(before.pcm)
                        assert len(rows) == len(baseline)
                    checks += 1
                    for tag in ('`[.1ara]', '`[.1ma] `[.1ra]'):
                        now, rr = capture(fa, scope, tag, True)
                        if old:
                            before, baseline = capture(old, old_scope, tag, True)
                            assert now.pcm != before.pcm, (gender, rate, speed, tag, 'no short-R change')
                            assert len(now.pcm) == len(before.pcm) and len(rr) == len(baseline)
                        checks += 1
                    # Long and word-final R still use the existing rules;
                    # the vocalic and consonant-cluster endings have their
                    # own separately labelled offline listening candidates.
                    for tag in ('`[.1ar]', '`[.1sefr]', '`[.1arra]', '`[.1kar.0re]'):
                        now, _ = capture(fa, scope, tag, True)
                        if old:
                            before, _ = capture(old, old_scope, tag, True)
                            assert now.pcm == before.pcm, (gender, rate, speed, tag)
                        checks += 1
                    # Only ordinary r is adapted; Persian R is the separate
                    # uvular carrier and must still render without failure.
                    capture(fa, scope, '`[.1Ra]', True)
                    checks += 1
        # Contextual Japanese rules examine neighboring phone feature enums.
        # Exercise all six Persian vowels after R and a range of preceding
        # consonants and vowels, including the separate /x/ phone.
        for gender in (0, 1):
            for speed in (40, 98, 170):
                for engine in (fa, old):
                    if engine:
                        for key, value in ((GENDER, gender), (SAMPLE_RATE, 3), (NATIVE_SPEED, speed)):
                            engine.set_parameter(key, value)
                for left in ('', 'a', 'i', 'u', 'm', 'n', 'p', 't', 'k', 's', 'f', 'l', 'G'):
                    for right in 'aAeoiu':
                        tag = '`[.1' + left + 'r' + right + ']'
                        current, rows = capture(fa, scope, tag, True)
                        if old:
                            before, baseline = capture(old, old_scope, tag, True)
                            assert len(current.pcm) == len(before.pcm) and len(rows) == len(baseline), (
                                gender, speed, tag, 'changed phone timing')
                        checks += 1
    finally:
        fa.close(); ja.close()
        if old:
            old.close()
    print(f'Passed {checks} Japanese/Persian R and isolation comparisons.')


if __name__ == '__main__':
    main()
