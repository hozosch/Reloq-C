"""The first utterance must wait for the complete selected dictionary snapshot."""
from pathlib import Path
import argparse
import json
import tempfile
import threading
import time

from _reloq_dictionary_manager import DictionaryManager, DictionarySource
from _reloq_eci_direct import DirectEciEngine, EciIndex
from _reloq_eci_transport import DictionaryStartupError, EciTextTransport
from test_eci_german_clock_native import render


class Player:
    def __init__(self): self.pcm = bytearray()
    def feed(self, pcm, onDone=None):
        self.pcm.extend(pcm)
        if onDone: onDone()
    def idle(self): pass
    def stop(self): pass
    def pause(self, switch): pass
    def close(self): pass


class Engine:
    def __init__(self):
        self.active = None
        self.started = threading.Event()
        self.calls = []
    def set_dictionary_path(self, path): self.active = Path(path) if path else None
    def set_parameter(self, parameter, value): pass
    def select_voice(self, language, voice): pass
    def speak(self, parts, on_pcm, on_index):
        text = ''.join(p for p in parts if isinstance(p, str))
        entries = (self.active/'ENUmain.dic').read_bytes() if self.active else b''
        self.calls.append((text, entries))
        self.started.set()
        on_pcm(b'pcm')
        return True
    def cancel(self): pass
    def close(self): pass


