"""Compare the native Korean donor interval with the original planner."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import random
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_persian_frames import FrameCapture, NativeFrame
from korean_contour import _donor_span


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    api = FrameCapture(ctypes.CDLL(str(args.library.resolve())))
    assert api.donor_span is not None, 'native donor interval is not exported'
    rng = random.Random(49_139)
    for case in range(10000):
        length = rng.randrange(0, 251)
        frames = (NativeFrame * length)()
        tick = rng.randrange(-30, 30)
        for frame in frames:
            tick += rng.randrange(0, 5)
            frame.tick = tick
            frame.controls[1] = rng.choice((0, 0, 50, 100, float('nan')))
            frame.controls[2] = rng.choice((0, 10, 10.01, 40))
        leading, syllables, trailing = rng.randrange(0, 5), rng.randrange(1, 15), rng.randrange(0, 5)
        expected = _donor_span(frames, leading, syllables, trailing)
        first, last = ctypes.c_size_t(), ctypes.c_size_t()
        status = api.donor_span(frames, length, leading, syllables, trailing,
                                ctypes.byref(first), ctypes.byref(last))
        assert status == 0 and (first.value, last.value) == expected, (case, expected, first.value, last.value)
    extreme = (NativeFrame * 1)()
    extreme[0].tick = 1 << 40
    first, last = ctypes.c_size_t(), ctypes.c_size_t()
    assert api.donor_span(extreme, 1, 1, 1, 1, ctypes.byref(first), ctypes.byref(last)) == -1
    print('Passed 10000 native Korean donor selections against the original planner.')


if __name__ == '__main__':
    main()
