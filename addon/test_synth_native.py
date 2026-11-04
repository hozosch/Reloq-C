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
import tempfile
import threading
import time
import traceback
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
                            for p in (native.SAMPLE_RATE, native.HIGH_RATE_PROFILE, native.PROCESS_TAGS,
                                      native.GERMAN_GLIDE_OPENING, native.GERMAN_GLIDE_BACKNESS,
                                      native.GERMAN_GLIDE_SOFTNESS)}
                self.calls.append((parts, settings))
                try:
                    return super().speak(parts, pcm, on_index)
                except BaseException as error:
                    # An interrupted job can be stale by the time its error
                    # reaches NVDA. Keep the originating exception visible
                    # when a native call leaves the handle busy.
                    if self._native_busy_diagnostic() == '1':
                        print('ORIGINAL NATIVE BUSY FAILURE:', repr(error), flush=True)
                        traceback.print_exc()
                    raise

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
                # The accepted fixed glide survives switching languages;
                # old NVDA slider settings no longer reach the DLL.
                driver.language = "de_DE"
                driver.voice = "1"
                glide_reference = run(["Homepage"])
                settings = spoken_engines[-1].calls[-1][1]
                assert [settings[str(p)] for p in (native.GERMAN_GLIDE_OPENING,
                        native.GERMAN_GLIDE_BACKNESS, native.GERMAN_GLIDE_SOFTNESS)] == [85, 45, 45]
                driver.language = "en_US"
                run(["Hello."])
                driver.language = "de_DE"
                assert run(["Homepage"]) == glide_reference, "language switch changed the fixed glide"
                report["checks"] += 2
                # The source profile and player clock must agree in the
                # actual packaged driver for every native output rate.
                for code, hz in native.SAMPLE_RATES.items():
                    driver.sampleRate = str(code)
                    run(["This is the sample rate test."])
                    settings = spoken_engines[-1].calls[-1][1]
                    assert settings[str(native.SAMPLE_RATE)] == code and players[-1].hz == hz
                    assert settings[str(native.HIGH_RATE_PROFILE)] == 3
                    assert settings[str(native.PROCESS_TAGS)] == 1
                    report["checks"] += 3
                # The accepted tuning is fixed, independent of old stored
                # choices. Check repeatable output at three actual rates.
                driver.language = "de_DE"
                driver.voice = "1"
                driver.voiceTags = False
                for code, hz in ((3, 16000), (2, 22050), (6, 48000)):
                    driver.sampleRate = str(code)
                    baseline = run(["Oma und Werner hören Musik."])
                    gentle = run(["Oma und Werner hören Musik."])
                    settings = spoken_engines[-1].calls[-1][1]
                    assert settings[str(native.HIGH_RATE_PROFILE)] == 3
                    assert settings[str(native.SAMPLE_RATE)] == code and players[-1].hz == hz
                    assert baseline == gentle
                    report["checks"] += 4
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
                # Issue #10: rapid navigation through long filenames used
                # to leave the native input queue unusable until NVDA was
                # restarted. Exercise the packaged driver's real queue and
                # speech preprocessing, then require normal speech on the
                # same worker without restarting it.
                filename = ("20171111 - Can_A_Real_Meal_Easy_Bake_Oven_Cook_Raw_Meat_"
                            "Macaroni_Cheese_-_Sloppy_Joe_Dinner.vtt  Extension: vtt, "
                            "Size: 22.0 kB, Attributes: ---A--------, "
                            "Modified: 11/2/2026 6:20:58 PM")
                names = (filename,
                    "20261102 - Ein_verschachtelter_Dateiname_mit_ä_ö_ü_und_Zahlen_"
                    "1234567890_über_mehrere_Wörter_mit_Bindestrich-und_Unterstrich.txt  "
                    "Extension: txt, Size: 22.0 kB, Attributes: ---A--------, Modified: 11/2/2026 6:20:58 PM",
                    "20261031 - Different_long_video_title_part_one_part_two_and_part_three_"
                    "followed_by_more_words_with_a_similar_name_but_a_different_extension.srt  "
                    "Extension: srt, Size: 22.0 kB, Attributes: ---A--------, Modified: 11/2/2026 6:20:58 PM")
                driver.voiceTags = False
                for repetition in range(4):
                    for rate in ("1", "6"):
                        driver.sampleRate = rate
                        for locale in ("de_DE", "en_US"):
                            driver.language = locale
                            recovery = "The following item is readable."
                            reference = run([recovery])
                            for number in range(80):
                                first.clear()
                                # Alternate entries like navigating down and back up.
                                row = (number // 2 + number % 2) % len(names)
                                api.speak([names[row] + " " + str(number)])
                                if number % 11 == 0:
                                    assert first.wait(20), (repetition, rate, locale, number, errors)
                                else:
                                    time.sleep((number % 4) * .004)
                                driver.cancel()
                                if number % 20 == 19:
                                    assert run([recovery]) == reference, (repetition, rate, locale, number, 'recovery changed')
                                    assert not errors, (repetition, rate, locale, errors)
                            report["checks"] += 84
                # Use the actual settings manager, queue and NVDA symbol
                # processor with a user-selected Root file. A full hyphenated
                # key must survive normal symbol levels and both settings of
                # the independent abbreviation dictionary.
                driver.language = "de_DE"
                driver.voice = "1"
                driver.voiceTags = False
                source_class = sys.modules[module.__name__ + "._reloq_dictionary_manager"].DictionarySource
                with tempfile.TemporaryDirectory() as folder:
                    own = Path(folder) / "mein-woerterbuch.dic"
                    own.write_bytes(b'eti-eloquence\t`[.1kIr.0S@]\n')
                    ready, completed = threading.Event(), []
                    driver.configure_dictionaries([source_class(str(own), 0x40000, 1)], False,
                        lambda error, details: (completed.append((error, details)), ready.set()))
                    assert ready.wait(20) and completed and completed[0][0] is None, completed
                    for enabled in (True, False):
                        driver.abbreviationDictionary = enabled
                        for level in (0, 200):
                            for source, target in (("Das ist ETI-Eloquence.", "Das ist Kirsche."),
                                                   ("„ETI-Eloquence“", "„Kirsche“")):
                                assert run([source], level=level) == run([target], level=level), (enabled, level, source)
                                report["checks"] += 1
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
