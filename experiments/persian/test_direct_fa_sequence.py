"""Check direct Persian ECI index projection and scoped native playback."""
from __future__ import annotations

import argparse
from pathlib import Path
import random
import re
import threading

from direct_fa_sequence import (
    EciIndex, EciPitch, EciPitchHz, EciProsody, EciText, ScopedDirectPersianEngine, project,
)
from hybrid_fa_frontend import DirectFirstPersian
from text_frontend import UnsupportedPronunciation
from _reloq_eci_direct import GENDER, PITCH_HZ, PROCESS_TAGS, SAMPLE_RATE
from _reloq_persian_projection import PersianProjectionError, project_persian


def only_text(parts):
    return ''.join(part.text for part in parts if isinstance(part, EciText))


def assert_rejected(parts, frontend):
    try:
        project(parts, frontend)
    except (UnsupportedPronunciation, ValueError, TypeError):
        return
    raise AssertionError(f'unsafe job was accepted: {parts!r}')


def projection_tests(frontend):
    # NVDA can submit an index without text, or after whitespace alone.
    for parts in ((), (EciIndex(90),), (' ', EciIndex(91))):
        empty_job = project(parts, frontend)
        assert [p for p in empty_job.parts if isinstance(p, EciIndex)] == [
            p for p in parts if isinstance(p, EciIndex)]
    inputs = (
        (EciIndex(1), 'سلام ', EciIndex(2), 'دنیا', EciIndex(3), '؟', EciIndex(4)),
        ('سل', EciIndex(5), 'ام ', EciIndex(6), '۳', EciIndex(7), '٫۱۴ دنیا؟'),
        ('من', EciIndex(8), ' ۱۲ آمد.'),
        ('ﺳﻼﻡ ', EciIndex(9), 'دنیا؟'),
        (EciIndex(10), 'س', EciIndex(11), 'ل', EciIndex(12), 'ام.'),
        (EciIndex(13), 'کتابم ', EciIndex(14), 'کتابت ', EciIndex(15), 'کتابها.'),
        (EciIndex(16), 'سلام (', EciIndex(17), 'دنیا) خوب', EciIndex(18)),
    )
    if frontend._project_direct is not None:
        # The native path and established source-map path must preserve all
        # text, punctuation, in-word indexes and controls byte for byte.
        native = frontend._project_direct
        comparisons = (*inputs, (), (EciIndex(90),), (' ', EciIndex(91)),
                       ('سلام', EciPitch(65), ' دنیا', EciProsody(6, 50), '؟'),
                       ('سلام ', EciText('ب', character_mode=True)))
        for parts in comparisons:
            frontend._project_direct = None
            try:
                expected = project(parts, frontend)
            finally:
                frontend._project_direct = native
            assert project(parts, frontend) == expected, parts
        # A rejected control must not be partially mapped or emitted.
        assert frontend.project_direct('سلام', ((2, False),)) is None
        assert frontend.project_direct('ناموجودمطمئن', ()) is None
        lexicon = Path(__file__).with_name('direct_fa_lexicon.inc').read_text(encoding='utf8')
        words = re.findall(r'^\s*\{"([^"\\]+)",\s*"', lexicon, re.M)
        assert len(words) > 10000
        rng = random.Random(112)
        for i in range(512):
            text = rng.choice((' ', '، ', '-')).join(rng.sample(words, rng.randint(1, 4)))
            text = rng.choice(('', '(', '[')) + text + rng.choice(('', '.', '؟', ')', ':'))
            at = rng.randrange(len(text) + 1)
            parts = (EciIndex(i), text[:at], EciIndex(i + 1000), text[at:], EciIndex(i + 2000))
            frontend._project_direct = None
            try:
                expected = project(parts, frontend)
            finally:
                frontend._project_direct = native
            assert project(parts, frontend) == expected, (i, text, at)
    for parts in inputs:
        job = project(parts, frontend)
        assert only_text(job.parts) == job.tagged
        assert [p for p in job.parts if isinstance(p, EciIndex)] == [p for p in parts if isinstance(p, EciIndex)]
        direct = frontend.analyze_direct(job.source).parts[0].text
        if '(' in job.source or ')' in job.source:
            assert job.tagged.replace(' ', '') == direct.replace(' ', '')
        else:
            assert job.tagged == direct
    inside = project(('سل', EciIndex(5), 'ام.'), frontend)
    assert isinstance(inside.parts[0], EciIndex) and inside.parts[0].value == 5
    split_number = project(('۳', EciIndex(7), '٫۱۴'), frontend)
    assert isinstance(split_number.parts[0], EciIndex) and split_number.parts[0].value == 7
    long_job = tuple(item for i in range(512) for item in (EciIndex(i), 'سلام '))
    long_projection = project(long_job, frontend)
    assert only_text(long_projection.parts) == long_projection.tagged
    assert [p.value for p in long_projection.parts if isinstance(p, EciIndex)] == list(range(512))
    boundary = project(('سلام', EciPitch(65), ' دنیا', EciProsody(6, 50), '؟'), frontend)
    assert [type(p) for p in boundary.parts if not isinstance(p, EciText)] == [EciPitch, EciProsody]
    spelled = project(('سلام ', EciText('ب', character_mode=True)), frontend)
    assert spelled.tagged == (frontend.analyze_direct('سلام ').parts[0].text +
                              frontend.analyze_character('ب').parts[0].text)
    for char in '()[]{}':
        spelled = project((EciText(char, character_mode=True),), frontend)
        assert spelled.tagged == frontend.analyze_character(char).parts[0].text
    for bad in (('سلام NVDA',), ('ناموجودمطمئن',), ('سل', EciPitch(65), 'ام.'),
                ('؟',), ('3,14',),
                ('سلام\0',), ('`vp80 سلام',), (EciIndex(-1), 'سلام')):
        assert_rejected(bad, frontend)
    tag = '`[.1a]'
    for mapping in ((0, 2, len(tag)), (0, -1, len(tag)), (0, 0.5, len(tag))):
        try:
            project_persian(('a', EciIndex(1), 'b'), lambda _: (tag, mapping))
            raise AssertionError('invalid source map was accepted')
        except PersianProjectionError:
            pass
    assert frontend._worker is None, 'direct projection unexpectedly started eSpeak'
    return inputs


