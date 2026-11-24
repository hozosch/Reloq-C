#!/usr/bin/env python3
"""Exercise sd_reloq through Speech Dispatcher's module protocol.

No server is involved: the test speaks the module protocol on the module's
standard input and reads its replies, events and PCM. With --library, plain
speech is also compared sample for sample with the same text synthesized
directly through libreloq_eci.so, which shows that no markup, control text or
punctuation filtering reached the engine as speech.
"""

import argparse
import ctypes
import os
from pathlib import Path
import select
import struct
import subprocess
import tempfile


class ModuleProcess:
    def __init__(self, binary, config):
        self.process = subprocess.Popen(
            [binary, config], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, bufsize=0)
        self.icons = []

    def send(self, data):
        self.process.stdin.write(data.encode("utf-8") if isinstance(data, str) else data)
        self.process.stdin.flush()

    def read_line(self, timeout=30):
        ready, _, _ = select.select([self.process.stdout], [], [], timeout)
        if not ready:
            raise AssertionError("module timed out")
        line = self.process.stdout.readline()
        if not line:
            error = self.process.stderr.read().decode("utf-8", "replace")
            raise AssertionError(f"module closed unexpectedly; stderr: {error}")
        return line

    def expect(self, wanted):
        line = self.read_line()
        assert line == wanted, (wanted, line)

    def command_with_data(self, command, lines, acknowledgement):
        self.send(command + "\n")
        self.expect(acknowledgement)
        self.send("".join(f"{line}\n" for line in lines) + ".\n")

    def set(self, **values):
        self.command_with_data("SET", [f"{name}={value}" for name, value in values.items()],
                               b"203 OK RECEIVING SETTINGS\n")
        self.expect(b"203 OK SETTINGS RECEIVED\n")

    def list_voices(self):
        self.send("LIST VOICES\n")
        voices = []
        while True:
            line = self.read_line()
            if line == b"200 OK VOICE LIST SENT\n":
                return voices
            voices.append(line.decode("utf-8").removeprefix("200-").rstrip("\n").split("\t"))

    def speak(self, text, command="SPEAK", interrupt=None):
        self.icons = []
        self.command_with_data(command, [text], b"202 OK RECEIVING MESSAGE\n")
        self.expect(b"200 OK SPEAKING\n")
        self.expect(b"701 BEGIN\n")
        audio = bytearray()
        marks = []
        metadata = {}
        interrupted = False
        while True:
            line = self.read_line()
            if line.startswith(b"705-AUDIO\0"):
                payload = line[len(b"705-AUDIO\0"):-1]
                at = 0
                while at < len(payload):
                    if payload[at] == 0x7D:
                        at += 1
                        audio.append(payload[at] ^ 0x20)
                    else:
                        audio.append(payload[at])
                    at += 1
                self.expect(b"705 AUDIO\n")
                if interrupt and not interrupted:
                    self.send(interrupt + "\n")
                    interrupted = True
            elif line.startswith(b"705-"):
                key, value = line[4:].decode("ascii").strip().split("=", 1)
                metadata[key] = int(value)
            elif line.startswith(b"700-"):
                marks.append(line[4:].decode("utf-8").strip())
                self.expect(b"700 INDEX MARK\n")
            elif line.startswith(b"706-"):
                self.icons.append(line[4:].decode("utf-8").strip())
                self.expect(b"706 ICON\n")
            elif line in (b"702 END\n", b"703 STOP\n", b"704 PAUSE\n"):
                return bytes(audio), metadata, marks, line.strip().decode("ascii")
            elif line.startswith(b"200 OK"):
                # The reply to an interrupting STOP or PAUSE.
                continue
            else:
                raise AssertionError(f"unexpected module response: {line!r}")

    def close(self):
        if self.process.poll() is None:
            self.send("QUIT\n")
            self.expect(b"210 OK QUIT\n")
        return self.process.wait(timeout=10)


