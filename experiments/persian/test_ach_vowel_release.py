"""Verify vowel-filter release while native Ach noise begins immediately."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
import tempfile

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (
    EciIndex, GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES,
)
from contextual_continuation import accepted_contextual_compare
from espeak_ng_probe import EspeakPhonemeProbe
from shared_sentence_contour import capture
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


def verify(base, released, expect: int) -> int:
    assert len(released.ach_release_edges) == expect
    assert released.ach_frication_cuts == base.ach_frication_cuts
    assert released.frame_counts == base.frame_counts
    assert released.bridged_edges == base.bridged_edges
    assert len(base.connected) == len(released.connected)
    source = list(range(9)) + list(range(35, 62))
    assert np.array_equal(base.controls[:, source], released.controls[:, source])
    starts = np.concatenate(([0.0], np.cumsum(base.controls[:, 0], dtype=np.float64)))
    allowed = np.zeros(len(base.controls), dtype=bool)
    for edge, duration in released.ach_release_edges:
        boundary = sum(base.frame_counts[:edge])
        assert 0 < duration <= 15
        assert released.controls[boundary - 1, 2] > 0
        assert released.controls[boundary, 2] == 0
        assert released.controls[boundary, 8] >= 20
        assert np.allclose(released.controls[boundary - 1, [9, 13, 15]],
                           released.controls[boundary, [9, 13, 15]], atol=.01)
        allowed |= ((starts[:-1] >= starts[boundary]) &
                    (starts[:-1] <= starts[boundary] + duration))
    assert np.array_equal(base.controls[~allowed], released.controls[~allowed])
    if expect:
        assert base.connected != released.connected
    else:
        assert np.array_equal(base.controls, released.controls) and base.connected == released.connected
    pcm = np.frombuffer(released.connected, '<i2').astype(np.int32)
    assert np.any(pcm) and np.max(np.abs(pcm)) < 32767
    return 9 + expect * 5


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
            cases = (('آیا NVDA خوب است؟', 1), ('NVDA خیلی خوب است.', 1),
                     ('آیا NVDA خراب است؟', 1), ('آیا Windows خراب است؟', 0),
                     ('من NVDA را دوست دارم.', 0), ('من یک laptop دارم.', 0),
                     ('NVDA خوب و NVDA خیلی خوب است.', 2))
            for mode in (1, 3, 6):
                rate = SAMPLE_RATES[mode]
                for gender in (0, 1):
                    for voice in (fa, en):
                        voice.set_parameter(SAMPLE_RATE, mode)
                        voice.set_parameter(GENDER, gender)
                        voice.set_parameter(NATIVE_PITCH, 81 if gender else 65)
                        voice.set_parameter(NATIVE_INFLECTION, 30)
                    for sentence, expected in cases:
                        parts = translate_text(api, sentence)
                        base = accepted_contextual_compare(parts, fa, en, args.probe, rate,
                                                           gender=gender, direct_ach=True)
                        release = accepted_contextual_compare(parts, fa, en, args.probe, rate,
                                                              gender=gender, direct_ach=True, ach_release=True)
                        checks += verify(base, release, expected)
            for voice in (fa, en):
                voice.set_parameter(SAMPLE_RATE, 3)
                voice.set_parameter(GENDER, 0)
                voice.set_parameter(NATIVE_PITCH, 65)
            parts = translate_text(api, cases[0][0])
            for speed in (40, 98, 200):
                for voice in (fa, en):
                    voice.set_parameter(NATIVE_SPEED, speed)
                base = accepted_contextual_compare(parts, fa, en, args.probe, 16000, direct_ach=True)
                release = accepted_contextual_compare(parts, fa, en, args.probe, 16000,
                                                       direct_ach=True, ach_release=True)
                checks += verify(base, release, 1)
            # At the default test speed the English vowel before the switch
            # has the same F1-F3 trajectory as native NVDA before English H.
            for voice in (fa, en):
                voice.set_parameter(NATIVE_SPEED, 50)
            mixed = accepted_contextual_compare(parts, fa, en, args.probe, 16000,
                                                direct_ach=True, ach_release=True)
            with tempfile.TemporaryDirectory() as temp:
                native = capture(en, ['NVDA ', EciIndex(1), 'hello.'], Path(temp) / 'f0',
                                 frames=Path(temp) / 'frames')
            assert native.frames is not None
            n = int(np.searchsorted(native.frames['tick'], round(native.indexes[1] * 1000 / 16000)))
            m = sum(mixed.frame_counts[:2])
            assert np.array_equal(native.frames['controls'][n - 100:n, [9, 13, 15]],
                                  mixed.controls[m - 100:m, [9, 13, 15]])
            checks += 1
    finally:
        fa.close()
        en.close()
    print(f'Passed {checks} vowel release, direct-noise and native-trajectory checks.')


if __name__ == '__main__':
    main()
