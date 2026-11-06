"""Development-only ECI ABI/ownership checks; no NVDA or voice data needed.

Run: python3 -B nvda/test_eci_direct.py
"""

from __future__ import annotations

import ctypes
import threading

from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciPitch, EciProsody, EciSynthesisError,
                               GERMAN_DIALECT)


HIGH_HANDLE = 0x12345678ABCDEF01


class Function:
    def __init__(self, implementation):
        self.implementation = implementation
        self.argtypes = None
        self.restype = None

    def __call__(self, *args):
        return self.implementation(*args)


class FakeEci:
    def __init__(self):
        self.callback = None
        self.callback_context = None
        self.buffer = None
        self.capacity = 0
        self.events = []
        self.returns = []
        self.stop_event = threading.Event()
        self.started = threading.Event()
        self.block_until_stop = False
        self.thread = None
        self.stopped = False
        self.deleted = False
        self.clear_calls = 0
        self.synthesize_calls = 0
        self.on_add_text = None
        self.failure = {}
        self.voice_mode = 0
        self.generic_parameters = {0x1004: 50, 0x1005: 50, 0x1006: 100}
        self.voice_parameters = {6: 98, 7: 100}
        self.lexicon_paths = []
        for name in (
            "eciNewEx", "eciDelete", "eciRegisterCallback", "eciSetOutputBuffer",
            "eciAddText", "eciInsertIndex", "eciInsertPitch", "eciSynthesize", "eciSynchronize",
            "eciClearInput", "eciStop", "eciGetParam", "eciSetParam",
            "eciGetVoiceParam", "eciSetVoiceParam",
            "eciLoadLexicon",
        ):
            setattr(self, name, Function(getattr(self, "_" + name)))

    def _eciNewEx(self, dialect):
        assert dialect == GERMAN_DIALECT
        return HIGH_HANDLE

    def _eciDelete(self, handle):
        assert handle == HIGH_HANDLE
        assert self.thread is None or not self.thread.is_alive(), "delete raced native callback"
        self.deleted = True
        return None

    def _eciRegisterCallback(self, handle, callback, context):
        assert handle == HIGH_HANDLE
        self.callback = callback
        self.callback_context = context

    def _eciSetOutputBuffer(self, handle, capacity, buffer):
        assert handle == HIGH_HANDLE
        self.capacity = capacity
        self.buffer = buffer
        return self.failure.get("eciSetOutputBuffer", 1)

    def _eciAddText(self, handle, text):
        assert handle == HIGH_HANDLE
        self.events.append(("text", text))
        if self.on_add_text is not None:
            self.on_add_text()
        return self.failure.get("eciAddText", 1)

    def _eciInsertIndex(self, handle, index):
        assert handle == HIGH_HANDLE
        self.events.append(("index", index))
        return self.failure.get("eciInsertIndex", 1)

    def _eciInsertPitch(self, handle, pitch):
        assert handle == HIGH_HANDLE
        self.events.append(("pitch", pitch))
        return self.failure.get("eciInsertPitch", 1)

    def _eciSynthesize(self, handle):
        assert handle == HIGH_HANDLE and self.buffer is not None
        self.synthesize_calls += 1
        self.stop_event.clear()
        self.started.clear()
        self.stopped = False

        def synthesize():
            self.started.set()
            self.buffer[0], self.buffer[1], self.buffer[2] = 1, -2, 32767
            self.returns.append(self.callback(handle, 0, 3, self.callback_context))
            if self.block_until_stop:
                assert self.stop_event.wait(2), "stop was not forwarded to ECI"
            if not self.stopped and self.returns[-1] == 1:
                self.returns.append(self.callback(handle, 2, 42, self.callback_context))

        if "eciSynthesize" in self.failure:
            return self.failure["eciSynthesize"]
        self.thread = threading.Thread(target=synthesize)
        self.thread.start()
        return 1

    def _eciSynchronize(self, handle):
        assert handle == HIGH_HANDLE
        self.thread.join(2)
        assert not self.thread.is_alive(), "synchronize did not join native work"
        return self.failure.get("eciSynchronize", 0 if self.stopped or self.returns[-1] == 2 else 1)

    def _eciStop(self, handle):
        assert handle == HIGH_HANDLE
        self.stopped = True
        self.stop_event.set()
        return 1

    def _eciClearInput(self, handle):
        assert handle == HIGH_HANDLE
        self.clear_calls += 1
        return 1

    def _eciGetParam(self, handle, parameter):
        assert handle == HIGH_HANDLE
        if parameter == 0x1003:
            return self.voice_mode
        if parameter in self.generic_parameters:
            return self.generic_parameters[parameter]
        return {0x1000: -2, 0x1001: 6, 0x1002: 2}[parameter]

    def _eciSetParam(self, handle, parameter, value):
        assert handle == HIGH_HANDLE and parameter in (0x1003, 0x1004, 0x1005, 0x1006)
        if self.failure.get("eciSetParam") is not None:
            return self.failure["eciSetParam"]
        if parameter == 0x1003:
            old, self.voice_mode = self.voice_mode, value
            return old
        old = self.generic_parameters[parameter]
        self.generic_parameters[parameter] = value
        return old

    def _eciLoadLexicon(self, handle, path):
        assert handle == HIGH_HANDLE
        self.lexicon_paths.append(path)
        return self.failure.get("eciLoadLexicon", 1)

    def _eciGetVoiceParam(self, handle, voice, parameter):
        assert handle == HIGH_HANDLE and voice == 0
        return self.voice_parameters[parameter]

    def _eciSetVoiceParam(self, handle, voice, parameter, value):
        assert handle == HIGH_HANDLE and voice == 0
        old = self.voice_parameters[parameter]
        self.voice_parameters[parameter] = value
        self.events.append(("prosody", parameter, value))
        return old


