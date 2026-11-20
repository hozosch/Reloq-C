"""Remove inherited voicing between English stops and Persian voiceless onsets."""
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
    observed = []
    size = ctypes.sizeof(NativeFrame)

    def check(plan, parts, en, rate):
        before = bytes(plan.frames)
        counts = plan.counts
        boundary = counts[0]
        first = plan.frames[boundary].controls[2]
        left = plan.frames[boundary - 1].controls[2]
        cuts = original(plan, parts, en, rate)
        if cuts:
            assert len(cuts) == 1 and cuts[0][0] == 1
            removed = counts[1] - plan.counts[1]
            assert removed > 0 and plan.frames[boundary].controls[2] == 0
            after = bytes(plan.frames)
            assert after == before[:boundary * size] + before[(boundary + removed) * size:]
            assert left == 0 and first > 0, (left, first, cuts)
        else:
            assert counts == plan.counts and before == bytes(plan.frames)
        observed.append((parts, left, first, cuts))
        return cuts

    continuous_plan.direct_unvoiced_onsets = check
    changed = checked = 0
    try:
        for sample_rate in (1, 3, 6):
            for gender in (0, 1):
                for speed in (40, 82, 140):
                    for key, value in ((SAMPLE_RATE, sample_rate), (GENDER, gender), (NATIVE_SPEED, speed)):
                        engine.set_parameter(key, value)
                    for english in ('internet', 'object', 'Persian text', 'cup', 'back'):
                        for persian in ('سلام', 'فایل', 'تست', 'کتاب', 'هوا', 'خانه'):
                            observed.clear(); pcm = bytearray(); marks = []
                            parts = (EciIndex(1), english + ' ', EciIndex(2), persian, EciIndex(3))
                            assert engine.speak(parts, pcm.extend, marks.append) and pcm
                            assert marks == [1, 2, 3] and observed, (english, persian, marks)
                            _parts, left, first, cuts = observed[-1]
                            if left == 0 and first > 0:
                                assert cuts, (english, persian, sample_rate, gender, speed)
                                changed += 1
                            checked += 1
        # A real Persian vowel must remain voiced; no consonant-onset rule
        # may remove it after the English stop.
        observed.clear(); pcm = bytearray()
        assert engine.speak(('Persian text ایران',), pcm.extend, lambda _: None) and pcm
        assert observed and not observed[-1][-1]
        assert changed >= 20, (changed, checked)
    finally:
        continuous_plan.direct_unvoiced_onsets = original
        engine.close()
    print(f'Passed {checked} English-stop/Persian onset checks ({changed} stale voiced leads removed).')


if __name__ == '__main__':
    main()
