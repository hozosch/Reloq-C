"""Check the short-vowel to D transition and its following H impulse."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE
from test_persian_r_after_i_contexts import capture
from test_us_articulation import voice


CASES = ('`[.1nad.0ha]', '`[.1nAd.0ha]', '`[.1ned.0ha]',
         '`[.1nid.0ha]', '`[.1nud.0ha]', '`[.1nA.1mad.0ha]')


def measurements(result, text):
    vowel = 3 if 'nA.1ma' in text else 1
    stop = vowel + 1
    following = stop + 1
    segments = result.syllables
    assert len(segments) > following, (text, segments)
    vow_start, vow_end = (x / 10000 for x in segments[vowel])
    stop_start, stop_end = (x / 10000 for x in segments[stop])
    next_start = segments[following][0] / 10000
    rows = result.frames()
    burst = max((row.controls[8] for row in rows if stop_start <= row.tick <= stop_end + 2), default=0)
    breath = max((row.controls[7] for row in rows if stop_end <= row.tick < next_start), default=0)
    assert burst >= 20 and breath >= 20, (text, burst, breath)
    return vow_start, vow_end, stop_start, stop_end, next_start


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    current, scope = voice(args.library, 0x90000)
    previous, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    checked = changed = 0
    try:
        for rate in (1, 3, 6):
            for gender, head in ((0, 50), (1, 22)):
                for speed in (40, 82, 140):
                    for engine in (current, previous):
                        if engine:
                            for key, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                               (HEAD_SIZE, head), (NATIVE_SPEED, speed)):
                                engine.set_parameter(key, value)
                    for text in CASES:
                        new = capture(current, scope, text)
                        a = measurements(new, text)
                        if previous:
                            old = capture(previous, old_scope, text)
                            b = measurements(old, text)
                            assert a[2:] == b[2:] and abs(a[0] - b[0]) <= 1, (text, rate, speed, a, b)
                            assert a[2] - a[1] >= b[2] - b[1], (text, rate, speed, a, b)
                            assert len(new.pcm) == len(old.pcm), (text, rate, speed)
                            source = lambda result: tuple(
                                (row.tick, *(row.controls[column] for column in (2, 7, 8)))
                                for row in result.frames() if a[2] <= row.tick < a[4])
                            assert source(new) == source(old), (text, rate, speed, 'D/H source changed')
                            changed += new.frame_bytes != old.frame_bytes
                        checked += 1
                    if previous:
                        for text in ('`[.1nat.0ha]', '`[.1nab.0ha]', '`[.1nag.0ha]', '`[.1nal.0ha]'):
                            new = capture(current, scope, text)
                            old = capture(previous, old_scope, text)
                            assert new.frame_bytes == old.frame_bytes and new.pcm == old.pcm, (text, rate, speed)
                            new = capture(current, scope, text, False)
                            old = capture(previous, old_scope, text, False)
                            assert new.frame_bytes == old.frame_bytes and new.pcm == old.pcm, (text, rate, speed)
        if previous:
            assert changed >= checked // 2, (changed, checked)
    finally:
        current.close()
        if previous:
            previous.close()
    print(f'Passed {checked} vowel-D-H burst and H-impulse checks ({changed} changed).')


if __name__ == '__main__':
    main()
