"""Compare native US/Persian word contour transfer to the established planner."""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
import random
import sys
from types import SimpleNamespace

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_persian_frames import NativeFrame, NativeSyllable
from continuous_plan import CapturedRows, fit_us_word_pitch, indexed_us_proxy


def rows(seed, lengths, syllables, *, silent=False):
    rng = random.Random(seed)
    count = sum(lengths)
    data = (NativeFrame * count)()
    for tick, row in enumerate(data):
        row.tick = tick
        row.controls[1] = 95 + 20 * rng.random() + tick * .04
        row.controls[2] = 0 if silent else (0 if tick % 97 < 7 else 45)
        row.controls[9] = 500 + 25 * rng.random()
    markers, pos = {}, 0
    for i, length in enumerate(lengths[:-1], 1):
        pos += length
        markers[i] = pos
    return CapturedRows(bytes(count * 2), markers, data, list(range(count)), syllables=syllables)


def clone(item):
    frames = (NativeFrame * len(item.frames)).from_buffer_copy(item.frames)
    return CapturedRows(item.pcm, dict(item.indexes), frames, item.ticks, syllables=item.syllables)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    dll = ctypes.CDLL(str(args.library.resolve()))
    dense = dll.eciPersianDenseCurve
    dense.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.c_uint,
                      ctypes.POINTER(ctypes.c_float), ctypes.c_size_t)
    fit = dll.eciPersianFitPitch
    fit.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.POINTER(ctypes.c_float),
                    ctypes.c_size_t, ctypes.c_int64, ctypes.c_int64, ctypes.c_int64, ctypes.c_int64)
    anchors = dll.eciPersianFitPitchAnchors
    anchors.argtypes = (*fit.argtypes, ctypes.POINTER(ctypes.c_int64),
                        ctypes.POINTER(ctypes.c_int64), ctypes.c_size_t)
    transfer = dll.eciPersianTransferUsWords
    transfer.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.POINTER(NativeFrame),
                         ctypes.c_size_t, ctypes.POINTER(ctypes.c_int64), ctypes.POINTER(ctypes.c_int64),
                         ctypes.c_size_t, ctypes.POINTER(NativeSyllable), ctypes.c_size_t,
                         ctypes.POINTER(NativeSyllable), ctypes.c_size_t, ctypes.POINTER(ctypes.c_uint32))
    fast = SimpleNamespace(dense_curve=dense, fit_pitch=fit, fit_pitch_anchors=anchors,
                           transfer_us_words=transfer)
    reference = SimpleNamespace(dense_curve=dense, fit_pitch=fit, fit_pitch_anchors=anchors,
                                transfer_us_words=None)
    checks = 0
    for seed in range(24):
        lengths = (100 + seed % 3, 110, 145)
        edge1, edge2 = lengths[0], lengths[0] + lengths[1]
        target_syllables = ((100000, 400000), (450000, (edge1-5)*10000),
                            ((edge1+10)*10000, (edge2-10)*10000),
                            ((edge2+5)*10000, (edge2+55)*10000),
                            ((edge2+60)*10000, (sum(lengths)-5)*10000))
        donor_syllables = ((50000, 350000), (400000, (edge1-8)*10000),
                           ((edge1+8)*10000, (edge2-9)*10000),
                           ((edge2+7)*10000, (edge2+50)*10000),
                           ((edge2+55)*10000, (sum(lengths)-4)*10000))
        for mode in ('groups', 'missing', 'incomplete', 'reset', 'no_counts', 'silent'):
            ts, ds = target_syllables, donor_syllables
            counts = (2, 1, 2)
            if mode == 'missing': ts, ds = (), ()
            if mode == 'incomplete': ts, ds = ts[:-1], ds[:-1]
            if mode == 'reset':
                ts = (*ts[:3], (50000, 550000), (600000, 1100000))
                ds = (*ds[:3], (70000, 500000), (550000, 1050000))
            if mode == 'no_counts': counts = ()
            source = rows(seed, lengths, ts)
            donor = rows(seed+100, lengths, ds, silent=mode == 'silent')
            a, b = clone(source), clone(source)
            marks = (1, 2)
            ra = fit_us_word_pitch(a, (0, sum(lengths)), donor, (0, sum(lengths)),
                                   marks, marks, 1000, fast, syllable_counts=counts)
            rb = fit_us_word_pitch(b, (0, sum(lengths)), donor, (0, sum(lengths)),
                                   marks, marks, 1000, reference, syllable_counts=counts)
            assert ra == rb, (seed, mode, ra, rb)
            assert bytes(a.frames) == bytes(b.frames), (seed, mode, 'frame mismatch')
            checks += 1
    if getattr(dll, 'eciNewEx', None) is not None:
        from _reloq_eci_direct import EciIndex, GENDER, SAMPLE_RATE, SAMPLE_RATES
        from _reloq_persian_frames import FrameCapture
        from test_us_articulation import voice
        fa, scope = voice(args.library, 0x90000)
        en, _ = voice(args.library, 0x10000)
        try:
            for rate in (1, 3, 6):
                for gender in (0, 1):
                    for engine in (fa, en):
                        engine.set_parameter(SAMPLE_RATE, rate)
                        engine.set_parameter(GENDER, gender)
                    for first, second in (('`[.0sA.1lam]', '`[.0don.1ja]'),
                                          ('`[.1ketab]', '`[.0mA.1nA]'),
                                          ('`[.1mAn]', '`[.0sA.1da]')):
                        tagged = first + ' ' + second + '?'
                        old = scope(1)
                        try:
                            target = FrameCapture(fa._dll).capture(
                                fa, (first, ' ', EciIndex(1), second, '?'), audio=False)
                        finally:
                            scope(old)
                        pilot = FrameCapture(en._dll).capture(
                            en, indexed_us_proxy(tagged, en, (1,)), audio=False)
                        item = CapturedRows.from_capture(target)
                        donor = CapturedRows.from_capture(pilot)
                        a, b = clone(item), clone(item)
                        bounds, donor_bounds = (0, len(item.pcm)//2), (0, len(donor.pcm)//2)
                        counts = (first.count('.'), second.count('.'))
                        ra = fit_us_word_pitch(a, bounds, donor, donor_bounds, (1,), (1,),
                                               SAMPLE_RATES[rate], fast, syllable_counts=counts)
                        rb = fit_us_word_pitch(b, bounds, donor, donor_bounds, (1,), (1,),
                                               SAMPLE_RATES[rate], reference, syllable_counts=counts)
                        assert ra == rb and bytes(a.frames) == bytes(b.frames), (rate, gender, tagged)
                        checks += 1
        finally:
            fa.close()
            en.close()
    print(f'Passed {checks} native versus established US/Persian contour plans.')


if __name__ == '__main__': main()
