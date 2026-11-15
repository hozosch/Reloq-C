"""Verify direct native Ach friction after a voiced English boundary."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from contextual_continuation import _direct_ach_frication, accepted_contextual_compare
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import SpeechPart, translate_text


def compare(base, direct, expect: int, rate: int) -> int:
    assert len(direct.ach_frication_cuts) == expect
    assert direct.bridged_edges == base.bridged_edges
    assert not direct.ach_onset_edges
    keep = np.ones(len(base.controls), dtype=bool)
    removed_ms = 0.0
    for index, duration in direct.ach_frication_cuts:
        start = sum(base.frame_counts[:index])
        output_start = sum(direct.frame_counts[:index])
        cut = base.frame_counts[index] - direct.frame_counts[index]
        assert cut > 0 and duration > 0
        removed = base.controls[start:start + cut]
        assert np.all(removed[:, 2] > 0), 'cut contains a non-voiced frame'
        assert np.isclose(np.sum(removed[:, 0]), duration)
        assert direct.controls[output_start - 1, 2] > 0
        assert direct.controls[output_start - 1, 8] == 0
        assert direct.controls[output_start, 2] == 0
        assert direct.controls[output_start, 8] >= 20
        assert np.array_equal(direct.controls[output_start], base.controls[start + cut])
        keep[start:start + cut] = False
        removed_ms += duration
    assert np.array_equal(base.controls[keep], direct.controls), 'unrelated acoustic controls changed'
    samples_removed = (len(base.connected) - len(direct.connected)) // 2
    assert abs(samples_removed - removed_ms * rate / 1000) <= 2
    pcm = np.frombuffer(direct.connected, '<i2').astype(np.int32)
    assert np.any(pcm) and np.max(np.abs(pcm)) < 32767
    if not expect:
        assert direct.connected == base.connected
    return 11 + expect * 8


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
        # Do not mistake an unrelated S-like fricative for the Ach marker.
        synthetic = np.zeros((14, 62), dtype=np.float32)
        synthetic[:, 0] = 1
        synthetic[:6, 2] = 50
        synthetic[6:, 8] = 50
        synthetic[6:, 38:40] = 70
        parts = (SpeechPart('en-us', 'NVDA'), SpeechPart('fa', '`[.1Gub]'))
        unchanged, counts, cuts = _direct_ach_frication(synthetic, (1, 13), parts)
        assert not cuts and counts == (1, 13) and np.array_equal(unchanged, synthetic)
        checks += 1
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
                        base = accepted_contextual_compare(parts, fa, en, args.probe, rate, gender=gender)
                        direct = accepted_contextual_compare(parts, fa, en, args.probe, rate,
                                                             gender=gender, direct_ach=True)
                        checks += compare(base, direct, expected, rate)
            for voice in (fa, en):
                voice.set_parameter(SAMPLE_RATE, 3)
                voice.set_parameter(GENDER, 0)
                voice.set_parameter(NATIVE_PITCH, 65)
            parts = translate_text(api, cases[0][0])
            for speed in (40, 98, 200):
                for voice in (fa, en):
                    voice.set_parameter(NATIVE_SPEED, speed)
                base = accepted_contextual_compare(parts, fa, en, args.probe, 16000)
                direct = accepted_contextual_compare(parts, fa, en, args.probe, 16000, direct_ach=True)
                checks += compare(base, direct, 1, 16000)
    finally:
        fa.close()
        en.close()
    print(f'Passed {checks} direct-friction, voiced-vowel and preserved-control checks.')


if __name__ == '__main__':
    main()
