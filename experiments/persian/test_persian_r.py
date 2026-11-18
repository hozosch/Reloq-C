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


def r_after_f_interval(rows):
    # Locate the old voiced R immediately after the final unvoiced F. Keep
    # this test independent of optional audio-comparison packages such as
    # numpy, which are not installed on the Windows test runner.
    end = 1 + max(i for i, row in enumerate(rows)
                  if row.controls[2] > 1 or row.controls[7] > 5 or row.controls[8] > 5)
    noise = [i for i in range(end) if rows[i].controls[8] > 20 and rows[i].controls[2] < 15]
    runs = []
    for _, group in groupby(enumerate(noise), lambda p: p[1] - p[0]):
        values = [value for _, value in group]
        if len(values) >= 4:
            runs.append((values[0], values[-1] + 1))
    assert len(runs) >= 2
    start = next(i for i in range(runs[-1][1], end) if rows[i].controls[2] > 15)
    assert end - start >= 5
    return start, end


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
                        if old and vowel != 'u':
                            after_pcm, _ = capture(fa, scope, tag, True)
                            before_pcm, _ = capture(old, old_scope, tag, True)
                            assert after_pcm.pcm == before_pcm.pcm, (gender, rate, speed, tag)
                        checks += 1
                    for text in ('`[.1pa]', '`[.1i]', '`[.1u]'):
                        current, _ = capture(fa, scope, text, True)
                        if old and text != '`[.1u]':
                            before, _ = capture(old, old_scope, text, True)
                            assert current.pcm == before.pcm, (gender, rate, speed, text)
                        checks += 1
                    # The final R after F is a distinct mid-band fricative;
                    # neither periodic voicing nor voiced aspiration may be
                    # present, including in a build with no old reference.
                    _, cluster = capture(fa, scope, '`[.1sefr]', True)
                    r_noise = [row.controls for row in cluster
                               if row.controls[8] >= 35 and row.controls[35] < 5
                               and row.controls[36] < 5 and row.controls[37] >= 55
                               and 30 <= row.controls[38] < 55]
                    assert len(r_noise) >= 5, (gender, rate, speed, 'missing F-R frication')
                    assert all(c[2] == 0 and c[7] == 0 for c in r_noise), (gender, rate, speed, 'voiced F-R')
                    checks += 1
                    # A final R following any stop receives the same
                    # voiceless frication, including voiced /d/ and /b/.
                    for stop in 'ptkbdg':
                        _, cluster = capture(fa, scope, '`[.1se' + stop + 'r]', True)
                        r_noise = [row.controls for row in cluster
                                   if row.controls[8] >= 35 and row.controls[35] < 5
                                   and row.controls[36] < 5 and row.controls[37] >= 55
                                   and 30 <= row.controls[38] < 55
                                   and row.controls[2] == 0 and row.controls[7] == 0]
                        assert len(r_noise) >= 5 and all(c[2] == 0 and c[7] == 0 for c in r_noise), (
                            gender, rate, speed, stop, 'missing or voiced stop-R')
                        checks += 1
                    # The rejected global R resonance adjustment is removed.
                    # Keep the complete onset identical to the accepted
                    # pre-change Persian R, including both upper formants.
                    _, current = capture(fa, scope, '`[.1ra]', True)
                    for col, low, high in ((15, 1900, 2300 if not gender else 2800),
                                           (17, 2300, 2900 if not gender else 3400)):
                        assert low <= current[0].controls[col] < high, (
                            gender, rate, speed, 'restored R', col)
                    if old:
                        _, before = capture(old, old_scope, '`[.1ra]', True)
                        assert len(current) == len(before)
                        for a, b in zip(current[:5], before[:5]):
                            for col in (2, 7, 8, 9, 13, 15, 17):
                                assert abs(a.controls[col]-b.controls[col]) < 1
                        _, current = capture(fa, scope, '`[.1sefr]', True)
                        _, before = capture(old, old_scope, '`[.1sefr]', True)
                        assert len(current) == len(before)
                        start, end = r_after_f_interval(before)
                        for i in range(start, end):
                            c = current[i].controls
                            assert c[2] == 0 and c[7] == 0, (gender, rate, speed, i, c[2], c[7])
                            assert c[8] > 0, (gender, rate, speed, i, 'missing frication')
                        # The F remains its own unvoiced consonant.
                        for i in range(max(0, start-12), start):
                            for col in (2, 7, 8, 35, 36, 37, 38, 39, 40):
                                assert current[i].controls[col] == before[i].controls[col], (gender, rate, speed, i, col)
                        # An /f/ at the end of one word must not devoice the
                        # initial R of the next; nor may a non-final /fr/.
                        for tag in ('`[.1sef] `[.1ra]', '`[.1se.0fra]'):
                            _, a = capture(fa, scope, tag, True)
                            _, b = capture(old, old_scope, tag, True)
                            assert len(a) == len(b), (gender, rate, speed, tag)
                            for x, y in zip(a, b):
                                for col in (2, 7, 8):
                                    assert x.controls[col] == y.controls[col], (gender, rate, speed, tag, col)
                        # Rewriting the already-queued R must not remove the
                        # source or stable vowel in the following word.
                        for tag in ('`[.1sefr] `[.1a]', '`[.1sefr] `[.1ra]'):
                            _, a = capture(fa, scope, tag, True)
                            _, b = capture(old, old_scope, tag, True)
                            assert len(a) == len(b), (gender, rate, speed, tag)
                            i = next(i for i in range(len(b)-1, -1, -1)
                                     if b[i].controls[2] >= 45 and b[i].controls[7] < 40
                                     and b[i].controls[8] < 1)
                            for col in (2, 7, 8, 9, 13, 15, 17):
                                assert abs(a[i].controls[col]-b[i].controls[col]) < 1, (
                                    gender, rate, speed, tag, i, col)
                    checks += 1
        for gender in (0, 1):
            for engine in (fa, old):
                if engine:
                    for key, value in ((GENDER, gender), (SAMPLE_RATE, 3), (NATIVE_SPEED, 65)):
                        engine.set_parameter(key, value)
            if old:
                # Vowel centers remain identical to the pre-change engine;
                # only the adjacent R and its transition may be modified.
                for vowel in 'aeio':
                    _, now = capture(fa, scope, '`[.1r' + vowel + ']', True)
                    _, before = capture(old, old_scope, '`[.1r' + vowel + ']', True)
                    left, right = nucleus(now), nucleus(before)
                    for col in (9, 13, 15, 17):
                        assert abs(left[col]-right[col]) < 1, (gender, vowel, col)
                    checks += 1
            for vowel in 'iu':
                _, plain = capture(fa, scope, '`[.1' + vowel + ']', True)
                _, after = capture(fa, scope, '`[.1r' + vowel + ']', True)
                target, actual = nucleus(plain), nucleus(after)
                if old and vowel != 'u':
                    _, old_after = capture(old, old_scope, '`[.1r' + vowel + ']', True)
                    previous_nucleus = nucleus(old_after)
                    for col in (9, 13, 15, 17):
                        assert abs(previous_nucleus[col]-actual[col]) < 1, (gender, vowel, col)
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
