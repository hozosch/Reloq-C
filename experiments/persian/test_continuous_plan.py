"""Compare in-memory full-context planning with the approved file probe."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import sys

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from continuous_plan import bridge_sonorants, context_plan, direct_ach
from contextual_continuation import contextual_compare
from espeak_ng_probe import EspeakPhonemeProbe
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import translate_text


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', required=True, type=Path)
    parser.add_argument('--reference', required=True, type=Path)
    parser.add_argument('--espeak-library', required=True, type=Path)
    parser.add_argument('--data-parent', required=True, type=Path)
    args = parser.parse_args()
    fa, en = engine(args.probe, 0x90000), engine(args.probe, 0x10000)
    old_fa, old_en = engine(args.reference, 0x90000), engine(args.reference, 0x10000)
    scope = fa._dll.reloq_fa_acoustic_scope
    scope.argtypes, scope.restype = (ctypes.c_int,), ctypes.c_int
    count = 0
    def compare_rows(new, old, sentence, rate, gender):
        assert new.counts == old.frame_counts, (sentence, new.counts, old.frame_counts)
        got = np.asarray([list(row.controls) for row in new.frames], dtype=np.float32)
        expected = old.controls
        assert got.shape == expected.shape
        differences = np.abs(got - expected)
        assert np.max(differences) < .02, (sentence, rate, gender,
                                           float(np.max(differences)),
                                           np.unravel_index(np.argmax(differences), differences.shape))
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            for rate_code in (1, 3, 6):
                for gender in (0, 1):
                    for voice in (fa, en, old_fa, old_en):
                        for key, value in ((SAMPLE_RATE, rate_code), (GENDER, gender),
                                           (NATIVE_PITCH, 81 if gender else 65),
                                           (NATIVE_INFLECTION, 30), (NATIVE_SPEED, 50)):
                            voice.set_parameter(key, value)
                    for sentence in ('NVDA خوب است.', 'من NVDA را دوست دارم.'):
                        parts = translate_text(api, sentence)
                        set_confirmed_persian_probe(d=2, z=2)
                        old = contextual_compare(parts, old_fa, old_en, args.reference,
                                                 SAMPLE_RATES[rate_code], gender=gender,
                                                 phone_context=True, selected_u_a=True)
                        assert not old.persian_vowel_glides
                        assert scope(1) == 0
                        try:
                            new = context_plan(parts, fa, en, SAMPLE_RATES[rate_code])
                        finally:
                            assert scope(0) == 1
                        assert new.edges == old.edges, (sentence, new.edges, old.edges)
                        compare_rows(new, old, sentence, rate_code, gender)
                        old_bridge = contextual_compare(parts, old_fa, old_en, args.reference,
                                                        SAMPLE_RATES[rate_code], gender=gender,
                                                        phone_context=True, bridge_ms=25, selected_u_a=True)
                        assert bridge_sonorants(new, parts, SAMPLE_RATES[rate_code]) == old_bridge.bridged_edges
                        compare_rows(new, old_bridge, sentence, rate_code, gender)
                        old_cut = contextual_compare(parts, old_fa, old_en, args.reference,
                                                     SAMPLE_RATES[rate_code], gender=gender,
                                                     phone_context=True, bridge_ms=25, direct_ach=True,
                                                     selected_u_a=True)
                        assert direct_ach(new, parts) == old_cut.ach_frication_cuts
                        compare_rows(new, old_cut, sentence, rate_code, gender)
                        count += 1
    finally:
        for voice in (fa, en, old_fa, old_en):
            voice.close()
    print(f'Passed {count} in-memory versus approved full-context plans.')


if __name__ == '__main__':
    main()
