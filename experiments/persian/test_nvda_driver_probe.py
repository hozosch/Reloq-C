"""Exercise Persian against the actual NVDA language and playback queues."""
from __future__ import annotations

import argparse
from pathlib import Path
import tempfile
import threading
import time

from direct_fa_sequence import EciIndex, EciText
from nvda_driver_probe import (PERSIAN_PROBE, PINGLISH_PROBE, PROFILES,
                               PersianProbeLanguageEngine, make_language_engine)
from text_frontend import UnsupportedPronunciation
from _reloq_eci_direct import EciLanguage, PROCESS_TAGS
from _reloq_eci_transport import EciTextTransport
from _reloq_languages import FINNISH, US_ENGLISH, language_for
from test_eci_transport import Player


def fake_routing_test():
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
    engine = PersianProbeLanguageEngine(Path('/unused'), language=PERSIAN_PROBE.id,
                                        languages=PROFILES, engine_factory=FakeEngine)
    try:
        pcm, indexes = bytearray(), []
        assert engine.speak((EciIndex(4), 'سلام', EciIndex(5)), pcm.extend, indexes.append)
        assert bytes(pcm) == b'\x7f' and indexes == [4, 5]
        pcm.clear()
        engine.select_voice(PINGLISH_PROBE.id, '1')
        assert engine.speak((EciIndex(14), 'salam', EciIndex(15)), pcm.extend, indexes.append)
        assert bytes(pcm) == b'\x7f' and indexes[-2:] == [14, 15]
        pcm.clear()
        try:
            engine.speak((EciLanguage(US_ENGLISH.id, '1'), 'NVDA ',
                          EciLanguage(PERSIAN_PROBE.id, '1'), 'خوب'),
                         pcm.extend, indexes.append)
            raise AssertionError('mixed language was played without continuous context')
        except UnsupportedPronunciation:
            assert not pcm
        assert engine.speak((EciLanguage(US_ENGLISH.id, '1'), 'Hello'),
                            pcm.extend, lambda _: None)
        assert bytes(pcm) == b'\x01'
    finally:
        engine.close()


def real_transport_tests(args):
    player, observed, errors = Player(), [], []
    event_started = threading.Event()

    def create():
        engine = make_language_engine(args.scope_probe, args.native_library, args.worker,
                                      args.espeak_library, args.data_parent)
        persian = engine._engines[PERSIAN_PROBE.id]
        reference = persian._frontend._fallback()
        original = reference.events
        def marked_events(text, *, voice='fa'):
            if len(text) > 1000:
                event_started.set()
            return original(text, voice=voice)
        reference.events = marked_events
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

        # Do not play the English prefix of a sentence whose cross-language
        # contour has not yet been implemented.
        previous_feeds = len(player.pcm)
        failed = transport.submit((EciLanguage(US_ENGLISH.id, '1'), 'NVDA ',
                                   EciLanguage(PERSIAN_PROBE.id, '1'), 'خوب'))
        drain_until(lambda: ('done', failed) in observed and bool(errors))
        assert len(player.pcm) == previous_feeds
        assert isinstance(errors.pop(), UnsupportedPronunciation)
        _, recovered = run(('سلام.', EciIndex(8)))
        assert recovered

        # Interrupt real helper work, discard its callbacks, then speak again.
        abandoned = transport.submit(('ناموجودمطمئن ', EciIndex(9),
                                      'سلام ناموجودمطمئن ' * 400))
        assert event_started.wait(15), 'reference events did not start'
        transport.cancel()
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
    paths = (args.native_library, args.worker, args.espeak_library, args.data_parent, args.scope_probe)
    if any(paths) and not all(paths):
        parser.error('provide all acoustic and reference paths, or none')
    fake_routing_test()
    if all(paths):
        real_transport_tests(args)
    print('Passed Persian NVDA language-routing guard' +
          (', native playback queue and cancellation.' if all(paths) else '.'))


if __name__ == '__main__':
    main()
