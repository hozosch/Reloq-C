"""Check real language joins against the continuous native source context."""
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
    def checked(plan, parts, en, rate):
        boundary = plan.counts[0]
        leading = ctypes.string_at(plan.frames, boundary * ctypes.sizeof(NativeFrame))
        old_counts = plan.counts
        before = tuple(tuple(plan.frames[i].controls[c] for c in (2, 7, 8))
                       for i in range(boundary, min(len(plan.frames), boundary + 40)))
        cuts = original(plan, parts, en, rate)
        assert leading == ctypes.string_at(plan.frames, boundary * ctypes.sizeof(NativeFrame))
        if cuts:
            assert cuts[0][0] == 1 and plan.counts[0] == old_counts[0]
            assert plan.counts[1] < old_counts[1]
            assert plan.frames[boundary].controls[2] == 0
        else:
            assert old_counts == plan.counts
            after = tuple(tuple(plan.frames[i].controls[c] for c in (2, 7, 8))
                          for i in range(boundary, min(len(plan.frames), boundary + 40)))
            assert before == after
        observed.append((parts[1].text, cuts, before))
        return cuts
    continuous_plan.direct_unvoiced_onsets = checked
    try:
        for word in ('سلام', 'سبز', 'فایل', 'شماره', 'کتاب', 'تست', 'پر', 'خوب', 'زود'):
            pcm = bytearray()
            assert engine.speak(('NVDA ' + word,), pcm.extend, lambda _: None) and pcm
            tag, cuts, before = observed[-1]
            if word in ('خوب', 'زود', 'تست', 'کتاب', 'سلام'):
                assert not cuts, (word, tag, cuts)
                if word in ('تست', 'کتاب', 'سلام'):
                    assert before[0][0] > 0 and any(row[0] == 0 for row in before), (word, before)
            elif cuts:
                assert cuts and 8 <= cuts[0][1] <= 120, (word, tag, cuts)
    finally:
        continuous_plan.direct_unvoiced_onsets = original
        engine.close()
    print('Passed real native T/K/S context joins, preceding vowel, voiced-Z and selected Ach checks.')


if __name__ == '__main__':
    main()
