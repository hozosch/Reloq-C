"""Language routing, persistence, cancellation and native PCM regression."""

from dataclasses import replace
from pathlib import Path
import hashlib
import sys
import tempfile
import threading
import time

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciPitch, EciLanguage
from _reloq_eci_direct import NATIVE_PITCH, NATIVE_SPEED, NATIVE_VOLUME, NATIVE_INFLECTION, PAUSE_MODE, PROCESS_TAGS, HEAD_SIZE, ROUGHNESS, BREATHINESS, GENDER
from _reloq_languages import GERMAN, LanguageEngine, language_for
from _reloq_eci_transport import EciTextTransport

ROOT = Path(__file__).resolve().parents[1]
ENGLISH = replace(GERMAN, id=0x10000, name="Test only", locale="en_GB")
CATALOG = {GERMAN.id: GERMAN, ENGLISH.id: ENGLISH}


class Native:
    instances = []
    calls = []
    entered = threading.Event()
    release = threading.Event()

    def __init__(self, path, *, dialect, encoding, **kwargs):
        self.owner = threading.get_ident()
        self.dialect = dialect
        self.encoding = encoding
        self.params = {}
        self.closed = False
        self.instances.append(self)

    def set_parameter(self, parameter, value):
        assert threading.get_ident() == self.owner
        self.params[parameter] = value

    def speak(self, parts, pcm, index):
        assert threading.get_ident() == self.owner
        self.calls.append((self.dialect, tuple(parts), dict(self.params)))
        for part in parts:
            if part == "HOLD":
                self.entered.set()
                assert self.release.wait(2)
                return False
            if isinstance(part, EciIndex):
                index(part.value)
            elif isinstance(part, str):
                pcm(b"\x01\x00" * 200)
        return True

    def cancel(self):
        self.release.set()

    def close(self):
        assert threading.get_ident() == self.owner
        self.closed = True


def until(predicate):
    end = time.monotonic() + 3
    while not predicate():
        if time.monotonic() >= end:
            raise AssertionError("worker did not finish")
        time.sleep(.002)


def routing():
    assert language_for("de-AT", GERMAN.id, CATALOG) == GERMAN.id
    assert language_for("en-US", GERMAN.id, CATALOG) == ENGLISH.id
    assert language_for("ja", GERMAN.id, CATALOG) is None
    assert language_for(None, GERMAN.id, CATALOG) == GERMAN.id
    engine = LanguageEngine("eci.dll", languages=CATALOG, engine_factory=Native)
    engine.set_parameter(PAUSE_MODE, 2)
    engine.set_parameter(PROCESS_TAGS, 1)
    engine.set_parameter(NATIVE_PITCH, 65)
    indices = []
    assert engine.speak(["eins ", EciLanguage(GERMAN.id, "1"), "zwei",
        EciPitch(80), EciLanguage(ENGLISH.id, "1"), "three", EciIndex(9),
        EciLanguage(GERMAN.id, "1"), "vier"], lambda b: None, indices.append)
    assert indices == [9]
    assert [c[0] for c in Native.calls] == [GERMAN.id, ENGLISH.id, GERMAN.id]
    assert Native.calls[0][1] == ("eins ", "zwei", EciPitch(80))
    assert Native.calls[1][1] == (EciPitch(80), "three", EciIndex(9))
    assert all(c[2] == {**dict(GERMAN.voices[0].parameters), PAUSE_MODE: 2, PROCESS_TAGS: 1, NATIVE_PITCH: 65} for c in Native.calls)
    assert engine.speak(["next"], lambda b: None, indices.append)
    assert Native.calls[-1][0:2] == (GERMAN.id, ("next",))
    try:
        engine.speak(["must not escape", EciLanguage(0xdead, "1")], lambda b: None, indices.append)
    except KeyError:
        pass
    else:
        raise AssertionError("unsupported language accepted")
    assert len(Native.calls) == 4
    engine.close()
    assert all(n.closed for n in Native.instances)


