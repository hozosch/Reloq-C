"""Verify the private Persian vowel-glide listening candidates."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from contextual_continuation import _TRACT
from espeak_ng_probe import EspeakPhonemeProbe
from persian_vowel_glide import compare_vowel_glide
from test_selected_probe import engine, set_confirmed_persian_probe


def verify(result, rate: int, expected: int) -> None:
    assert len(result.changed) == expected, (result.tags, result.changed, expected)
    assert result.selected == result.glide_pitch  # confirmed listening variant 2
    a, b, c = result.original_controls, result.glide_controls, result.glide_pitch_controls
    assert len(result.baseline) == len(result.glide) == len(result.glide_pitch)
    assert a.shape == b.shape == c.shape
    # The first candidate changes only the filter; its actual voiced/noise
    # sources, glottal onset, pitch and frame durations stay identical.
    assert np.array_equal(a[:, :9], b[:, :9])
    other = [j for j in range(a.shape[1]) if j not in _TRACT]
    assert np.array_equal(a[:, other], b[:, other])
    # The second candidate adds only a local F0 adjustment.
    assert np.array_equal(b[:, :1], c[:, :1])
    assert np.array_equal(b[:, 2:], c[:, 2:])
    tract_allowed = np.zeros(len(a), dtype=bool)
    f0_allowed = np.zeros(len(a), dtype=bool)
    for word, start, end, peak, pitch_end in result.changed:
        edge_ms = round(result.indexes[word] * 1000 / rate)
        edge = int(np.searchsorted(result.ticks, edge_ms, side='left'))
        prev_ms = round(result.indexes[word - 1] * 1000 / rate) if word > 1 else 0
        assert result.ticks[start] >= prev_ms + (edge_ms - prev_ms) / 2
        assert start < edge < end and 'w' not in result.tags[word - 1] + result.tags[word]
        assert np.all(a[start:end + 1, 2] > 0) and np.all(a[start:end + 1, 8] == 0)
        assert np.all(a[start:end + 1, 6] == 0)
        assert np.array_equal(a[start, list(_TRACT)], b[start, list(_TRACT)])
        assert np.array_equal(a[end, list(_TRACT)], b[end, list(_TRACT)])
        assert a[end, 9] - a[edge, 9] >= 100 and a[end, 13] - a[edge, 13] >= 100
        # The first native A plateau is the endpoint; no later vowel or
        # nasal may be absorbed into this inter-vowel transition.
        if end + 1 < len(a):
            assert a[end + 1, 9] <= a[end, 9] + .25
        tract_allowed[start:end + 1] = True
        if peak >= 0:
            assert edge <= peak < pitch_end and peak <= end
            assert c[peak, 1] == a[peak, 1] and c[pitch_end, 1] == a[pitch_end, 1]
            f0_allowed[peak:pitch_end + 1] = True
    assert np.array_equal(a[~tract_allowed], b[~tract_allowed])
    assert np.array_equal(b[~f0_allowed, 1], c[~f0_allowed, 1])
    if expected:
        assert result.baseline != result.glide
        if any(peak >= 0 for _, _, _, peak, _ in result.changed):
            assert result.glide != result.glide_pitch
    else:
        assert result.baseline == result.glide == result.glide_pitch
    for pcm in (result.baseline, result.glide, result.glide_pitch):
        samples = np.frombuffer(pcm, '<i2').astype(np.int32)
        assert np.any(samples) and np.max(np.abs(samples)) < 32767


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa = engine(args.probe, 0x90000)
    checks = 0
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            cases = (('او آمد.', 1), ('او آرام بود.', 1), ('او آنجا بود.', 1),
                     ('او آمد و او آنجا بود.', 2), ('او ایران را دید.', 0),
                     ('من ایران را دوست دارم.', 0), ('خوب است.', 0),
                     ('من ساعت دارم.', 0), ('ایران', 0), ('او، آمد.', 0))
            for mode in (1, 3, 6):
                for gender in (0, 1):
                    for parameter, value in ((SAMPLE_RATE, mode), (GENDER, gender),
                                             (NATIVE_PITCH, 81 if gender else 65),
                                             (NATIVE_INFLECTION, 30), (NATIVE_SPEED, 50)):
                        fa.set_parameter(parameter, value)
                    set_confirmed_persian_probe()
                    for text, count in cases:
                        result = compare_vowel_glide(api, fa, args.probe, text, SAMPLE_RATES[mode], gender=gender)
                        try:
                            verify(result, SAMPLE_RATES[mode], count)
                        except AssertionError as error:
                            raise AssertionError((mode, gender, text, result.changed)) from error
                        if (mode, gender, text) == (3, 0, 'او آمد.'):
                            _, _, _, peak, end = result.changed[0]
                            original_step = np.max(np.abs(np.diff(result.original_controls[peak:end + 1, 1])))
                            smoother_step = np.max(np.abs(np.diff(result.glide_pitch_controls[peak:end + 1, 1])))
                            assert 0 < smoother_step < original_step * .8
                        checks += 1
                    # An existing glottalized boundary must not be smoothed
                    # or treated as an uninterrupted vowel transition.
                    set_confirmed_persian_probe(no_auto_glottal=0)
                    result = compare_vowel_glide(api, fa, args.probe, 'او آمد.', SAMPLE_RATES[mode], gender=gender)
                    verify(result, SAMPLE_RATES[mode], 0)
                    checks += 1
            for speed in (40, 98, 200):
                for parameter, value in ((SAMPLE_RATE, 3), (GENDER, 0), (NATIVE_PITCH, 65),
                                         (NATIVE_SPEED, speed)):
                    fa.set_parameter(parameter, value)
                set_confirmed_persian_probe()
                result = compare_vowel_glide(api, fa, args.probe, 'او آمد.', 16000)
                verify(result, 16000, 1)
                checks += 1
    finally:
        fa.close()
    print(f'Passed {checks} U-to-A, source, first-onset and context-isolation cases.')


if __name__ == '__main__':
    main()
