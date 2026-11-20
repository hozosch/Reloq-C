"""Keep voiced stop closures while removing an inherited vowel after English."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
import continuous_plan
from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import NativeFrame
from nvda_driver_probe import make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args()
    engine = make_language_engine(args.library, args.library, None, Path('/unavailable'), Path('/unavailable'))
    original = continuous_plan.direct_unvoiced_onsets
    observations, size = [], ctypes.sizeof(NativeFrame)

    def check(plan, parts, en, rate):
        before, counts = bytes(plan.frames), plan.counts
        boundary = counts[0]
        first = tuple(plan.frames[boundary].controls[c] for c in (2, 7, 8))
        left = tuple(plan.frames[boundary-1].controls[c] for c in (2, 7, 8))
        cuts = original(plan, parts, en, rate)
        if cuts:
            assert len(cuts) == 1 and cuts[0][0] == 1
            removed = counts[1] - plan.counts[1]
            assert removed > 0 and left[0] == 0 and first[0] >= 30 and first[1] >= 5
            assert bytes(plan.frames) == before[:boundary*size] + before[(boundary+removed)*size:]
            # Preserve the native voiced-closure frames and following burst.
            head = plan.frames[boundary].controls
            assert 0 <= head[2] <= 30 and head[7] == head[8] == 0
            assert any(plan.frames[i].controls[8] >= 20 for i in
                       range(boundary, min(len(plan.frames), boundary + 250)))
        else:
            assert counts == plan.counts and before == bytes(plan.frames)
        observations.append((left, first, cuts))
        return cuts

    continuous_plan.direct_unvoiced_onsets = check
    changed = checked = 0
    try:
        for sample_rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 65, 82, 140):
                    for key, value in ((SAMPLE_RATE, sample_rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                        engine.set_parameter(key, value)
                    for english in ('internet', 'object', 'Persian text', 'cup', 'back'):
                        for persian in ('بازی', 'دختر', 'گفته'):
                            pcm, marks = bytearray(), []
                            observations.clear()
                            assert engine.speak((EciIndex(1), english + ' ', EciIndex(2), persian, EciIndex(3)),
                                                pcm.extend, marks.append) and pcm
                            assert marks == [1, 2, 3] and observations
                            left, first, cuts = observations[-1]
                            if left[0] == 0 and first[0] >= 30 and first[1] >= 5:
                                assert cuts, (sample_rate, gender, speed, english, persian, first)
                                changed += 1
                            checked += 1
        assert changed >= 50, (changed, checked)
    finally:
        continuous_plan.direct_unvoiced_onsets = original
        engine.close()
    print(f'Passed {checked} English-to-voiced-stop contexts ({changed} inherited vowels removed).')


if __name__ == '__main__':
    main()