class WedgeFake:
    """First handle is poisoned: it reports busy=1 and refuses input/clear.

    A rebuilt handle behaves. Mirrors the field failure where one cancelled
    utterance left native busy=1 and every later job was rejected.
    """

    def __init__(self, wedge_mode):
        self.wedge_mode = wedge_mode
        self.next_handle = 1000
        self.new_calls = 0
        self.deleted = []
        self.callback = None
        self.buffer = None
        self.parameters = {}
        self.dictionary_paths = {}
        self.added = []
        for name in (
            "eciNewEx", "eciDelete", "eciRegisterCallback", "eciSetOutputBuffer",
            "eciAddText", "eciAddTextUtf8", "eciInsertIndex", "eciInsertPitch",
            "eciSynthesize", "eciSynchronize", "eciClearInput", "eciStop",
            "eciGetParam", "eciSetParam", "eciGetVoiceParam", "eciSetVoiceParam",
            "eciSetDictionaryPath", "eciIsBeingReentered",
        ):
            setattr(self, name, Function(getattr(self, "_" + name)))

    def _wedged(self, handle):
        return handle == 1001

    def _eciNewEx(self, dialect):
        self.new_calls += 1
        self.next_handle += 1
        self.parameters[self.next_handle] = {}
        return self.next_handle

    def _eciDelete(self, handle):
        self.deleted.append(handle)
        return None

    def _eciRegisterCallback(self, handle, callback, context):
        self.callback = callback

    def _eciSetOutputBuffer(self, handle, capacity, buffer):
        self.buffer = buffer
        return 1

    def _eciAddText(self, handle, text):
        if self._wedged(handle) and self.wedge_mode == "add":
            return 0
        self.added.append((handle, text))
        return 1

    def _eciAddTextUtf8(self, handle, data, length, mode):
        if self._wedged(handle) and self.wedge_mode == "add":
            return 0
        self.added.append((handle, data))
        return 1

    def _eciInsertIndex(self, handle, index):
        return 0 if self._wedged(handle) and self.wedge_mode == "add" else 1

    def _eciInsertPitch(self, handle, pitch):
        return 1

    def _eciSynthesize(self, handle):
        self.buffer[0] = 7
        self.callback(handle, 0, 1, None)
        return 1

    def _eciSynchronize(self, handle):
        return 1

    def _eciClearInput(self, handle):
        return 0 if self._wedged(handle) else 1

    def _eciStop(self, handle):
        return 1

    def _eciIsBeingReentered(self, handle):
        return 1 if self._wedged(handle) else 0

    def _eciGetParam(self, handle, parameter):
        return self.parameters[handle].get(parameter, 0)

    def _eciSetParam(self, handle, parameter, value):
        old = self.parameters[handle].get(parameter, 0)
        self.parameters[handle][parameter] = value
        return old

    def _eciGetVoiceParam(self, handle, voice, parameter):
        return 100

    def _eciSetVoiceParam(self, handle, voice, parameter, value):
        return 100

    def _eciSetDictionaryPath(self, handle, path):
        self.dictionary_paths[handle] = path
        return 1