def playback_tests(frontend, probe, reference, inputs):
    from listening import render
    from test_selected_probe import engine, set_confirmed_persian_probe
    voice = ScopedDirectPersianEngine(probe, frontend)
    fa = engine(reference, 0x90000)
    set_confirmed_persian_probe(d=2, z=2)
    try:
        # NVDA can disable user tags without disabling our generated tags.
        voice.set_parameter(PROCESS_TAGS, 0)
        assert voice._voice._dll.eciGetParam(voice._voice._handle, PROCESS_TAGS) == 1
        for parts, expected in (((), []), ((EciIndex(90),), [90]),
                                ((' ', EciIndex(91)), [91])):
            pcm, indexes = bytearray(), []
            assert voice.speak(parts, pcm.extend, indexes.append)
            assert not pcm and indexes == expected
        for rate in (1, 3, 6):
            for gender in (0, 1):
                for target in (voice, fa):
                    target.set_parameter(SAMPLE_RATE, rate)
                    target.set_parameter(GENDER, gender)
                for parts in inputs:
                    job = project(parts, frontend)
                    expected = render(fa, job.tagged)
                    pcm, indexes = bytearray(), []
                    assert voice.speak(parts, pcm.extend, lambda i: indexes.append((i, len(pcm) // 2)))
                    assert bytes(pcm) == expected, (rate, gender, parts)
                    assert [i for i, _ in indexes] == [p.value for p in parts if isinstance(p, EciIndex)]
                    assert [at for _, at in indexes] == sorted(at for _, at in indexes)
                    if parts == inputs[0]:
                        assert indexes[0][1] == 0 < indexes[1][1] < len(pcm) // 2
                        assert indexes[2][1] == indexes[3][1] == len(pcm) // 2
                    if parts == inputs[1]:
                        # The mark inside one decimal maps to the start of
                        # that number, without splitting a phoneme tag.
                        assert indexes[0][1] == 0 < indexes[1][1] == indexes[2][1]
        # Boundary-local pitch, speed and volume commands must pass through
        # the same ECI queue as the unmarked native reference.
        controls = ('سلام', EciPitch(75), ' دنیا', EciProsody(6, 70),
                    ' ۱۲', EciProsody(7, 80), ' آمد.')
        job = project(controls, frontend)
        expected, actual = bytearray(), bytearray()
        assert fa.speak(job.parts, expected.extend, lambda _: None)
        assert voice.speak(controls, actual.extend, lambda _: None)
        assert bytes(actual) == bytes(expected) and actual
        # Temporary controls must not leak into the next utterance.
        plain = project(('سلام دنیا.',), frontend)
        expected, actual = bytearray(), bytearray()
        assert fa.speak(plain.parts, expected.extend, lambda _: None)
        assert voice.speak(('سلام دنیا.',), actual.extend, lambda _: None)
        assert bytes(actual) == bytes(expected)
        for target in (voice, fa):
            target.set_parameter(PITCH_HZ, 130)
        hz_controls = ('سلام', EciPitchHz(180), ' دنیا.', EciPitchHz(130))
        expected, actual = bytearray(), bytearray()
        assert fa.speak(project(hz_controls, frontend).parts, expected.extend, lambda _: None)
        assert voice.speak(hz_controls, actual.extend, lambda _: None)
        assert bytes(actual) == bytes(expected) and actual
        expected, actual = bytearray(), bytearray()
        assert fa.speak(plain.parts, expected.extend, lambda _: None)
        assert voice.speak(('سلام دنیا.',), actual.extend, lambda _: None)
        assert bytes(actual) == bytes(expected)
        # A failed text job must not emit audio or retain the acoustic scope.
        pcm = bytearray()
        try:
            voice.speak(('سلام NVDA',), pcm.extend, lambda _: None)
            raise AssertionError('unsupported mixed job was accepted')
        except UnsupportedPronunciation:
            assert not pcm
        assert voice._scope(0) == 0
    finally:
        voice.close()
        fa.close()

    # Cancel from another thread while a native audio callback is blocked.
    started, release = threading.Event(), threading.Event()
    shared, outcome = {}, []
    def run():
        voice = ScopedDirectPersianEngine(probe, frontend)
        shared['voice'] = voice
        def receive(_pcm):
            started.set()
            if not release.wait(5):
                raise RuntimeError('cancel test timed out')
        try:
            outcome.append(voice.speak(('سلام دنیا ' * 40,), receive, lambda _: None))
            pcm = bytearray()
            outcome.append(voice.speak(('سلام.',), pcm.extend, lambda _: None) and bool(pcm))
        finally:
            voice.close()
    thread = threading.Thread(target=run)
    thread.start()
    assert started.wait(10)
    canceller = threading.Thread(target=shared['voice'].cancel)
    canceller.start()
    canceller.join(1)
    prompt = not canceller.is_alive()
    release.set()
    thread.join(10)
    canceller.join(10)
    assert prompt and not thread.is_alive() and outcome == [False, True], outcome

    # Cancellation while text is being projected must not be cleared by the
    # following native speak call; a subsequent job must still work.
    started, release = threading.Event(), threading.Event()
    shared, outcome, emitted = {}, [], bytearray()
    class SlowFrontend:
        def analyze_direct(self, text):
            started.set()
            if not release.wait(5):
                raise RuntimeError('projection cancellation test timed out')
            return frontend.analyze_direct(text)
    def project_while_cancelled():
        voice = ScopedDirectPersianEngine(probe, SlowFrontend())
        shared['voice'] = voice
        try:
            outcome.append(voice.speak(('سلام.',), emitted.extend, lambda _: None))
            outcome.append(voice.speak(('سلام.',), emitted.extend, lambda _: None))
        finally:
            voice.close()
    thread = threading.Thread(target=project_while_cancelled)
    thread.start()
    assert started.wait(10)
    shared['voice'].cancel()
    release.set()
    thread.join(10)
    assert not thread.is_alive() and outcome == [False, True] and emitted, outcome

    # Exercise the narrow race between projection and the native adapter's
    # own cancellation reset. No first audio chunk may escape after cancel.
    started, release = threading.Event(), threading.Event()
    shared, outcome, first_pcm, second_pcm = {}, [], bytearray(), bytearray()
    def start_after_cancel():
        voice = ScopedDirectPersianEngine(probe, frontend)
        shared['voice'] = voice
        original = voice._voice.speak
        def delayed(*args):
            started.set()
            if not release.wait(5):
                raise RuntimeError('native-start cancellation test timed out')
            return original(*args)
        voice._voice.speak = delayed
        try:
            outcome.append(voice.speak(('سلام.',), first_pcm.extend, lambda _: None))
            voice._voice.speak = original
            outcome.append(voice.speak(('سلام.',), second_pcm.extend, lambda _: None))
        finally:
            voice.close()
    thread = threading.Thread(target=start_after_cancel)
    thread.start()
    assert started.wait(10)
    shared['voice'].cancel()
    release.set()
    thread.join(10)
    assert not thread.is_alive() and outcome == [False, True] and not first_pcm and second_pcm, outcome


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--scope-probe', type=Path)
    parser.add_argument('--reference', type=Path)
    args = parser.parse_args()
    if bool(args.scope_probe) != bool(args.reference):
        parser.error('provide both acoustic libraries or neither')
    unused = Path('/nonexistent/reloq-reference')
    with DirectFirstPersian(args.native_library, unused, unused, unused) as frontend:
        cases = projection_tests(frontend)
        if args.scope_probe:
            playback_tests(frontend, args.scope_probe, args.reference, cases)
    print('Passed direct Persian position projection' +
          (', native sound, cancellation and scope checks.' if args.scope_probe else '.'))


if __name__ == '__main__':
    main()
