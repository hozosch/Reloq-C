"""Compare the in-memory selected transitions with the accepted listening probe."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import FrameReplay
from continuous_plan import context_plan, selected_sentence
from contextual_continuation import accepted_persian_sentence_compare
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', required=True, type=Path)
    parser.add_argument('--reference', required=True, type=Path)
    parser.add_argument('--espeak-library', required=True, type=Path)
    parser.add_argument('--data-parent', required=True, type=Path)
    parser.add_argument('--full', action='store_true')
    args = parser.parse_args()
    voices = [engine(args.probe, d) for d in (0x90000, 0x10000, 0x40000)]
    old_voices = [engine(args.reference, d) for d in (0x90000, 0x10000, 0x40000)]
    fa, en, german = voices
    old_fa, old_en, old_german = old_voices
    scope = fa._dll.reloq_fa_acoustic_scope
    scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
    modes = [(mode, gender) for mode in (1, 3, 6) for gender in (0, 1)] if args.full else [(3, 0)]
    sentences = ('NVDA خوب است.', 'او آمد و NVDA خوب است.', 'من NVDA را دوست دارم.',
                 'NVDA خوب است و او آمد.', 'او، آمد و NVDA خوب است.')
    count = 0
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            for rate_code, gender in modes:
                for voice in voices + old_voices:
                    for key, value in ((SAMPLE_RATE, rate_code), (GENDER, gender),
                                       (NATIVE_PITCH, 81 if gender else 65),
                                       (NATIVE_INFLECTION, 30), (NATIVE_SPEED, 50)):
                        voice.set_parameter(key, value)
                for sentence in sentences:
                    parts = translate_text(api, sentence)
                    set_confirmed_persian_probe(d=2, z=2)
                    expected = accepted_persian_sentence_compare(parts, old_fa, old_en, old_german,
                                                                 args.reference, SAMPLE_RATES[rate_code],
                                                                 gender=gender)
                    assert scope(1) == 0
                    try:
                        plan = context_plan(parts, fa, en, SAMPLE_RATES[rate_code])
                        details = selected_sentence(plan, parts, en, german, SAMPLE_RATES[rate_code])
                    finally:
                        assert scope(0) == 1
                    assert plan.counts == expected.frame_counts, (sentence, plan.counts, expected.frame_counts)
                    assert details[0] == expected.bridged_edges
                    assert details[1] == expected.ach_frication_cuts
                    assert details[-1] == expected.persian_vowel_glides
                    got = np.asarray([list(row.controls) for row in plan.frames], dtype=np.float32)
                    difference = np.abs(got - expected.controls)
                    assert difference.shape == expected.controls.shape
                    assert np.max(difference) < .05, (sentence, rate_code, gender,
                                                     float(np.max(difference)),
                                                     np.unravel_index(np.argmax(difference), difference.shape))
                    pcm = bytearray()
                    assert FrameReplay(fa._dll).replay(fa, plan.frames, (), pcm.extend, lambda _: None)
                    assert len(pcm) == len(expected.connected), (sentence, rate_code, len(pcm), len(expected.connected))
                    signal = np.frombuffer(pcm, dtype='<i2').astype(np.int32)
                    reference_signal = np.frombuffer(expected.connected, dtype='<i2').astype(np.int32)
                    error = signal - reference_signal
                    assert np.sqrt(np.mean(error.astype(np.float64) ** 2)) < 2, (
                        sentence, rate_code, gender, float(np.sqrt(np.mean(error.astype(np.float64) ** 2))))
                    count += 1
    finally:
        for voice in voices + old_voices:
            voice.close()
    print(f'Passed {count} selected in-memory sentence and replay comparisons.')


if __name__ == '__main__':
    main()