def start(binary, config):
    module = ModuleProcess(binary, config)
    module.send("INIT\n")
    status = module.read_line()
    assert status.startswith(b"299-ReLoq C ") and status.endswith(b" initialized\n"), status
    module.expect(b"299 OK LOADED SUCCESSFULLY\n")
    module.command_with_data("AUDIO", ["audio_output_method=server"],
                             b"207 OK RECEIVING AUDIO SETTINGS\n")
    module.expect(b"203 OK AUDIO INITIALIZED\n")
    module.set(rate="0", pitch="0", pitch_range="0", volume="0", punctuation_mode="none",
               spelling_mode="off", cap_let_recogn="none", voice="male1",
               language="en-US", synthesis_voice="NULL")
    return module


def peak(audio):
    return max((abs(v) for v in struct.unpack(f"<{len(audio) // 2}h", audio)), default=0)


class Direct:
    """The same synthesis, through the public ECI library."""

    def __init__(self, library):
        h, i = ctypes.c_void_p, ctypes.c_int
        self.callback_type = ctypes.CFUNCTYPE(i, h, i, i, h)
        self.lib = ctypes.CDLL(str(Path(library).resolve()))
        for name, args, result in (
                ("eciNewEx", (i,), h), ("eciDelete", (h,), h),
                ("eciSetParam", (h, i, i), i),
                ("eciAddTextUtf8", (h, ctypes.c_char_p, ctypes.c_size_t, i), i),
                ("eciSynthesize", (h,), i), ("eciSynchronize", (h,), i),
                ("eciRegisterCallback", (h, self.callback_type, h), None),
                ("eciSetOutputBuffer", (h, i, ctypes.POINTER(ctypes.c_int16)), i)):
            function = getattr(self.lib, name)
            function.argtypes, function.restype = args, result
        self.buffer = (ctypes.c_int16 * 551)()
        self.audio = bytearray()
        self.receive = self.callback_type(self._receive)

    def _receive(self, _handle, message, value, _data):
        if message == 0:
            self.audio += ctypes.string_at(self.buffer, value * 2)
        return 1

    def speak(self, language, text, *, pitch=65, speed=50):
        handle = self.lib.eciNewEx(language)
        assert handle
        try:
            self.lib.eciRegisterCallback(handle, self.receive, None)
            assert self.lib.eciSetOutputBuffer(handle, len(self.buffer), self.buffer) > 0
            # The module's defaults with the Reed preset.
            for parameter, value in ((0x1003, 1), (5, 1), (0x100b, 0), (3, 0), (11, 0),
                                     (0x1015, 0), (0x1016, 0), (0x100c, 0), (0x101b, 2),
                                     (0x101c, 0), (0x101d, 0), (0x1010, 0), (0x100d, 50),
                                     (0x100e, 0), (0x100f, 0), (0x1008, pitch), (0x1009, 30),
                                     (0x100a, 100), (0x1007, speed)):
                assert self.lib.eciSetParam(handle, parameter, value) >= 0, hex(parameter)
            self.audio = bytearray()
            data = text.encode("utf-8")
            assert self.lib.eciAddTextUtf8(handle, data, len(data), 0)
            assert self.lib.eciSynthesize(handle) > 0 and self.lib.eciSynchronize(handle)
            return bytes(self.audio)
        finally:
            self.lib.eciDelete(handle)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("module", type=Path)
    parser.add_argument("config", type=Path)
    parser.add_argument("--library", type=Path,
                        help="libreloq_eci.so for sample-exact comparisons")
    args = parser.parse_args()
    binary, config = str(args.module.resolve()), str(args.config.resolve())

    module = start(binary, config)
    try:
        print("speechd: voices", flush=True)
        voices = module.list_voices()
        names = [voice[0] for voice in voices]
        assert len(voices) == 15 * 8 and len(set(names)) == len(names), len(voices)
        assert ["Reed (American English)", "en-US", "none"] in voices
        assert ["Shelly (German)", "de-DE", "none"] in voices
        assert ["Grandpa (Aragonese)", "an-ES", "none"] in voices
        assert {voice[1] for voice in voices} == {
            "en-US", "en-GB", "es-ES", "es-MX", "fr-FR", "fr-CA", "de-DE", "it-IT",
            "zh-CN", "pt-BR", "ja-JP", "fi-FI", "ko-KR", "el-GR", "an-ES"}

        print("speechd: plain speech", flush=True)
        text = "The quick brown fox jumps over the lazy dog."
        audio, metadata, marks, event = module.speak(text)
        assert event == "702 END" and marks == []
        assert (metadata["bits"], metadata["num_channels"], metadata["sample_rate"]) == (16, 1, 11025), metadata
        assert audio and len(audio) % 2 == 0
        direct = Direct(args.library) if args.library else None
        if direct:
            assert audio == direct.speak(0x10000, text), "module speech differs from direct synthesis"

        print("speechd: punctuation none keeps sentence punctuation only", flush=True)
        symbols, _, _, _ = module.speak("Files # and @ folders, it's done.")
        spaced, _, _, _ = module.speak("Files   and   folders, it's done.")
        assert symbols == spaced
        if direct:
            assert spaced == direct.speak(0x10000, "Files   and   folders, it's done.")
        module.set(punctuation_mode="all")
        named, _, _, _ = module.speak("Files # and @ folders, it's done.")
        assert named != spaced
        module.set(punctuation_mode="none")

        print("speechd: rate, pitch, range and volume", flush=True)
        module.set(rate="100")
        fast, _, _, _ = module.speak(text)
        module.set(rate="-100")
        slow, _, _, _ = module.speak(text)
        assert len(fast) < len(audio) < len(slow), (len(fast), len(audio), len(slow))
        if direct:
            assert fast == direct.speak(0x10000, text, speed=250)
        module.set(rate="0", pitch="60")
        pitched, _, _, _ = module.speak(text)
        assert pitched != audio
        if direct:
            assert pitched == direct.speak(0x10000, text, pitch=65 + 60 * 35 // 100)
        module.set(pitch="0", pitch_range="60")
        ranged, _, _, _ = module.speak(text)
        assert ranged != audio
        module.set(pitch_range="0", volume="-60")
        quiet, _, _, _ = module.speak(text)
        assert peak(quiet) < peak(audio) * 0.6, (peak(quiet), peak(audio))
        module.set(volume="0")

        print("speechd: languages and named voices", flush=True)
        module.set(language="de")
        german, _, _, event = module.speak("Guten Morgen, wie geht es dir?")
        assert event == "702 END" and german
        if direct:
            assert german == direct.speak(0x40000, "Guten Morgen, wie geht es dir?")
        module.set(synthesis_voice="Shelly (German)", language="en-US")
        shelly, _, _, _ = module.speak("Guten Morgen, wie geht es dir?")
        assert shelly and shelly != german
        module.set(synthesis_voice="NULL", voice="female1", language="de-DE")
        female, _, _, _ = module.speak("Guten Morgen, wie geht es dir?")
        assert female == shelly
        module.set(voice="male1", language="zh-TW")
        mandarin, _, _, event = module.speak("你好，世界。")
        assert event == "702 END" and mandarin
        module.set(language="ja")
        japanese, _, _, _ = module.speak("こんにちは。")
        module.set(language="el")
        greek, _, _, _ = module.speak("Καλημέρα.")
        assert japanese and greek
        module.set(language="xx-YY")
        unknown, _, _, _ = module.speak("Καλημέρα.")
        assert unknown == greek, "an unsupported language must keep the current one"
        module.set(language="en-GB")
        british, _, _, _ = module.speak(text)
        module.set(language="en")
        still_british, _, _, _ = module.speak(text)
        assert british == still_british, "a bare language keeps the current variant"
        module.set(language="en-US")

        print("speechd: SSML marks and entities", flush=True)
        ssml = ('<speak>The <mark name="first"/>quick &amp; brown '
                '<mark name="second"/>fox &#x41;.</speak>')
        marked, _, marks, event = module.speak(ssml)
        assert event == "702 END" and marks == ["first", "second"], marks
        if direct:
            # "&" is a symbol that punctuation mode none filters.
            assert marked == direct.speak(0x10000, " The quick   brown fox A. ")

        print("speechd: characters, keys, spelling and capitals", flush=True)
        small, _, _, _ = module.speak("a", "CHAR")
        capital, _, _, _ = module.speak("A", "CHAR")
        assert small and small == capital
        key, _, _, _ = module.speak("a", "KEY")
        assert key == small
        shifted, _, _, _ = module.speak("shift_a", "KEY")
        assert shifted and shifted != small
        module.set(cap_let_recogn="spell")
        raised, _, _, _ = module.speak("A", "CHAR")
        assert raised != small and len(raised) == len(small)
        module.set(cap_let_recogn="icon")
        _, _, _, event = module.speak("Hello World and NASA", "SPEAK")
        assert module.icons == ["capital", "capital", "capital"], module.icons
        module.set(cap_let_recogn="none")
        module.set(spelling_mode="on")
        spelled, _, _, _ = module.speak("ab")
        module.set(spelling_mode="off")
        word, _, _, _ = module.speak("ab")
        assert spelled != word

        print("speechd: sound icons", flush=True)
        silent, _, _, event = module.speak("message", "SOUND_ICON")
        assert event == "702 END" and silent == b"" and module.icons == ["message"]

        print("speechd: unusual text", flush=True)
        literal, _, _, event = module.speak("NULL")
        assert event == "702 END" and literal
        # Malformed UTF-8 must neither crash the module nor stall it.
        module.send(b"SPEAK\n")
        module.expect(b"202 OK RECEIVING MESSAGE\n")
        module.send(b"caf\xc3\xa9 bad \xff\xfe \xe2\x82 bytes\n.\n")
        while module.read_line() not in (b"702 END\n", b"703 STOP\n"):
            pass

        print("speechd: stop and recovery", flush=True)
        long_text = "This sentence keeps the engine busy for a while. " * 60
        full, _, _, _ = module.speak(long_text)
        stopped, _, _, event = module.speak(long_text, interrupt="STOP")
        assert event == "703 STOP" and len(stopped) < len(full) // 2, (len(stopped), len(full))
        again, _, _, event = module.speak(text)
        assert event == "702 END" and again == audio, "speech after a stop must be unaffected"

        print("speechd: pause at Speech Dispatcher's own mark", flush=True)
        pausable = "".join(f'<mark name="__spd_{n}"/>Sentence number {n}. ' for n in range(40))
        _, _, marks, event = module.speak(f"<speak>{pausable}</speak>", interrupt="PAUSE")
        assert event == "704 PAUSE" and marks and marks[-1].startswith("__spd_"), (event, marks[-3:])
        assert len(marks) < 40
        again, _, _, event = module.speak(text)
        assert event == "702 END" and again == audio
    finally:
        assert module.close() == 0

    print("speechd: configuration file", flush=True)
    with tempfile.TemporaryDirectory(prefix="reloq-speechd-") as directory:
        custom = Path(directory, "reloq.conf")
        custom.write_text("SampleRate 22050\nPauseMode 7\nNoSuchSetting 1\n"
                          'DictionaryDirectory "/nonexistent/reloq"\n')
        module = start(binary, str(custom))
        try:
            wide, metadata, _, event = module.speak(text)
            assert event == "702 END" and metadata["sample_rate"] == 22050
            assert len(wide) > len(audio) * 1.8
        finally:
            assert module.close() == 0
        errors = module.process.stderr.read().decode("utf-8", "replace")
        assert "PauseMode 7" in errors and "NoSuchSetting" in errors, errors
        assert "/nonexistent/reloq" in errors, errors
        # A missing configuration file is not an error.
        module = start(binary, os.path.join(directory, "missing.conf"))
        assert module.close() == 0
    print("speechd: all checks passed")


if __name__ == "__main__":
    main()
