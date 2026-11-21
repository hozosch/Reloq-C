"""Check sample-clock and millisecond US contour alignment are identical."""
from __future__ import annotations
import argparse
import ctypes as c
import math
from pathlib import Path
import random
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_persian_frames import FrameCapture, NativeFrame


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = (c.WinDLL if sys.platform == 'win32' else c.CDLL)(str(args.library.resolve()))
    api = FrameCapture(dll)
    assert api.transfer_us_samples is not None and api.transfer_us_words is not None
    rng = random.Random(200037)
    frame_count = 480
    rows = (NativeFrame * frame_count)()
    donor = (NativeFrame * frame_count)()
    for i in range(frame_count):
        rows[i].tick = donor[i].tick = i
        rows[i].controls[1] = 110 + 5 * math.sin(i / 25)
        donor[i].controls[1] = 130 + 12 * math.sin(i / 37)
        rows[i].controls[2] = donor[i].controls[2] = 50
    checks = 0
    for rate in (8000, 11025, 16000, 22050, 32000, 44100, 48000):
        for trial in range(250):
            words = 1 + trial % 4
            # Non-integer positions, half-ms ties and repeated marks exercise
            # rounding, clamping and the non-alignment fallback.
            left = [0] + sorted(rng.randrange(1, rate * 430 // 1000)
                                for _ in range(words - 1)) + [rate * 51 // 100]
            right = [0] + sorted(rng.randrange(1, rate * 430 // 1000)
                                 for _ in range(words - 1)) + [rate * 51 // 100]
            if words > 1 and trial % 11 == 0:
                left[1] = left[0]
            a = (c.c_uint64 * (words + 1))(*left)
            b = (c.c_uint64 * (words + 1))(*right)
            ta = [round(value * 1000 / rate) for value in left]
            db = [round(value * 1000 / rate) for value in right]
            ta[-1] = min(ta[-1], rows[-1].tick + 1)
            db[-1] = min(db[-1], donor[-1].tick + 1)
            ta = (c.c_int64 * (words + 1))(*ta)
            db = (c.c_int64 * (words + 1))(*db)
            actual = (NativeFrame * frame_count).from_buffer_copy(rows)
            expected = (NativeFrame * frame_count).from_buffer_copy(rows)
            one = api.transfer_us_samples(actual, frame_count, donor, frame_count,
                                          a, b, words, rate, None, 0, None, 0, None)
            two = api.transfer_us_words(expected, frame_count, donor, frame_count,
                                        ta, db, words, None, 0, None, 0, None)
            assert one == two and bytes(actual) == bytes(expected), (rate, trial, one, two)
            checks += 1
    invalid = (c.c_uint64 * 2)(0, 2**64-1)
    before = bytes(rows)
    assert api.transfer_us_samples(rows, frame_count, donor, frame_count,
                                   invalid, invalid, 1, 16000, None, 0, None, 0, None) == -1
    assert api.transfer_us_samples(rows, frame_count, donor, frame_count,
                                   invalid, invalid, 1, 0, None, 0, None, 0, None) == -1
    assert bytes(rows) == before
    print(f'Passed {checks} sample-to-frame contour comparisons at seven rates.')

if __name__ == '__main__':
    main()
