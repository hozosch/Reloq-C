"""Compare native Korean-to-Persian pitch transfer with the listening probe."""
from __future__ import annotations

import argparse
from bisect import bisect_left
import ctypes
from pathlib import Path
from statistics import median

from compare_korean_contour import CASES
from hybrid_fa_frontend import DirectFirstPersian
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import NativeFrame


def reference(rows, donor):
    fa = [(r.tick, r.controls[1]) for r in rows if r.controls[2] > 10 and r.controls[1] > 0]
    ko = [(r.tick, r.controls[1]) for r in donor if r.controls[2] > 10 and r.controls[1] > 0]
    ticks, pitches = zip(*ko)
    start, end = fa[0][0], fa[-1][0]

    def mapped(tick):
        fraction = max(0.0, min(1.0, (tick - start) / (end - start)))
        at = ticks[0] + fraction * (ticks[-1] - ticks[0])
        index = bisect_left(ticks, at)
        if index == 0:
            return pitches[0]
        if index == len(ticks):
            return pitches[-1]
        part = (at - ticks[index - 1]) / (ticks[index] - ticks[index - 1])
        return pitches[index - 1] + part * (pitches[index] - pitches[index - 1])

    scale = median(value for _, value in fa) / median(mapped(tick) for tick, _ in fa)
    return [mapped(row.tick) * scale if row.controls[1] > 0 else row.controls[1] for row in rows]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--frame-library', type=Path)
    args = parser.parse_args()
    dll = ctypes.CDLL(str((args.frame_library or args.library).resolve()))
    transfer = (getattr(dll, 'eciPersianTransferKoreanPitch', None) or
                getattr(dll, 'reloq_fa_transfer_korean_pitch'))
    transfer.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                         ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.c_uint, ctypes.c_int64)
    transfer.restype = ctypes.c_int
    aligned = (getattr(dll, 'eciPersianTransferKoreanAligned', None) or
               getattr(dll, 'reloq_fa_transfer_korean_aligned'))
    aligned.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                        ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                        ctypes.POINTER(ctypes.c_int64), ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t)
    aligned.restype = ctypes.c_int
    # Two equally syllabic donor words have a break at tick 100. The first
    # Persian word is deliberately twice as long as the second: its word
    # break must meet the donor break, not two thirds of the donor phrase.
    donor = (NativeFrame * 201)()
    for i, row in enumerate(donor):
        row.tick = i
        row.controls[1] = 800.0 + i
        row.controls[2] = 40.0 if i < 90 or i > 110 else 0.0
    ticks = (ctypes.c_int64 * 1)(200)
    syllables = (ctypes.c_uint32 * 2)(1, 1)
    targets = []
    for initial_pitch in (500.0, 1600.0):
        rows = (NativeFrame * 301)()
        for i, row in enumerate(rows):
            row.tick = i
            row.controls[1], row.controls[2], row.controls[9] = initial_pitch, 40.0, 432.0
        assert aligned(rows, len(rows), donor, len(donor), ticks, syllables, 2) == 0
        assert abs(rows[200].controls[1] - 900.0) < 0.001
        assert all(800 <= row.controls[1] <= 1000 and row.controls[9] == 432 for row in rows)
        targets.append(bytes(rows))
    assert targets[0] == targets[1], 'Finnish pitch leaked into the Korean transfer'
    invalid = (NativeFrame * 2)()
    invalid[0].tick, invalid[1].tick = 1, 1
    original_invalid = bytes(invalid)
    assert aligned(invalid, 2, donor, len(donor), None, syllables, 2) < 0
    assert bytes(invalid) == original_invalid
    frontend = DirectFirstPersian(args.library, None, None, None)
    fa, fa_scope = voice(args.library, 0x90000)
    ko, ko_scope = voice(args.library, 0xa0000)
    tested = 0
    try:
        for rate, gender, speed in ((1, 0, 65), (3, 0, 65), (6, 1, 98)):
            for engine in (fa, ko):
                for parameter, value in ((SAMPLE_RATE, rate), (NATIVE_SPEED, speed), (GENDER, gender)):
                    engine.set_parameter(parameter, value)
            for text, korean in CASES:
                analysis = frontend.analyze_direct(text)
                assert analysis is not None, text
                _, target = capture(fa, fa_scope, analysis.parts[0].text, True)
                _, donor = capture(ko, ko_scope, korean, False)
                expected = reference(target, donor)
                original = bytes(target)
                assert transfer(target, len(target), donor, len(donor), 0, 45) == 0
                actual = bytes(target)
                for index, (row, pitch) in enumerate(zip(target, expected)):
                    assert abs(row.controls[1] - pitch) < 0.0001, (text, index, row.controls[1], pitch)
                # F0 is the only changed frame field.
                before = (NativeFrame * len(target)).from_buffer_copy(original)
                for a, b in zip(before, target):
                    a.controls[1] = b.controls[1]
                assert bytes(before) == actual
                # Switch-edge fading must leave the first and last voiced
                # pitch unchanged and change some interior target pitch.
                faded = (NativeFrame * len(target)).from_buffer_copy(original)
                assert transfer(faded, len(faded), donor, len(donor), 3, 45) == 0
                voiced = [i for i, row in enumerate(faded) if row.controls[2] > 10 and row.controls[1] > 0]
                source = (NativeFrame * len(target)).from_buffer_copy(original)
                assert faded[voiced[0]].controls[1] == source[voiced[0]].controls[1]
                assert faded[voiced[-1]].controls[1] == source[voiced[-1]].controls[1]
                assert any(a.controls[1] != b.controls[1] for a, b in zip(faded, source))
                tested += 1
        silent = (NativeFrame * 4)()
        old = bytes(silent)
        assert transfer(silent, len(silent), silent, len(silent), 0, 45) != 0
        assert bytes(silent) == old
    finally:
        frontend.close()
        fa.close()
        ko.close()
    print(f'Passed {tested} native contour transfers against the reviewed listening algorithm.')


if __name__ == '__main__':
    main()
