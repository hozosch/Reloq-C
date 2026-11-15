"""Check the two private English A-to-Ach transition experiments."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from contextual_continuation import accepted_german_ach_compare
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


def verify(base, short, long, expected: int, *, can_extend: bool = True) -> None:
    assert base.frame_counts == short.frame_counts == long.frame_counts
    assert base.ach_frication_cuts == short.ach_frication_cuts == long.ach_frication_cuts
    assert len(base.german_ach_edges) == len(short.german_ach_edges) == expected
    assert len(long.english_a_extensions) == (expected if can_extend else 0)
    b, s, l = base.controls, short.controls, long.controls
    assert np.array_equal(b[:, 0], s[:, 0])
    assert np.array_equal(b[:, 1:8], s[:, 1:8])
    assert np.array_equal(b[:, 1:8], l[:, 1:8])
    assert len(base.connected) == len(short.connected)
    if not expected:
        assert np.array_equal(b, s) and np.array_equal(b, l)
        assert base.connected == short.connected == long.connected
        return
    starts = np.concatenate(([0.0], np.cumsum(b[:, 0], dtype=np.float64)))
    duration_changes = np.flatnonzero(b[:, 0] != l[:, 0])
    allowed_durations = np.zeros(len(b), dtype=bool)
    for edge, width in short.german_ach_edges:
        boundary = sum(base.frame_counts[:edge])
        previous_start = sum(base.frame_counts[:edge - 1])
        allowed_durations[previous_start:boundary] = True
        assert width <= dict(base.german_ach_edges)[edge]
        assert b[boundary, 2] == s[boundary, 2] == l[boundary, 2] == 0
        assert b[boundary, 8] == s[boundary, 8] == l[boundary, 8] >= 20
        assert np.all(b[duration_changes, 2] > 0)
        # Any resonance adjustment at the first X frames must have completed
        # after one 15-ms pitch-period bridge. The following vowel is native.
        part_end = sum(base.frame_counts[:edge + 1])
        rest = np.flatnonzero((starts[:-1] >= starts[boundary] + 15.01) &
                              (np.arange(len(b)) < part_end))
        assert np.allclose(b[rest], s[rest], rtol=0, atol=.01)
        assert np.allclose(b[rest], l[rest], rtol=0, atol=.01)
    assert np.all(allowed_durations[duration_changes])
    if can_extend:
        assert len(duration_changes) and len(long.connected) > len(base.connected)
        assert base.connected != long.connected
    else:
        assert not len(duration_changes)
    for item in (base, short, long):
        samples = np.frombuffer(item.connected, '<i2').astype(np.int32)
        assert np.any(samples) and np.max(np.abs(samples)) < 32767


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa, en, de = (engine(args.probe, d) for d in (0x90000, 0x10000, 0x40000))
    checked = 0
    try:
        set_confirmed_persian_probe()
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            cases = (('آیا NVDA خوب است؟', 1), ('NVDA خیلی خوب است.', 1),
                     ('آیا NVDA خراب است؟', 1), ('من یک laptop دارم.', 0),
                     ('من NVDA را دوست دارم.', 0))
            settings = [(mode, gender, 50) for mode in (1, 3, 6) for gender in (0, 1)]
            settings += [(3, 0, speed) for speed in (40, 98, 200)]
            for mode, gender, speed in settings:
                rate = SAMPLE_RATES[mode]
                for voice in (fa, en, de):
                    for parameter, value in ((SAMPLE_RATE, mode), (GENDER, gender),
                                             (NATIVE_PITCH, 81 if gender else 65),
                                             (NATIVE_INFLECTION, 30), (NATIVE_SPEED, speed)):
                        voice.set_parameter(parameter, value)
                for sentence, expected in cases:
                    parts = translate_text(api, sentence)
                    kwargs = {'gender': gender}
                    base = accepted_german_ach_compare(parts, fa, en, de, args.probe, rate, **kwargs)
                    short = accepted_german_ach_compare(parts, fa, en, de, args.probe, rate,
                                                         german_overlap=0.4, **kwargs)
                    long = accepted_german_ach_compare(parts, fa, en, de, args.probe, rate,
                                                        english_a_stretch=1.25, **kwargs)
                    try:
                        verify(base, short, long, expected, can_extend=speed != 200)
                    except AssertionError as error:
                        raise AssertionError((mode, gender, speed, sentence,
                                              base.german_ach_edges, short.german_ach_edges,
                                              long.english_a_extensions)) from error
                    checked += 1
            for voice in (fa, en, de):
                for parameter, value in ((SAMPLE_RATE, 3), (GENDER, 0),
                                         (NATIVE_PITCH, 65), (NATIVE_SPEED, 50)):
                    voice.set_parameter(parameter, value)
            parts = translate_text(api, 'NVDA خوب و NVDA خیلی خوب است.')
            base = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000)
            short = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000, german_overlap=.4)
            long = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000, english_a_stretch=1.25)
            verify(base, short, long, 2)
            checked += 1
    finally:
        for voice in (fa, en, de):
            voice.close()
    print(f'Passed {checked} incoming-transition cases across rates, voices and speeds.')


if __name__ == '__main__':
    main()
