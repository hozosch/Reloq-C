"""Exercise real German and Japanese fricatives at small female head sizes."""
from __future__ import annotations

import argparse
import array
import ctypes
import math
from pathlib import Path

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import FrameCapture, FrameReplay, NativeFrame

CASES = (('German', 0x40000, 'sechs süße Sachen'),
         ('Japanese', 0x80000, 'すし さしすせそ'))


def power(samples, rate, frequency):
    coefficient = 2 * math.cos(2 * math.pi * frequency / rate)
    a = b = 0.0
    for index, sample in enumerate(samples):
        window = .5 - .5 * math.cos(2 * math.pi * index / (len(samples) - 1))
        c = sample * window + coefficient * a - b
        b, a = a, c
    return max(1e-10, a*a + b*b - coefficient*a*b)


def prominence(engine, text, rate):
    captured = FrameCapture(engine._dll).capture(engine, (text,))
    frames = [r for r in captured.frames() if r.controls[8] >= 45 and r.controls[2] <= 1
              and sum(r.controls[i] for i in (37, 38, 39)) >= 40]
    assert frames, text
    mid = frames[len(frames) // 3]
    assert mid.controls[39] > 0 and mid.controls[48] == 0
    raw = ctypes.string_at(ctypes.byref(mid), ctypes.sizeof(mid))
    repeated = (NativeFrame * 1300)()
    for index, row in enumerate(repeated):
        ctypes.memmove(ctypes.byref(row), raw, len(raw))
        row.tick = index
        row.controls[0] = 1
    pcm = bytearray()
    assert FrameReplay(engine._dll).replay(engine, repeated, (), pcm.extend, lambda _: None)
    samples = array.array('h'); samples.frombytes(pcm)
    data = samples[rate // 5:rate // 5 + 4096]
    assert len(data) == 4096
    frequencies = (4300, 4400, 4500, 4600, 4700, 4800, 4900)
    values = [power(data, rate, freq) for freq in frequencies]
    return 10 * math.log10(values[3] / (sum(values[:3] + values[4:]) / 6))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--previous', type=Path)
    args = parser.parse_args()
    checks = 0
    for name, dialect, sample in CASES:
        engine, scope = voice(args.candidate, dialect)
        old, old_scope = voice(args.previous, dialect) if args.previous else (None, None)
        try:
            for v in (engine, old):
                if v:
                    v.set_parameter(NATIVE_SPEED, 40)
                    v.set_parameter(GENDER, 1)
            for head in (0, 15):
                for v in (engine, old):
                    if v: v.set_parameter(HEAD_SIZE, head)
                for code in (2, 4, 5, 6):
                    for v in (engine, old):
                        if v: v.set_parameter(SAMPLE_RATE, code)
                    level = prominence(engine, sample, SAMPLE_RATES[code])
                    assert level < 9, (name, head, code, level)
                    if old:
                        previous = prominence(old, sample, SAMPLE_RATES[code])
                        assert previous - level > 10, (name, head, code, previous, level)
                    checks += 1
                if old:
                    for v in (engine, old): v.set_parameter(SAMPLE_RATE, 3)
                    assert capture(engine, scope, sample, False)[0].pcm == capture(old, old_scope, sample, False)[0].pcm
                    checks += 1
        finally:
            engine.close()
            if old: old.close()
    if args.previous:
        a, sa = voice(args.candidate, 0x10000)
        b, sb = voice(args.previous, 0x10000)
        try:
            for rate in (3, 2, 4, 5, 6):
                for v in (a,b):
                    v.set_parameter(GENDER, 1); v.set_parameter(HEAD_SIZE, 15); v.set_parameter(SAMPLE_RATE, rate)
                assert capture(a, sa, 'six slippery stones', False)[0].pcm == capture(b, sb, 'six slippery stones', False)[0].pcm
                checks += 1
        finally:
            a.close(); b.close()
    print(f'Passed {checks} small-female-head whistle and isolation checks.')


if __name__ == '__main__':
    main()
