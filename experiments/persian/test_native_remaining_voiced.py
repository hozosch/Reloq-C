"""Compare the native voiced bridge against its established frame behavior."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import random
import subprocess
import sys
import tempfile
from types import SimpleNamespace

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_persian_frames import NativeFrame
from continuous_plan import (bridge_remaining_voiced, _bracket_boundary, _copy_frames,
                             _edges_by_count, _ease)
from bisect import bisect_left, bisect_right


def reference_bridge_remaining_voiced(plan, parts, bridged, cuts, window_ms=15.0):
    """Smooth abrupt voiced joins when a contextual bridge was unavailable.

    Only already voiced frames are reshaped, so stop closures are preserved.
    Aspiration is tapered at a voiced join or bridged across a brief spurious
    voiced onset. The selected A-to-Ach path and its established sonorant
    bridges remain unchanged.
    """
    excluded = set(bridged) | {edge for edge, _ in cuts}
    original, output = plan.frames, None
    starts, bounds = plan.timeline(), _edges_by_count(plan.counts)
    changed = []
    for edge in range(1, len(parts)):
        if edge in excluded or _bracket_boundary(parts, edge):
            continue
        boundary = bounds[edge - 1]
        left_limit = 0 if edge == 1 else bounds[edge - 2]
        right_limit = bounds[edge]
        left, right = original[boundary - 1].controls, original[boundary].controls
        if left[2] <= 0 or right[2] <= 0:
            continue
        # Some contexts contribute only 1–4 ms of artificial voicing before
        # a voiceless /h/. Bridge the existing aspiration through those rows
        # instead of introducing an isolated voiced click or a noise hole.
        short = boundary
        while (short < right_limit and original[short].controls[2] > 0 and
               starts[short] - starts[boundary] < 5.0):
            short += 1
        if (short > boundary and short < right_limit and original[short].controls[2] == 0 and
                starts[short] - starts[boundary] <= 5.0 and left[2] <= 20 and
                left[7] > 0 and original[short].controls[7] > 0):
            destination = original[short].controls
            width = short - boundary
            if output is None:
                output = _copy_frames(original)
            for i in range(boundary, short):
                fraction = (i - boundary + 1) / (width + 1)
                output[i].controls[2] = round(left[2] * (1 - fraction))
                output[i].controls[7] = round(left[7] + (destination[7] - left[7]) * fraction)
                for column in (9, 13, 15):
                    output[i].controls[column] = left[column] + (destination[column] - left[column]) * fraction
            changed.append(edge)
            continue
        amplitude_jump = right[2] - left[2]
        aspiration_jump = right[7] - left[7]
        frequencies = (9, 13, 15)
        if (abs(amplitude_jump) < 12 and abs(aspiration_jump) < 12 and
                max(abs(right[c] - left[c]) for c in frequencies) < 100):
            continue
        first = max(left_limit, bisect_left(starts, starts[boundary] - window_ms))
        last = min(right_limit, bisect_right(starts, starts[boundary] + window_ms))
        while first < boundary and any(original[j].controls[2] <= 0 for j in range(first, boundary)):
            first += 1
        while last > boundary and any(original[j].controls[2] <= 0 for j in range(boundary, last)):
            last -= 1
        if boundary - first < 3 or last - boundary < 3 or starts[last - 1] <= starts[first]:
            continue
        span = starts[last - 1] - starts[first]
        if output is None:
            output = _copy_frames(original)
        for column in frequencies:
            a, b = original[first].controls[column], original[last - 1].controls[column]
            for i in range(first, last):
                weight = _ease((starts[i] - starts[first]) / span)
                output[i].controls[column] = a + (b - a) * weight
        if amplitude_jump >= 12 and starts[boundary - 1] > starts[first]:
            duration = starts[boundary - 1] - starts[first]
            for i in range(first, boundary):
                weight = _ease((starts[i] - starts[first]) / duration)
                output[i].controls[2] = min(80, round(original[i].controls[2] + amplitude_jump * weight))
        elif amplitude_jump <= -12 and starts[last - 1] > starts[boundary]:
            duration = starts[last - 1] - starts[boundary]
            for i in range(boundary, last):
                weight = 1 - _ease((starts[i] - starts[boundary]) / duration)
                output[i].controls[2] = min(80, round(original[i].controls[2] - amplitude_jump * weight))
        if left[7] > 0 and right[7] == 0 and starts[boundary - 1] > starts[first]:
            duration = starts[boundary - 1] - starts[first]
            for i in range(first, boundary):
                weight = _ease((starts[i] - starts[first]) / duration)
                output[i].controls[7] = round(original[i].controls[7] * (1 - weight))
        elif left[7] == 0 and right[7] > 0 and starts[last - 1] > starts[boundary]:
            duration = starts[last - 1] - starts[boundary]
            for i in range(boundary, last):
                weight = _ease((starts[i] - starts[boundary]) / duration)
                output[i].controls[7] = round(original[i].controls[7] * weight)
        changed.append(edge)
    if output is not None:
        plan.frames = output
    return tuple(changed)



class Plan:
    def __init__(self, frames, native):
        self.frames = (NativeFrame * len(frames)).from_buffer_copy(frames)
        self.counts = (25, 25)
        self._native_bridge_remaining = native

    def timeline(self):
        times = [0.0]
        for frame in self.frames:
            times.append(times[-1] + frame.controls[0])
        return times


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cc', default='cc')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='reloq-fa-voiced-') as directory:
        library = Path(directory) / 'transition.so'
        subprocess.run([args.cc, '-shared', '-fPIC', '-std=c11', '-O2', '-ffp-contract=off',
                        '-Wall', '-Wextra', '-Werror', '-I', str(ROOT / 'include'), '-I', str(ROOT / 'src'),
                        str(ROOT / 'src/reloq_persian_transition.c'), '-lm', '-o', str(library)], check=True)
        fn = ctypes.CDLL(str(library)).reloq_persian_bridge_remaining
        fn.argtypes = (ctypes.POINTER(NativeFrame), ctypes.POINTER(NativeFrame),
                       ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_size_t),
                       ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t, ctypes.c_size_t,
                       ctypes.c_double, ctypes.POINTER(ctypes.c_ubyte))
        fn.restype = ctypes.c_int
        randomizer = random.Random(185)
        parts = (SimpleNamespace(text='NVDA '), SimpleNamespace(text='`[.1bar]'))
        changes = 0
        for case in range(400):
            rows = (NativeFrame * 50)()
            left_voice, right_voice = randomizer.randrange(10, 65), randomizer.randrange(10, 65)
            left_asp, right_asp = randomizer.choice((0, 34)), randomizer.choice((0, 34))
            for i, row in enumerate(rows):
                row.controls[0] = randomizer.choice((0.5, 1.0, 2.0))
                row.controls[2] = left_voice if i < 25 else right_voice
                row.controls[7] = left_asp if i < 25 else right_asp
                for column in (9, 13, 15):
                    row.controls[column] = randomizer.randrange(200, 3000) / 3
            if case % 8 == 0:
                # A brief inherited voiced island immediately before H.
                rows[24].controls[2], rows[24].controls[7] = 12, 34
                for row in rows[27:]:
                    row.controls[2], row.controls[7] = 0, 40
            elif case % 5 == 0:
                # A stop closure must not be included in a voiced bridge.
                rows[20].controls[2] = rows[30].controls[2] = 0
            python, native = Plan(rows, None), Plan(rows, fn)
            a = reference_bridge_remaining_voiced(python, parts, (), ())
            b = bridge_remaining_voiced(native, parts, (), ())
            assert a == b and bytes(python.frames) == bytes(native.frames), case
            changes += bool(a)
        assert changes > 100
        print(f'Passed 400 native voiced-bridge comparisons ({changes} modified), including H and stop closures.')


if __name__ == '__main__':
    main()
