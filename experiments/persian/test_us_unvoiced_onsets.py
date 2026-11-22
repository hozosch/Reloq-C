"""Verify that shared consonant joins use genuine, bounded US source frames."""
from __future__ import annotations

import argparse
from bisect import bisect_left
import ctypes
from pathlib import Path
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
import continuous_plan
from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import NativeFrame
from nvda_driver_probe import make_language_engine


def unit(library, symbol='eciPersianMatchUsUnvoicedOnset'):
    function = getattr(ctypes.CDLL(str(library)), symbol)
    function.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.c_size_t,
                         ctypes.c_size_t, ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                         ctypes.c_size_t, ctypes.c_size_t, ctypes.c_ubyte)
    function.restype = ctypes.c_int
    target = (NativeFrame * 60)()
    reference = (NativeFrame * 60)()
    for i in range(60):
        for array, voiced_until in ((target, 22), (reference, 13)):
            row = array[i]
            row.tick = i
            row.controls[0], row.controls[1] = 1, 120 + i
            row.controls[2] = 50 if i < voiced_until else 0
            row.controls[7] = 30 if i < voiced_until else 0
            row.controls[8] = min(55, max(0, (i-5)*5))
            for col in range(35, 43):
                row.controls[col] = 20
    before, donor = bytes(target), bytes(reference)
    assert function(target, 60, 5, 60, reference, 60, 5, 60, ord('f')) == 1
    old = (NativeFrame * 60).from_buffer_copy(before)
    for i in range(60):
        assert target[i].tick == old[i].tick
        for col in range(62):
            if 5 <= i < 22 and col in (2, 7, 8, *range(35, 43)):
                assert target[i].controls[col] == reference[i].controls[col], (i, col)
            else:
                assert target[i].controls[col] == old[i].controls[col], (i, col)
    assert bytes(reference) == donor
    # An incompatible noise profile is not a reference for this F. No
    # partial edits are permitted, even after the target has gone unvoiced.
    target = (NativeFrame * 60).from_buffer_copy(before)
    for row in reference:
        row.controls[38] = 70
    assert function(target, 60, 5, 60, reference, 60, 5, 60, ord('f')) == 0
    assert bytes(target) == before
    assert function(target, 60, 5, 60, target, 60, 5, 60, ord('f')) == -1
    assert function(target, 60, 0, 60, reference, 60, 5, 60, ord('f')) == -1
    assert function(target, 60, 5, 60, reference, 60, 5, 60, 0) == -1
    assert function(target, 60, 5, 60, reference, 60, 5, 60, ord('r')) == -1
    reference[5].controls[0] = 0
    assert function(target, 60, 5, 60, reference, 60, 5, 60, ord('f')) == -1
    assert bytes(target) == before


def integration(library):
    voice = make_language_engine(library, library, None, Path('/unavailable'), Path('/unavailable'))
    original = continuous_plan.match_us_unvoiced_onsets
    applied = []
    def observe(plan, parts, en, rate):
        before = [tuple(row.controls) for row in plan.frames]
        counts, marks = plan.counts, plan.source_marks
        donor = plan.contexts['en-us']
        donor_before = bytes(donor.frames)
        result = original(plan, parts, en, rate)
        assert (plan.counts, plan.source_marks, bytes(donor.frames)) == (counts, marks, donor_before)
        bounds = continuous_plan._edges_by_count(counts)
        for edge in result:
            at = bounds[edge-1]
            first = bisect_left(donor.ticks, round(plan.edges['en-us'][edge]*1000/rate))
            last = bisect_left(donor.ticks, round(plan.edges['en-us'][edge+1]*1000/rate))
            elapsed, ref_time, ref = 0.0, 0.0, first
            differences = 0
            for i in range(len(plan.frames)):
                row = plan.frames[i]
                if at <= i < bounds[edge]:
                    while ref + 1 < last and ref_time + donor.frames[ref].controls[0] <= elapsed + 1e-6:
                        ref_time += donor.frames[ref].controls[0]
                        ref += 1
                    for col in (2, 7, 8, *range(35, 43)):
                        if row.controls[col] != before[i][col]:
                            assert row.controls[col] == donor.frames[ref].controls[col], (i, col)
                            differences += 1
                    elapsed += row.controls[0]
                for col in range(62):
                    if at <= i < bounds[edge] and col in (2, 7, 8, *range(35, 43)):
                        continue
                    assert row.controls[col] == before[i][col], (i, col)
            assert differences
            applied.append((parts[edge-1].text, parts[edge].text))
        return result
    checked = 0
    try:
        with patch.object(continuous_plan, 'match_us_unvoiced_onsets', new=observe):
            for rate, gender, speed in ((1,0,40), (3,0,82), (3,1,100), (6,1,140)):
                for key, value in ((SAMPLE_RATE,rate), (GENDER,gender), (NATIVE_SPEED,speed)):
                    voice.set_parameter(key,value)
                for english in ('NVDA', 'internet'):
                    for persian in ('فایل', 'سلام', 'شماره', 'کتاب', 'تست', 'پر', 'هوا'):
                        pcm, marks = bytearray(), []
                        assert voice.speak((EciIndex(11), english + ' ' + persian, EciIndex(12)),
                                           pcm.extend, marks.append)
                        assert pcm and marks == [11,12], (english,persian,marks)
                        checked += 1
        assert any('f' in part.lower() for _,part in applied), applied
    finally:
        voice.close()
    print(f'Passed {checked} mixed US consonant joins, {len(applied)} verified US source transfers.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--unit-symbol', default='eciPersianMatchUsUnvoicedOnset')
    parser.add_argument('--unit-only', action='store_true')
    args = parser.parse_args()
    library = args.library.resolve()
    unit(library, args.unit_symbol)
    if not args.unit_only:
        integration(library)


if __name__ == '__main__':
    main()
