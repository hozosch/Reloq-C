"""Test word-anchored Persian reference fallback without playing eSpeak."""
from __future__ import annotations

import argparse
from dataclasses import replace
from pathlib import Path
import threading

from direct_fa_sequence import EciIndex, ScopedDirectPersianEngine, project
from hybrid_fa_frontend import DirectFirstPersian
from reference_alignment import _reference_words, _tokens, source_map_for_reference
from text_frontend import SpeechPart, UnsupportedPronunciation


def check_mapping(frontend, text):
    result = frontend.analyze_positioned(text)
    assert result and result.source_map is not None, text
    assert result.parts == frontend.analyze(text).parts, text
    raw = text.encode('utf8')
    assert len(result.source_map) == len(raw) + 1
    assert result.source_map[-1] == len(result.parts[0].text)
    assert all(a <= b for a, b in zip(result.source_map, result.source_map[1:]))
    return result


def alignment_tests(frontend):
    samples = (
        'ناموجودمطمئن سلام',
        'سلام ناموجودمطمئن!',
        'ناموجودمطمئن،سلام؟',
        'سلام  ناموجودمطمئن',
        'ساعت چنده',
        'کاربرگ‌های دیجیتالی',
        'من می‌خوانم.',
        'نرم‌افزارها خوبند.',
        'کامپیوترها کار می‌کنند.',
        'من بدبخت آمد.',
        'سلام آبگوشت آمد.',
        'سلام آیلار آمد.',
        'آیلار',
        'اداء آمد',
        'سلام،آیلار',
        'اداء،سلام',
        'اداء آیلار',
        'اداء. آیلار',
        'اداء،،سلام',
        'بدبخت؟',
        'من می‌خوانم ۱۲.',
        'ناموجودمطمئن ۳٫۱۴ سلام.',
        'ناموجودمطمئن 12، ۳۴ خوب؟',
    )
    results = {text: check_mapping(frontend, text) for text in samples}
    one = results[samples[0]]
    second = len('ناموجودمطمئن '.encode('utf8'))
    assert one.source_map[second] == one.parts[0].text.index('`[', 1)
    assert all(one.source_map[i] == 0 for i in range(len('ناموجودمطمئن'.encode('utf8'))))
    assert frontend.analyze_positioned('سلام NVDA') is None
    assert frontend.analyze_positioned('ناموجودمطمئن ۱۲') is not None
    assert results['من بدبخت آمد.'].parts[0].text == "`[.0mAn] `[.1bAd.0bAGt] `[.1'a:'.0mAd]."
    assert not results['من بدبخت آمد.'].used_fallback
    for text in ('سلام،آیلار', 'اداء،سلام'):
        assert results[text].parts[0].text.count(',') == 1, text
    assert ',' not in results['اداء آیلار'].parts[0].text
    assert '`[.1waj.0lar]' in results['اداء آیلار'].parts[0].text
    assert '`[.1aj.0lar]' in results['سلام،آیلار'].parts[0].text
    assert results['آیلار'].parts[0].text == '`[.1aj.0lar]'
    assert "`[.1w'a:'.0mAd]" in results['اداء آمد'].parts[0].text
    assert 'w' not in frontend.analyze_direct('او آمد.').parts[0].text
    assert ',' not in results['اداء. آیلار'].parts[0].text
    assert frontend.analyze_positioned('اداء،،سلام').parts[0].text.count(',') == 2

    worker = frontend._fallback()
    original_events = worker.events
    def unexpected_events(*_args, **_kwargs):
        raise AssertionError('unneeded eSpeak event synthesis')
    worker.events = unexpected_events
    try:
        for text in ('من می‌خوانم.', 'من می‌خوانم ۱۲.'):
            job = project((EciIndex(20), text, EciIndex(21)), frontend, allow_fallback=True)
            assert job.needs_native_validation
            assert job.tagged == frontend.analyze(text).parts[0].text
    finally:
        worker.events = original_events
    numbered = project(('من می‌خوانم ', EciIndex(22), '۱', EciIndex(23), '۲.'),
                       frontend, allow_fallback=True)
    assert numbered.needs_native_validation
    assert [p.value for p in numbered.parts if isinstance(p, EciIndex)] == [22, 23]
    for parts in (('`vp80 من می‌خوانم.',), ('من NVDA می‌خوانم.',)):
        try:
            project(parts, frontend, allow_fallback=True)
            raise AssertionError('unsafe or unmapped source was accepted')
        except UnsupportedPronunciation:
            pass
    core = samples[0]
    tokens = _tokens(core)
    assert tokens
    reference = worker.translate(core, separator='|')
    events = worker.events(core)
    assert _reference_words(worker, core, tokens)
    assert worker.events('') == ()

    class FakeWorker:
        def __init__(self, altered):
            self.altered = altered
        def translate(self, text, *, voice='fa', separator):
            assert text == core and voice == 'fa' and separator == '|'
            return reference
        def events(self, text, *, voice='fa'):
            assert text == core and voice == 'fa'
            return self.altered

    positions = [i for i, event in enumerate(events) if event.kind == 1]
    assert len(positions) == 2
    wrong_start = list(events)
    wrong_start[positions[1]] = replace(wrong_start[positions[1]], position=2)
    wrong_phone = list(events)
    first_phone = next(i for i, event in enumerate(events) if event.kind == 7 and event.phone not in ('_', '_:'))
    wrong_phone[first_phone] = replace(wrong_phone[first_phone], phone='z')
    for altered in (events[:positions[1]] + events[positions[1] + 1:],
                    tuple(wrong_start), tuple(wrong_phone)):
        fake = FakeWorker(altered)
        assert _reference_words(fake, core, tokens) is None
        assert source_map_for_reference(core, one.parts, fake) is None
    assert source_map_for_reference(core, (SpeechPart('en-us', 'wrong'),), worker) is None
    # A wrong word anchor must also reject a number surrounded by fallback
    # text, even when the stand-in word has the expected phones.
    def wrong_number_events(text, *, voice='fa'):
        found = list(original_events(text, voice=voice))
        first = next(i for i, event in enumerate(found) if event.kind == 1)
        found[first] = replace(found[first], position=found[first].position + 1)
        return tuple(found)
    worker.events = wrong_number_events
    try:
        assert frontend.analyze_positioned('ناموجودمطمئن ۱۲') is None
    finally:
        worker.events = original_events
    return samples


