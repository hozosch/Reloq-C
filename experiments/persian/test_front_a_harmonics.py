"""Compare front-A harmonics at matched pitches in different contexts."""
from __future__ import annotations

import argparse
import array
import ctypes
import math
from pathlib import Path

from test_us_articulation import capture, voice
from test_british_front_a import vowel
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameReplay, NativeFrame


def harmonics(engine, scope, row, selected, pitch):
    frames = (NativeFrame * 100)()
    raw = ctypes.string_at(ctypes.byref(row), ctypes.sizeof(row))
    for i in range(len(frames)):
        ctypes.memmove(ctypes.byref(frames, i * len(raw)), raw, len(raw))
        frames[i].tick = i
        frames[i].controls[1] = pitch * 10
    pcm = bytearray()
    previous = scope(int(selected))
    try:
        assert FrameReplay(engine._dll).replay(engine, frames, (), pcm.extend, lambda _: None)
    finally:
        scope(previous)
    samples = array.array('h')
    samples.frombytes(pcm)
    samples = samples[16 * 40:16 * 80]
    assert len(samples) == 640
    window = [s * (0.5 - 0.5 * math.cos(2 * math.pi * i / (len(samples) - 1)))
              for i, s in enumerate(samples)]
    powers = {}
    for hz in range(pitch, 6000, pitch):
        coefficient = 2 * math.cos(2 * math.pi * hz / 16000)
        a = b = 0.0
        for sample in window:
            c = sample + coefficient * a - b
            b, a = a, c
        powers[hz] = max(0.0, a * a + b * b - coefficient * a * b)
    total = sum(value for hz, value in powers.items() if hz >= 200)
    assert total > 0
    return {hz: 10 * math.log10(max(value / total, 1e-12))
            for hz, value in powers.items()}


def distance(actual, reference):
    errors = [(actual[hz] - value) ** 2 for hz, value in reference.items()
              if hz >= 1800 and value > -48]
    assert errors
    return sum(errors) / len(errors)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    fa, scope = voice(args.candidate, 0x90000)
    old, old_scope = voice(args.previous, 0x90000) if args.previous else (None, None)
    british, british_scope = voice(args.candidate, 0x10001)
    checks = 0
    try:
        for gender in (0, 1):
            for engine in (fa, old, british):
                if engine:
                    for param, value in ((SAMPLE_RATE, 3), (NATIVE_SPEED, 98), (GENDER, gender)):
                        engine.set_parameter(param, value)
            for fa_text in ("`[.1s'A:'d]", '`[.1bAt]', '`[.1GAt]', '`[.1sAGt]'):
                _, fa_rows = capture(fa, scope, fa_text, True)
                _, gb_rows = capture(british, british_scope, 'sad', False)
                old_row = None
                if old:
                    _, old_rows = capture(old, old_scope, fa_text, True)
                    old_row = vowel(old_rows)
                distances, prior = [], []
                for pitch in ((100, 120, 140) if not gender else (180, 200, 220)):
                    ref = harmonics(british, british_scope, vowel(gb_rows), False, pitch)
                    got = distance(harmonics(fa, scope, vowel(fa_rows), True, pitch), ref)
                    if old_row is not None:
                        before = distance(harmonics(old, old_scope, old_row, True, pitch), ref)
                        prior.append(before)
                    assert got < 5, (gender, fa_text, pitch, got)
                    distances.append(got)
                    checks += 1
                if prior:
                    # These figures aid comparison but cannot decide which
                    # vowel sounds right. In dev123 the accepted dev121 vowel
                    # is restored after the user rejected the closer match.
                    print('front vowel harmonic distance', gender, fa_text,
                          'candidate', sum(distances), 'previous', sum(prior))
    finally:
        fa.close()
        british.close()
        if old:
            old.close()
    print(f'Passed {checks} matched-pitch front-vowel harmonic comparisons.')


if __name__ == '__main__':
    main()
