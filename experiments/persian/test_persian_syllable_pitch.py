"""Check that Persian US pitch follows captured syllable timing."""
from __future__ import annotations

import argparse
from bisect import bisect_left
from pathlib import Path
import re
import sys
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import EciIndex, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import FrameCapture
from continuous_driver import _PlanningVoice
from continuous_plan import CapturedRows, _dense, _syllable_edges, _syllable_word_edges, fit_us_word_pitch, indexed_us_proxy, word_boundary
from direct_fa_sequence import project
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, US_ENGLISH
import continuous_driver


class ObservedApi:
    def __init__(self, wrapped):
        self.wrapped = wrapped
        self.anchored = 0

    def __getattr__(self, name):
        return getattr(self.wrapped, name)

    def fit_pitch_anchors(self, *args):
        result = self.wrapped.fit_pitch_anchors(*args)
        if result == 0:
            self.anchored += 1
        return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args = parser.parse_args()
    voice = make_language_engine(args.library, args.library, None,
                                 Path('/unavailable-espeak'), Path('/unavailable'))
    voice.set_parameter(NATIVE_PITCH, 65)
    voice.set_parameter(NATIVE_INFLECTION, 30)
    fk, ek = voice._key(PERSIAN_PROBE.id, '1'), voice._key(US_ENGLISH.id, '1')
    engine = voice._activate(fk)
    fa, en = _PlanningVoice(voice, fk, engine), _PlanningVoice(voice, ek, voice._activate(ek))
    en.raw.set_parameter(PROCESS_TAGS, 1)
    api = FrameCapture(fa._dll)
    assert api.fit_pitch_anchors is not None and api._syllables is not None

    def sequence(tagged):
        matches = tuple(re.finditer(r'`\[[^]]+\]', tagged))
        marks = tuple(range(1, len(matches)))
        parts, pos = [], 0
        for i, mark in enumerate(marks, 1):
            boundary = word_boundary(tagged, matches, i)
            parts.extend((tagged[pos:boundary], EciIndex(mark)))
            if boundary < len(tagged) and tagged[boundary] in '.,?!:;()[]{}':
                parts.append(' ')
            pos = boundary
        parts.append(tagged[pos:])
        return matches, marks, tuple(parts)

    try:
        for sample_rate in (2, 3, 4):
            voice.set_parameter(SAMPLE_RATE, sample_rate)
            rate = SAMPLE_RATES[sample_rate]
            for speed in (45, 65, 90):
                voice.set_parameter(NATIVE_SPEED, speed)
                for text in ('راه‌اندازی', 'راه‌اندازی مجدد', 'سیستم راه‌اندازی شد', 'خانه‌ها',
                             'راه‌اندازی. سیستم راه‌اندازی شد.'):
                    tagged = project((text,), engine._frontend, allow_fallback=False).tagged
                    matches, marks, parts = sequence(tagged)
                    counts = tuple(len(re.findall(r'\.[012]', m.group())) for m in matches)
                    target = CapturedRows.from_capture(api.capture(fa, parts, audio=False))
                    donor = CapturedRows.from_capture(api.capture(en, indexed_us_proxy(tagged, en, marks), audio=False))
                    before = [tuple(row.controls) for row in target.frames]
                    observed = ObservedApi(api)
                    assert fit_us_word_pitch(target, (0, len(target.pcm)//2), donor, (0, len(donor.pcm)//2),
                                             marks, marks, rate, observed, syllable_counts=counts), (text, speed, rate)
                    assert observed.anchored, (text, speed, rate, target.syllables, donor.syllables)
                    assert all(all(old[i] == row.controls[i] for i in range(62) if i != 1)
                               for old, row in zip(before, target.frames)), text
                    target_bounds = [0] + [round(target.indexes[m]*1000/rate) for m in marks] + [target.ticks[-1]+1]
                    donor_bounds = [0] + [round(donor.indexes[m]*1000/rate) for m in marks] + [donor.ticks[-1]+1]
                    target_groups = _syllable_word_edges(target, target_bounds, counts)
                    donor_groups = _syllable_word_edges(donor, donor_bounds, counts)
                    if '.' in text:
                        assert target_groups is not None and donor_groups is not None, (text, target_bounds, donor_bounds)
                    curve = _dense(donor, 1, native=api.dense_curve)
                    for i, count in enumerate(counts):
                        if count < 2:
                            continue
                        left = (target_groups[i] if target_groups is not None else
                                _syllable_edges(target, target_bounds[i], target_bounds[i+1], count))
                        right = (donor_groups[i] if donor_groups is not None else
                                 _syllable_edges(donor, donor_bounds[i], donor_bounds[i+1], count))
                        for t, d in zip(left, right):
                            at = bisect_left(target.ticks, t)
                            if at < len(target.ticks) and target.ticks[at] == t:
                                assert abs(target.frames[at].controls[1] - curve[d]) < .01, (text, speed, rate, t, d)

        # A clock reset or a differing syllable count must never invent a
        # boundary; the normal word-level transfer remains available.
        target.syllables = ((0, 20000), (0, 30000))
        assert not _syllable_edges(target, 0, 100, 2)
        target.syllables = ((0, 20000), (30000, 40000))
        assert not _syllable_edges(target, 0, 100, 3)

        # Verify that the actual Persian utterance path requests and uses
        # syllable fitting, not only the standalone alignment helper.
        voice.set_parameter(SAMPLE_RATE, 3)
        voice.set_parameter(NATIVE_SPEED, 65)
        seen = []
        def track(*args, **kwargs):
            args = list(args)
            observed = ObservedApi(args[7])
            args[7] = observed
            result = fit_us_word_pitch(*args, **kwargs)
            seen.append(observed.anchored)
            return result
        audio = bytearray()
        with patch.object(continuous_driver, 'fit_us_word_pitch', side_effect=track):
            assert voice.speak(('راه‌اندازی',), audio.extend, lambda _: None)
        assert audio and any(seen), seen
    finally:
        voice.close()
    print('Passed observed Persian/US syllable pitch alignment at three rates and speeds.')


if __name__ == '__main__':
    main()