def cancellation():
    class Player:
        def feed(self, pcm, onDone=None):
            if onDone:
                onDone()
        def idle(self):
            pass
        def stop(self):
            pass
        def pause(self, value):
            pass
        def close(self):
            pass

    errors, done, indices = [], [], []
    Native.calls.clear()
    Native.entered.clear()
    Native.release.clear()
    transport = EciTextTransport(lambda: LanguageEngine("eci.dll", languages=CATALOG,
        engine_factory=Native), Player(), indices.append, done.append, errors.append)
    transport.submit([EciLanguage(ENGLISH.id, "1"), "HOLD", EciIndex(1)])
    assert Native.entered.wait(2)
    transport.select_voice(ENGLISH.id, "2")
    transport.set_parameter(PAUSE_MODE, 2)
    transport.set_parameter(HEAD_SIZE, 22)
    transport.set_parameter(ROUGHNESS, 37)
    transport.cancel()
    number = transport.submit(["after cancel", EciIndex(2)])
    until(lambda: done == [number])
    assert indices == [2] and not errors
    assert Native.calls[-1] == (ENGLISH.id, ("after cancel", EciIndex(2)), {**dict(ENGLISH.voice("2").parameters), PAUSE_MODE: 2, HEAD_SIZE: 22, ROUGHNESS: 37})
    assert transport.shutdown()
    assert all(n.closed for n in Native.instances)


def voice_presets():
    before = len(Native.instances)
    Native.calls.clear()
    engine = LanguageEngine("eci.dll", engine_factory=Native)
    engine.set_parameter(NATIVE_SPEED, 176)
    engine.set_parameter(PAUSE_MODE, 2)
    for ident in ("2", "3", "7", "1", "2"):
        voice = GERMAN.voice(ident)
        engine.select_voice(GERMAN.id, ident)
        assert engine.speak(["Stimmwechsel"], lambda b: None, lambda n: None)
        expected = dict(voice.parameters)
        expected.update({NATIVE_SPEED: 176, PAUSE_MODE: 2})
        assert Native.calls[-1][2] == expected
    engine.set_parameter(HEAD_SIZE, 70)
    engine.set_parameter(ROUGHNESS, 27)
    engine.set_parameter(BREATHINESS, 61)
    engine.select_voice(GERMAN.id, "2")
    assert engine.speak(["Eigene Werte"], lambda b: None, lambda n: None)
    assert Native.calls[-1][2][HEAD_SIZE] == 70
    assert Native.calls[-1][2][ROUGHNESS] == 27
    assert Native.calls[-1][2][BREATHINESS] == 61
    engine.close()
    assert len(Native.instances) == before + 1, "presets duplicated their language core"


def native_regression():
    lib = ROOT / "eci/libreloq_eci.so"
    texts = ["Ausführen Dialogfeld.", "Administratorberechtigungen",
             "des Lesens", "Eingeklappt. Silbengrenze.", "NVDA-Menü",
             "Samstag 10 Oktober 2026", "Chance und Restaurant.",
             "Komma", "Komma folgt.", "Hören Sie mich?", "81 und 81000",
             "keine Zeit", "Aktualisieren und Ausführen von Recheck-Skripten."]
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        (path / "eci.dll").symlink_to(lib)
        previous = DirectEciEngine(lib, experimental_generic=True)
        current = LanguageEngine(path / "eci.dll")
        for engine in (previous, current):
            for parameter, value in ((NATIVE_SPEED, 98), (NATIVE_PITCH, 50),
                                      (NATIVE_INFLECTION, 30), (NATIVE_VOLUME, 100)):
                engine.set_parameter(parameter, value)

        def render(engine, parts):
            chunks, marks = [], []
            assert engine.speak(parts, chunks.append,
                lambda n: marks.append((n, sum(map(len, chunks)))))
            return b"".join(chunks), marks

        checks = 0
        for mode in range(3):
            for engine in (previous, current):
                engine.set_parameter(PAUSE_MODE, mode)
            for text in texts:
                parts = [text, EciIndex(7)]
                before = render(previous, parts)
                after = render(current, [EciLanguage(GERMAN.id, "1"), *parts])
                assert before == after, (mode, text)
                assert after[0]
                checks += 1
        for engine in (previous, current):
            engine.set_parameter(PROCESS_TAGS, 1)
        parts = ["Hallo `vb70 Welt `p20 und `vv60 Ende.", EciIndex(8)]
        assert render(previous, parts) == render(current, parts)
        previous.close()
        current.close()
    print(f"Native regression: {checks + 1} PCM/index comparisons byte-identical")


if __name__ == "__main__":
    routing()
    cancellation()
    voice_presets()
    native_regression()
    print("Language interface: routing, restore, settings and cancel OK")
