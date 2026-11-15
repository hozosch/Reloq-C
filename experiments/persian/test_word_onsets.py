"""Check private Persian word-initial glottalization comparisons."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from espeak_ng_probe import EspeakPhonemeProbe
from persian_word_onsets import compare_onsets
from test_selected_probe import engine, set_confirmed_persian_probe


def verify(result, expected: int) -> None:
    assert len(result.changed) == expected, (result.tags, result.changed, expected)
    a, b = result.original_controls, result.softer_controls
    assert len(result.baseline) == len(result.softer)
    assert np.array_equal(a[:, :3], b[:, :3])
    assert np.array_equal(a[:, 4:], b[:, 4:])
    allowed = np.zeros(len(a), dtype=bool)
    for i, start, end, minimum, normal in result.changed:
        assert i > 0 and 'w' not in result.tags[i] and 'w' not in result.tags[i - 1]
        assert minimum < normal * .5 and end > start
        allowed[start:end] = True
        assert np.all(b[start:end, 3] >= a[start:end, 3])
        assert np.isclose(np.min(b[start:end, 3]), (minimum + normal) / 2)
    assert np.array_equal(a[~allowed, 3], b[~allowed, 3])
    if expected:
        assert result.baseline != result.softer
    else:
        assert result.baseline == result.softer
    for audio in (result.baseline, result.softer):
        samples = np.frombuffer(audio, '<i2').astype(np.int32)
        assert np.any(samples) and np.max(np.abs(samples)) < 32767


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa = engine(args.probe, 0x90000)
    checked = 0
    try:
        # This historical comparison intentionally captures the original
        # automatic Finnish onset before applying its 50% frame-level trial.
        set_confirmed_persian_probe(no_auto_glottal=0)
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            assert '?' in api.translate('ساعت', separator='|')[0].phonemes
            assert '?' not in api.translate('ایران', separator='|')[0].phonemes
            cases = (('من ایران را دوست دارم.', 1), ('او آمد.', 1),
                     ('من امروز اینجا هستم.', 2), ('خوب است.', 1),
                     ('یک آدم.', 1), ('من ساعت دارم.', 0), ('ایران', 0),
                     ('ساعت ایران.', 0))
            settings = [(mode, gender, 50) for mode in (1, 3, 6) for gender in (0, 1)]
            for mode, gender, speed in settings:
                for parameter, value in ((SAMPLE_RATE, mode), (GENDER, gender),
                                         (NATIVE_PITCH, 81 if gender else 65),
                                         (NATIVE_INFLECTION, 30), (NATIVE_SPEED, speed)):
                    fa.set_parameter(parameter, value)
                for text, expected in cases:
                    result = compare_onsets(api, fa, args.probe, text, SAMPLE_RATES[mode], gender=gender)
                    verify(result, expected)
                    checked += 1
            for speed in (40, 98, 200):
                for parameter, value in ((SAMPLE_RATE, 3), (GENDER, 0), (NATIVE_PITCH, 65),
                                         (NATIVE_SPEED, speed)):
                    fa.set_parameter(parameter, value)
                for text, expected in cases[:2]:
                    # At speed 200 the native onset constriction is absent;
                    # the conservative detector must leave it untouched.
                    verify(compare_onsets(api, fa, args.probe, text, 16000),
                           0 if speed == 200 else expected)
                    checked += 1
    finally:
        fa.close()
    print(f'Passed {checked} Persian word-onset, explicit-stop and source checks.')


if __name__ == '__main__':
    main()
