"""Check the private English A voice-release listening candidates."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from contextual_continuation import accepted_german_ach_compare, accepted_persian_ach_compare, replay
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


def verify(base, faded, stretched, expected: int, *, extreme: bool = False) -> None:
    assert base.frame_counts == faded.frame_counts == stretched.frame_counts
    assert base.ach_frication_cuts == faded.ach_frication_cuts == stretched.ach_frication_cuts
    a, b, c = base.controls, faded.controls, stretched.controls
    assert np.array_equal(a[:, :2], b[:, :2])
    assert np.array_equal(a[:, 3:], b[:, 3:])
    assert np.array_equal(a[:, 1], c[:, 1])
    assert np.array_equal(a[:, 3:8], c[:, 3:8])
    assert len(base.connected) == len(faded.connected)
    if not expected:
        assert not faded.english_voice_releases and not stretched.english_a_extensions
        assert np.array_equal(a, b) and np.array_equal(a, c)
        assert base.connected == faded.connected == stretched.connected
        return
    if not extreme:
        assert len(faded.english_voice_releases) == len(stretched.english_voice_releases) == expected
        assert len(stretched.english_a_extensions) == expected
        assert base.connected != faded.connected != stretched.connected
        assert len(stretched.connected) > len(base.connected)
    allowed = np.zeros(len(a), dtype=bool)
    starts = np.concatenate(([0.0], np.cumsum(a[:, 0], dtype=np.float64)))
    for edge, width, amount in faded.english_voice_releases:
        n = sum(base.frame_counts[:edge])
        left = sum(base.frame_counts[:edge - 1])
        allowed[left:n] = True
        assert 0 < width <= 24.1 and amount == 12
        assert a[n, 2] == b[n, 2] == c[n, 2] == 0
        assert a[n, 8] == b[n, 8] == c[n, 8] >= 20
        assert a[n - 1, 2] - b[n - 1, 2] == 12
        assert b[n - 1, 2] > 0
        # The very start of X and its following vowel retain the same source.
        part_end = sum(base.frame_counts[:edge + 1])
        assert np.array_equal(a[n:part_end, 2], b[n:part_end, 2])
        assert np.array_equal(a[:, 8], b[:, 8])
        assert np.all(b[left:n, 2] <= a[left:n, 2])
        assert np.all(b[left:n, 2] >= 0)
    changed = a[:, 2] != b[:, 2]
    assert np.all(allowed[changed])
    if not extreme:
        assert np.any(changed)
    for result in (base, faded, stretched):
        samples = np.frombuffer(result.connected, '<i2').astype(np.int32)
        assert np.any(samples) and np.max(np.abs(samples)) < 32767


def verify_audible_voice_change(base, faded, library: Path) -> None:
    """Measure isolated periodic output; do not confuse added noise with AV."""
    edge = faded.english_voice_releases[0][0]
    boundary = sum(base.frame_counts[:edge])
    sample = round(float(np.sum(base.controls[:boundary, 0])) * 16)
    data = []
    for item in (base, faded):
        controls = item.controls.copy()
        controls[:, 7] = 0  # suppress AH for this diagnostic only
        controls[:, 8] = 0  # suppress AF for this diagnostic only
        data.append(np.frombuffer(replay(library, controls, 16000), '<i2').astype(float))
    def rms(values):
        return float(np.sqrt(np.mean(values * values)))
    assert np.array_equal(data[0][:sample - 25 * 16], data[1][:sample - 25 * 16])
    ratio = rms(data[1][sample - 6 * 16:sample]) / rms(data[0][sample - 6 * 16:sample])
    assert .1 < ratio < .8, ratio


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa, en, de = (engine(args.probe, d) for d in (0x90000, 0x10000, 0x40000))
    checked = 0
    try:
        set_confirmed_persian_probe(no_auto_glottal=1)
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            cases = (('آیا NVDA خوب است؟', 1), ('NVDA خیلی خوب است.', 1),
                     ('آیا NVDA خراب است؟', 1), ('من یک laptop دارم.', 0))
            settings = [(mode, gender, 50) for mode in (1, 3, 6) for gender in (0, 1)]
            settings += [(3, 0, speed) for speed in (40, 98, 200)]
            for mode, gender, speed in settings:
                for voice in (fa, en, de):
                    for parameter, value in ((SAMPLE_RATE, mode), (GENDER, gender),
                                             (NATIVE_PITCH, 81 if gender else 65),
                                             (NATIVE_INFLECTION, 30), (NATIVE_SPEED, speed)):
                        voice.set_parameter(parameter, value)
                rate = SAMPLE_RATES[mode]
                for sentence, expected in cases:
                    parts = translate_text(api, sentence)
                    kw = {'gender': gender}
                    base = accepted_german_ach_compare(parts, fa, en, de, args.probe, rate, **kw)
                    fade = accepted_german_ach_compare(parts, fa, en, de, args.probe, rate,
                                                        voice_release_ms=24, **kw)
                    long = accepted_german_ach_compare(parts, fa, en, de, args.probe, rate,
                                                        voice_release_ms=24, english_a_stretch=1.15, **kw)
                    try:
                        verify(base, fade, long, expected, extreme=speed == 200)
                    except AssertionError as error:
                        raise AssertionError((mode, gender, speed, sentence)) from error
                    if (mode, gender, speed, sentence) == (3, 0, 50, cases[0][0]):
                        verify_audible_voice_change(base, fade, args.probe)
                        selected = accepted_persian_ach_compare(parts, fa, en, de, args.probe, rate)
                        assert np.array_equal(selected.controls, long.controls)
                        assert selected.connected == long.connected
                    checked += 1
            for voice in (fa, en, de):
                for parameter, value in ((SAMPLE_RATE, 3), (GENDER, 0),
                                         (NATIVE_PITCH, 65), (NATIVE_SPEED, 50)):
                    voice.set_parameter(parameter, value)
            parts = translate_text(api, 'NVDA خوب و NVDA خیلی خوب است.')
            base = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000)
            fade = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000, voice_release_ms=24)
            long = accepted_german_ach_compare(parts, fa, en, de, args.probe, 16000,
                                                voice_release_ms=24, english_a_stretch=1.15)
            verify(base, fade, long, 2)
            selected = accepted_persian_ach_compare(parts, fa, en, de, args.probe, 16000)
            assert np.array_equal(selected.controls, long.controls) and selected.connected == long.connected
            checked += 1
    finally:
        for voice in (fa, en, de):
            voice.close()
    print(f'Passed {checked} voice-release cases plus isolated-source audio measurement.')


if __name__ == '__main__':
    main()