def check_busy_recovery():
    from _reloq_eci_direct import EciText, NATIVE_SPEED
    # Busy refusal at submission: rebuilt handle must deliver the job once,
    # with the caller's parameters and dictionary replayed.
    fake = WedgeFake("add")
    engine = DirectEciEngine("ignored.dll", dialect=0x00010000,
                             encoding="utf-8", _loader=lambda _path: fake)
    try:
        engine.set_parameter(NATIVE_SPEED, 120)
        engine.set_dictionary_path("dictionaries")
        heard = []
        assert engine.speak([EciText("hello world")],
                            lambda pcm: heard.append(pcm),
                            lambda index: None)
        assert fake.new_calls == 2 and fake.deleted == [1001], (
            fake.new_calls, fake.deleted)
        assert heard == [b"\x07\x00"], heard
        assert fake.parameters[1002][NATIVE_SPEED] == 120
        assert fake.dictionary_paths[1002] == b"dictionaries"
        assert engine.speak([EciText("again")],
                            lambda pcm: None, lambda index: None)
        assert fake.new_calls == 2, "healthy handle must be reused"
    finally:
        engine.close()
    # Cancelled job whose input cannot be cleared: the next job must still
    # run, on a rebuilt handle, instead of failing busy forever.
    fake = WedgeFake("clear")
    engine = DirectEciEngine("ignored.dll", dialect=0x00010000,
                             encoding="utf-8", _loader=lambda _path: fake)
    try:
        engine._dll.eciAddTextUtf8  # binding exists for utf-8 engines
        original_add = fake._eciAddTextUtf8
        def add_then_cancel(handle, data, length, mode):
            result = original_add(handle, data, length, mode)
            engine.cancel()
            return result
        fake.eciAddTextUtf8 = Function(add_then_cancel)
        assert engine.speak([EciText("cancel me")],
                            lambda pcm: None, lambda index: None) is False
        fake.eciAddTextUtf8 = Function(original_add)
        assert fake.new_calls == 2 and fake.deleted == [1001], (
            fake.new_calls, fake.deleted)
        assert engine.speak([EciText("recovery")],
                            lambda pcm: None, lambda index: None)
        assert fake.new_calls == 2
    finally:
        engine.close()
    print("busy recovery: poisoned handle rebuilt, job and next job delivered")


