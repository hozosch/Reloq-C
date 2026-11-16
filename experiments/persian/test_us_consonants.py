"""Compare actual Persian consonant trajectories with the US provider.

The long /i/ context gives both providers an equivalent neighboring vowel.
This checks rendered control frames rather than copied rule constants.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import threading

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE

VOWEL_NUCLEI = {
    'i': lambda x: x[9] < 400 and x[13] > 1900,
    'u': lambda x: x[9] < 400 and x[13] < 1550,
    'o': lambda x: 350 < x[9] < 600 and x[13] < 1500,
    'e': lambda x: 400 < x[9] < 700 and x[13] > 1400,
    'A': lambda x: x[9] > 500 and x[13] > 1300,
    'a': lambda x: x[9] > 500 and x[13] < 1550,
}


def consonant(engine, scope, phone: str, onset: bool, persian: bool, context='i'):
    vowel = ("'i:'" if persian else 'i') if context == 'i' else (
        'E' if context == 'e' and not persian else context)
    tag = f'`[.1{phone}{vowel}]' if onset else f'`[.1{vowel}{phone}]'
    _, frames = capture(engine, scope, tag, persian)
    runs = []
    start = None
    for i in range(len(frames) + 1):
        c = frames[i].controls if i < len(frames) else None
        nuclear = c is not None and c[2] > 25 and c[8] < 2 and VOWEL_NUCLEI[context](c)
        if nuclear and start is None:
            start = i
        elif not nuclear and start is not None:
            if i - start >= 5:
                runs.append((start, i))
            start = None
    assert runs, ('cannot locate vowel nucleus', tag)
    begin, end = max(runs, key=lambda pair: pair[1] - pair[0])
    side = list(frames[:begin] if onset else frames[end:])
    noise = [r for r in side if r.controls[8] > 5]
    assert noise, ('missing consonant noise', tag)
    peak = max(noise, key=lambda row: row.controls[8])
    aspiration = sum(row.controls[7] > 10 and row.controls[2] < 5 for row in side)
    return len(noise), aspiration, tuple(peak.controls)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    args = parser.parse_args()
    # All four threads enter the imported US rules for the first time at
    # once. This checks the one-time, shared arena copy as well as the
    # request-local acoustic scope and capture storage.
    barrier = threading.Barrier(4)
    def first_speech(_):
        engine, local_scope = voice(args.candidate, 0x90000)
        try:
            barrier.wait(timeout=15)
            return capture(engine, local_scope, '`[.1pi.0tA.0ki.0Ca]', True)[0].pcm
        finally:
            engine.close()
    with ThreadPoolExecutor(max_workers=4) as pool:
        output = list(pool.map(first_speech, range(4)))
    assert all(sample == output[0] for sample in output), 'concurrent first-use output differs'
    fa, fa_scope = voice(args.candidate, 0x90000)
    us, us_scope = voice(args.candidate, 0x10000)
    checks = 0
    try:
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 98, 170):
                    for e in (fa, us):
                        for p, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                         (NATIVE_SPEED, speed)):
                            e.set_parameter(p, value)
                    for phone in 'ptkbdgfvszSZCJ':
                        for onset in (True, False):
                            a = consonant(fa, fa_scope, phone, onset, True)
                            b = consonant(us, us_scope, phone, onset, False)
                            context = (rate, gender, speed, phone, onset, a[:2], b[:2])
                            # Compare the complete affricate phase, including
                            # its closure/release, rather than testing /t/ and
                            # /sh/ independently.
                            # Initial /g/ retains a small measured timing
                            # difference at very slow speeds (up to 14 ms).
                            fraction = .25 if phone == 'g' and onset else .20
                            tolerance = 3 if phone in 'CJ' else max(8, int(b[0] * fraction))
                            assert abs(a[0] - b[0]) <= tolerance, context
                            for column in (8, 35, 36, 37, 38, 39, 40, 43):
                                assert abs(a[2][column] - b[2][column]) <= 7, (context, column)
                            if phone in 'ptk':
                                # US stops have their own aspiration after the
                                # burst, including word-final stops.
                                assert a[1] >= 3 and b[1] >= 3, context
                                assert abs(a[1] - b[1]) <= max(6, int(b[1] * .20)), context
                            checks += 1
        # Verify complete /tʃ/ and /dʒ/, and the stop aspiration, on both
        # edges of all six vowel contexts. Affricate closure and frication
        # must not be replaced by an isolated /sh/ or /zh/ approximation.
        for gender in (0, 1):
            for speed in (40, 98, 170):
                for e in (fa, us):
                    for p, value in ((SAMPLE_RATE, 3), (GENDER, gender),
                                     (NATIVE_SPEED, speed)):
                        e.set_parameter(p, value)
                for phone in 'CJptk':
                    for context in 'Aaeou':
                        for onset in (True, False):
                            a = consonant(fa, fa_scope, phone, onset, True, context)
                            b = consonant(us, us_scope, phone, onset, False, context)
                            desc = (gender, speed, phone, context, onset)
                            assert abs(a[0] - b[0]) <= 3, (desc, a[:2], b[:2])
                            if phone in 'CJ':
                                for column in (8, 35, 36, 37, 38, 39, 40, 43):
                                    assert abs(a[2][column] - b[2][column]) <= 3, (desc, column)
                            else:
                                assert a[1] >= 3 and b[1] >= 3, (desc, a[:2], b[:2])
                                assert abs(a[1] - b[1]) <= 3, (desc, a[:2], b[:2])
                            checks += 1
    finally:
        fa.close()
        us.close()
    print(f'Passed {checks} onset/coda US consonant and aspiration comparisons.')


if __name__ == '__main__':
    main()
