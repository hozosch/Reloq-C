"""Check that language joins do not add a voiced carrier to voiceless onsets."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
import continuous_plan
from _reloq_persian_frames import NativeFrame
from nvda_driver_probe import make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=True)
    args = parser.parse_args()
    engine = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                  args.espeak_library, args.data_parent)
    original, observed = continuous_plan.direct_unvoiced_onsets, []
    def checked(plan, parts, en):
        boundary = plan.counts[0]
        leading = ctypes.string_at(plan.frames, boundary * ctypes.sizeof(NativeFrame))
        old_counts = plan.counts
        cuts = original(plan, parts, en)
        assert leading == ctypes.string_at(plan.frames, boundary * ctypes.sizeof(NativeFrame))
        if cuts:
            assert cuts[0][0] == 1 and plan.counts[0] == old_counts[0]
            assert plan.counts[1] < old_counts[1]
            assert plan.frames[boundary].controls[2] == 0
        observed.append((parts[1].text, cuts))
        return cuts
    continuous_plan.direct_unvoiced_onsets = checked
    try:
        for word in ('سلام', 'سبز', 'فایل', 'شماره', 'کتاب', 'تست', 'پر', 'خوب', 'زود'):
            pcm = bytearray()
            assert engine.speak(('NVDA ' + word,), pcm.extend, lambda _: None) and pcm
            tag, cuts = observed[-1]
            if word in ('خوب', 'زود'):
                assert not cuts, (word, tag, cuts)
            else:
                assert cuts and 8 <= cuts[0][1] <= 120, (word, tag, cuts)
    finally:
        continuous_plan.direct_unvoiced_onsets = original
        engine.close()
    print('Passed real voiceless onset, preceding vowel, voiced-Z and selected Ach checks.')


if __name__ == '__main__':
    main()