def main():
    fake = FakeEci()
    engine = DirectEciEngine("ignored.dll", _loader=lambda _path: fake,
                             buffer_samples=16)
    assert engine._dll.eciNewEx.restype is ctypes.c_void_p
    for name in ("eciRegisterCallback", "eciAddText", "eciStop"):
        assert fake.__dict__[name].argtypes[0] is ctypes.c_void_p
    assert engine._handle == HIGH_HANDLE, "the 64-bit handle was truncated"
    seen = []
    try:
        assert engine.speak(["Mäuse ", EciPitch(78), EciIndex(42), "Öl",
                             EciPitch(50)],
                            lambda pcm: seen.append(("pcm", pcm)),
                            lambda index: seen.append(("index", index)))
        assert fake.events == [
            ("text", b"M\xe4use "), ("pitch", 78), ("index", 42),
            ("text", b"\xd6l"), ("pitch", 50),
        ]
        assert seen == [
            ("pcm", b"\x01\x00\xfe\xff\xff\x7f"),
            ("index", 42),
        ], "PCM callback length is samples, not bytes; index order changed"
        assert fake.returns == [1, 1]
        assert engine.speak(["one ", EciProsody(6, 75), EciProsody(7, 40), "word"],
            lambda pcm: None, lambda index: None)
        assert fake.events[-5:] == [("prosody", 6, 75), ("prosody", 7, 40),
            ("text", b"word"), ("prosody", 6, 98), ("prosody", 7, 100)]
        assert fake.voice_parameters == {6: 98, 7: 100}

        # Ordinary long text stays in one round: every extra round is a
        # phrase-final pause the listener can hear. The reported example
        # (207 bytes, short sentences, ordinary words) must not split.
        ordinary_text = (
            "Yeah yeah yeah! The fix works! Man NVAccess should incorporate "
            "your add-on changes and improvements! They won't, I know. But "
            "who cares! At the end of the day, those who want it can find "
            "what you have done! :")
        assert len(ordinary_text.encode("cp1252")) > 200
        assert len(DirectEciEngine._rounds(
            DirectEciEngine._prepare([ordinary_text], "cp1252"))) == 1
        assert len(DirectEciEngine._rounds(DirectEciEngine._prepare(
            ["word " * 60], "cp1252"))) == 1

        # A long phrase carrying hard tokens is synthesized in bounded
        # word-boundary rounds (upstream issue #19); joining the rounds
        # restores the input.
        rounds_before = fake.synthesize_calls
        events_before = len(fake.events)
        long_text = (
            "ai-data-centre-72mw-at-thames-valley-park-reading-stop-"
            "construction A2O3GeBaf9fNSvi39WoAAHicyyvNyQEABF8BvDNmYzM3YTIy "
            + "word " * 40)
        heard_indices = []
        assert engine.speak([long_text, EciIndex(9)],
                            lambda pcm: None,
                            lambda index: heard_indices.append(index))
        assert fake.synthesize_calls - rounds_before >= 2, (
            fake.synthesize_calls, rounds_before)
        joined = b"".join(event[1] for event in fake.events[events_before:]
                          if event[0] == "text")
        assert joined == long_text.encode("cp1252"), joined[:80]
        # The fake answers every synthesis round with its fixed index 42.
        assert heard_indices and all(index == 42 for index in heard_indices), heard_indices
        assert len(heard_indices) == fake.synthesize_calls - rounds_before
        rounds = DirectEciEngine._rounds(
            DirectEciEngine._prepare([long_text], "cp1252"))
        assert len(rounds) >= 2
        assert all(sum(len(p) for p in round_parts) <= 200
                   for round_parts in rounds), rounds
        assert b"".join(p for round_parts in rounds for p in round_parts) == \
            long_text.encode("cp1252")
        # Safe text before a risky phrase is not resubmitted with it:
        # the risky phrase starts a round of its own.
        mixed = "Hello world. " + long_text
        rounds = DirectEciEngine._rounds(
            DirectEciEngine._prepare([mixed], "cp1252"))
        assert b"".join(rounds[0]) == b"Hello world.", rounds[0]
        assert b"".join(p for round_parts in rounds for p in round_parts) == \
            mixed.encode("cp1252")

        previous = len(fake.events)
        for invalid in (["a", "\U0001f600"], ["a\0b"], [EciIndex(-1)],
                        [EciPitch(-1)], [EciPitch(101)], [EciProsody(2, 50)],
                        [EciProsody(6, 251)], [EciProsody(7, 101)], [EciProsody(True, 50)]):
            try:
                engine.speak(invalid, lambda _pcm: None, lambda _index: None)
            except (UnicodeEncodeError, ValueError):
                pass
            else:
                raise AssertionError(f"invalid ECI input accepted: {invalid!r}")
            assert len(fake.events) == previous, "validation queued partial text"

        def broken(_pcm):
            raise LookupError("audio queue rejected buffer")

        try:
            engine.speak([EciProsody(7, 40), "Maus"], broken, lambda _index: None)
        except LookupError as error:
            assert str(error) == "audio queue rejected buffer"
        else:
            raise AssertionError("ctypes callback swallowed an audio error")
        assert fake.returns[-1] == 2
        assert fake.voice_parameters[7] == 100, "callback failure leaked temporary volume"

        fake.block_until_stop = True
        outcome = []

        def cancel_from_callback(_pcm):
            threading.Thread(target=engine.cancel).start()

        assert engine.speak([EciProsody(6, 75), "Maus"], cancel_from_callback,
                            lambda index: outcome.append(index)) is False
        assert not outcome and fake.stopped
        assert fake.voice_parameters[6] == 98, "cancellation leaked temporary speed"
        assert not fake.thread.is_alive()

        before = fake.clear_calls
        fake.on_add_text = engine.cancel
        assert engine.speak(["Hieb.", "Pack."],
                            lambda _pcm: None,
                            lambda _index: None) is False
        assert fake.clear_calls == before + 1, "cancelled queue was not cleared"
        fake.on_add_text = None
        # Preserve the input failure while reporting the reentry state that
        # distinguishes a stuck native engine from an invalid input queue.
        fake.eciIsBeingReentered = Function(lambda handle: 1)
        fake.failure["eciAddText"] = 0
        try:
            engine.speak(["long filename"], lambda _pcm: None, lambda _index: None)
        except RuntimeError as error:
            assert "eciAddText rejected input (code 0; native busy=1)" in str(error)
        else:
            raise AssertionError("rejected input was not reported")
        del fake.failure["eciAddText"]
    finally:
        engine.close()
    assert fake.deleted
    generic_dll = FakeEci()
    generic = DirectEciEngine("ignored.dll", _loader=lambda _path: generic_dll,
                              native_voice=True,
                              lexicon_path=__file__)
    assert generic_dll.voice_mode == 1
    assert generic_dll.lexicon_paths and generic_dll.lexicon_paths[0].endswith(
        b"test_eci_direct.py")
    generic.set_parameter(0x1004, 70)
    assert generic_dll.generic_parameters[0x1004] == 70
    generic.close()
    rejected_lexicon = FakeEci()
    rejected_lexicon.failure["eciLoadLexicon"] = 0
    fallback = DirectEciEngine("ignored.dll", _loader=lambda _path: rejected_lexicon,
                               native_voice=True, lexicon_path=__file__)
    assert rejected_lexicon.voice_mode == 1
    fallback.close()
    refused_dll = FakeEci()
    refused_dll.failure["eciSetParam"] = -1
    try:
        DirectEciEngine("ignored.dll", _loader=lambda _path: refused_dll,
                        native_voice=True)
    except RuntimeError as error:
        assert "native voice" in str(error)
    else:
        raise AssertionError("native voice mode refusal not surfaced")
    assert refused_dll.deleted
    for failing in ("eciSetOutputBuffer", "eciAddText", "eciInsertIndex",
                    "eciInsertPitch",
                    "eciSynthesize", "eciSynchronize"):
        broken_dll = FakeEci()
        broken_dll.failure[failing] = -2
        if failing == "eciSetOutputBuffer":
            try:
                DirectEciEngine("ignored.dll", _loader=lambda _path: broken_dll)
            except RuntimeError as error:
                assert "eciSetOutputBuffer" in str(error)
            else:
                raise AssertionError("negative output-buffer error accepted")
            assert broken_dll.deleted
            continue
        broken_engine = DirectEciEngine("ignored.dll", _loader=lambda _path: broken_dll)
        try:
            parts = ["Maus", EciIndex(42)]
            if failing == "eciInsertIndex":
                parts = [EciIndex(42)]
            elif failing == "eciInsertPitch":
                parts = [EciPitch(75)]
            try:
                broken_engine.speak(parts, lambda _pcm: None, lambda _index: None)
            except RuntimeError as error:
                assert failing in str(error)
                if failing == "eciSynthesize":
                    assert isinstance(error, EciSynthesisError)
                    assert (error.status, error.stage, error.utf8_offset,
                            error.character_position) == (-2, 2, 6, 5)
            else:
                raise AssertionError(f"negative {failing} error accepted")
        finally:
            broken_engine.close()
    check_busy_recovery()
    print("direct ECI binding: 64-bit handle, CP1252, PCM, index, cancel/join OK")


if __name__ == "__main__":
    main()
