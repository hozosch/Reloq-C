"""Exercise Persian against the actual NVDA language and playback queues."""
from __future__ import annotations

import argparse
import ctypes
from pathlib import Path
import tempfile
import threading
import time

from direct_fa_sequence import EciIndex, EciText
from nvda_driver_probe import (PERSIAN_PROBE, PINGLISH_PROBE, PROFILES,
                               PersianProbeLanguageEngine, _bracket_continuations, _persian_part_runs,
                               _script_runs, make_language_engine)
from _reloq_eci_direct import EciLanguage, PROCESS_TAGS
from _reloq_eci_transport import EciTextTransport
from _reloq_languages import FINNISH, US_ENGLISH, language_for
from test_eci_transport import Player


def fake_routing_test():
    assert _script_runs('سلام 4 of 14') == (('fa', 'سلام '), ('en', '4 of 14'))
    assert _script_runs('4 of 14 خوب') == (('en', '4 of 14 '), ('fa', 'خوب'))
    assert _script_runs('سلام ˌchi-kən-ˈgu̇n-yə خوب') == (
        ('fa', 'سلام '), ('en', 'ˌchi-kən-ˈgu̇n-yə '), ('fa', 'خوب'))
    split = ('سلام 4', EciIndex(101), ' of ', EciIndex(102), '14 خوب')
    mapped = _persian_part_runs(split, (PERSIAN_PROBE.id, '1'))
    assert mapped == {0: (('fa', 'سلام '), ('en', '4')), 2: (('en', ' of '),),
                      4: (('en', '14 '), ('fa', 'خوب'))}
    source = (EciText('سلام (NVDA)'), EciIndex(5), EciText(': خوب'))
    assert _bracket_continuations(source) == (
        EciText('سلام (NVDA'), EciIndex(5), EciText(': خوب'))
    assert _bracket_continuations(('سلام (دنیا)',)) == ('سلام (دنیا),',)
    assert _bracket_continuations(('سلام (دنیا) : خوب',)) == ('سلام (دنیا: خوب',)
    for source in ((EciText('word '), EciText(')', character_mode=True)),
                   ('سلام (دنیا؟)',), ('سلام (دنیا).',), (')',),
                   ('`[.1abc]',), ('سلام (دنیا) خوب',)):
        assert _bracket_continuations(source) == source, source
    class FakeEngine:
        def __init__(self, _dll, *, dialect, encoding, native_voice):
            self.dialect = dialect
            self.closed = False
            assert encoding == 'utf-8' and native_voice
        def set_parameter(self, _parameter, _value): pass
        def set_dictionary_path(self, _path): pass
        def speak(self, parts, on_pcm, on_index):
            on_pcm(bytes([self.dialect >> 16 & 0xff]))
            for part in parts:
                if isinstance(part, EciIndex):
                    on_index(part.value)
            return True
        def cancel(self): pass
        def close(self): self.closed = True

    assert language_for('fa-IR', US_ENGLISH.id, PROFILES) == PERSIAN_PROBE.id
    assert language_for('fa-Latn', US_ENGLISH.id, PROFILES) == PINGLISH_PROBE.id
    assert language_for('fa', PINGLISH_PROBE.id, PROFILES) == PINGLISH_PROBE.id
    # Selecting English also constructs Persian before any Persian utterance,
    # but keeps English active and does not emit speech during selection.
    english_home = PersianProbeLanguageEngine(Path('/unused'), language=US_ENGLISH.id,
                                               languages=PROFILES, engine_factory=FakeEngine)
    try:
        assert PERSIAN_PROBE.id in english_home._engines
        assert english_home._active is english_home._engines[US_ENGLISH.id]
        assert english_home._home[0] == US_ENGLISH.id
    finally:
        english_home.close()
    class FailingPersian(FakeEngine):
        def __init__(self, dll, *, dialect, encoding, native_voice):
            if dialect == PERSIAN_PROBE.id:
                raise OSError('simulated Persian preload failure')
            super().__init__(dll, dialect=dialect, encoding=encoding, native_voice=native_voice)
    english_home = PersianProbeLanguageEngine(Path('/unused'), language=US_ENGLISH.id,
                                               languages=PROFILES, engine_factory=FailingPersian)
    try:
        assert english_home._active is english_home._engines[US_ENGLISH.id]
        pcm = bytearray()
        assert english_home.speak(('Hello',), pcm.extend, lambda _: None)
        assert pcm == bytes([(US_ENGLISH.id >> 16) & 0xff])
    finally:
        english_home.close()
    engine = PersianProbeLanguageEngine(Path('/unused'), language=PERSIAN_PROBE.id,
                                        languages=PROFILES, engine_factory=FakeEngine)
    persian_byte = bytes([(PERSIAN_PROBE.id >> 16) & 0xff])
    pinglish_byte = bytes([(PINGLISH_PROBE.id >> 16) & 0xff])
    english_byte = bytes([(US_ENGLISH.id >> 16) & 0xff])
    try:
        pcm, indexes = bytearray(), []
        assert engine.speak((EciIndex(4), 'سلام', EciIndex(5)), pcm.extend, indexes.append)
        assert bytes(pcm) == persian_byte and indexes == [4, 5]
        pcm.clear()
        engine.select_voice(PINGLISH_PROBE.id, '1')
        assert engine.speak((EciIndex(14), 'salam', EciIndex(15)), pcm.extend, indexes.append)
        assert bytes(pcm) == pinglish_byte and indexes[-2:] == [14, 15]
        pcm.clear()
        assert engine.speak((EciLanguage(US_ENGLISH.id, '1'), EciIndex(16), 'NVDA ',
                             EciLanguage(PERSIAN_PROBE.id, '1'), EciIndex(17), 'خوب', EciIndex(18)),
                            pcm.extend, indexes.append)
        assert bytes(pcm) == english_byte + persian_byte and indexes[-3:] == [16, 17, 18]
        pcm.clear()
        engine.select_voice(PERSIAN_PROBE.id, '1')
        assert engine.speak((EciIndex(19), 'NVDA ', EciIndex(20), 'خوب', EciIndex(21)),
                            pcm.extend, indexes.append)
        assert bytes(pcm) == english_byte + persian_byte and indexes[-3:] == [19, 20, 21]
        pcm.clear()
        assert engine.speak((EciLanguage(US_ENGLISH.id, '1'), 'Hello'),
                            pcm.extend, lambda _: None)
        assert bytes(pcm) == english_byte
    finally:
        engine.close()


