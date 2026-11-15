"""Verify the German-guided Ach and following-vowel flow comparisons."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (
    GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES,
)
from contextual_continuation import accepted_german_ach_compare
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


def verify(base, f3_only, both, expected: int) -> int:
    assert len(f3_only.german_flow_edges) == len(both.german_flow_edges) == expected
    assert base.frame_counts == f3_only.frame_counts == both.frame_counts
    assert base.german_ach_edges == f3_only.german_ach_edges == both.german_ach_edges
    assert base.ach_frication_cuts == f3_only.ach_frication_cuts == both.ach_frication_cuts
    assert len(base.connected) == len(f3_only.connected) == len(both.connected)
    source = list(range(9)) + list(range(35, 62))
    assert np.array_equal(base.controls[:, source], f3_only.controls[:, source])
    assert np.array_equal(base.controls[:, source], both.controls[:, source])
    assert np.array_equal(base.controls[:, 13], f3_only.controls[:, 13])
    assert np.array_equal(f3_only.controls[:, 15], both.controls[:, 15])
    starts = np.concatenate(([0.0], np.cumsum(base.controls[:, 0], dtype=np.float64)))
    f3_allowed = np.zeros(len(base.controls), dtype=bool)
    f2_allowed = np.zeros(len(base.controls), dtype=bool)
    for (edge, f3_ms, f2_ms), (edge1, f3_ms1, unused) in zip(
            both.german_flow_edges, f3_only.german_flow_edges):
        assert edge == edge1 and f3_ms == f3_ms1 and unused == 0
        assert f3_ms > 0 and f2_ms > f3_ms
        n = sum(base.frame_counts[:edge])
        f3_indices = np.flatnonzero((starts[:-1] >= starts[n]) & (starts[:-1] <= starts[n] + f3_ms))
        f2_indices = np.flatnonzero((starts[:-1] >= starts[n]) & (starts[:-1] <= starts[n] + f2_ms))
        f3_allowed[f3_indices] = True
        f2_allowed[f2_indices] = True
        assert np.isclose(both.controls[n, 15], base.controls[n - 1, 15], atol=.01)
        assert np.isclose(both.controls[f3_indices[-1], 15], base.controls[f3_indices[-1], 15], atol=.01)
        assert np.isclose(both.controls[n, 13], base.controls[n, 13], atol=.01)
        assert np.isclose(both.controls[f2_indices[-1], 13], base.controls[f2_indices[-1], 13], atol=.01)
        # No extra voiced onset; the first following vowel frame remains
        # the native F3 target, and the final F2 is the native vowel target.
        assert np.all(both.controls[f3_indices[:-1], 2] == 0)
        assert both.controls[f3_indices[-1], 2] > 0
        assert both.controls[n, 8] >= 20
        differences = np.diff(both.controls[f3_indices, 15])
        assert np.all(differences >= -.01) or np.all(differences <= .01)
    assert np.array_equal(base.controls[~f3_allowed, 15], f3_only.controls[~f3_allowed, 15])
    assert np.array_equal(base.controls[~f2_allowed, 13], both.controls[~f2_allowed, 13])
    for result in (f3_only, both):
        data = np.frombuffer(result.connected, '<i2').astype(np.int32)
        assert np.any(data) and np.max(np.abs(data)) < 32767
    if expected:
        assert base.connected != f3_only.connected and f3_only.connected != both.connected
    else:
        assert base.connected == f3_only.connected == both.connected
    return 12 + 10 * expected


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa, en, de = (engine(args.probe, dialect) for dialect in (0x90000, 0x10000, 0x40000))
    checks = 0
    try:
        set_confirmed_persian_probe()
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            cases = (('آیا NVDA خوب است؟', 1), ('NVDA خیلی خوب است.', 1),
                     ('آیا NVDA خراب است؟', 1), ('من یک laptop دارم.', 0))
            for mode in (1, 3, 6):
                rate = SAMPLE_RATES[mode]
                for gender in (0, 1):
                    for voice in (fa, en, de):
                        for parameter, value in ((SAMPLE_RATE, mode), (GENDER, gender),
                                                 (NATIVE_PITCH, 81 if gender else 65),
                                                 (NATIVE_INFLECTION, 30), (NATIVE_SPEED, 50)):
                            voice.set_parameter(parameter, value)
                    for sentence, expected in cases:
                        parts = translate_text(api, sentence)
                        base = accepted_german_ach_compare(parts, fa, en, de, args.probe, rate, gender=gender)
                        one = accepted_german_ach_compare(parts, fa, en, de, args.probe, rate,
                                                          gender=gender, german_flow=1)
                        two = accepted_german_ach_compare(parts, fa, en, de, args.probe, rate,
                                                          gender=gender, german_flow=2)
                        checks += verify(base, one, two, expected)
            for voice in (fa, en, de):
                voice.set_parameter(SAMPLE_RATE, 3)
                voice.set_parameter(GENDER, 0)
                voice.set_parameter(NATIVE_PITCH, 65)
            parts = translate_text(api, cases[0][0])
            for speed in (40, 98, 200):
                for voice in (fa, en, de):
                    voice.set_parameter(NATIVE_SPEED, speed)
                base = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000)
                one = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000, german_flow=1)
                two = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000, german_flow=2)
                checks += verify(base, one, two, 1)
            for voice in (fa, en, de):
                voice.set_parameter(NATIVE_SPEED, 50)
            parts = translate_text(api, 'NVDA خوب و NVDA خیلی خوب است.')
            base = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000)
            one = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000, german_flow=1)
            two = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000, german_flow=2)
            checks += verify(base, one, two, 2)
    finally:
        for voice in (fa, en, de):
            voice.close()
    print(f'Passed {checks} German-guided flow, source and vowel-target checks.')


if __name__ == '__main__':
    main()
