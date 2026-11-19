"""Check that Persian US pitch follows captured syllable timing."""
from __future__ import annotations

import argparse
from bisect import bisect_left
import ctypes
from pathlib import Path
import re
import sys
import threading
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import DirectEciEngine, EciIndex, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import FrameCapture
from continuous_driver import _PlanningVoice
from continuous_plan import CapturedRows, _dense, _syllable_edges, _syllable_word_edges, fit_us_word_pitch, indexed_us_proxy, word_boundary
from direct_fa_sequence import project
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, US_ENGLISH
import continuous_driver
import continuous_plan


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
        for sample_rate in (0, 1, 2, 3, 4, 5, 6):
            voice.set_parameter(SAMPLE_RATE, sample_rate)
            rate = SAMPLE_RATES[sample_rate]
            for speed in (45, 65, 90):
                voice.set_parameter(NATIVE_SPEED, speed)
                for text in ('راه‌اندازی', 'راه‌اندازی مجدد', 'سیستم راه‌اندازی شد', 'خانه‌ها', '123', '۱۷۱',
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
        with patch.object(continuous_driver, 'fit_us_word_pitch', side_effect=track), \
             patch.object(continuous_plan, 'fit_us_word_pitch', side_effect=track):
            for text in ('NVDA راه‌اندازی', 'راه‌اندازی NVDA', 'سلام NVDA راه‌اندازی',
                         'راه‌اندازی NVDA مجدد'):
                seen.clear()
                audio.clear()
                assert voice.speak((text,), audio.extend, lambda _: None)
                assert audio and any(seen), (text, seen)

        # Invalid or unvoiced anchors must leave the prior contour intact.
        tagged = project(('راه‌اندازی',), engine._frontend, allow_fallback=False).tagged
        target = CapturedRows.from_capture(api.capture(fa, (tagged,), audio=False))
        donor = CapturedRows.from_capture(api.capture(en, indexed_us_proxy(tagged, en, ()), audio=False))
        curve = _dense(donor, 1, native=api.dense_curve)
        voiced = [r.tick for r in donor.frames if r.controls[2] > 0 and r.controls[1] > 0]
        first, last = voiced[0], voiced[-1]
        initial = bytes(target.frames)
        bad_target = (ctypes.c_int64 * 1)(0)
        source = (ctypes.c_int64 * 1)(first + 1)
        end = min(round(len(target.pcm) * 1000 / (2 * 16000)), target.ticks[-1] + 1)
        assert api.fit_pitch_anchors(target.frames, len(target.frames), curve, len(curve),
                                     0, end, first, last, bad_target, source, 1) == 1
        assert bytes(target.frames) == initial
        assert api.fit_pitch_anchors(target.frames, len(target.frames), curve, len(curve),
                                     0, end, first, last, None, source, 1) == -1
        assert bytes(target.frames) == initial
    finally:
        voice.close()

    barrier, errors = threading.Barrier(2), []
    def worker(dialect, text, expected):
        raw_voice = None
        try:
            raw_voice = DirectEciEngine(args.library, dialect=dialect, encoding='utf8', native_voice=True)
            capture = FrameCapture(raw_voice._dll)
            for _ in range(10):
                barrier.wait(timeout=10)
                result = capture.capture(raw_voice, (text,), audio=False)
                assert result and bool(result.syllables) == expected, (text, result.syllables)
        except BaseException as error:
            errors.append(error)
        finally:
            if raw_voice is not None:
                raw_voice.close()
    threads = (threading.Thread(target=worker, args=(0x10000, 'hello radio', True)),
               threading.Thread(target=worker, args=(0x40000, 'guten tag', False)))
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join(timeout=20)
    assert all(not thread.is_alive() for thread in threads) and not errors, errors
    print('Passed observed Persian/US syllable pitch alignment at seven rates and three speeds.')


if __name__ == '__main__':
    main()