def playback_tests(frontend, probe, reference, samples):
    from listening import render
    from test_selected_probe import engine, set_confirmed_persian_probe
    from _reloq_eci_direct import GENDER, PHRASE_PREDICTION, SAMPLE_RATE

    voice = ScopedDirectPersianEngine(probe, frontend, allow_fallback=True)
    fa = engine(reference, 0x90000)
    set_confirmed_persian_probe()
    try:
        # A consonant before the next consonant does not introduce a comma,
        # regardless of the user's phrase-prediction setting.
        explicit = "`[.0mAn] `[.1bAd.0bAGt] `[.1'a:'.0mAd]."
        for prediction in (0, 1):
            for target in (voice, fa):
                target.set_parameter(PHRASE_PREDICTION, prediction)
            pcm = bytearray()
            assert voice.speak(('من بدبخت آمد.',), pcm.extend, lambda _: None)
            assert bytes(pcm) == render(fa, explicit)
            assert bytes(pcm) != render(fa, explicit.replace('bAd.0b', 'bAd],`[.0b'))
        for target in (voice, fa):
            target.set_parameter(PHRASE_PREDICTION, 0)
        # Boundary-only indexes need no synthesized reference events, even
        # when a directly generated number occurs in the same fallback text.
        worker = frontend._fallback()
        previous_events = worker.events
        def forbidden_events(*_args, **_kwargs):
            raise AssertionError('unneeded eSpeak event synthesis')
        worker.events = forbidden_events
        try:
            for text in ('من می‌خوانم.', 'من می‌خوانم ۱۲.'):
                parts = (EciIndex(20), text, EciIndex(21))
                job = project(parts, frontend, allow_fallback=True)
                assert job.needs_native_validation
                pcm, marks = bytearray(), []
                assert voice.speak(parts, pcm.extend, lambda i: marks.append((i, len(pcm) // 2)))
                assert bytes(pcm) == render(fa, job.tagged)
                assert marks == [(20, 0), (21, len(pcm) // 2)]
        finally:
            worker.events = previous_events
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for target in (voice, fa):
                    target.set_parameter(SAMPLE_RATE, rate)
                    target.set_parameter(GENDER, gender)
                for text in samples:
                    split = text.find(' ')
                    if split < 0:
                        comma = text.find('،')
                        split = comma + 1 if comma >= 0 else 1
                    parts = (EciIndex(1), text[:split], EciIndex(2), text[split:], EciIndex(3))
                    job = project(parts, frontend, allow_fallback=True)
                    assert job.needs_native_validation == frontend.analyze(text).used_fallback
                    expected = render(fa, job.tagged)
                    pcm, marks = bytearray(), []
                    assert voice.speak(parts, pcm.extend, lambda i: marks.append((i, len(pcm) // 2)))
                    assert bytes(pcm) == expected, (text, rate, gender)
                    assert [index for index, _ in marks] == [1, 2, 3]
                    assert marks[0][1] == 0 and marks[-1][1] == len(pcm) // 2
                    assert marks[0][1] <= marks[1][1] <= marks[-1][1]
        try:
            voice.speak(('ناموجودمطمئن NVDA',), lambda _: None, lambda _: None)
            raise AssertionError('unmapped mixed-language fallback was accepted')
        except UnsupportedPronunciation:
            pass
        # A native parser mismatch must fail before any audio.
        original = voice._voice._phonemes_for_probe
        voice._voice._phonemes_for_probe = lambda _: '`[.1i]'
        pcm = bytearray()
        try:
            voice.speak((samples[0],), pcm.extend, lambda _: None)
            raise AssertionError('an unaccepted fallback phoneme tag was spoken')
        except UnsupportedPronunciation:
            assert not pcm
        finally:
            voice._voice._phonemes_for_probe = original
        voice._voice._phonemes_for_probe = lambda text: original(text).replace('waj', 'aj')
        pcm = bytearray()
        try:
            voice.speak(('سلام آیلار آمد.',), pcm.extend, lambda _: None)
            raise AssertionError('a lost glottal phone was accepted')
        except UnsupportedPronunciation:
            assert not pcm
        finally:
            voice._voice._phonemes_for_probe = original
        assert voice._scope(0) == 0
    finally:
        voice.close()
        fa.close()

    # The isolated eSpeak helper must unblock a cancelled fallback; the same
    # frontend and ECI handle must be able to speak the next request.
    worker = frontend._fallback()
    original_events = worker.events
    started = threading.Event()
    shared, result, first, second = {}, [], bytearray(), bytearray()
    def delayed_events(text, *, voice='fa'):
        started.set()
        return original_events(text, voice=voice)
    worker.events = delayed_events
    def run():
        voice = ScopedDirectPersianEngine(probe, frontend, allow_fallback=True)
        shared['voice'] = voice
        try:
            long_parts = ('ناموجودمطمئن ', EciIndex(99), 'سلام ناموجودمطمئن ' * 400)
            result.append(voice.speak(long_parts, first.extend, lambda _: None))
            worker.events = original_events
            result.append(voice.speak((samples[0],), second.extend, lambda _: None))
        finally:
            voice.close()
    thread = threading.Thread(target=run)
    thread.start()
    try:
        assert started.wait(15), 'reference event analysis did not start'
        shared['voice'].cancel()
        thread.join(15)
        assert not thread.is_alive() and result == [False, True] and not first and second, result
    finally:
        worker.events = original_events


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--scope-probe', type=Path)
    parser.add_argument('--reference', type=Path)
    args = parser.parse_args()
    if bool(args.scope_probe) != bool(args.reference):
        parser.error('provide both acoustic libraries or neither')
    with DirectFirstPersian(args.native_library, args.worker,
                            args.espeak_library, args.data_parent) as frontend:
        samples = alignment_tests(frontend)
        if args.scope_probe:
            playback_tests(frontend, args.scope_probe, args.reference, samples)
    print('Passed reference word anchors, checked phonemes and fail-closed cases' +
          (', native PCM and cancellation.' if args.scope_probe else '.'))


if __name__ == '__main__':
    main()