def real_engine(library, root):
    own = root/'real-own.dic'
    own.write_bytes(b'Unigram\t`[.1tEst]\n')
    expected_dir = root/'expected'
    expected_dir.mkdir()
    (expected_dir/'ENUmain.dic').write_bytes(own.read_bytes())
    direct = DirectEciEngine(library, dialect=0x10000, encoding='utf8', native_voice=True)
    try:
        without = render(direct, ['Unigram', EciIndex(7)])[0]
        direct.set_dictionary_path(expected_dir)
        expected = render(direct, ['Unigram', EciIndex(7)])[0]
        assert expected != without, 'the test dictionary must alter the native pronunciation'
    finally:
        direct.close()
    setting = root/'real-settings.json'
    setting.write_text(json.dumps({'version': 1, 'sources': [
        DictionarySource(str(own), 0x10000, 0).record()]}), encoding='utf8')
    player, done, errors, indexes = Player(), threading.Event(), [], []
    transport = EciTextTransport(lambda: DirectEciEngine(library, dialect=0x10000,
        encoding='utf8', native_voice=True), player, indexes.append, lambda number: done.set(), errors.append)
    holder = {}
    def deliver(revision, path):
        if holder['manager'].current(revision): transport.set_dictionary_path(path)
    def failure(revision):
        if holder['manager'].current(revision): transport.fail_dictionary_load()
    manager = holder['manager'] = DictionaryManager(root/'bundled', root/'real-cache',
        deliver, errors.append, settings_path=setting, on_load_error=failure)
    entered, resume = threading.Event(), threading.Event()
    build = manager._build
    def delayed_build(*args):
        entered.set()
        assert resume.wait(3)
        return build(*args)
    manager._build = delayed_build
    try:
        transport.begin_dictionary_load();manager.set_enabled(False)
        assert entered.wait(2)
        transport.submit(['Unigram', EciIndex(7)])
        assert not done.wait(.05) and not player.pcm
        resume.set()
        assert done.wait(3)
        assert bytes(player.pcm) == expected, 'the first native utterance used an incomplete dictionary'
        assert indexes == [7] and not errors, (indexes, errors)
    finally:
        resume.set();manager.close();assert transport.shutdown()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        if args.library:
            real_engine(args.library.resolve(), root)
        source = root/'own.dic'
        source.write_bytes(b'Unigram\tcustom pronunciation\n')
        setting = root/'settings.json'
        record = DictionarySource(str(source), 0x10000, 0).record()
        setting.write_text(json.dumps({'version': 1, 'sources': [record]}), encoding='utf8')
        engine, done, errors = Engine(), threading.Event(), []
        transport = EciTextTransport(lambda: engine, Player(), lambda index: None,
                                     lambda number: done.set(), errors.append)
        holder = {}
        def deliver(revision, path):
            if holder['manager'].current(revision): transport.set_dictionary_path(path)
        def failure(revision):
            if holder['manager'].current(revision): transport.fail_dictionary_load()
        manager = holder['manager'] = DictionaryManager(root/'bundled', root/'cache',
            deliver, errors.append, settings_path=setting, on_load_error=failure)
        entered, resume = threading.Event(), threading.Event()
        build = manager._build
        def delayed_build(*args):
            entered.set()
            assert resume.wait(3), 'dictionary worker did not resume'
            return build(*args)
        manager._build = delayed_build
        try:
            transport.begin_dictionary_load()
            manager.set_enabled(False)
            assert entered.wait(2)
            transport.submit(['Unigram', EciIndex(1)])
            assert not engine.started.wait(.05), 'first utterance outran its dictionary'
            transport.cancel()
            transport.submit(['Amir', EciIndex(2)])
            assert not engine.started.wait(.05), 'cancellation removed the load barrier'
            resume.set()
            assert done.wait(3), 'queued speech did not resume'
            assert engine.calls == [('Amir', b'Unigram\tcustom pronunciation\n')], engine.calls
            assert not errors, errors
        finally:
            resume.set();manager.close();assert transport.shutdown()

        # A new selection supersedes an unresolved result without releasing
        # the already queued speech ahead of the newer snapshot.
        engine, done, errors = Engine(), threading.Event(), []
        transport = EciTextTransport(lambda: engine, Player(), lambda index: None,
                                     lambda number: done.set(), errors.append)
        try:
            first, second = root/'first', root/'second'
            first.mkdir();second.mkdir()
            (first/'ENUmain.dic').write_bytes(b'old\told\n')
            (second/'ENUmain.dic').write_bytes(b'new\tnew\n')
            transport.begin_dictionary_load()
            transport.set_dictionary_path(first)
            transport.begin_dictionary_load()
            transport.submit(['new', EciIndex(1)])
            assert not engine.started.wait(.05)
            transport.set_dictionary_path(second)
            assert done.wait(3)
            assert engine.calls == [('new', b'new\tnew\n')], engine.calls
            assert not errors
        finally:
            assert transport.shutdown()

        # A local snapshot may be ready before a subsequent online refresh
        # fails. Failure must not erase that successful queued selection.
        engine, done, errors = Engine(), threading.Event(), []
        transport = EciTextTransport(lambda: engine, Player(), lambda index: None,
                                     lambda number: done.set(), errors.append)
        try:
            # Prevent the worker from consuming the barrier between these
            # calls so that this exercises the actual race deterministically.
            with transport._condition:
                transport.begin_dictionary_load()
                transport.set_dictionary_path(first)
                transport.fail_dictionary_load()
                transport.submit(['old', EciIndex(1)])
            assert done.wait(3)
            assert engine.calls == [('old', b'old\told\n')], engine.calls
            assert not errors
        finally:
            assert transport.shutdown()

        # A newer, invalid selection must fall back to the previous complete
        # snapshot even when the worker has not installed that snapshot yet.
        engine, done, errors = Engine(), threading.Event(), []
        transport = EciTextTransport(lambda: engine, Player(), lambda index: None,
                                     lambda number: done.set(), errors.append)
        try:
            with transport._condition:
                transport.begin_dictionary_load()
                transport.set_dictionary_path(first)
                transport.begin_dictionary_load()
                transport.fail_dictionary_load()
                transport.submit(['old', EciIndex(1)])
            assert done.wait(3)
            assert engine.calls == [('old', b'old\told\n')], engine.calls
            assert not errors
        finally:
            assert transport.shutdown()

        # The first failed selection must never release speech without the
        # requested dictionary. A later successful selection can recover.
        engine, done, errors = Engine(), threading.Event(), []
        transport = EciTextTransport(lambda: engine, Player(), lambda index: None,
                                     lambda number: done.set(), errors.append)
        try:
            transport.begin_dictionary_load()
            transport.submit(['recover', EciIndex(1)])
            assert not engine.started.wait(.05)
            assert not transport.fail_dictionary_load()
            assert transport.dictionary_startup_blocked()
            assert not engine.started.wait(.05) and not done.is_set()
            transport.begin_dictionary_load()
            transport.set_dictionary_path(first)
            assert done.wait(3) and engine.calls == [('recover', b'old\told\n')]
            assert not transport.dictionary_startup_blocked()
            # A later failed refresh retains the already installed set.
            done.clear();engine.started.clear()
            transport.begin_dictionary_load()
            assert transport.fail_dictionary_load()
            transport.submit(['refresh', EciIndex(2)])
            assert done.wait(3) and engine.calls[-1] == ('refresh', b'old\told\n')
            transport.begin_dictionary_load()
            start = time.monotonic()
            assert transport.shutdown() and time.monotonic()-start < 1
        finally:
            transport.shutdown()

        # A validated snapshot can still fail during the native installation.
        # It must not be treated as installed until that call actually returns.
        class FailedInstallation(Engine):
            def set_dictionary_path(self, path):
                if path and Path(path) == first:
                    raise RuntimeError('simulated native dictionary error')
                super().set_dictionary_path(path)
        engine, done, errors, reported = FailedInstallation(), threading.Event(), [], threading.Event()
        def report(error):
            errors.append(error);reported.set()
        transport = EciTextTransport(lambda: engine, Player(), lambda index: None,
                                     lambda number: done.set(), report)
        try:
            transport.begin_dictionary_load()
            transport.set_dictionary_path(first)
            transport.submit(['native', EciIndex(1)])
            assert reported.wait(3) and isinstance(errors[0], DictionaryStartupError)
            assert transport.dictionary_startup_blocked()
            assert not engine.started.wait(.05) and not done.is_set()
            transport.begin_dictionary_load()
            transport.set_dictionary_path(second)
            assert done.wait(3) and engine.calls == [('native', b'new\tnew\n')]
        finally:
            transport.shutdown()

        # A newer selection that arrives during a failing native load must
        # also precede speech that was already queued behind the first one.
        entered, resume = threading.Event(), threading.Event()
        class SlowFailure(Engine):
            def set_dictionary_path(self, path):
                if path and Path(path) == first:
                    entered.set()
                    assert resume.wait(3)
                    raise RuntimeError('simulated delayed installation error')
                super().set_dictionary_path(path)
        engine, done, errors = SlowFailure(), threading.Event(), []
        transport = EciTextTransport(lambda: engine, Player(), lambda index: None,
                                     lambda number: done.set(), errors.append)
        try:
            transport.begin_dictionary_load()
            transport.set_dictionary_path(first)
            assert entered.wait(3)
            transport.submit(['superseded', EciIndex(1)])
            transport.begin_dictionary_load()
            transport.set_dictionary_path(second)
            resume.set()
            assert done.wait(3)
            assert engine.calls == [('superseded', b'new\tnew\n')]
            assert len(errors) == 1 and isinstance(errors[0], DictionaryStartupError)
        finally:
            resume.set();transport.shutdown()
    print('Dictionary startup ordering, cancellation, supersession and failure recovery passed')


if __name__ == '__main__': main()
