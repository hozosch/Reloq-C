"""Check the experimental English-to-Persian Ach-onset transition."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from contextual_continuation import accepted_contextual_compare
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa, en = engine(args.probe, 0x90000), engine(args.probe, 0x10000)
    checks = 0
    try:
        set_confirmed_persian_probe()
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            sentences = ('آیا NVDA خوب است؟', 'NVDA خیلی خوب است.',
                         'آیا NVDA خراب است؟', 'آیا Windows خراب است؟',
                         'من NVDA را دوست دارم.', 'من یک laptop دارم.')
            for mode in (1, 3, 6):
                rate = SAMPLE_RATES[mode]
                for gender in (0, 1):
                    for voice in (fa, en):
                        voice.set_parameter(SAMPLE_RATE, mode)
                        voice.set_parameter(GENDER, gender)
                        voice.set_parameter(NATIVE_PITCH, 81 if gender else 65)
                        voice.set_parameter(NATIVE_INFLECTION, 30)
                    for sentence in sentences:
                        parts = translate_text(api, sentence)
                        base = accepted_contextual_compare(parts, fa, en, args.probe, rate, gender=gender)
                        adjusted = accepted_contextual_compare(parts, fa, en, args.probe, rate,
                                                               gender=gender, ach_onset_ms=25)
                        assert base.frame_counts == adjusted.frame_counts
                        assert base.bridged_edges == adjusted.bridged_edges
                        assert len(base.connected) == len(adjusted.connected)
                        source = list(range(9)) + list(range(35, 62))
                        assert np.array_equal(base.controls[:, source], adjusted.controls[:, source])
                        checks += 4
                        data = np.frombuffer(adjusted.connected, '<i2').astype(np.int32)
                        assert np.any(data) and np.max(np.abs(data)) < 32767
                        checks += 1
                        if sentence in sentences[:3]:
                            assert len(adjusted.ach_onset_edges) == 1, (mode, gender, sentence)
                            edge = adjusted.ach_onset_edges[0]
                            frame = sum(base.frame_counts[:edge])
                            assert np.array_equal(base.controls[frame:], adjusted.controls[frame:])
                            assert adjusted.connected != base.connected
                            # The two voiced tract targets now meet at X onset.
                            assert np.allclose(adjusted.controls[frame - 1, [9, 13, 15]],
                                               adjusted.controls[frame, [9, 13, 15]], atol=.01)
                            starts = np.concatenate(([0.0], np.cumsum(base.controls[:, 0], dtype=np.float64)))
                            earlier = starts[:-1] < starts[frame] - 25
                            assert np.array_equal(base.controls[earlier], adjusted.controls[earlier])
                            checks += 5
                        else:
                            # Fricated Windows, non-X starts, the accepted E
                            # and the K/laptop stop remain byte-identical.
                            assert not adjusted.ach_onset_edges
                            assert np.array_equal(base.controls, adjusted.controls)
                            assert base.connected == adjusted.connected
                            checks += 3
                        if sentence.endswith('؟'):
                            final = parts[-1]
                            trace = adjusted.context_after[final.language]
                            bounds = adjusted.edges[final.language]
                            first = round(bounds[-2] * 1000 / rate)
                            last = min(round(bounds[-1] * 1000 / rate), len(trace.av))
                            voiced = np.flatnonzero(trace.av[first:last] > 0)
                            begin, end = first + int(voiced[0]), first + int(voiced[-1])
                            assert trace.f0[end] > trace.f0[begin + 3 * (end - begin) // 4]
                            checks += 1
            # Shorten the preparation to the actual contiguous vowel tail at
            # high speeds, instead of altering the preceding stop or skipping
            # the transition merely because the full 25 ms window overlaps it.
            for voice in (fa, en):
                voice.set_parameter(SAMPLE_RATE, 3)
                voice.set_parameter(GENDER, 0)
                voice.set_parameter(NATIVE_PITCH, 65)
            parts = translate_text(api, sentences[0])
            for speed in (40, 98, 200):
                for voice in (fa, en):
                    voice.set_parameter(NATIVE_SPEED, speed)
                base = accepted_contextual_compare(parts, fa, en, args.probe, 16000)
                adjusted = accepted_contextual_compare(parts, fa, en, args.probe, 16000, ach_onset_ms=25)
                assert adjusted.ach_onset_edges == (2,), ('Ach onset skipped', speed)
                edge = sum(base.frame_counts[:2])
                assert np.array_equal(base.controls[edge:], adjusted.controls[edge:])
                changed = np.any(base.controls != adjusted.controls, axis=1)
                assert np.all(base.controls[changed, 2] > 0) and np.all(base.controls[changed, 8] == 0)
                assert len(base.connected) == len(adjusted.connected)
                checks += 4
    finally:
        fa.close()
        en.close()
    print(f'Passed {checks} Ach onset, preserved-source, timing and melody checks.')


if __name__ == '__main__':
    main()
