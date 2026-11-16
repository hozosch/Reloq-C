"""Verify independent in-process eSpeak, cancellation and native playback."""
from __future__ import annotations

import argparse
import ctypes
import json
from pathlib import Path
import sys
import threading
import time
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import EciIndex
from espeak_ng_probe import EspeakPhonemeProbe
from espeak_worker import EspeakWorker
from inprocess_espeak import InProcessEspeak, _Callback
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, make_language_engine


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=True)
    args = parser.parse_args()
    cases = (('fa', 'سلام. او آمد!'), ('fa', 'پژوهشگر سلام'),
             ('fa', 'آیا NVDA خوب است؟'), ('fa', 'ناموجودمطمئن،سلام؟'),
             ('fa-latn', 'salam donya'), ('fa-latn', 'khubtar'),
             ('en-us', 'NVDA is reading English.'), ('en-us', 'one, two, three?'))
    corpus = tuple((row['voice'], row['text']) for row in
                   (json.loads(line) for line in Path(__file__).with_name('reference_espeak_1_51.jsonl')
                    .read_text(encoding='utf8').splitlines()) if row['voice'] in ('fa', 'fa-latn'))
    with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as host, \
         EspeakWorker(args.worker, args.espeak_library, args.data_parent) as isolated:
        expected = {(voice, text, sep): isolated.translate(text, voice=voice, separator=sep)
                    for voice, text in cases for sep in ('', '|')}
        events = {(voice, text, slow): isolated.events(text, voice=voice, normal_speed=slow)
                  for voice, text in cases for slow in (False, True)}
        corpus_phones = {(voice, text): isolated.translate(text, voice=voice, separator='|')
                         for voice, text in corpus}
        corpus_events = {(voice, text): isolated.events(text, voice=voice) for voice, text in corpus}
        host._dll.espeak_SetParameter(1, 235, 0)
        host._dll.espeak_GetCurrentVoice.restype = ctypes.c_void_p
        host._dll.espeak_SetSynthCallback.argtypes = (_Callback,)
        host._dll.espeak_Synth.argtypes = (ctypes.c_void_p, ctypes.c_size_t, ctypes.c_uint, ctypes.c_int,
                                           ctypes.c_uint, ctypes.c_uint, ctypes.c_void_p, ctypes.c_void_p)
        host_callbacks = []
        @_Callback
        def host_callback(_audio, _samples, _events):
            host_callbacks.append(1)
            return 0
        host._dll.espeak_SetSynthCallback(host_callback)
        host.translate('NVDA is reading English.', voice='en-us')
        host_voice = host._dll.espeak_GetCurrentVoice()
        assert host_voice and host._dll.espeak_GetParameter(1, 1) == 235
        with patch('subprocess.Popen', side_effect=AssertionError('unexpected helper process')):
            first = InProcessEspeak(args.espeak_library, args.data_parent)
            second = InProcessEspeak(args.espeak_library, args.data_parent)
            try:
                for voice, text in cases:
                    for sep in ('', '|'):
                        assert first.translate(text, voice=voice, separator=sep) == expected[voice, text, sep]
                        assert second.translate(text, voice=voice, separator=sep) == expected[voice, text, sep]
                    for slow in (False, True):
                        assert first.events(text, voice=voice, normal_speed=slow) == events[voice, text, slow]
                    assert host._dll.espeak_GetCurrentVoice() == host_voice
                    assert host._dll.espeak_GetParameter(1, 1) == 235
                for voice, text in corpus:
                    assert first.translate(text, voice=voice, separator='|') == corpus_phones[voice, text]
                    assert first.events(text, voice=voice) == corpus_events[voice, text]
                sentence = ctypes.create_string_buffer(b'Testing the host eSpeak callback.\0')
                assert host._dll.espeak_Synth(sentence, len(sentence), 0, 1, 0, 1, None, None) == 0
                assert host_callbacks, 'the private callback replaced the host callback'
                # Stop both event synthesis and clause analysis from a
                # different thread. A fresh request can queue immediately.
                for operation in (first.events, first.translate):
                    started, failed, fresh, fresh_errors = threading.Event(), [], [], []
                    def long_analysis():
                        started.set()
                        try:
                            operation(' '.join(['پژوهشگر'] * 20000))
                        except RuntimeError as error:
                            failed.append(error)
                    def next_analysis():
                        try:
                            fresh.append(first.translate('پژوهشگر سلام', separator='|'))
                        except Exception as error:
                            fresh_errors.append(error)
                    thread = threading.Thread(target=long_analysis, daemon=True)
                    thread.start()
                    assert started.wait(5)
                    time.sleep(.01)
                    first.cancel()
                    successor = threading.Thread(target=next_analysis, daemon=True)
                    successor.start()
                    thread.join(5)
                    successor.join(5)
                    assert not thread.is_alive() and not successor.is_alive() and failed, 'in-process reference analysis did not cancel'
                    assert not fresh_errors and fresh == [expected['fa', 'پژوهشگر سلام', '|']]
                first.close()
                assert second.translate('salam donya', voice='fa-latn') == expected['fa-latn', 'salam donya', '']
                assert host._dll.espeak_GetCurrentVoice() == host_voice
                assert host._dll.espeak_GetParameter(1, 1) == 235
            finally:
                first.close()
                second.close()

    jobs = ((PERSIAN_PROBE.id, ('NVDA ', EciIndex(10), 'پژوهشگر ', EciIndex(11), 'سلام')),
            (PERSIAN_PROBE.id, ('سلام ', EciIndex(12), 'پژوهشگر ', EciIndex(13), 'window')),
            (PERSIAN_PROBE.id, ('ناموجودمطمئن ', EciIndex(14), 'سلام')),
            (PINGLISH_PROBE.id, ('salam ', EciIndex(15), 'khubtar?')))
    def render(worker):
        engine = make_language_engine(args.scope_probe, args.native_library, worker,
                                      args.espeak_library, args.data_parent)
        output = []
        try:
            for language, parts in jobs:
                engine.select_voice(language, '1')
                pcm, marks = bytearray(), []
                assert engine.speak(parts, pcm.extend, lambda value: marks.append((value, len(pcm))))
                output.append((bytes(pcm), tuple(marks)))
            if worker is None:
                assert isinstance(engine._engines[PERSIAN_PROBE.id]._frontend._worker, InProcessEspeak)
        finally:
            engine.close()
        return output
    reference = render(args.worker)
    with patch('subprocess.Popen', side_effect=AssertionError('unexpected helper process')):
        assert render(None) == reference
    print('Passed independent eSpeak state, no helper process, cancellation and Persian/Pinglish PCM/index parity.')


if __name__ == '__main__':
    main()
