"""Check the built NVDA preview without pretending to run NVDA on Windows.

This uses fake NVDA UI/audio modules and a fake ECI backend. Native ABI and
text-to-PCM behavior have separate tests in eci/ and nvda/.
"""

from __future__ import annotations

import builtins
import ctypes
import importlib
from pathlib import Path
import sys
import tempfile
import threading
import time
import types
from unittest import mock
import zipfile

from build_preview import assert_x64_pe, build


ROOT = Path(__file__).resolve().parents[1]


def fake_module(name: str, **properties):
    result = types.ModuleType(name)
    vars(result).update(properties)
    sys.modules[name] = result
    return result


def wait_until(condition, timeout=2):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if condition():
            return
        time.sleep(0.002)
    raise AssertionError("preview worker did not finish in time")


class Player:
    def __init__(self, **kwargs):
        assert kwargs == dict(channels=1, samplesPerSec=11025,
                              bitsPerSample=16, outputDevice="default")
        self.blocks = []
        self.stopped = 0
        self.closed = False

    def feed(self, pcm, onDone=None):
        self.blocks.append(pcm)
        if onDone:
            onDone()

    def stop(self):
        self.stopped += 1

    def pause(self, switch):
        pass

    def close(self):
        self.closed = True


class Engine:
    def __init__(self, _dll):
        self.texts = []
        self.closed = False
        self.cancelled = False

    def speak(self, parts, on_pcm, on_index):
        text = "".join(parts)
        self.texts.append(text)
        if text == "nicht abgedeckt":
            raise RuntimeError("eciSynthesize failed")
        on_pcm(b"\x00\x00" * 1100)
        return True

    def cancel(self):
        self.cancelled = True

    def close(self):
        self.closed = True


