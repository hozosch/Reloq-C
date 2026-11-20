"""Exercise the standalone C transition planner and its reference guards."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_persian_frames import NativeFrame


def row(voicing, aspiration, noise):
    item = NativeFrame()
    item.controls[0] = 1
    item.controls[2], item.controls[7], item.controls[8] = voicing, aspiration, noise
    return item


def word(phone, lead=14, burst=65):
    if phone in 'bdg':
        return ([row(52, 34, 0) for _ in range(lead)] +
                [row(20, 0, 0) for _ in range(25)] +
                [row(0, 0, burst)] + [row(50, 34, 0)] * 4)
    if phone in 'tpk':
        return ([row(52, 34, 0) for _ in range(lead)] +
                [row(0, 0, 0) for _ in range(32)] +
                [row(0, 0, burst)] + [row(0, 47, 50)] + [row(50, 34, 0)] * 4)
    if phone in 'fsSG':
        return ([row(52, 34, min(60, 4 * i)) for i in range(lead)] +
                [row(0, 0, 60)] * 5 + [row(50, 34, 0)] * 4)
    return ([row(52, 34, 0) for _ in range(lead)] +
            [row(0, 43, 0)] * 5 + [row(50, 34, 0)] * 4)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cc', default='cc')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='reloq-fa-onsets-') as directory:
        library = Path(directory) / 'transition.so'
        subprocess.run([args.cc, '-shared', '-fPIC', '-std=c11', '-O2', '-Wall', '-Wextra',
                        '-Werror', '-I', str(ROOT / 'include'), '-I', str(ROOT / 'src'),
                        str(ROOT / 'src/reloq_persian_transition.c'), '-lm', '-o', str(library)],
                       check=True)
        function = ctypes.CDLL(str(library)).reloq_persian_plan_unvoiced_onsets
        function.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t,
            ctypes.POINTER(ctypes.c_size_t), ctypes.c_size_t, ctypes.POINTER(ctypes.c_ubyte),
            ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t),
            ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_double),
            ctypes.POINTER(ctypes.c_size_t))
        function.restype = ctypes.c_int

        def check(phone, target, donor, keep, *, unvoiced_before=False, english_before=True):
            left = [row(53, 34, 0) for _ in range(9)] + [
                row(0, 0, 55) if unvoiced_before else row(53, 34, 0)]
            frames = (NativeFrame * (len(left) + len(target)))(*left, *target)
            reference = (NativeFrame * (len(left) + len(donor)))(*left, *donor)
            ends = (ctypes.c_size_t * 2)(len(left), len(frames))
            edges = (ctypes.c_size_t * 3)(0, len(left), len(reference))
            onsets = (ctypes.c_ubyte * 2)(0, ord(phone) | (128 if english_before else 0))
            counts, removed, total = (ctypes.c_size_t * 2)(), (ctypes.c_double * 2)(), ctypes.c_size_t()
            original = bytes(frames)
            assert function(frames, len(frames), ends, 2, onsets, reference, len(reference),
                            edges, counts, removed, ctypes.byref(total)) == 0
            assert counts[0] == len(left) and bytes(frames)[:10 * ctypes.sizeof(NativeFrame)] == original[:10 * ctypes.sizeof(NativeFrame)]
            if keep:
                assert counts[1] == len(target) and total.value == len(frames)
                assert removed[1] == 0 and bytes(frames) == original
            else:
                lead = next(i for i, frame in enumerate(target) if (
                    frame.controls[2] <= 30 and frame.controls[7] == 0 if phone in 'bdg'
                    else frame.controls[2] <= 0))
                assert counts[1] == len(target) - lead and total.value == len(frames) - lead
                assert removed[1] == sum(frame.controls[0] for frame in target[:lead])
                assert frames[10].controls[2] == (20 if phone in 'bdg' else 0)
                assert bytes(frames)[10 * ctypes.sizeof(NativeFrame):total.value * ctypes.sizeof(NativeFrame)] == original[(10 + lead) * ctypes.sizeof(NativeFrame):]

        for phone in 'tpkfsSh':
            check(phone, word(phone), word(phone, 16), True)
            check(phone, word(phone), word(phone, 30), False)
        # A superficially timed match may not erase or invent a burst.
        check('t', word('t'), word('t', burst=5), False)
        # A neutral donor supplies no contradictory consonant. Preserve only
        # a bounded target with an actual burst or continuous frication.
        check('p', word('p'), word('h'), True)
        check('f', word('f'), word('h'), True)
        check('p', word('p'), word('p', lead=0), False)
        check('f', word('f'), word('f', lead=0), False)
        check('f', word('f', lead=50), word('h'), False)
        discontinuous = word('f')
        discontinuous[0].controls[2] = 20
        check('f', discontinuous, word('h'), False)
        check('p', word('p', burst=5), word('h'), False)
        # After an actual voiceless English release, a stale voiced vowel is
        # not a permissible lead-in to a voiceless Persian consonant, even
        # if the separate donor also contains that stale material.
        for phone in 'tpkfsShG':
            check(phone, word(phone), word(phone), False, unvoiced_before=True)
            check(phone, word(phone), word(phone), True,
                  unvoiced_before=True, english_before=False)
        check('h', word('h', lead=1), word('h'), False, unvoiced_before=True)
        for phone in 'bdg':
            check(phone, word(phone), word(phone), False, unvoiced_before=True)
            check(phone, word(phone), word(phone), True)
            check(phone, word(phone), word(phone), True,
                  unvoiced_before=True, english_before=False)
            check(phone, word(phone, burst=5), word(phone), True, unvoiced_before=True)
        print('Passed native onset planning, matching/mismatched references and preserved source frames.')


if __name__ == '__main__':
    main()
