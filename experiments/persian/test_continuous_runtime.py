"""Test real driver continuity, source indexes, and cancellation without NVDA."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import random
import struct
import sys
import threading

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from continuous_driver import _PlanningVoice, bracket_jobs, continuation_comma
from continuous_plan import (CapturedRows, _context, _dense, _fit_context_pitch, _last_audible,
                             _starts, persian_proxy_for_english)
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH, make_language_engine
from _reloq_eci_direct import EciCapitalWord, EciIndex, EciLanguage, EciPitch, EciText, GENDER, PAUSE_MODE, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture, NativeFrame
from text_frontend import SpeechPart


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=option != 'worker')
    args = parser.parse_args()

    planned = _context((SpeechPart('en-us', 'swapfile again'),), 'en-us', None, None, {},
                       ((0, 4, 0, 123, 8),), ((0, 0, 8),))
    assert planned == [EciText('swap', segmentation_guarded=True), EciIndex(123),
                       EciText('file', segmentation_guarded=True), ' again']
    assert continuation_comma(('hello?', ')', EciIndex(1))) == ('hello?', ')', EciIndex(1))
    assert continuation_comma(('hello', ')', EciIndex(1))) == (
        'hello', ')', EciText(',', internal_punctuation=True), EciIndex(1))
    for text in ('version 3 - 5 ', 'version 3 – 5 ', 'well-known '):
        parts = (EciLanguage(US_ENGLISH.id, '1'), text,
                 EciLanguage(PERSIAN_PROBE.id, '1'), 'خوب')
        assert len(bracket_jobs(parts, (PERSIAN_PROBE.id, '1'),
                                fa_ids=(PERSIAN_PROBE.id, PINGLISH_PROBE.id),
                                english_id=US_ENGLISH.id)) == 1, text
    spelled = (EciLanguage(US_ENGLISH.id, '1'), EciText('-', character_mode=True),
               EciLanguage(PERSIAN_PROBE.id, '1'), 'خوب')
    assert len(bracket_jobs(spelled, (PERSIAN_PROBE.id, '1'),
                            fa_ids=(PERSIAN_PROBE.id, PINGLISH_PROBE.id),
                            english_id=US_ENGLISH.id)) == 1
    class ProxyProbe:
        def __init__(self, alter_context=False):
            self.calls, self.alter_context = [], alter_context
        def _phonemes_for_probe(self, value):
            self.calls.append(value)
            return value.replace('.1do', '.1to') if self.alter_context and ' ' in value else value
    proxy_source = '`[.1sA] `[.1do]'
    ordinary = ProxyProbe()
    assert persian_proxy_for_english(proxy_source, ordinary) == proxy_source
    assert len(ordinary.calls) == 1
    contextual = ProxyProbe(alter_context=True)
    assert persian_proxy_for_english(proxy_source, contextual) == proxy_source
    # Recheck the changed second word, but reuse the exact first word from
    # the batch. This preserves the contextual safeguard without repeating
    # every native probe in a long sentence.
    assert contextual.calls == [proxy_source, '`[.1do]']
    class FlapAndMismatchProbe:
        def __init__(self):
            self.calls = []
        def _phonemes_for_probe(self, value):
            self.calls.append(value)
            if ' ' in value:
                return value.replace('.1do', '.1Fo').replace('.1ka', '.1pa')
            return value
    mixed = FlapAndMismatchProbe()
    mixed_source = '`[.1do] `[.1ka]'
    assert persian_proxy_for_english(mixed_source, mixed) == mixed_source
    assert mixed.calls == [mixed_source, '`[.1ka]']
    class MovedCodaProbe:
        def _phonemes_for_probe(self, value):
            return value.replace('.0SAs.0to', '.0SA.0sto')
    # A coda moved to the next onset keeps the same phones and vowels in
    # the same syllables; it must not collapse a contour pilot to /m/.
    assert persian_proxy_for_english('`[.0SAs.0to.1SeS]', MovedCodaProbe()) == '`[.0SAs.0to.1SES]'
    class ChangedVowelProbe:
        def _phonemes_for_probe(self, value):
            return value.replace('.1pA', '.1pe')
    # A different stressed vowel is still unsafe and uses the neutral pilot.
    assert persian_proxy_for_english('`[.1pA.0te]', ChangedVowelProbe()) == '`[.1mA.0mE]'
    unmarked = '`[.0sA.0lam] `[.0do.0nja]'
    uniform_pilot = persian_proxy_for_english(unmarked, ProxyProbe())
    assert uniform_pilot == '`[.1sA.0lam] `[.1do.0nya]'
    assert unmarked == '`[.0sA.0lam] `[.0do.0nja]'

    def create():
        return make_language_engine(args.scope_probe, args.native_library, args.worker,
                                    args.espeak_library, args.data_parent)

    def render(voice, parts):
        pcm, indexes = bytearray(), []
        assert voice.speak(parts, pcm.extend, lambda value: indexes.append((value, len(pcm))))
        return bytes(pcm), indexes

    engine = create()
    try:
        native = engine._engines[PERSIAN_PROBE.id]
        api = FrameCapture(native._voice._dll)
        assert api._controls_only is not None and api._sample_count is not None and \
            api.dense_curve is not None and api.fit_pitch is not None, \
            'the native planning capabilities are not exported'
        assert all((api.last_audible_sample, api.frame_ticks, api.frame_timeline, api.recompute_frication))
        rng = random.Random(20261115)
        for _ in range(40):
            count = rng.randrange(100, 500)
            rows, ticks, tick = (NativeFrame * count)(), [], 0
            for row in rows:
                tick += rng.randrange(1, 4)
                row.tick = tick
                row.controls[0] = rng.choice((.5, 1, 1.5, 2.5))
                row.controls[1] = rng.uniform(70, 300)
                row.controls[2] = rng.choice((0, 0, 25, 40, 60))
                row.controls[8] = rng.choice((0, 25, 50))
                for col in (37, 38, 39):
                    row.controls[col] = rng.uniform(0, 70)
                ticks.append(tick)
            item = CapturedRows(b'', {}, rows, ticks)
            native_ticks = (ctypes.c_int64 * count)()
            assert api.frame_ticks(rows, count, native_ticks) == 0 and list(native_ticks) == ticks
            times = (ctypes.c_double * (count + 1))()
            assert api.frame_timeline(rows, count, times) == 0 and list(times) == _starts(rows)
            expected_flags = [int(c[8] > 0 and (c[38] > c[37] + 10 or c[39] > c[37] + 10))
                              for c in (row.controls for row in rows)]
            api.recompute_frication(rows, count)
            assert [row.s_frication for row in rows] == expected_flags
            samples = [rng.randrange(-32768, 32768) for _ in range(count)] + [0, -4, 4, 0] * 20
            pcm = struct.pack('<' + 'h' * len(samples), *samples)
            for rate in (8000, 11025, 16000, 22050, 48000):
                assert _last_audible(pcm, rate, native=api.last_audible_sample) == _last_audible(pcm, rate)
            reference = _dense(item, 1)
            accelerated = _dense(item, 1, native=api.dense_curve)
            assert len(reference) == len(accelerated) and all(
                struct.pack('f', a) == struct.pack('f', b) for a, b in zip(reference, accelerated))
            left = CapturedRows(b'', {}, (NativeFrame * count).from_buffer_copy(rows), ticks)
            right = CapturedRows(b'', {}, (NativeFrame * count).from_buffer_copy(rows), ticks)
            middle = (ticks[0] + ticks[-1] + 1) // 2
            bounds, pilot_bounds = (ticks[0], middle, ticks[-1] + 1), (0, middle - 1, ticks[-1])
            parts = (SpeechPart('fa', 'first'), SpeechPart('en-us', 'second'))
            for language in ('fa', 'en-us'):
                _fit_context_pitch(left, bounds, accelerated, pilot_bounds, parts, language, 1000)
                _fit_context_pitch(right, bounds, accelerated, pilot_bounds, parts, language, 1000,
                                   native=api.fit_pitch, native_edges=api.fit_pitch_edges)
            assert bytes(left.frames) == bytes(right.frames)
        assert native._frontend._worker is None
        assert render(engine, ('کتابم کتابت کتابش کتابها.',))[0]
        assert render(engine, ('من NVDA را دوست دارم.',))[0]
        assert native._frontend._worker is None, 'covered text started the reference worker'
        script, script_indexes = render(engine, (EciIndex(1), 'او ', EciIndex(2), 'آمد.', EciIndex(3)))
        engine.select_voice(PINGLISH_PROBE.id, '1')
        roman, roman_indexes = render(engine, (EciIndex(1), 'u ', EciIndex(2), 'amad.', EciIndex(3)))
        assert roman == script and roman_indexes == script_indexes
        assert engine._engines[PINGLISH_PROBE.id]._frontend._worker is None
        engine.select_voice(PERSIAN_PROBE.id, '1')
        for rate, gender in ((3, 0), (6, 1)):
            engine.set_parameter(SAMPLE_RATE, rate)
            engine.set_parameter(GENDER, gender)
            for text in ('او آمد.', 'او آرام بود.', 'او آنجا بود.'):
                plain, _ = render(native, (text,))
                marked, indexes = render(engine, (EciIndex(1), text, EciIndex(2)))
                assert marked != plain and len(marked) == len(plain), text
                assert indexes == [(1, 0), (2, len(marked))], (text, indexes)
            marked, indexes = render(engine, (EciIndex(3), 'او ', EciIndex(4),
                                               'آمد.', EciIndex(5)))
            unmarked, _ = render(engine, ('او آمد.',))
            assert marked == unmarked and indexes[0] == (3, 0) and indexes[-1] == (5, len(marked))
            assert 0 < indexes[1][1] < len(marked) and indexes[1][0] == 4
            for text in ('سلام دنیا.', 'بدبخت', 'تعجب', 'کتابم کتابها', 'او، آمد.'):
                output, baseline = render(engine, (text,))[0], render(native, (text,))[0]
                assert len(output) == len(baseline), text
                assert output != baseline, text  # US pitch over unchanged Persian frame timing
            # Inline pitch preceding a split job must not bypass the selected
            # Persian vowel-to-vowel transition.
            pitched, pitched_marks = render(engine, (EciIndex(6), EciPitch(80), 'او آمد.', EciIndex(7)))
            native_pitched, _ = render(native, (EciPitch(80), 'او آمد.'))
            assert pitched != native_pitched and len(pitched) == len(native_pitched)
            assert pitched_marks == [(6, 0), (7, len(pitched))]

            # The English audio at a bracket boundary is exactly the native
            # English job with end silence suppressed; the following audio is
            # exactly the Persian job, including its first sound.
            engine.set_parameter(PAUSE_MODE, 1)
            english_bracket, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), 'hello) ,'))
            english_final, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), 'hello) '))
            assert english_bracket != english_final, 'the bracket tone fell back to sentence-final'
            engine.set_parameter(PAUSE_MODE, 0)
            persian_after, _ = render(engine, ('دنیا',))
            bracket, bracket_marks = render(engine, (EciIndex(30), 'hello) ', EciIndex(31),
                                                     'دنیا', EciIndex(32)))
            assert bracket == english_bracket + persian_after
            assert bracket_marks == [(30, 0), (31, len(english_bracket)), (32, len(bracket))]
            english_voice = engine._engines[US_ENGLISH.id]
            assert english_voice._dll.eciGetParam(english_voice._handle, PAUSE_MODE) == 0

            # At a hyphen which coincides with a script switch, keep each
            # language's own nonfinal melody and its complete first sound.
            # An English-internal compound is still a single native segment.
            engine.set_parameter(PAUSE_MODE, 1)
            english_dash, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), 'NVDA-,'))
            persian_dash, _ = render(engine, ('سلام-,',))
            engine.set_parameter(PAUSE_MODE, 0)
            english_after, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), 'NVDA'))
            persian_after, _ = render(engine, ('خوب',))
            dashed, marks = render(engine, (EciIndex(130), 'NVDA-', EciIndex(131),
                                             'خوب', EciIndex(132)))
            assert dashed == english_dash + persian_after
            assert marks == [(130, 0), (131, len(english_dash)), (132, len(dashed))]
            dashed, marks = render(engine, (EciIndex(133), 'سلام-', EciIndex(134),
                                             'NVDA', EciIndex(135)))
            assert dashed == persian_dash + english_after
            assert marks == [(133, 0), (134, len(persian_dash)), (135, len(dashed))]
            assert render(engine, ('سلام-NVDA-خوب',))[0] == persian_dash + english_dash + persian_after
            for dash in ('‐', '–', '—'):
                assert render(engine, ('سلام' + dash + 'NVDA' + dash + 'خوب',))[0] == \
                    persian_dash + english_dash + persian_after, dash
            explicit, _ = render(engine, ('سلام-', EciLanguage(US_ENGLISH.id, '1'), 'NVDA-',
                                          EciLanguage(PERSIAN_PROBE.id, '1'), 'خوب'))
            assert explicit == persian_dash + english_dash + persian_after
            assert english_voice._dll.eciGetParam(english_voice._handle, PAUSE_MODE) == 0

            # English with a spaced dash has its own native pause even when
            # the following English word precedes a later Persian switch.
            engine.set_parameter(PAUSE_MODE, 1)
            spaced_prefix, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), 'NVDA - ,'))
            engine.set_parameter(PAUSE_MODE, 0)
            mixed_suffix, _ = render(engine, ('Text خوب',))
            spaced, marks = render(engine, (EciIndex(140), 'NVDA - ', EciIndex(141),
                                             'Text خوب', EciIndex(142)))
            assert spaced == spaced_prefix + mixed_suffix
            assert marks == [(140, 0), (141, len(spaced_prefix)), (142, len(spaced))]
            # Markers may divide the three characters around the dash.
            divided, _ = render(engine, ('NVDA ', EciIndex(143), '-', EciIndex(144),
                                         ' Text خوب'))
            assert divided == spaced
            for dash in ('‐', '–', '—'):
                assert render(engine, ('NVDA ' + dash + ' Text خوب',))[0] == spaced, dash

            controlled, _ = render(engine, (EciPitch(80), 'hello) ', 'او آمد.'))
            engine.set_parameter(PAUSE_MODE, 1)
            expected_english, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), EciPitch(80), 'hello) ,'))
            engine.set_parameter(PAUSE_MODE, 0)
            expected_persian, _ = render(engine, (EciPitch(80), 'او آمد.'))
            assert controlled == expected_english + expected_persian

            # The same mixed fragment can appear with different pause modes;
            # each request must be independently planned.
            standalone, _ = render(engine, ('NVDA خوب (',))
            assert render(engine, ('NVDA خوب (hello) دنیا',))[0]
            assert render(engine, ('NVDA خوب (',))[0] == standalone

            english, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), 'NVDA '))
            persian, _ = render(engine, ('خوب است.',))
            mixed, marks = render(engine, (EciIndex(10), 'NVDA ', EciIndex(11),
                                            'خوب است.', EciIndex(12)))
            assert mixed != english + persian
            assert marks[0] == (10, 0) and marks[-1] == (12, len(mixed))
            assert 0 < marks[1][1] < len(mixed) and marks[1][0] == 11
            # New marks for the same text must be calculated again, with the
            # same sound and positions and no retained sentence result.
            captures, original_capture = [], FrameCapture.capture
            def observe_capture(self, *args, **kwargs):
                captures.append(1)
                return original_capture(self, *args, **kwargs)
            FrameCapture.capture = observe_capture
            try:
                repeated, remapped = render(engine, (EciIndex(110), 'NVDA ', EciIndex(111),
                                                      'خوب است.', EciIndex(112)))
            finally:
                FrameCapture.capture = original_capture
            assert repeated == mixed
            assert remapped == [(110, marks[0][1]), (111, marks[1][1]), (112, marks[2][1])]
            assert captures, 'repeated sentence bypassed the native planner'
            for name in ('_persian_plan_cache', '_persian_phoneme_cache', '_persian_ach_cache'):
                assert not hasattr(engine, name), name
            engine.set_dictionary_path(engine._dictionary_path)
            assert render(engine, (EciIndex(110), 'NVDA ', EciIndex(111),
                                   'خوب است.', EciIndex(112))) == (repeated, remapped)
            explicit, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), 'NVDA ',
                                          EciLanguage(PERSIAN_PROBE.id, '1'), 'خوب است.'))
            assert explicit == mixed
            engine.select_voice(US_ENGLISH.id, '1')
            engine.set_parameter(GENDER, gender)
            from_english, _ = render(engine, ('NVDA ', EciLanguage(PERSIAN_PROBE.id, '1'), 'خوب است.'))
            assert from_english == mixed
            engine.select_voice(PERSIAN_PROBE.id, '1')
            engine.set_parameter(GENDER, gender)
            capital, _ = render(engine, (EciCapitalWord('NVDA', 'NV DA'), ' خوب است.'))
            assert capital and capital != english + persian
            shared_cache = engine._uppercase_cache
            previous_entries = tuple(shared_cache.items())
            capital_probes, original_probe = [], english_voice.spells_uppercase
            def observe_capital(value):
                capital_probes.append(value)
                return original_probe(value)
            english_voice.spells_uppercase = observe_capital
            try:
                for _ in range(2):
                    assert render(engine, (EciCapitalWord('NVDA', 'NV DA'), ' خوب است.'))[0] == capital
            finally:
                english_voice.spells_uppercase = original_probe
            assert len(capital_probes) >= 2, 'capitalization was retained between Persian requests'
            assert engine._uppercase_cache is shared_cache and tuple(shared_cache.items()) == previous_entries
            protected, indexes = render(engine, (EciText('swapfile ', segmentation_guarded=True),
                                                  EciIndex(13), 'خوب است.'))
            assert protected and indexes[0][0] == 13 and 0 < indexes[0][1] < len(protected)

            first, _ = render(engine, ('این یک آزمایش است.',))
            second, _ = render(engine, (' NVDA خوب است.',))
            sentence, marks = render(engine, (EciIndex(20), 'این یک آزمایش است.', EciIndex(21),
                                               ' NVDA ', EciIndex(22), 'خوب است.', EciIndex(23)))
            assert sentence == first + second
            assert [value for value, _ in marks] == [20, 21, 22, 23]
            assert marks[0][1] == 0 and marks[-1][1] == len(sentence)
    finally:
        engine.close()

    # A stop during sentence planning must not emit stale PCM or prevent the
    # next request. The engine is always created and closed by its worker.
    started, release, stop_done = threading.Event(), threading.Event(), threading.Event()
    shared, outcomes = {}, []
    original = _PlanningVoice._phonemes_for_probe
    def blocked(self, text):
        if not started.is_set():
            started.set()
            if not release.wait(5):
                raise RuntimeError('cancellation test timed out')
        return original(self, text)
    _PlanningVoice._phonemes_for_probe = blocked
    def worker():
        instance = create()
        shared['engine'] = instance
        try:
            abandoned = bytearray()
            outcomes.append(instance.speak(('NVDA خوب است.',), abandoned.extend, lambda _: None))
            outcomes.append(not abandoned)
            recovered, _ = render(instance, ('سلام.',))
            outcomes.append(bool(recovered))
        finally:
            instance.close()
    try:
        thread = threading.Thread(target=worker)
        thread.start()
        assert started.wait(10)
        def stop():
            shared['engine'].cancel()
            stop_done.set()
        canceller = threading.Thread(target=stop)
        canceller.start()
        assert stop_done.wait(1), 'stop blocked on sentence planning'
        release.set()
        thread.join(10)
        canceller.join(10)
        assert not thread.is_alive() and outcomes == [False, True, True], outcomes
    finally:
        release.set()
        _PlanningVoice._phonemes_for_probe = original

    # Also interrupt a control-only native context capture after it starts.
    # No discarded planning audio may leak, and a subsequent job must work.
    started, release, stopped = threading.Event(), threading.Event(), threading.Event()
    shared, outcomes = {}, []
    original_capture = FrameCapture.capture
    def blocking_capture(self, voice, parts, *, maximum=60000, audio=True):
        if audio or started.is_set():
            return original_capture(self, voice, parts, maximum=maximum, audio=audio)
        class BlockedVoice:
            def speak(self, sequence, on_pcm, on_index):
                def blocked_index(index):
                    started.set()
                    if not release.wait(5):
                        raise RuntimeError('control-only cancellation test timed out')
                    on_index(index)
                return voice.speak(sequence, on_pcm, blocked_index)
        return original_capture(self, BlockedVoice(), parts, maximum=maximum, audio=False)
    def capture_worker():
        instance = create()
        shared['engine'] = instance
        try:
            abandoned = bytearray()
            outcomes.append(instance.speak(('NVDA خوب است.',), abandoned.extend, lambda _: None))
            outcomes.append(not abandoned)
            recovered, _ = render(instance, ('سلام.',))
            outcomes.append(bool(recovered))
        finally:
            instance.close()
    FrameCapture.capture = blocking_capture
    try:
        thread = threading.Thread(target=capture_worker)
        thread.start()
        assert started.wait(10), 'control-only capture did not start'
        def stop_capture():
            shared['engine'].cancel()
            stopped.set()
        canceller = threading.Thread(target=stop_capture)
        canceller.start()
        assert stopped.wait(1), 'stop blocked on control-only capture'
        release.set()
        thread.join(10)
        canceller.join(10)
        assert not thread.is_alive() and outcomes == [False, True, True], outcomes
    finally:
        release.set()
        FrameCapture.capture = original_capture
    print('Passed continuous Persian/English runtime, source indexes, glide and cancellation tests.')


if __name__ == '__main__':
    main()