def test_package() -> None:
    with tempfile.TemporaryDirectory() as temporary:
        target = Path(temporary) / "preview.nvda-addon"
        build(target, ROOT / "eci" / "eci.dll")
        with zipfile.ZipFile(target) as archive:
            names = set(archive.namelist())
            assert "manifest.ini" in names
            assert "globalPlugins/reloqPreview/eci.dll" in names
            assert "globalPlugins/reloqPreview/__init__.py" in names
            assert not any("__pycache__" in name or name.endswith(".pyc") for name in names)
            assert not any(name.startswith("synthDrivers/") for name in names)
            assert not any("Apple" in name or "OpenEVV" in name for name in names)
            assert_x64_pe(archive.read("globalPlugins/reloqPreview/eci.dll"))
            archive.extractall(temporary)

        messages = []
        players = []
        engines = []
        dialogs = []

        def player_factory(**kwargs):
            player = Player(**kwargs)
            players.append(player)
            return player

        class BasePlugin:
            def __init__(self):
                self.base_terminated = False

            def terminate(self):
                self.base_terminated = True

        popup_calls = []
        frame = types.SimpleNamespace(
            prePopup=lambda: popup_calls.append("pre"),
            postPopup=lambda: popup_calls.append("post"),
        )

        class Dialog:
            def __init__(self, parent, prompt, title, default):
                assert parent is frame and prompt and title
                self.default = default
                self.destroyed = False
                dialogs.append(self)

            def ShowModal(self):
                return 1

            def GetValue(self):
                return next_text[0]

            def Destroy(self):
                self.destroyed = True

        fake_module("addonHandler", initTranslation=lambda: None)
        fake_module("config", conf={"audio": {"outputDevice": "default"}})
        fake_module("gui", mainFrame=frame)
        fake_module("globalPluginHandler", GlobalPlugin=BasePlugin)
        fake_module("logHandler", log=types.SimpleNamespace(error=lambda *a, **k: None,
                                                             warning=lambda *a, **k: None))
        fake_module("nvwave", WavePlayer=player_factory)
        fake_module("scriptHandler", script=lambda **kwargs: lambda func: func)
        fake_module("ui", message=messages.append)
        fake_module("wx", CallAfter=lambda fn, *args: fn(*args),
                    TextEntryDialog=Dialog, ID_OK=1)
        original_translate = getattr(builtins, "_", None)
        builtins._ = lambda value: value
        sys.path.insert(0, temporary)
        try:
            plugin_module = importlib.import_module("globalPlugins.reloqPreview")

            def engine_factory(path):
                assert Path(path).name == "eci.dll"
                engine = Engine(path)
                engines.append(engine)
                return engine

            plugin_module.DirectEciEngine = engine_factory
            plugin = plugin_module.GlobalPlugin()
            assert not players and not engines and not messages

            next_text = ["Pappe. Packen. Tappen."]
            # An ARM64EC NVDA process has 64-bit pointers but may report the
            # ARM64 host to IsWow64Process2. Never reject it on that basis:
            # the actual x64 DLL load in the engine is the deciding check.
            with (mock.patch.object(sys, "platform", "win32"),
                  mock.patch.object(ctypes, "sizeof", return_value=8),
                  mock.patch.object(ctypes, "WinDLL", create=True,
                                    side_effect=AssertionError("no architecture probe"))):
                assert plugin_module._is_64_bit_process()
                plugin.script_previewText(None)
                wait_until(lambda: players and players[0].blocks)
            assert engines[0].texts == ["Pappe. Packen. Tappen."]
            assert dialogs[0].default == "Pappe. Packen. Tappen." and dialogs[0].destroyed
            assert popup_calls == ["pre", "post"]
            assert not messages

            # Distinguish a missing written form from a missing acoustic
            # rule, and turn the C engine's UTF-8 byte offset into a spoken
            # character position. The ordinary NVDA voice stays selected.
            diagnostic = plugin_module.EciSynthesisError
            plugin._on_worker_error(diagnostic(-2, 2, 6, 7))
            assert "Wort oder Zeichen" in messages[-1]
            assert "Zeichen 7" in messages[-1]
            plugin._on_worker_error(diagnostic(-2, 3, 6, 7))
            assert "Klangmodell" in messages[-1]
            assert "Zeichen 7" in messages[-1]
            messages.clear()

            next_text[0] = "nicht abgedeckt"
            plugin.script_previewText(None)
            wait_until(lambda: bool(messages))
            assert "nicht abgedeckt" in messages[-1]
            assert len(players[0].blocks) == 1, "unsupported text produced PCM"
            assert dialogs[1].default == "Pappe. Packen. Tappen."

            next_text[0] = "Hass?"
            with mock.patch.object(ctypes, "sizeof", return_value=4):
                assert not plugin_module._is_64_bit_process()
                plugin.script_previewText(None)
            assert "64-Bit-Prozess" in messages[-1]
            assert len(players[0].blocks) == 1
            assert engines[0].texts == ["Pappe. Packen. Tappen.", "nicht abgedeckt"]

            plugin.script_cancelPreview(None)
            assert engines[0].cancelled and players[0].stopped
            assert "abgebrochen" in messages[-1]
            plugin.terminate()
            assert engines[0].closed and players[0].closed
            assert plugin.base_terminated

            def failed_engine(_path):
                error = OSError("not a valid Win32 application")
                error.winerror = 193
                raise error

            plugin_module.DirectEciEngine = failed_engine
            broken = plugin_module.GlobalPlugin()
            next_text[0] = "Pack."
            before_failure = len(messages)
            broken.script_previewText(None)
            wait_until(lambda: any("x64-DLL" in message
                                   for message in messages))
            assert len(messages) == before_failure + 1, messages
            assert "WinError 193" in messages[-1]
            assert "not a valid Win32 application" in messages[-1]
            assert not players[-1].blocks, "failed startup emitted PCM"

            # A later command retries once the failing DLL loader is fixed.
            failed_player = players[-1]
            plugin_module.DirectEciEngine = engine_factory
            broken.script_previewText(None)
            wait_until(lambda: players[-1] is not failed_player and players[-1].blocks)
            assert failed_player.closed and engines[-1].texts == ["Pack."]
            broken.terminate()
            assert players[-1].closed and broken.base_terminated
        finally:
            sys.path.remove(temporary)
            for key in tuple(sys.modules):
                if key.startswith("globalPlugins"):
                    del sys.modules[key]
            if original_translate is None:
                del builtins._
            else:
                builtins._ = original_translate


if __name__ == "__main__":
    test_package()
    print("NVDA preview package: x64 PE, ARM64EC gate, PCM/error/cancel/close OK")
