"""Packaged NVDA speech preprocessing -> real driver -> real ECI -> player.

NVDA's pinned speech/symbol code is executed, not imitated. Only UI services,
the audio device and the speech queue scheduler are replaced. PCM stays in
RAM. On Windows --library is packaged into the test add-on and loaded from
that extraction; Linux uses the same packaged Python code with a native .so.
"""
import argparse
import ctypes
import hashlib
import json
from pathlib import Path
import sys
import threading
import time
import zipfile
from unittest import mock

from nvda_speech_test_api import speech_api
from test_synth import main as contract_test


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--addon", type=Path, help="Test this exact existing package without rebuilding its translations")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    library = args.library.resolve()
    loader = ctypes.WinDLL if sys.platform == "win32" else ctypes.CDLL
    report = {"library": str(library), "checks": 0, "startup": []}
    if args.addon:
        with zipfile.ZipFile(args.addon) as package:
            report["source_commit"] = json.loads(package.read("doc/source.json"))["source_commit"]
        report["addon_sha256"] = hashlib.sha256(args.addon.read_bytes()).hexdigest()

    def inspect(module):
        native = sys.modules[module.__name__ + "._reloq_eci_direct"]
        commands = sys.modules["speech.commands"]
        done, first = threading.Event(), threading.Event()
        errors, players, engines, received, first_times = [], [], [], [], []
        spoken_engines = []
        submitted_times, native_times, native_pcm_times = [], [], []

        class NativeEngine(native.DirectEciEngine):
            def __init__(self, path, **kwargs):
                # WinDLL is patched by the external-service fixture. Use the
                # loader captured before that patch and the extracted DLL.
                actual = path if sys.platform == "win32" else library
                if sys.platform == "win32":
                    assert Path(actual).read_bytes() == library.read_bytes(), "packaged DLL differs from the requested test DLL"
                super().__init__(actual, _loader=loader, **kwargs)
                engines.append(self)
                self.calls = []

            def speak(self, parts, on_pcm, on_index):
                spoken_engines.append(self)
                native_times.append(time.perf_counter())
                have_pcm = False
                def pcm(data):
                    nonlocal have_pcm
                    if data and not have_pcm:
                        native_pcm_times.append(time.perf_counter())
                        have_pcm = True
                    on_pcm(data)
                settings = {str(p): self._dll.eciGetParam(self._handle, p)
                            for p in (native.SAMPLE_RATE, native.HIGH_RATE_PROFILE, native.PROCESS_TAGS)}
                self.calls.append((parts, settings))
                return super().speak(parts, pcm, on_index)

        class Player:
            def __init__(self, hz):
                self.hz, self.buffers, self.closed = hz, [], False
                players.append(self)

            def feed(self, pcm, onDone=None):
                assert not self.closed and len(pcm) % 2 == 0
                if not first.is_set():
                    first_times.append(time.perf_counter())
                    first.set()
                self.buffers.append(pcm)
                if onDone:
                    onDone()

            def idle(self): pass
            def pause(self, state): pass
            def stop(self): pass
            def close(self): self.closed = True

        class Driver(module.SynthDriver):
            @staticmethod
            def _make_player(hz): return Player(hz)
            def _on_done(self, job): done.set()
            def _on_error(self, error): errors.append(error); done.set()

        old_factory = module.DirectEciEngine
        module.DirectEciEngine = NativeEngine
        driver = Driver()
        with mock.patch.object(sys.modules["synthDriverHandler"], "getSynth", return_value=driver):
            api = speech_api(driver, lambda sequence: (submitted_times.append(time.perf_counter()),
                received.append(sequence), driver.speak(sequence)))

            def run(sequence, *, first_only=False, level=0):
                done.clear(); first.clear(); first_times.clear()
                submitted_times.clear(); native_times.clear(); native_pcm_times.clear()
                start = time.perf_counter()
                before = {id(p): len(p.buffers) for p in players}
                api.speak(sequence, symbolLevel=level)
                if first_only:
                    assert first.wait(20), (sequence[:1], errors)
                    elapsed = (first_times[0] - start) * 1000
                    timings = dict(first_audio_ms=round(elapsed, 3),
                        nvda_processing_ms=round((submitted_times[0]-start)*1000, 3),
                        worker_wait_ms=round((native_times[0]-submitted_times[0])*1000, 3),
                        native_first_pcm_ms=round((native_pcm_times[0]-native_times[0])*1000, 3))
                    driver.cancel()
                    return timings
                assert done.wait(30), errors
                if errors:
                    raise errors[-1]
                pcm = b"".join(block for player in players
                    for block in player.buffers[before.get(id(player), 0):])
                assert pcm, "no real PCM reached WavePlayer"
                return pcm

            try:
                driver.sampleRate = "3"
                driver.highRateProfile = "1"
                driver.voiceTags = True
                cases = {
                    "de_DE": ("Das ist eine ", "Nachricht", " mit einem Test."),
                    "en_US": ("This is a ", "message", " for the test."),
                    "es_ES": ("Esta es una ", "semana", " para recordar."),
                    "fr_FR": ("Ceci est une ", "maison", " pour le test."),
                }
                for locale, (prefix, word, suffix) in cases.items():
                    driver.language = locale
                    driver.voice = "1"
                    driver.pitch = 65
                    driver.rate = 50
                    driver.volume = 100
                    baseline = run([prefix + word + suffix])
                    # Demonstrate the actual upstream failure independently
                    # of our protection: a backtick never reaches the driver.
                    for level in (0, 200, 300):
                        assert "`" not in api.processText(locale, prefix + "`vv30" + word + suffix, level)
                        for tag in ("`vv30", "`vb80", "`vs40", "`vf10", "`vh60", "`vy20", "`vr10"):
                            changed = run([prefix + tag + word + suffix], level=level)
                            cut = len(word) // 2
                            interior = run([prefix + word[:cut] + tag + word[cut:] + suffix], level=level)
                            assert changed == interior, (locale, tag, "interior word split by NVDA")
                            assert changed != baseline, (locale, tag, "control lost")
                            report["checks"] += 2
                    # Real NVDA commands also survive without enabling ECI
                    # text tags, and must not leak into subsequent jobs.
                    driver.voiceTags = False
                    for command in (commands.RateCommand(multiplier=.6), commands.VolumeCommand(multiplier=.4)):
                        altered = run([prefix, command, word, type(command)(), suffix])
                        assert altered != baseline, (locale, repr(command), engines[-1].calls[-1], received[-1])
                        assert run([prefix + word + suffix]) == baseline, (locale, "prosody leaked")
                        report["checks"] += 2
                    driver.voiceTags = True
                # The source profile and player clock must agree in the
                # actual packaged driver for every native output rate.
                for code, hz in native.SAMPLE_RATES.items():
                    driver.sampleRate = str(code)
                    run(["This is the sample rate test."])
                    settings = spoken_engines[-1].calls[-1][1]
                    assert settings[str(native.SAMPLE_RATE)] == code and players[-1].hz == hz
                    assert settings[str(native.HIGH_RATE_PROFILE)] == 1
                    assert settings[str(native.PROCESS_TAGS)] == 1
                    report["checks"] += 3
                # Verify the selectable C profile through the packaged NVDA
                # driver, real ECI DLL and player. B remains the foundation.
                driver.language = "de_DE"
                driver.voice = "1"
                driver.voiceTags = False
                for code, hz in ((3, 16000), (2, 22050), (6, 48000)):
                    driver.sampleRate = str(code)
                    driver.highRateProfile = "1"
                    baseline = run(["Oma und Werner hören Musik."])
                    driver.highRateProfile = "3"
                    gentle = run(["Oma und Werner hören Musik."])
                    settings = spoken_engines[-1].calls[-1][1]
                    assert settings[str(native.HIGH_RATE_PROFILE)] == 3
                    assert settings[str(native.SAMPLE_RATE)] == code and players[-1].hz == hz
                    assert len(baseline) == len(gentle)
                    assert (baseline == gentle) == (hz == 16000)
                    report["checks"] += 4
                driver.highRateProfile = "1"
                driver.voiceTags = True
                driver.sampleRate = "3"
                for locale, sentence in (
                    ("de_DE", "Die Stimme liest diesen langen Text und bleibt dabei gut verständlich"),
                    ("en_US", "The voice reads this long text and remains easy to understand"),
                ):
                    driver.language = locale
                    driver.voice = "2"
                    driver.pitch = 50
                    run([sentence + "."])
                    for style in ("sentences", "single"):
                        text = ((sentence + (". " if style == "sentences" else " ")) * 128).rstrip() + "."
                        timings = run([text], first_only=True)
                        report["startup"].append(dict(language=locale, style=style,
                            characters=len(text), **timings))
                        # Wait for cancellation and verify the next job works.
                        run([sentence + "."])
                assert not errors
            finally:
                driver.terminate()
                module.DirectEciEngine = old_factory
        print("Packaged driver/native integration:", json.dumps(report, ensure_ascii=False))

    previous_argv = sys.argv
    sys.argv = [sys.argv[0]]
    try:
        contract_test(inspect, dll_path=library if sys.platform == "win32" else None,
            inspect_only=True, package_path=args.addon.resolve() if args.addon else None)
    finally:
        sys.argv = previous_argv
    if args.output:
        args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
