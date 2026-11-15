"""Validate the private shared-F0 experiment against native speech frames."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
import threading

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, PAUSE_MODE, SAMPLE_RATE, SAMPLE_RATES
from espeak_ng_probe import EspeakPhonemeProbe
from listening import render
from mixed_language_probe import last_phone_is_stop, splice
from shared_sentence_contour import OVERRIDE, TRACE, _continuation_cut, _voiced_interval, compare, stream_shared
from test_selected_probe import KEYS, engine, set_confirmed_persian_probe
from text_frontend import SpeechPart, translate_text


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    args = parser.parse_args()
    fa = engine(args.probe, 0x90000)
    en = engine(args.probe, 0x10000)
    original = engine(args.baseline, 0x90000)
    checks = 0
    try:
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            sentences = ('من NVDA را دوست دارم.', 'آیا NVDA خوب است؟',
                         'من Windows و NVDA را دوست دارم.',
                         'من یک laptop دارم.', 'این یک test است.')
            for mode in (1, 3, 6):
                for gender in (0, 1):
                    for voice in (fa, en, original):
                        voice.set_parameter(SAMPLE_RATE, mode)
                        voice.set_parameter(GENDER, gender)
                        voice.set_parameter(NATIVE_PITCH, 81 if gender else 65)
                        voice.set_parameter(NATIVE_INFLECTION, 30)
                    set_confirmed_persian_probe()
                    # Merely linking the research hook must not change sound.
                    text = '`[.1ka.0RAz] `[.1dAhw]'
                    assert render(fa, text) == render(original, text), ('disabled hook changed audio', mode, gender)
                    checks += 1
                    for sentence in sentences:
                        parts = translate_text(api, sentence)
                        outcome = compare(parts, fa, en, SAMPLE_RATES[mode])
                        assert outcome.short_pause != outcome.baseline and len(outcome.short_pause) == len(outcome.baseline), (
                            'shared contour not applied', mode, gender, sentence)
                        assert len(outcome.shared) < len(outcome.short_pause), ('switch pauses remain', mode, gender, sentence)
                        checks += 1
                        for before, after in zip(outcome.before, outcome.after):
                            assert len(before.pcm) == len(after.pcm)
                            assert np.array_equal(before.av, after.av), ('voicing target changed', mode, gender)
                            checks += 1
                        for previous, following in zip(outcome.after, outcome.after[1:]):
                            end = _voiced_interval(previous)[1]
                            start = _voiced_interval(following)[0]
                            assert abs(float(previous.f0[end] - following.f0[start])) < .02, (
                                'discontinuous pitch targets', mode, gender, sentence,
                                previous.f0[end], following.f0[start])
                            checks += 1
                        final = outcome.after[-1]
                        first, last = _voiced_interval(final)
                        earlier = first + (last - first) * 3 // 4
                        if sentence.endswith('؟'):
                            assert final.f0[last] > final.f0[earlier], ('question rise lost', mode, gender)
                        else:
                            assert final.f0[last] < final.f0[earlier], ('statement ending lost', mode, gender)
                        checks += 1
                        assert TRACE not in os.environ and OVERRIDE not in os.environ, 'native hook leaked'
                        checks += 1
                        for index, item in enumerate(outcome.after[:-1]):
                            data = np.frombuffer(item.pcm, '<i2').astype(np.int32)
                            audible = np.flatnonzero(np.abs(data) > 4)
                            assert outcome.connected_cuts[index] == max(outcome.native_cuts[index], int(audible[-1]) + 1), (
                                'release or native stop cut', mode, gender, sentence, index)
                            assert outcome.connected_cuts[index] < len(data), ('terminal pause kept', mode, gender, sentence)
                            cut = outcome.connected_cuts[index]
                            following = np.frombuffer(outcome.after[index + 1].pcm, '<i2').astype(np.int32)
                            window = max(2, SAMPLE_RATES[mode] // 100)
                            local = max(int(np.max(np.abs(np.diff(data[max(0, cut-window):cut])))),
                                        int(np.max(np.abs(np.diff(following[:window])))))
                            jump = abs(int(data[cut-1]) - int(following[0]))
                            assert jump <= max(512, local), ('unexpected join spike', mode, gender, sentence, index, jump)
                            checks += 3
                        assert b''.join(item.pcm[:2 * cut] for item, cut in zip(outcome.after, outcome.connected_cuts)) == outcome.shared
                        assert all(v._dll.eciGetParam(v._handle, PAUSE_MODE) == 0 for v in (fa, en))
                        checks += 2
                        if sentence == sentences[0]:
                            streamed, indexes = bytearray(), []
                            assert stream_shared(parts, outcome, fa, en, SAMPLE_RATES[mode],
                                                 streamed.extend, indexes.append)
                            voices = {'fa': fa, 'en-us': en}
                            assert bytes(streamed) == outcome.shared, ('streamed output changed', mode, gender)
                            assert indexes == list(range(1, len(parts) + 1)), ('part markers', mode, gender, indexes)
                            checks += 3
                            old_stream, old_indexes = bytearray(), []
                            assert stream_shared(parts, outcome, fa, en, SAMPLE_RATES[mode],
                                                 old_stream.extend, old_indexes.append, seamless=False)
                            expected = splice([(item.pcm, part.text,
                                               last_phone_is_stop(voices[part.language]._phonemes_for_probe(part.text)))
                                              for part, item in zip(parts, outcome.after)], SAMPLE_RATES[mode])
                            assert bytes(old_stream) == expected
                            checks += 2
                            if mode == 3 and gender == 0:
                                cancelled = threading.Event()
                                delivered, stopped_indexes = [], []

                                def stop_after_first_audio(pcm: bytes) -> None:
                                    delivered.append(pcm)
                                    cancelled.set()

                                assert not stream_shared(parts, outcome, fa, en, SAMPLE_RATES[mode],
                                                         stop_after_first_audio, stopped_indexes.append, cancelled)
                                assert len(delivered) == 1 and len(stopped_indexes) <= 1, 'cancel leaked later audio or markers'
                                checks += 2
            # Explicit punctuation and explicit long pauses are not removed.
            sample = np.array([40, 40, 40] + [0] * 20, dtype='<i2').tobytes()
            assert _continuation_cut(sample, 3, 'word,') == len(sample) // 2
            assert _continuation_cut(sample, len(sample) // 2, 'word') == len(sample) // 2
            checks += 2
            for voice in (fa, en):
                voice.set_parameter(SAMPLE_RATE, 3)
                voice.set_parameter(GENDER, 0)
            for policy in (1, 2):
                for voice in (fa, en):
                    voice.set_parameter(PAUSE_MODE, policy)
                set_confirmed_persian_probe()
                parts = translate_text(api, sentences[0])
                outcome = compare(parts, fa, en, 16000)
                assert all(v._dll.eciGetParam(v._handle, PAUSE_MODE) == policy for v in (fa, en))
                streamed = bytearray()
                assert stream_shared(parts, outcome, fa, en, 16000, streamed.extend, lambda _: None)
                assert bytes(streamed) == outcome.shared
                checks += 3
            for voice in (fa, en):
                voice.set_parameter(PAUSE_MODE, 0)
            for explicit in (250, 500):
                set_confirmed_persian_probe()
                parts = (SpeechPart('fa', f'`[.0mAn] `p{explicit}'), SpeechPart('en-us', 'NVDA.'))
                outcome = compare(parts, fa, en, 16000)
                assert outcome.connected_cuts[0] == len(outcome.after[0].pcm) // 2, (
                    'explicit pause was shortened', explicit)
                checks += 1
            try:
                compare((SpeechPart('fa', '`[.0mAn],'), SpeechPart('en-us', 'NVDA.')), fa, en, 16000)
            except ValueError as error:
                assert 'separate clause' in str(error)
                checks += 1
            else:
                raise AssertionError('punctuated language boundary must not be silently joined')
            for speed in (40, 200):
                for voice in (fa, en):
                    voice.set_parameter(NATIVE_SPEED, speed)
                set_confirmed_persian_probe()
                outcome = compare(translate_text(api, sentences[3]), fa, en, 16000)
                assert len(outcome.shared) < len(outcome.short_pause), ('speed lost connected boundary', speed)
                checks += 1
    finally:
        for voice in (fa, en, original):
            voice.close()
        for key in (*KEYS, TRACE, OVERRIDE):
            os.environ.pop(key, None)
    print(f'Passed {checks} shared-contour, pitch-boundary and preserved-voice checks.')


if __name__ == '__main__':
    main()
