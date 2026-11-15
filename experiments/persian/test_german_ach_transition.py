"""Check a German-timed, voiced-vowel-to-unvoiced-Ach transition."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
import tempfile

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (
    GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES,
)
from contextual_continuation import accepted_contextual_compare
from espeak_ng_probe import EspeakPhonemeProbe
from german_ach_transition import capture_german_ach
from shared_sentence_contour import capture
from test_selected_probe import engine, set_confirmed_persian_probe, set_probe
from text_frontend import translate_text


def verify(base, noise, shaped, expected: int) -> int:
    assert len(noise.german_ach_edges) == len(shaped.german_ach_edges) == expected
    assert noise.frame_counts == shaped.frame_counts == base.frame_counts
    assert noise.ach_frication_cuts == shaped.ach_frication_cuts == base.ach_frication_cuts
    assert len(base.connected) == len(noise.connected) == len(shaped.connected)
    # No voiced phoneme or pitch is inserted: the same English vowel is
    # present while its last frames receive the beginning of Ach noise.
    assert np.array_equal(base.controls[:, :8], noise.controls[:, :8])
    assert np.array_equal(base.controls[:, :8], shaped.controls[:, :8])
    starts = np.concatenate(([0.0], np.cumsum(base.controls[:, 0], dtype=np.float64)))
    allowed = np.zeros(len(base.controls), dtype=bool)
    for edge, width in noise.german_ach_edges:
        n = sum(base.frame_counts[:edge])
        assert 0 < width <= 100
        assert base.controls[n - 1, 2] > 0 and base.controls[n, 2] == 0
        assert base.controls[n - 1, 8] == 0 and base.controls[n, 8] >= 20
        assert noise.controls[n - 1, 2] == base.controls[n - 1, 2]
        assert noise.controls[n - 1, 8] > 0
        end = sum(base.frame_counts[:edge + 1])
        assert np.array_equal(noise.controls[n:end, 8], base.controls[n:end, 8])
        assert np.array_equal(noise.controls[n, [8, 35, 36, 37, 38, 39, 40, 41, 42]],
                              base.controls[n, [8, 35, 36, 37, 38, 39, 40, 41, 42]])
        allowed |= ((starts[:-1] >= starts[n] - width) & (starts[:-1] < starts[n]))
    assert np.array_equal(noise.controls[~allowed, 8], base.controls[~allowed, 8])
    assert np.array_equal(noise.controls[:, 9:35], base.controls[:, 9:35])
    assert len(shaped.german_f3_edges) == expected
    allowed_f3 = np.zeros(len(base.controls), dtype=bool)
    for edge, width in shaped.german_f3_edges:
        n = sum(base.frame_counts[:edge])
        allowed_f3 |= ((starts[:-1] >= starts[n]) & (starts[:-1] <= starts[n] + width))
    assert np.array_equal(noise.controls[~allowed_f3, 15], shaped.controls[~allowed_f3, 15])
    assert np.all(shaped.controls[allowed_f3, 2] == 0)
    assert np.all(shaped.controls[allowed_f3, 8] > 0)
    for candidate in (noise, shaped):
        pcm = np.frombuffer(candidate.connected, '<i2').astype(np.int32)
        assert np.any(pcm) and np.max(np.abs(pcm)) < 32767
    if expected:
        assert base.connected != noise.connected
        if any(width > 1 for _, width in noise.german_ach_edges):
            assert noise.connected != shaped.connected
    else:
        assert base.connected == noise.connected == shaped.connected
    return 11 + 7 * expected


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa, en, de = (engine(args.probe, dialect) for dialect in (0x90000, 0x10000, 0x40000))
    checks = 0
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            cases = (('آیا NVDA خوب است؟', 1), ('NVDA خیلی خوب است.', 1),
                     ('آیا Windows خراب است؟', 0), ('من یک laptop دارم.', 0),
                     ('NVDA خوب و NVDA خیلی خوب است.', 2))
            for mode in (1, 3, 6):
                rate = SAMPLE_RATES[mode]
                for gender in (0, 1):
                    for voice in (fa, en, de):
                        for parameter, value in ((SAMPLE_RATE, mode), (GENDER, gender),
                                                 (NATIVE_PITCH, 81 if gender else 65),
                                                 (NATIVE_INFLECTION, 30), (NATIVE_SPEED, 50)):
                            voice.set_parameter(parameter, value)
                    set_confirmed_persian_probe()
                    template = capture_german_ach(de)
                    assert 10 <= template.duration <= 20
                    checks += 1
                    for sentence, expected in cases:
                        parts = translate_text(api, sentence)
                        base = accepted_contextual_compare(parts, fa, en, args.probe, rate,
                                                           gender=gender, direct_ach=True, ach_release=True)
                        noise = accepted_contextual_compare(parts, fa, en, args.probe, rate,
                                                            gender=gender, direct_ach=True, ach_release=True,
                                                            german=de)
                        shaped = accepted_contextual_compare(parts, fa, en, args.probe, rate,
                                                             gender=gender, direct_ach=True, ach_release=True,
                                                             german=de, german_formants=True)
                        checks += verify(base, noise, shaped, expected)
            for voice in (fa, en, de):
                voice.set_parameter(SAMPLE_RATE, 3)
                voice.set_parameter(GENDER, 0)
                voice.set_parameter(NATIVE_PITCH, 65)
            parts = translate_text(api, cases[0][0])
            for speed in (40, 98, 200):
                for voice in (fa, en, de):
                    voice.set_parameter(NATIVE_SPEED, speed)
                base = accepted_contextual_compare(parts, fa, en, args.probe, 16000,
                                                   direct_ach=True, ach_release=True)
                noise = accepted_contextual_compare(parts, fa, en, args.probe, 16000,
                                                    direct_ach=True, ach_release=True, german=de)
                shaped = accepted_contextual_compare(parts, fa, en, args.probe, 16000,
                                                     direct_ach=True, ach_release=True, german=de,
                                                     german_formants=True)
                checks += verify(base, noise, shaped, 1)
            # The timing reference is corroborated by an ordinary German
            # word with a diphthong before /x/, not only by the forced tag.
            for voice in (fa, en, de):
                voice.set_parameter(NATIVE_SPEED, 50)
            set_probe()
            with tempfile.TemporaryDirectory() as tmp:
                spoken = capture(de, ['rauchen'], Path(tmp) / 'f0', frames=Path(tmp) / 'frames')
            c = spoken.frames['controls']
            begin = np.flatnonzero((c[:, 2] > 0) & (c[:, 8] > 0) & (c[:, 36] > 0) & (c[:, 38] > 0))
            end = np.flatnonzero((c[:, 2] == 0) & (c[:, 8] >= 20) & (c[:, 36] >= 50) & (c[:, 38] >= 35))
            assert len(begin) and len(end) and 10 <= int(end[0] - begin[0]) <= 20
            checks += 1
            set_confirmed_persian_probe()
            de.set_parameter(NATIVE_SPEED, 98)
            try:
                accepted_contextual_compare(parts, fa, en, args.probe, 16000, direct_ach=True,
                                            german=de)
                raise AssertionError('mismatched German reference speed accepted')
            except ValueError:
                checks += 1
    finally:
        for voice in (fa, en, de):
            voice.close()
    print(f'Passed {checks} German-template, voiced-vowel and direct-Ach checks.')


if __name__ == '__main__':
    main()
