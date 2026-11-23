"""Exercise the stop-to-Persian-R correction in the active NVDA planner."""
from __future__ import annotations

import argparse
import ctypes
import os
from pathlib import Path
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]

from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import NativeFrame
import continuous_driver
import continuous_plan
from nvda_driver_probe import PERSIAN_PROBE, make_language_engine


def frame_bytes(frames):
    return [bytes(frame) for frame in frames]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--transition-library', type=Path,
                        help='optional separate transition library for baseline diagnostics')
    args = parser.parse_args()
    transition_library = ctypes.CDLL(str(args.transition_library)) if args.transition_library else None
    transition = None
    if transition_library:
        transition = transition_library.reloq_persian_plan_stop_r_onsets
        transition.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                               ctypes.POINTER(ctypes.c_size_t), ctypes.c_size_t,
                               ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_size_t),
                               ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_size_t))
        transition.restype = ctypes.c_int
    voice = make_language_engine(args.library, args.library, None, Path('/unavailable'),
                                 Path('/unavailable'), language=PERSIAN_PROBE.id)
    previous_streaming = os.environ.get('RELOQ_FA_STREAM_PREFIX')
    original = continuous_plan.direct_stop_r_onsets
    observed = []

    def inspect(plan, parts, en, rate):
        if transition:
            plan._native_plan_stop_r_onsets = transition
        before = frame_bytes(plan.frames)
        original_counts = plan.counts
        cuts = original(plan, parts, en, rate)
        after = frame_bytes(plan.frames)
        removed = sum(original_counts) - len(after)
        assert len(after) + removed == len(before)
        assert sum(plan.counts) == len(after)
        if cuts:
            assert len(cuts) == 1 and len(parts) == 2 and cuts[0][0] == 1
            boundary = original_counts[0]
            assert plan.counts[0] == boundary
            assert after[:boundary] == before[:boundary], 'English stop was changed'
            assert removed > 0 and plan.counts[1] == original_counts[1] - removed
            # All true R frames survive. Only their early voicing may be
            # lowered; aspiration, frication and every other field must match.
            changed_voice = 0
            for old, new in zip(before[boundary + removed:], after[boundary:]):
                old_frame, new_frame = NativeFrame.from_buffer_copy(old), NativeFrame.from_buffer_copy(new)
                assert new_frame.controls[2] <= old_frame.controls[2]
                changed_voice += new_frame.controls[2] != old_frame.controls[2]
                new_frame.controls[2] = old_frame.controls[2]
                assert bytes(new_frame) == old, 'real R or its noise was altered'
            assert changed_voice <= 8
        else:
            assert not removed and after == before
        observed.append((parts, cuts))
        return cuts

    def render(prefix, streaming):
        os.environ['RELOQ_FA_STREAM_PREFIX'] = '1' if streaming else '0'
        pcm, marks = bytearray(), []
        observed.clear()
        with patch.object(continuous_plan, 'direct_stop_r_onsets', new=inspect):
            assert voice.speak((EciIndex(1), prefix, EciIndex(2), 'ریال', EciIndex(3)),
                               pcm.extend, lambda value: marks.append((value, len(pcm))))
        assert pcm and len(observed) <= 1, (prefix, observed)
        assert tuple(value for value, _ in marks) == (1, 2, 3)
        assert marks[0][1] <= marks[1][1] < marks[2][1] == len(pcm)
        return bytes(pcm), tuple(marks), observed[0][1] if observed else ()

    try:
        checks = 0
        for rate, gender, speed in ((3, 0, 65), (2, 1, 40), (6, 0, 100)):
            for parameter, value in ((SAMPLE_RATE, rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                voice.set_parameter(parameter, value)
            for prefix, require_cut in (('heading    level 3  ۱,۹۰۰,۰۰۰ ', True),
                                        ('heading level 3 1,900,000 ', True),
                                        ('bad ', False), ('hello ', False),
                                        ('heading level 3 1,900,000, ', False)):
                full = render(prefix, False)
                incremental = render(prefix, True)
                assert full == incremental, (rate, gender, speed, prefix)
                assert bool(full[2]) == require_cut, (rate, gender, speed, prefix, full[2])
                checks += 2
    finally:
        voice.close()
        if previous_streaming is None:
            os.environ.pop('RELOQ_FA_STREAM_PREFIX', None)
        else:
            os.environ['RELOQ_FA_STREAM_PREFIX'] = previous_streaming
    print(f'Passed {checks} actual stop/R, punctuation, streaming and index comparisons.')


if __name__ == '__main__':
    main()
