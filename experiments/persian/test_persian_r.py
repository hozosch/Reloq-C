"""Check Portuguese final R, restored Persian R and vowel isolation."""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from itertools import groupby
from pathlib import Path
import threading

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE


def frication(rows):
    return [r.controls for r in rows if r.controls[8] >= 40 and r.controls[37] >= 55]


def nucleus(rows):
    # The retained R has a breathier source; select the following vowel's
    # voiced stretch and examine its stable interior, not the transition.
    indices = [i for i, r in enumerate(rows) if r.controls[2] >= 45 and
               r.controls[7] < 40 and r.controls[8] < 1]
    assert len(indices) >= 12
    return rows[indices[(2 * len(indices)) // 3]].controls


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path, help='the pre-Japanese-R Persian engine')
    parser.add_argument('--previous-japanese', type=Path, help='the accepted Japanese nasal engine')
    args = parser.parse_args()
    barrier = threading.Barrier(4)
    def cold_start(_):
        engine, scope = voice(args.candidate, 0x90000)
        try:
            barrier.wait(timeout=15)
            return capture(engine, scope, '`[.1ar] `[.1ir]', True)[0].pcm
        finally:
            engine.close()
    with ThreadPoolExecutor(max_workers=4) as pool:
        outputs = list(pool.map(cold_start, range(4)))
    assert outputs and all(value == outputs[0] for value in outputs)

    fa, scope = voice(args.candidate, 0x90000)
    pt, pt_scope = voice(args.candidate, 0x70000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checks = 0
    try:
        for gender in (0, 1):
            for rate in (1, 3, 6):
                for speed in (40, 98, 170):
                    for engine in (fa, pt, old):
                        if engine:
                            for key, value in ((GENDER, gender), (SAMPLE_RATE, rate),
                                               (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    for vowel in 'aeoiu':
                        tag = '`[.1' + vowel + 'r]'
                        _, rows = capture(fa, scope, tag, True)
                        _, donor = capture(pt, pt_scope, tag, False)
                        current, reference = frication(rows), frication(donor)
                        assert current and reference and len(current) == len(reference), (
                            gender, rate, speed, tag, len(current), len(reference))
                        for left, right in zip(current, reference):
                            for col in (2, 7, 8, 9, 13, 15, 17, 35, 36, 37, 38):
                                assert abs(left[col]-right[col]) <= 2, (gender, rate, speed, tag, col)
                        checks += 1
                    for text in ('`[.1sefr]', '`[.1pa]', '`[.1i]', '`[.1u]'):
                        current, _ = capture(fa, scope, text, True)
                        if old:
                            before, _ = capture(old, old_scope, text, True)
                            assert current.pcm == before.pcm, (gender, rate, speed, text)
                        checks += 1
                    # R initial stays the previous R; its adjacent vowel has
                    # its own neutral targets, checked independently below.
                    _, current = capture(fa, scope, '`[.1ra]', True)
                    if old:
                        _, before = capture(old, old_scope, '`[.1ra]', True)
                        for a, b in zip(current[:5], before[:5]):
                            for col in (2, 7, 8, 9, 13, 15, 17):
                                assert abs(a.controls[col]-b.controls[col]) < 1
                    checks += 1
        for gender in (0, 1):
            for engine in (fa, old):
                if engine:
                    for key, value in ((GENDER, gender), (SAMPLE_RATE, 3), (NATIVE_SPEED, 65)):
                        engine.set_parameter(key, value)
            for vowel in 'iu':
                _, plain = capture(fa, scope, '`[.1' + vowel + ']', True)
                _, after = capture(fa, scope, '`[.1r' + vowel + ']', True)
                target, actual = nucleus(plain), nucleus(after)
                for col, tolerance in ((9, 30), (13, 100), (15, 150), (17, 200)):
                    assert abs(target[col]-actual[col]) <= tolerance, (gender, vowel, col, target[col], actual[col])
                checks += 1
                _, between = capture(fa, scope, '`[.1' + vowel + 'r.0' + vowel + ']', True)
                indices = [i for i, r in enumerate(between) if r.controls[2] >= 45 and
                           r.controls[7] < 40 and r.controls[8] < 1]
                runs = [list(group) for _, group in groupby(enumerate(indices), lambda x: x[1]-x[0])]
                runs = [[item[1] for item in run] for run in runs if len(run) >= 8]
                assert len(runs) >= 2, (gender, vowel, 'missing vowel on either side of R')
                for run in (runs[0], runs[-1]):
                    sample = between[run[(2*len(run))//3]].controls
                    for col, tolerance in ((9, 45), (13, 150), (15, 200), (17, 250)):
                        assert abs(target[col]-sample[col]) <= tolerance, (
                            gender, vowel, col, target[col], sample[col])
                    checks += 1
    finally:
        fa.close(); pt.close()
        if old:
            old.close()
    if args.previous_japanese:
        now, ns = voice(args.candidate, 0x80000)
        before, bs = voice(args.previous_japanese, 0x80000)
        try:
            for text in ('3', '4', 'さん', 'よん', '`[.1aN.0ha]'):
                a, _ = capture(now, ns, text, False)
                b, _ = capture(before, bs, text, False)
                assert a.pcm == b.pcm, text
                checks += 1
        finally:
            now.close(); before.close()
    print(f'Passed {checks} Brazilian final-R, Persian vowel and language-isolation checks.')


if __name__ == '__main__':
    main()