def real_transport_tests(args):
    player, observed, errors = Player(), [], []
    event_started = threading.Event()
    reference_release = threading.Event()

    def create():
        engine = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                      args.espeak_library, args.data_parent)
        persian = engine._engines[PERSIAN_PROBE.id]
        reference = persian._frontend._fallback()
        original = reference.translate
        def marked_translate(text, *, voice='fa', separator=''):
            if 'ناموجودمطمئن' in text:
                event_started.set()
                if not reference_release.wait(15):
                    raise TimeoutError('reference cancellation test was not released')
            return original(text, voice=voice, separator=separator)
        reference.translate = marked_translate
        return engine

    transport = EciTextTransport(create, player, lambda value: observed.append(('index', value)),
                                 lambda value: observed.append(('done', value)), errors.append,
                                 complete_on_error=True)

    def drain_until(predicate, seconds=15):
        deadline = time.monotonic() + seconds
        while not predicate():
            if time.monotonic() > deadline:
                raise AssertionError(f'transport did not complete: {observed!r}, {errors!r}')
            callback = None
            with player.cv:
                if not player.callbacks:
                    player.cv.wait(.03)
                if player.callbacks:
                    callback = player.callbacks.pop(0)
                    player.cv.notify_all()
            if callback is not None:
                callback()

    def run(parts):
        start = len(player.pcm)
        number = transport.submit(parts)
        drain_until(lambda: ('done', number) in observed)
        assert not errors, errors
        return number, b''.join(player.pcm[start:])

    directory = tempfile.TemporaryDirectory(prefix='reloq-fa-driver-dictionaries-')
    try:
        transport.set_dictionary_path(directory.name)
        transport.set_parameter(PROCESS_TAGS, 0)
        first, speech = run((EciIndex(1), 'من بدبخت ', EciIndex(2), 'آمد.', EciIndex(3)))
        assert speech
        assert observed[-4:] == [('index', 1), ('index', 2), ('index', 3), ('done', first)]
        second, letter = run((EciIndex(4), EciText('ق', character_mode=True), EciIndex(5)))
        assert letter
        assert observed[-3:] == [('index', 4), ('index', 5), ('done', second)]

        transport.select_voice(FINNISH.id, '1')
        _, finnish = run(('viisi.', EciIndex(6)))
        transport.select_voice(PERSIAN_PROBE.id, '1')
        _, again = run(('من بدبخت آمد.', EciIndex(7)))
        assert finnish and again and again == speech

        # The two test profiles produce the same selected acoustics for a
        # directly covered pair, while retaining original Pinglish indexes.
        _, script = run(('سلام دنیا؟', EciIndex(31)))
        transport.select_voice(PINGLISH_PROBE.id, '1')
        third, roman = run((EciIndex(32), 'salam ', EciIndex(33), 'donya?', EciIndex(34)))
        assert roman == script and roman
        assert observed[-4:] == [('index', 32), ('index', 33), ('index', 34), ('done', third)]
        _, latin = run((EciText('w', character_mode=True), EciIndex(35)))
        _, acronym = run((EciText('NV', character_mode=True), EciIndex(38),
                          EciText('DA', character_mode=True)))
        _, unknown = run(('salam ', EciIndex(36), 'khubtar?', EciIndex(37)))
        assert latin and acronym and unknown
        transport.select_voice(PERSIAN_PROBE.id, '1')

        # Explicit and automatic switches use the same continuous sentence.
        _, english = run((EciLanguage(US_ENGLISH.id, '1'), 'NVDA ', EciIndex(40)))
        _, persian = run(('خوب', EciIndex(41)))
        mixed_job, mixed = run((EciLanguage(US_ENGLISH.id, '1'), EciIndex(42), 'NVDA ',
                                EciLanguage(PERSIAN_PROBE.id, '1'), EciIndex(43), 'خوب', EciIndex(44)))
        dll = ctypes.CDLL(str(args.scope_probe.resolve()))
        continuous = hasattr(dll, 'eciPersianReplayFrames') or hasattr(dll, 'reloq_fa_replay_frames')
        assert english and persian and mixed
        assert (mixed != english + persian) if continuous else (mixed == english + persian)
        assert observed[-4:] == [('index', 42), ('index', 43), ('index', 44), ('done', mixed_job)]
        plain_job, plain_mixed = run((EciIndex(45), 'NVDA ', EciIndex(46), 'خوب', EciIndex(47)))
        assert plain_mixed == mixed
        assert observed[-4:] == [('index', 45), ('index', 46), ('index', 47), ('done', plain_job)]
        _, explicit_roundtrip = run(('سلام ', EciLanguage(US_ENGLISH.id, '1'), 'NVDA ',
                                     EciLanguage(PERSIAN_PROBE.id, '1'), 'خوب'))
        roundtrip_job, plain_roundtrip = run((EciIndex(48), 'سلام ', EciIndex(49), 'NVDA ',
                                              EciIndex(50), 'خوب', EciIndex(51)))
        assert plain_roundtrip == explicit_roundtrip
        assert observed[-5:] == [('index', 48), ('index', 49), ('index', 50),
                                 ('index', 51), ('done', roundtrip_job)]
        _, recovered = run(('سلام.', EciIndex(8)))
        assert recovered

        # Hold an actual unknown-word reference request, cancel its transport
        # job, then release it. The reference is now requested per word, not
        # necessarily as one large events call; stale callbacks must vanish.
        abandoned = transport.submit(('ناموجودمطمئن ', EciIndex(9),
                                      'سلام ناموجودمطمئن ' * 400))
        try:
            assert event_started.wait(15), 'reference translation did not start'
            transport.cancel()
        finally:
            reference_release.set()
        current, recovered = run(('سلام.', EciIndex(10)))
        assert recovered and ('done', abandoned) not in observed
        assert observed[-2:] == [('index', 10), ('done', current)]
    finally:
        assert transport.shutdown()
        directory.cleanup()
    assert player.closed


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path)
    parser.add_argument('--worker', type=Path)
    parser.add_argument('--espeak-library', type=Path)
    parser.add_argument('--data-parent', type=Path)
    parser.add_argument('--scope-probe', type=Path)
    args = parser.parse_args()
    paths = (args.native_library, args.espeak_library, args.data_parent, args.scope_probe)
    if any(paths) and not all(paths):
        parser.error('provide all acoustic and reference paths, or none')
    fake_routing_test()
    if all(paths):
        real_transport_tests(args)
    print('Passed Persian NVDA language routing' +
          (', native playback queue and cancellation.' if all(paths) else '.'))


if __name__ == '__main__':
    main()
