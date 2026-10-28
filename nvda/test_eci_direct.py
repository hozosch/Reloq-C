"""Development-only ECI ABI/ownership checks; no NVDA or voice data needed.

Run: python3 -B nvda/test_eci_direct.py
"""

from __future__ import annotations

import ctypes
import threading

from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciPitch, EciSynthesisError,
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
        self.on_add_text = None
        self.failure = {}
        self.voice_mode = 0
        self.generic_parameters = {0x1004: 50, 0x1005: 50, 0x1006: 100}
        self.lexicon_paths = []
        for name in (
            "eciNewEx", "eciDelete", "eciRegisterCallback", "eciSetOutputBuffer",
            "eciAddText", "eciInsertIndex", "eciInsertPitch", "eciSynthesize", "eciSynchronize",
            "eciClearInput", "eciStop", "eciGetParam", "eciSetParam",
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

        previous = len(fake.events)
        for invalid in (["a", "\U0001f600"], ["a\0b"], [EciIndex(-1)],
                        [EciPitch(-1)], [EciPitch(101)]):
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
            engine.speak(["Maus"], broken, lambda _index: None)
        except LookupError as error:
            assert str(error) == "audio queue rejected buffer"
        else:
            raise AssertionError("ctypes callback swallowed an audio error")
        assert fake.returns[-1] == 2

        fake.block_until_stop = True
        outcome = []

        def cancel_from_callback(_pcm):
            threading.Thread(target=engine.cancel).start()

        assert engine.speak(["Maus"], cancel_from_callback,
                            lambda index: outcome.append(index)) is False
        assert not outcome and fake.stopped
        assert not fake.thread.is_alive()

        before = fake.clear_calls
        fake.on_add_text = engine.cancel
        assert engine.speak(["Hieb.", "Pack."],
                            lambda _pcm: None,
                            lambda _index: None) is False
        assert fake.clear_calls == before + 1, "cancelled queue was not cleared"
        fake.on_add_text = None
    finally:
        engine.close()
    assert fake.deleted
    generic_dll = FakeEci()
    generic = DirectEciEngine("ignored.dll", _loader=lambda _path: generic_dll,
                              experimental_generic=True,
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
                               experimental_generic=True, lexicon_path=__file__)
    assert rejected_lexicon.voice_mode == 1
    fallback.close()
    refused_dll = FakeEci()
    refused_dll.failure["eciSetParam"] = -1
    try:
        DirectEciEngine("ignored.dll", _loader=lambda _path: refused_dll,
                        experimental_generic=True)
    except RuntimeError as error:
        assert "experimental generic" in str(error)
    else:
        raise AssertionError("experimental mode refusal not surfaced")
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
    print("direct ECI binding: 64-bit handle, CP1252, PCM, index, cancel/join OK")


if __name__ == "__main__":
    main()
