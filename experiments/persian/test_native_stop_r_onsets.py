"""Validate bounded C removal of a stale vowel before a Persian R."""
from __future__ import annotations

import argparse
import ctypes as c
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_persian_frames import NativeFrame


def row(voice, aspiration, noise):
    frame = NativeFrame()
    frame.controls[0], frame.controls[2], frame.controls[7], frame.controls[8] = 1, voice, aspiration, noise
    frame.controls[9], frame.controls[13], frame.controls[15], frame.controls[17] = 500, 1500, 2700, 3300
    return frame


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cc', default='cc')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='reloq-stop-r-') as temp:
        library = Path(temp) / 'transition.so'
        subprocess.run([args.cc, '-shared', '-fPIC', '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                        '-I', str(ROOT/'include'), '-I', str(ROOT/'src'),
                        str(ROOT/'src/reloq_persian_transition.c'), '-lm', '-o', str(library)], check=True)
        fn = c.CDLL(str(library)).reloq_persian_plan_stop_r_onsets
        fn.argtypes = (c.POINTER(NativeFrame), c.c_size_t, c.POINTER(c.c_size_t), c.c_size_t,
                       c.POINTER(c.c_size_t), c.POINTER(c.c_size_t), c.POINTER(c.c_double),
                       c.POINTER(c.c_size_t))
        fn.restype = c.c_int

        def check(left, lead, onset, rest, *, expected, start_adjust=0):
            source = [*left, *lead, *onset, *rest]
            frames = (NativeFrame * len(source))(*source)
            before = [bytes(item) for item in frames]
            boundary, start = len(left), len(left)+len(lead)+start_adjust
            ends = (c.c_size_t * 2)(boundary, len(source))
            starts = (c.c_size_t * 2)(0, start)
            counts, removed, length = (c.c_size_t * 2)(), (c.c_double * 2)(), c.c_size_t()
            assert fn(frames, len(source), ends, 2, starts, counts, removed, c.byref(length)) == 0
            assert counts[0] == boundary and bytes(frames)[:boundary*c.sizeof(NativeFrame)] == b''.join(before[:boundary])
            if expected:
                assert removed[1] == len(lead) and length.value == len(source)-len(lead)
                for i, original in enumerate(source[start:]):
                    actual = frames[boundary+i]
                    for col in range(62):
                        if col == 2 and i < 3:
                            assert actual.controls[col] == min(original.controls[col], 5)
                        else:
                            assert actual.controls[col] == original.controls[col], (i, col)
            else:
                assert removed[1] == 0 and length.value == len(source) and bytes(frames) == b''.join(before)

        stop = [row(35, 0, 0)] * 5 + [row(0, 0, 39)] * 3
        lead = [row(48, 34, 0)] * 35
        real_r = [row(47, 34, 0)] * 3 + [row(5, 25, 0)] * 15 + [row(40, 45, 0)] * 20
        vowel = [row(53, 34, 0)] * 40
        check(stop, lead, real_r, vowel, expected=True)
        # A female voice may have almost no aspiration during its lead.
        check(stop, [row(48, 3, 0)] * 35, real_r, vowel, expected=True)
        closure = row(35, 0, 0)
        closure.controls[9] = 228
        check([row(48, 34, 0)] * 5 + [closure] * 8, lead, real_r, vowel, expected=True)
        check([row(48, 34, 0)] * 5 + [closure] * 3, lead, real_r, vowel, expected=False)
        check([row(35, 0, 0)] * 8, lead, real_r, vowel, expected=False)
        check([row(48, 34, 0)] * 8, lead, real_r, vowel, expected=False)
        check(stop, [row(20, 0, 0)] * 35, real_r, vowel, expected=False)
        check(stop, [row(48, 34, 20)] * 35, real_r, vowel, expected=False)
        check(stop, lead, [row(48, 34, 0)] * 20, vowel, expected=False)
        check(stop, lead, [row(5, 25, 0)] * 70, vowel, expected=False)
        check(stop, [row(48, 34, 0)] * 2, real_r, vowel, expected=False)
        check(stop, [row(48, 34, 0)] * 101, real_r, vowel, expected=False)
        # Two eligible boundaries in one utterance must compact independently,
        # without moving either stop or corrupting the R after the first cut.
        source = [*stop, *lead, *real_r, *stop, *lead, *real_r]
        frames = (NativeFrame * len(source))(*source)
        boundaries = (c.c_size_t * 4)(len(stop), len(stop)+len(lead)+len(real_r),
                                     2*len(stop)+len(lead)+len(real_r), len(source))
        starts = (c.c_size_t * 4)(0, len(stop)+len(lead), 0,
                                  2*len(stop)+2*len(lead)+len(real_r))
        counts, removed, length = (c.c_size_t * 4)(), (c.c_double * 4)(), c.c_size_t()
        assert fn(frames, len(source), boundaries, 4, starts, counts, removed, c.byref(length)) == 0
        assert tuple(counts) == (len(stop), len(real_r), len(stop), len(real_r))
        assert tuple(removed) == (0, len(lead), 0, len(lead))
        assert length.value == 2*(len(stop)+len(real_r))
        assert bytes(frames[0]) == bytes(stop[0])
        assert bytes(frames[len(stop)+len(real_r)]) == bytes(stop[0])
        print('Passed verified stop-to-R compaction, multiple joins and negative controls.')


if __name__ == '__main__':
    main()
