"""Check both selected transitions in the same private mixed-language plan."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_glide import TRACT
from contextual_continuation import accepted_persian_ach_compare, accepted_persian_sentence_compare
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


def verify(old, new, parts, expected):
    assert len(new.persian_vowel_glides) == expected, (parts, new.persian_vowel_glides)
    assert old.frame_counts == new.frame_counts
    assert old.ach_frication_cuts == new.ach_frication_cuts
    assert old.german_ach_edges == new.german_ach_edges
    assert old.english_a_extensions == new.english_a_extensions
    assert old.english_voice_releases == new.english_voice_releases
    assert len(old.connected) == len(new.connected)
    before, after = old.controls, new.controls
    assert before.shape == after.shape
    permitted_tract = np.zeros(len(before), dtype=bool)
    permitted_pitch = np.zeros(len(before), dtype=bool)
    for part, word, first, end, peak, pitch_end in new.persian_vowel_glides:
        assert parts[part].language == 'fa' and word > 0
        offset = sum(new.frame_counts[:part])
        assert 0 <= first < end < new.frame_counts[part]
        permitted_tract[offset + first:offset + end + 1] = True
        if peak >= 0:
            assert first <= peak < pitch_end < new.frame_counts[part]
            permitted_pitch[offset + peak:offset + pitch_end + 1] = True
    for column in range(before.shape[1]):
        allowed = permitted_tract if column in TRACT else permitted_pitch if column == 1 else None
        if allowed is None:
            assert np.array_equal(before[:, column], after[:, column]), column
        else:
            assert np.array_equal(before[~allowed, column], after[~allowed, column]), column
    if expected:
        assert old.connected != new.connected
    else:
        assert np.array_equal(before, after) and old.connected == new.connected
    for pcm in (old.connected, new.connected):
        samples = np.frombuffer(pcm, '<i2').astype(np.int32)
        assert np.any(samples) and np.max(np.abs(samples)) < 32767


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa, en, de = (engine(args.probe, dialect) for dialect in (0x90000, 0x10000, 0x40000))
    checked = 0
    try:
        set_confirmed_persian_probe()
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            cases = (('او آمد و NVDA خوب است.', 1),
                     ('NVDA خوب است و او آمد.', 1),
                     ('او آمد و او آنجا بود و NVDA خوب است.', 2),
                     ('او، آمد و NVDA خوب است.', 0),
                     ('او ایران را دید و NVDA خوب است.', 0),
                     ('من ساعت دارم و NVDA خوب است.', 0),
                     ('NVDA خوب است.', 0))
            settings = [(mode, gender, 50) for mode in (1, 3, 6) for gender in (0, 1)]
            settings.extend((3, 0, speed) for speed in (40, 98, 200))
            for mode, gender, speed in settings:
                for voice in (fa, en, de):
                    for parameter, value in ((SAMPLE_RATE, mode), (GENDER, gender),
                                             (NATIVE_PITCH, 81 if gender else 65),
                                             (NATIVE_INFLECTION, 30), (NATIVE_SPEED, speed)):
                        voice.set_parameter(parameter, value)
                for sentence, expected in cases:
                    parts = translate_text(api, sentence)
                    old = accepted_persian_ach_compare(parts, fa, en, de, args.probe,
                                                        SAMPLE_RATES[mode], gender=gender)
                    new = accepted_persian_sentence_compare(parts, fa, en, de, args.probe,
                                                             SAMPLE_RATES[mode], gender=gender)
                    try:
                        verify(old, new, parts, expected)
                    except AssertionError as error:
                        raise AssertionError((mode, gender, speed, sentence)) from error
                    if sentence == cases[0][0] and speed == 50:
                        assert new.ach_frication_cuts and new.english_voice_releases
                    checked += 1
    finally:
        for voice in (fa, en, de):
            voice.close()
    print(f'Passed {checked} combined Persian/English sentence cases.')


if __name__ == '__main__':
    main()
