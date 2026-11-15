"""Test real driver continuity, source indexes, and cancellation without NVDA."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
import threading

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from continuous_driver import _PlanningVoice
from continuous_plan import _context
from nvda_driver_probe import PERSIAN_PROBE, PINGLISH_PROBE, US_ENGLISH, make_language_engine
from _reloq_eci_direct import EciCapitalWord, EciIndex, EciLanguage, EciPitch, EciText, GENDER, PAUSE_MODE, SAMPLE_RATE
from text_frontend import SpeechPart


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for option in ('scope-probe', 'native-library', 'worker', 'espeak-library', 'data-parent'):
        parser.add_argument('--' + option, type=Path, required=True)
    args = parser.parse_args()

    planned = _context((SpeechPart('en-us', 'swapfile again'),), 'en-us', None, None, {},
                       ((0, 4, 0, 123, 8),), ((0, 0, 8),))
    assert planned == [EciText('swap', segmentation_guarded=True), EciIndex(123),
                       EciText('file', segmentation_guarded=True), ' again']

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
                assert render(engine, (text,))[0] == render(native, (text,))[0], text
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
            english_bracket, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), 'hello) '))
            engine.set_parameter(PAUSE_MODE, 0)
            persian_after, _ = render(engine, ('دنیا',))
            bracket, bracket_marks = render(engine, (EciIndex(30), 'hello) ', EciIndex(31),
                                                     'دنیا', EciIndex(32)))
            assert bracket == english_bracket + persian_after
            assert bracket_marks == [(30, 0), (31, len(english_bracket)), (32, len(bracket))]
            english_voice = engine._engines[US_ENGLISH.id]
            assert english_voice._dll.eciGetParam(english_voice._handle, PAUSE_MODE) == 0

            controlled, _ = render(engine, (EciPitch(80), 'hello) ', 'او آمد.'))
            engine.set_parameter(PAUSE_MODE, 1)
            expected_english, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), EciPitch(80), 'hello) '))
            engine.set_parameter(PAUSE_MODE, 0)
            expected_persian, _ = render(engine, (EciPitch(80), 'او آمد.'))
            assert controlled == expected_english + expected_persian

            # The same mixed fragment can also appear at the end of a job.
            # Its cached frames must not be reused with the other pause mode.
            engine._persian_plan_cache.clear()
            standalone, _ = render(engine, ('NVDA خوب (',))
            assert len(engine._persian_plan_cache) == 1
            assert render(engine, ('NVDA خوب (hello) دنیا',))[0]
            assert len(engine._persian_plan_cache) == 2
            assert render(engine, ('NVDA خوب (',))[0] == standalone

            english, _ = render(engine, (EciLanguage(US_ENGLISH.id, '1'), 'NVDA '))
            persian, _ = render(engine, ('خوب است.',))
            mixed, marks = render(engine, (EciIndex(10), 'NVDA ', EciIndex(11),
                                            'خوب است.', EciIndex(12)))
            assert mixed != english + persian
            assert marks[0] == (10, 0) and marks[-1] == (12, len(mixed))
            assert 0 < marks[1][1] < len(mixed) and marks[1][0] == 11
            # The second request has new NVDA index values but the same text
            # and settings. It must reuse control frames and remap marks.
            cache = engine._persian_plan_cache
            before = tuple(cache.items())
            repeated, remapped = render(engine, (EciIndex(110), 'NVDA ', EciIndex(111),
                                                  'خوب است.', EciIndex(112)))
            assert repeated == mixed
            assert remapped == [(110, marks[0][1]), (111, marks[1][1]), (112, marks[2][1])]
            assert len(cache) == len(before) and cache[next(reversed(cache))] is before[-1][1]
            engine.set_dictionary_path(engine._dictionary_path)
            assert cache, 'same immutable dictionary snapshot discarded the frame cache'
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
    print('Passed continuous Persian/English runtime, source indexes, glide and cancellation tests.')


if __name__ == '__main__':
    main()
