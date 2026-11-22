"""Use the verified US /z/ onset without changing timing or other sources."""
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


def make_rows(rise):
    rows = (NativeFrame * 64)()
    for i, row in enumerate(rows):
        c = row.controls
        c[0], c[1], c[2], c[7] = 1, 1200, 48, 0
        c[9], c[13], c[15] = 300, 1700, 2550
        value = max(0, min(65, round((i - 8) * 65 / rise)))
        c[8] = value
        c[38] = c[39] = round(value * 50 / 65)
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path)
    parser.add_argument('--cc', default='cc')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='reloq-us-z-') as directory:
        if args.library:
            library = args.library.resolve()
            name = 'eciPersianMatchUsZOnset'
        else:
            library = Path(directory) / 'transition.so'
            subprocess.run([args.cc, '-shared', '-fPIC', '-std=c11', '-O2', '-Wall', '-Wextra',
                            '-Werror', '-I', str(ROOT / 'include'), '-I', str(ROOT / 'src'),
                            str(ROOT / 'src/reloq_persian_transition.c'), '-lm', '-o', str(library)],
                           check=True)
            name = 'reloq_persian_match_us_z_onset'
        fn = getattr(ctypes.CDLL(str(library)), name)
        fn.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.c_size_t, ctypes.c_size_t,
                       ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.c_size_t)
        fn.restype = ctypes.c_int
        target, donor = make_rows(2), make_rows(16)
        before, reference = bytes(target), bytes(donor)
        assert fn(target, 64, 8, 64, donor, 64, 8) == 1
        old = (NativeFrame * 64).from_buffer_copy(before)
        for i in range(64):
            for column in range(62):
                if 8 <= i <= 24 and (column == 8 or 35 <= column <= 42):
                    assert target[i].controls[column] == donor[i].controls[column], (i, column)
                else:
                    assert target[i].controls[column] == old[i].controls[column], (i, column)
        assert bytes(donor) == reference
        # The actual US onset can be faster than or unrelated to the target;
        # these cases must leave the input untouched.
        for alter in ('faster', 'wrong_band', 'no_fricative', 'late_pause', 'initial_noise'):
            left, right = make_rows(2), make_rows(16)
            if alter == 'faster': right = make_rows(1)
            elif alter == 'wrong_band':
                for row in right:
                    row.controls[37], row.controls[38], row.controls[39] = 60, 0, 0
            elif alter == 'no_fricative':
                for row in right: row.controls[8] = 0
            elif alter == 'late_pause':
                for row in left[14:]: row.controls[8] = 0
            else: right[8].controls[8] = 65
            before = bytes(left)
            assert fn(left, 64, 8, 64, right, 64, 8) == 0, alter
            assert bytes(left) == before, alter
        same = make_rows(2)
        before = bytes(same)
        assert fn(same, 64, 8, 64, same, 64, 8) == -1 and bytes(same) == before
        reverse = getattr(ctypes.CDLL(str(library)),
                          'eciPersianMatchHToUsZ' if args.library else 'reloq_persian_match_h_to_us_z')
        reverse.argtypes, reverse.restype = fn.argtypes, ctypes.c_int
        def h_to_z():
            target, donor = (NativeFrame * 110)(), (NativeFrame * 110)()
            for i in range(110):
                for rows in (target, donor):
                    c = rows[i].controls
                    c[0], c[1] = 1, 1200
                    value = max(0, min(65, round((i - 20) * 65 / 16)))
                    c[8] = value
                    c[38] = c[39] = round(value * 50 / 65)
                if 10 <= i < 19:
                    target[i].controls[7] = donor[i].controls[7] = 52
                if i >= 20:
                    target[i].controls[2] = 48
                    target[i].controls[7] = 34 if i < 36 else 0
                    donor[i].controls[2] = min(48, i - 20)
            return target, donor
        target, donor = h_to_z()
        before, reference = bytes(target), bytes(donor)
        assert reverse(target, 110, 20, 110, donor, 110, 20) == 1
        old = (NativeFrame * 110).from_buffer_copy(before)
        assert target[20].controls[2] == donor[20].controls[2] == 0
        for i in range(110):
            for column in range(62):
                if 20 <= i < 70 and column in (2, 7):
                    continue
                assert target[i].controls[column] == old[i].controls[column], (i, column)
        assert bytes(donor) == reference
        # At slow speech the sibilant's noise and voicing develop over much
        # longer than 35 and 90 ms respectively. It must still use the
        # genuine H-to-Z trajectory, without changing the noise or timing.
        target, donor = (NativeFrame * 260)(), (NativeFrame * 260)()
        for i in range(260):
            for rows in (target, donor):
                c = rows[i].controls
                c[0], c[1] = 1, 1200
                c[8] = max(0, min(65, round((i-20)*65/64)))
                c[38] = c[39] = round(c[8]*50/65)
            if 10 <= i < 19:
                target[i].controls[7] = donor[i].controls[7] = 52
            if i >= 20:
                target[i].controls[2] = max(38, 57-(i-20)*19/175)
                target[i].controls[7] = 34 if i < 84 else 0
                donor[i].controls[2] = min(38, (i-20)*38/175)
        before, reference = bytes(target), bytes(donor)
        assert reverse(target, 260, 20, 260, donor, 260, 20) == 1
        old = (NativeFrame * 260).from_buffer_copy(before)
        assert target[20].controls[2] == 0
        assert target[120].controls[2] < old[120].controls[2]
        for i in range(260):
            for column in range(62):
                if 20 <= i < 220 and column in (2, 7):
                    continue
                assert target[i].controls[column] == old[i].controls[column], (i, column)
        assert bytes(donor) == reference
        for alter in ('no_h', 'wrong_band', 'already_unvoiced', 'no_convergence'):
            target, donor = h_to_z()
            if alter == 'no_h':
                for row in target[:20]: row.controls[7] = 0
            elif alter == 'wrong_band':
                for row in donor: row.controls[38] = 0
            elif alter == 'already_unvoiced': target[20].controls[2] = 0
            else:
                for row in donor[20:]: row.controls[2] = 10
            before = bytes(target)
            assert reverse(target, 110, 20, 110, donor, 110, 20) == 0, alter
            assert bytes(target) == before, alter
    print('Passed US /z/ onset matching, source/timing preservation and rejected mismatches.')


if __name__ == '__main__':
    main()
