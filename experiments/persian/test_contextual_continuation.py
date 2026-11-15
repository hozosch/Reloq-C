"""Verify full-context multilingual frames and continuous-session replay."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
import tempfile

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, ROUGHNESS, SAMPLE_RATE, SAMPLE_RATES
from contextual_continuation import contextual_compare, replay
from espeak_ng_probe import EspeakPhonemeProbe
from shared_sentence_contour import FRAME_TRACE, OVERRIDE, TRACE, capture
from test_selected_probe import KEYS, engine, set_confirmed_persian_probe
from text_frontend import translate_text


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa = engine(args.probe, 0x90000)
    en = engine(args.probe, 0x10000)
    checks = 0
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api, tempfile.TemporaryDirectory() as folder:
            directory = Path(folder)
            sentences = ('من NVDA را دوست دارم.', 'آیا NVDA خوب است؟',
                         'من یک laptop دارم.', 'من Windows و NVDA را دوست دارم.')
            for mode in (1, 3, 6):
                rate = SAMPLE_RATES[mode]
                for gender in (0, 1):
                    for voice in (fa, en):
                        voice.set_parameter(SAMPLE_RATE, mode)
                        voice.set_parameter(GENDER, gender)
                        voice.set_parameter(NATIVE_PITCH, 81 if gender else 65)
                        voice.set_parameter(NATIVE_INFLECTION, 30)
                    set_confirmed_persian_probe()
                    # Replaying a whole set of frames must reproduce the real
                    # engine exactly, including consonants and wideband tuning.
                    for label, voice, text in (('fa', fa, '`[.1jek] `[.1sefr]'),
                                               ('en', en, 'Windows laptop')):
                        item = capture(voice, [text], directory / (label + '-f0'),
                                       frames=directory / (label + '-frames'))
                        assert item.frames is not None
                        same = replay(args.probe, item.frames['controls'], rate, gender=gender)
                        assert same == item.pcm, ('native replay differs', mode, gender, label)
                        checks += 1
                    for sentence in sentences:
                        parts = translate_text(api, sentence)
                        result = contextual_compare(parts, fa, en, args.probe, rate, gender=gender)
                        assert result.connected and len(result.connected) < len(result.prior.shared), (
                            'context did not remove terminal timing', mode, gender, sentence)
                        assert sum(result.frame_counts) == len(result.controls)
                        assert len(result.frame_counts) == len(parts)
                        audio = np.frombuffer(result.connected, '<i2').astype(np.int32)
                        assert np.any(audio) and np.max(np.abs(audio)) < 32767, ('empty or clipped', mode, gender)
                        checks += 3
                        # The replay consists exactly of contextual controls;
                        # only the shared pitch is changed from those plans.
                        assert replay(args.probe, result.controls, rate, gender=gender) == result.connected
                        checks += 1
                        intervals = []
                        for index, part in enumerate(parts):
                            item = result.context_after[part.language]
                            bounds = result.edges[part.language]
                            start = round(bounds[index] * 1000 / rate)
                            end = min(round(bounds[index + 1] * 1000 / rate), len(item.av))
                            voiced = np.flatnonzero(item.av[start:end] > 0)
                            assert len(voiced), ('no contextual voice', mode, gender, index)
                            intervals.append((item.f0[start + int(voiced[0])],
                                              item.f0[start + int(voiced[-1])]))
                        for previous, following in zip(intervals, intervals[1:]):
                            assert abs(float(previous[1] - following[0])) < .02, (
                                'pitch seam', mode, gender, sentence, previous, following)
                            checks += 1
                        # The final question rise and statement fall must
                        # survive the full-context frame selection too.
                        last_part = parts[-1]
                        final = result.context_after[last_part.language]
                        bounds = result.edges[last_part.language]
                        start = round(bounds[-2] * 1000 / rate)
                        end = min(round(bounds[-1] * 1000 / rate), len(final.av))
                        voiced = np.flatnonzero(final.av[start:end] > 0)
                        first, last = start + int(voiced[0]), start + int(voiced[-1])
                        earlier = first + (last - first) * 3 // 4
                        if sentence.endswith('؟'):
                            assert final.f0[last] > final.f0[earlier], ('question rise lost', mode, gender)
                        else:
                            assert final.f0[last] < final.f0[earlier], ('statement fall lost', mode, gender)
                        checks += 1
                        if sentence == sentences[0]:
                            # The previous isolated phrase fades to zero at
                            # each switch; full context retains voiced sources.
                            for index in (0, 1):
                                old = result.prior.after[index]
                                old_end = result.prior.connected_cuts[index] * 1000 // rate
                                contextual = result.context_before[parts[index].language]
                                end = result.edges[parts[index].language][index + 1] * 1000 // rate
                                assert np.mean(old.av[old_end - 10:old_end]) < 5
                                assert np.mean(contextual.av[end - 10:end]) > 20, (
                                    'continuation still fades', mode, gender, index)
                                checks += 2
                        assert all(k not in os.environ for k in (TRACE, OVERRIDE, FRAME_TRACE))
                        checks += 1
            # Do not silently replay voices with settings the session cannot
            # reproduce, such as a mismatched rate or nonzero roughness.
            parts = translate_text(api, sentences[0])
            fa.set_parameter(ROUGHNESS, 20)
            try:
                try:
                    contextual_compare(parts, fa, en, args.probe, 48000, gender=1)
                    raise AssertionError('unsupported roughness accepted')
                except ValueError:
                    checks += 1
            finally:
                fa.set_parameter(ROUGHNESS, 0)
            try:
                contextual_compare(parts, fa, en, args.probe, 16000, gender=1)
                raise AssertionError('mismatched rate accepted')
            except ValueError:
                checks += 1
    finally:
        for voice in (fa, en):
            voice.close()
        for key in (*KEYS, TRACE, OVERRIDE, FRAME_TRACE, 'RELOQ_FA_TRACE_E'):
            os.environ.pop(key, None)
    print(f'Passed {checks} full-context, continuous-replay and pitch checks.')


if __name__ == '__main__':
    main()
