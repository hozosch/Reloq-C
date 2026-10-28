"""Package and asynchronous NVDA driver checks without a Windows NVDA host."""

from __future__ import annotations

import builtins
import ctypes
import gettext
from dataclasses import replace
import importlib
from pathlib import Path
import sys
import tempfile
import threading
import time
import types
from unittest import mock
import zipfile

from build_preview import assert_x64_pe
from build_synth import build, lexicon_rows, merged_german_lexicon
from nvda_test_api import (
    configuration_manager, install as install_nvda_contract, load as load_nvda_api,
)


ROOT = Path(__file__).resolve().parents[1]


def fake_module(name, **attributes):
    result = types.ModuleType(name)
    vars(result).update(attributes)
    sys.modules[name] = result
    return result


def until(predicate, seconds=2):
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        if predicate():
            return
        time.sleep(.002)
    raise AssertionError("NVDA driver worker did not finish")


class Player:
    def __init__(self, **kwargs):
        assert kwargs == dict(channels=1, samplesPerSec=kwargs["samplesPerSec"],
                              bitsPerSample=16, outputDevice="default")
        assert kwargs["samplesPerSec"] in (8000,11025,16000,22050,32000,44100,48000)
        self.sample_rate = kwargs["samplesPerSec"]
        self.buffers = []
        self.callbacks = []
        self.stops = 0
        self.closed = False
        self.paused = False
        self.cv = threading.Condition()

    def feed(self, pcm, onDone=None):
        with self.cv:
            self.buffers.append(pcm)
            self.callbacks.append(onDone)
            self.cv.notify_all()

    def play_one(self):
        with self.cv:
            callback = self.callbacks.pop(0)
            self.cv.notify_all()
        callback()

    def idle(self):
        with self.cv:
            assert self.cv.wait_for(lambda: not self.callbacks, 2)

    def stop(self):
        with self.cv:
            self.stops += 1
            self.callbacks.clear()
            self.cv.notify_all()

    def pause(self, switch):
        self.paused = switch

    def close(self):
        self.closed = True


class Engine:
    instances = []

    def __init__(self, path, *, experimental_generic=False, lexicon_path=None,
                 dialect=0x40000, encoding="cp1252"):
        assert Path(path).name == "eci.dll"
        assert experimental_generic
        assert dialect in (0x40000, 0x10000, 0x10001, 0x20000, 0x20001, 0x30000, 0x30001, 0x50000, 0x70000, 0x90000, 0x60000, 0x60001, 0x80000, 0xa0000, 0x120000) and encoding == "utf-8"
        assert lexicon_path is None or Path(lexicon_path).name == "german_apple_observed.rlex"
        self.dialect = dialect
        self.parts = []
        self.closed = False
        self.cancelled = False
        self.parameters = []
        self.dictionary_paths = []
        self.__class__.instances.append(self)

    def speak(self, parts, on_pcm, on_index):
        self.parts.append(tuple(parts))
        for part in parts:
            if type(part).__name__ == "EciText" and part.text:
                on_pcm(part.text.encode("utf8"))
            elif type(part).__name__ == "EciIndex":
                on_index(part.value)
        return True

    def cancel(self):
        self.cancelled = True

    def set_dictionary_path(self, path):
        self.dictionary_paths.append(path)

    def set_parameter(self, parameter, value):
        self.parameters.append((parameter, value))

    def close(self):
        self.closed = True


def main(inspect_adapter=None):
    original = (ROOT / "data/german_apple_observed.rlex").read_bytes()
    supplement = (ROOT / "data/german_apple_hunspell_context.rlex").read_bytes()
    original_rows = dict(lexicon_rows(original))
    supplement_rows = dict(lexicon_rows(supplement))
    dictionary_rows = dict(lexicon_rows(
        (ROOT / "data/german_apple_dictionary.rlex").read_bytes()))
    verb_rows = dict(lexicon_rows(
        (ROOT / "data/german_apple_verb_paradigms.rlex").read_bytes()))
    assert len(original_rows) == 1598
    assert len(supplement_rows) == 1337
    assert len(dictionary_rows) == 13379
    assert len(verb_rows) == 184
    assert not verb_rows.keys() & (original_rows.keys() | supplement_rows.keys() | dictionary_rows.keys())
    assert not original_rows.keys() & supplement_rows.keys()
    assert not dictionary_rows.keys() & (original_rows.keys() | supplement_rows.keys())
    merged = dict(lexicon_rows(merged_german_lexicon()))
    root_rows = dict(lexicon_rows((ROOT / "data/german_apple_roots82.rlex").read_bytes()))
    assert len(root_rows) == 2
    assert all(merged[key] == line for key, line in root_rows.items())
    assert len(merged) == len(root_rows) + len(original_rows) + len(supplement_rows) + len(dictionary_rows) + len(verb_rows)
    assert all(merged[key] == line for key, line in original_rows.items())
    assert all(merged[key] == line for key, line in supplement_rows.items())
    assert all(merged[key] == line for key, line in dictionary_rows.items())
    assert all(merged[key] == line for key, line in verb_rows.items())
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory)
        package = path / "synth.nvda-addon"
        build(package, ROOT / "eci" / "eci.dll", refresh_dictionaries=False)
        # A normal build re-resolves master even when all verified Git blobs
        # already match. Reuse avoids redundant downloads, never the head check.
        import build_synth as builder
        from test_dictionaries import Server
        from _reloq_dictionaries import fetch_master, validate_snapshot
        server = Server()
        server.commit = validate_snapshot(ROOT / "data/community")["commit"]
        with mock.patch.object(builder, "fetch_master", side_effect=lambda cache: fetch_master(cache, fetch=server)):
            build(package, ROOT / "eci" / "eci.dll")
        assert len(server.calls) == 1 and server.calls[0].endswith("/branches/master")
        with zipfile.ZipFile(package) as archive:
            names = set(archive.namelist())
            assert "synthDrivers/reloqC/__init__.py" in names
            assert "synthDrivers/reloqC/eci.dll" in names
            assert "synthDrivers/reloqC/german_apple_observed.rlex" in names
            assert "synthDrivers/reloqC/_reloq_unicode.py" not in names
            assert "doc/licenses/Unicode-LICENSE.txt" in names
            licenses={"doc/licenses/OpenEVV-LICENSE.txt","doc/licenses/OpenEVV-NOTICE.txt"}
            assert licenses<=set(names)
            assert not any("openevv" in name.lower() for name in names if name not in licenses)
            assert "locale/de/LC_MESSAGES/nvda.mo" in names
            assert "locale/de/manifest.ini" in names
            assert b"fifteen language variants" in archive.read("manifest.ini")
            assert "Japanisch" in archive.read("locale/de/manifest.ini").decode("utf-8")
            assert "Griechisch" in archive.read("locale/de/manifest.ini").decode("utf-8")
            assert "globalPlugins/reloqCDictionaries/__init__.py" in names
            assert not any("__pycache__" in name or name.endswith(".pyc")
                           for name in names)
            assert_x64_pe(archive.read("synthDrivers/reloqC/eci.dll"))
            assert archive.read(
                "synthDrivers/reloqC/german_apple_observed.rlex"
            ) == merged_german_lexicon()
            archive.extractall(path)

        ui_language = sys.argv[1] if len(sys.argv) > 1 else "en"
        translation = gettext.translation("nvda", path / "locale", languages=[ui_language], fallback=True)

        def init_translation():
            # NVDA injects the add-on catalog into the CALLER's globals, not
            # into builtins. Exercise that boundary with the packaged MO file.
            sys._getframe(1).f_globals["_"] = translation.gettext

        fake_module("addonHandler", initTranslation=init_translation)

        class IndexCommand:
            def __init__(self, index):
                self.index = index

        class BreakCommand:
            def __init__(self, time):
                self.time = time

        class PhonemeCommand:
            def __init__(self, text):
                self.text = text

        class CharacterModeCommand:
            def __init__(self, state):
                self.state = state

        class PitchCommand:
            def __init__(self, multiplier=1.0):
                self.multiplier = multiplier

        class LangChangeCommand:
            def __init__(self, lang):
                self.lang = lang

        notifications = []
        players = []
        fake_module("autoSettingsUtils", __path__=[])
        fake_module("autoSettingsUtils.driverSetting",
                    DriverSetting=lambda name, label, **kwargs: (name, label),
                    BooleanDriverSetting=lambda name, label, **kwargs: (name, label))
        fake_module("autoSettingsUtils.utils",
                    StringParameterInfo=lambda id, label: types.SimpleNamespace(id=id, displayName=label))
        fake_module("config", conf={"audio": {"outputDevice": "default"}})
        fake_module("logHandler", log=types.SimpleNamespace(
            error=lambda *args, **kw: notifications.append(("error", args)),
            warning=lambda *args, **kw: notifications.append(("warning", args))))
        fake_module("nvwave", WavePlayer=lambda **kw: players.append(Player(**kw)) or players[-1])
        fake_module("speech", __path__=[])
        fake_module("speech.commands", IndexCommand=IndexCommand,
                    BreakCommand=BreakCommand, PhonemeCommand=PhonemeCommand,
                    CharacterModeCommand=CharacterModeCommand,
                    PitchCommand=PitchCommand, LangChangeCommand=LangChangeCommand)
        class Action:
            def __init__(self, name):
                self.name = name

            def notify(self, **kw):
                notifications.append((self.name, kw))

        fake_module("synthDriverHandler",
                    SynthDriver=type("BaseSynth", (), {
                        "VoiceSetting": classmethod(lambda cls: ("voice",)),
                        "RateSetting": classmethod(lambda cls: ("rate",)),
                        "RateBoostSetting": classmethod(lambda cls: ("rateBoost",)),
                        "PitchSetting": classmethod(lambda cls: ("pitch",)),
                        "InflectionSetting": classmethod(lambda cls: ("inflection",)),
                        "VolumeSetting": classmethod(lambda cls: ("volume",)),
                    }),
                    VoiceInfo=lambda *args: args,
                    synthDoneSpeaking=Action("done"),
                    synthIndexReached=Action("index"))
        original_translate = getattr(builtins, "_", None)
        builtins._ = lambda value: value
        real_settings, SettingsPanel = install_nvda_contract()
        active_driver = []
        sys.modules["synthDriverHandler"].getSynth = lambda: active_driver[0]
        commands = load_nvda_api("speech.commands", "speech/commands.py")
        IndexCommand, BreakCommand = commands.IndexCommand, commands.BreakCommand
        PhonemeCommand, PitchCommand = commands.PhonemeCommand, commands.PitchCommand
        CharacterModeCommand, LangChangeCommand = commands.CharacterModeCommand, commands.LangChangeCommand
        sys.path.insert(0, str(path))
        try:
            with (mock.patch.object(ctypes, "sizeof", return_value=8),
                  mock.patch.object(ctypes, "WinDLL", create=True,
                                    return_value=object())):
                driver_module = importlib.import_module("synthDrivers.reloqC")
                driver_module.DirectEciEngine = Engine
                assert driver_module.SynthDriver.check()
                with mock.patch.object(ctypes, "sizeof", return_value=4):
                    assert not driver_module.SynthDriver.check()

                fake_module("globalVars", appArgs=types.SimpleNamespace(configPath=str(path / "profile")))
                driver = driver_module.SynthDriver()
                active_driver.append(driver)
                assert driver.name == "reloqC"
                assert driver._get_language() == "de_DE"
                assert driver.voice == "1"
                assert list(driver.availableVoices) == list(map(str, range(1, 9)))
                german_ui = ui_language.replace("-", "_").split("_", 1)[0] == "de"
                assert driver.description == ("Eloquence C (experimentell)" if german_ui else "Eloquence C (experimental)")
                assert driver.availableLanguages["de_DE"].displayName == "German"
                assert isinstance(driver.availableLanguages["de_DE"], sys.modules["synthDriverHandler"].LanguageInfo)
                assert [v.displayName for v in driver.availableVoices.values()] == [
                    "Reed", "Shelly", "Bobby", "Rocko", "Glen", "Sandy", "Grandma", "Grandpa"]
                assert driver.availablePausemodes["1"].displayName == ("Nur am Textende" if german_ui else "Only at text end")
                assert driver.availableSamplerates["1"].displayName == ("11,025 kHz" if german_ui else "11.025 kHz")
                head_setting = next(item for item in driver.supportedSettings if item.id == "headSize")
                assert head_setting.displayNameWithAccelerator == ("Kopf&größe" if german_ui else "&Head size")
                driver.voice = "deu_experimental"
                assert driver.voice == "1", "old voice IDs must migrate"
                driver.voice = "262144:1"
                assert driver.voice == "1"
                assert PitchCommand in driver.supportedCommands
                assert CharacterModeCommand in driver.supportedCommands
                assert [setting.id for setting in driver.supportedSettings] == [
                    "language", "voice", "rate", "rateBoost", "pitch", "inflection", "volume", "headSize", "roughness", "breathiness", "voiceTags", "_dictionaryCommunity", "pauseMode", "sampleRate"]
                assert driver.supportedSettings[0].availableInSettingsRing
                driver.initSettings()
                panel = SettingsPanel(driver)
                for setting in driver.supportedSettings:
                    value = getattr(driver, setting.id)
                    if type(setting) is real_settings.DriverSetting:
                        control = panel.make(setting, driver)
                        assert control.selection is not None, setting.id
                        choices = getattr(driver, f"available{setting.id.capitalize()}s")
                        assert value in choices and all(v.displayName for v in choices.values())
                # The optional setting applies a bundled native dictionary set
                # immediately; failed network refreshes cannot stall speech.
                from urllib.error import URLError
                fake_module("globalVars", appArgs=types.SimpleNamespace(configPath=str(path / "profile")))
                real_updater = driver_module.CommunityDictionaries
                def offline(*args, **kwargs):
                    raise URLError("offline test")
                def updater(*args, **kwargs):
                    return real_updater(*args, **kwargs, fetch=offline)
                driver._dictionary_updater.close()
                driver._dictionary_updater = None
                with mock.patch.object(driver_module, "CommunityDictionaries", side_effect=updater):
                    driver.communityDictionaries = True
                    until(lambda: any(instance.dictionary_paths and instance.dictionary_paths[-1] is not None for instance in Engine.instances))
                    worker = next(instance for instance in Engine.instances if instance.dictionary_paths and instance.dictionary_paths[-1] is not None)
                    assert Path(worker.dictionary_paths[-1]).name == "communityDictionaries"
                    until(lambda: any(item[0] == "warning" and item[1][0].startswith("Community dictionary refresh failed") for item in notifications))
                    notifications[:] = [item for item in notifications if not (
                        item[0] == "warning" and item[1][0].startswith("Community dictionary refresh failed"))]
                    driver.cancel()
                    driver.communityDictionaries = False
                    until(lambda: worker.dictionary_paths[-1] is None)
                assert driver.communityDictionaries is False
                # The same real NVDA GUI method rejects the shipped 0.2.54 getter.
                legacy_namespace = {"__file__": driver_module.__file__,
                    "__name__": "synthDrivers.reloqC.legacy", "__package__": "synthDrivers.reloqC"}
                # This historical GUI fixture imports the old normalizer;
                # load it only for the fixture, never into the new package.
                legacy_name = "synthDrivers.reloqC._reloq_unicode"
                spec = importlib.util.spec_from_file_location(legacy_name, ROOT / "nvda/_reloq_unicode.py")
                legacy_module = importlib.util.module_from_spec(spec)
                sys.modules[legacy_name] = legacy_module
                spec.loader.exec_module(legacy_module)
                exec((ROOT / "addon/testdata/reloq-0.2.54-driver.py").read_text(), legacy_namespace)
                legacy = legacy_namespace["SynthDriver"].__new__(legacy_namespace["SynthDriver"])
                legacy._pauseMode = "0"
                setting = next(s for s in driver.supportedSettings if s.id == "pauseMode")
                try:
                    SettingsPanel(legacy).make(setting, legacy)
                except AttributeError as error:
                    assert "availablePausemodes" in str(error)
                else:
                    raise AssertionError("test did not reproduce the reported 0.2.54 failure")
                assert set(driver.availableLanguages) == {"de_DE", "en_US", "en_GB", "es_ES", "es_MX", "fr_FR", "fr_CA", "it_IT", "pt_BR", "fi_FI", "ja_JP", "ko_KR", "zh_CN", "el_GR"}
                assert list(driver.availableLanguages) == ["zh_CN", "de_DE", "en_GB", "en_US", "es_MX", "es_ES", "fi_FI", "fr_FR", "fr_CA", "it_IT", "ja_JP", "ko_KR", "pt_BR", "el_GR"]
                assert driver_module.language_for("en", 0x40000) == 0x10000
                assert driver_module.language_for("zh-TW", 0x40000) == 0x60000
                assert driver_module.language_for("zh-HK", 0x40000) == 0x60000
                assert driver_module.language_for("zh-Hans", 0x40000) == 0x60000
                assert driver_module.language_for("zh-Hant", 0x40000) == 0x60000
                assert driver.languageIsSupported("de-AT")
                assert driver.languageIsSupported("en-US")
                assert driver.languageIsSupported("pt-BR")
                assert driver.languageIsSupported("fi-FI")
                assert driver.availableLanguages["pt_BR"].displayName == "Brazilian Portuguese"
                assert driver.availableLanguages["fi_FI"].displayName == "Finnish"
                assert LangChangeCommand in driver.supportedCommands
                assert driver_module._parts(["Ein", LangChangeCommand("de-AT"),
                    " Satz", LangChangeCommand("hu-HU"), " bleibt", LangChangeCommand(None),
                    " zusammen."]) == [driver_module.EciText("Ein Satz bleibt zusammen.")]
                assert driver.availableLanguages["en_US"].displayName == "American English"
                assert driver_module._parts(["Hello", LangChangeCommand("en-US"), "world",
                    LangChangeCommand(None), "Hallo"]) == [driver_module.EciText("Hello"), driver_module.EciLanguage(65536,"1"),
                    driver_module.EciText("world"), driver_module.EciLanguage(262144,"1"), driver_module.EciText("Hallo")]
                german = driver_module.GERMAN
                english = replace(german, id=0x10000, locale="en_GB", name="Test only")
                catalog = {german.id: german, english.id: english}
                assert driver_module._parts(["Deutsch.", LangChangeCommand("en-US"),
                    "English.", LangChangeCommand(None), "Wieder deutsch."], languages=catalog) == [
                        driver_module.EciText("Deutsch."), driver_module.EciLanguage(0x10000, "1"), driver_module.EciText("English."),
                        driver_module.EciLanguage(0x40000, "1"), driver_module.EciText("Wieder deutsch.")]
                until(lambda: len(Engine.instances) == 1)
                until(lambda: len(Engine.instances[0].parameters) == 14)
                assert Engine.instances[0].parameters == list(german.voices[0].parameters) + [(0x1007, 98), (0x1008, 50),
                                                          (0x1009, 30),
                                                          (0x100a, 100), (0x100d, 50), (0x100e, 0), (0x100f, 0)]
                driver._set_rate(70)
                driver._set_pitch(65)
                driver._set_inflection(75)
                driver._set_volume(85)

                class IgnoredCommand:
                    pass

                driver.speak(["Packte ", IndexCommand(12), IgnoredCommand(),
                              "Kippte.", IndexCommand(13)])
                until(lambda: len(players[0].buffers) == 2)
                worker = Engine.instances[0]
                assert worker.parameters == list(german.voices[0].parameters) + [(0x1007, 98), (0x1008, 50),
                                             (0x1009, 30), (0x100a, 100),
                                             (0x100d, 50), (0x100e, 0), (0x100f, 0),
                                             (0x1007, 121), (0x1008, 65),
                                             (0x1009, 75),
                                             (0x100a, 85)]
                assert driver._get_rate() == 70
                assert driver._get_pitch() == 65
                assert driver._get_inflection() == 75
                assert [tuple(type(x).__name__ for x in items)
                        for items in worker.parts] == [
                            ("EciText", "EciIndex", "EciText", "EciIndex")]
                assert [item.value for item in worker.parts[0]
                        if isinstance(item, driver_module.EciIndex)] == [12, 13]
                assert not notifications
                players[0].play_one()
                assert [(kind, data.get("index")) for kind, data in notifications] == [
                    ("index", 12)]
                players[0].play_one()
                until(lambda: notifications and notifications[-1][0] == "done")
                assert [(kind, data.get("index")) for kind, data in notifications] == [
                    ("index", 12), ("index", 13), ("done", None)]

                paragraph = "Dies ist ein längerer Absatz mit mehreren Wörtern."
                driver.speak([paragraph])
                until(lambda: len(worker.parts) == 2 and len(players[0].buffers) == 3)
                assert worker.parts[1] == (driver_module.EciText(paragraph),), "paragraph was fragmented"
                players[0].play_one()
                until(lambda: notifications and notifications[-1][0] == "done")
                assert notifications[-1][0] == "done"
                assert driver_module._parts(["zwei", BreakCommand(200),
                                             "Wörter", PhonemeCommand("", text=" Ende")]) == [
                                                 driver_module.EciText("zwei Wörter Ende")]
                assert driver_module._parts(["Mäuse 😊漢字.", IndexCommand(99)]) == [
                    driver_module.EciText("Mäuse 😊漢字."), driver_module.EciIndex(99)]
                assert driver_module._parts(["`[.1bEt] im Code."]) == [
                    driver_module.EciText("`[.1bEt] im Code.")]
                assert driver_module._parts([IndexCommand(1), IndexCommand(2),
                                             "Text", IndexCommand(3)]) == [
                                                 driver_module.EciIndex(1),
                                                 driver_module.EciIndex(2),
                                                 driver_module.EciText("Text"), driver_module.EciIndex(3)]
                assert driver_module._parts([
                    PitchCommand(multiplier=1.4), CharacterModeCommand(True), "A",
                    CharacterModeCommand(False), PitchCommand(), "b",
                    IndexCommand(4)], base_pitch=65) == [
                        driver_module.EciPitch(91), driver_module.EciText("A", True),
                        driver_module.EciPitch(65), driver_module.EciText(" b"),
                        driver_module.EciIndex(4)]
                assert driver_module._parts([
                    CharacterModeCommand(True), "ABC123", "e\u0308ß",
                    CharacterModeCommand(False), "weiter"]) == [
                        driver_module.EciText("ABC123", True), driver_module.EciText(" "), driver_module.EciText("e\u0308ß", True), driver_module.EciText(" weiter")]
                assert driver_module._parts([
                    "Text", CharacterModeCommand(True), "A\u0308C",
                    IndexCommand(8), "FN", CharacterModeCommand(False),
                    "Ende"]) == [driver_module.EciText("Text "), driver_module.EciText("A\u0308C", True), driver_module.EciIndex(8),
                                driver_module.EciText(" "), driver_module.EciText("FN", True), driver_module.EciText(" Ende")]
                assert driver_module._parts([
                    CharacterModeCommand(True), "CD", PitchCommand(multiplier=1.4),
                    "LM", PitchCommand(), CharacterModeCommand(False),
                    " normal"]) == [driver_module.EciText("CD", True), driver_module.EciPitch(70),
                                   driver_module.EciText(" "), driver_module.EciText("LM", True), driver_module.EciPitch(50), driver_module.EciText(" normal")]
                assert driver_module._parts([PitchCommand(offset=30), "A"], base_pitch=0) == [
                    driver_module.EciPitch(30), driver_module.EciText("A")]
                driver.speak([PitchCommand(multiplier=1.4), CharacterModeCommand(True),
                              "A", CharacterModeCommand(False),
                              PitchCommand(), "b", IndexCommand(4)])
                until(lambda: len(worker.parts) == 3 and
                      len(players[0].buffers) == 4)
                assert worker.parts[2] == (
                    driver_module.EciPitch(91), driver_module.EciText("A", True),
                    driver_module.EciPitch(65), driver_module.EciText(" b"),
                    driver_module.EciIndex(4))
                players[0].play_one()
                until(lambda: notifications and notifications[-1][0] == "done")
                assert notifications[-2][1]["index"] == 4
                driver.speak(["normal", IndexCommand(5)])
                until(lambda: len(worker.parts) == 4 and
                      len(players[0].buffers) == 5)
                assert worker.parts[3] == (driver_module.EciText("normal"), driver_module.EciIndex(5)), (
                    "temporary spelling pitch escaped into later speech")
                players[0].play_one()
                until(lambda: notifications and notifications[-1][0] == "done")
                driver.pause(True)
                assert players[0].paused
                driver.cancel()
                assert players[0].stops and worker.cancelled
                driver._set_rate(60)
                until(lambda: worker.parameters[-1:] == [(0x1007, 110)])
                driver._set_rateBoost(True)
                until(lambda: worker.parameters[-1:] == [(0x1007, 176)])
                assert driver._get_rate() == 60 and driver._get_rateBoost()
                driver._set_rateBoost(False)
                until(lambda: worker.parameters[-1:] == [(0x1007, 110)])
                assert not driver._get_rateBoost()
                entered, release = threading.Event(), threading.Event()
                assert driver_module._parts(["Hallo `vb80 Welt."], voice_tags=True) == [
                    driver_module.EciText("Hallo `vb80 Welt.")]
                assert driver_module._parts([CharacterModeCommand(True), "\uf6ecЖΩ`"],
                    voice_tags=True) == [driver_module.EciText("\uf6ecЖΩ`", True)]
                assert driver_module._parts(["Hallo `vb80 Welt."])[0].text == "Hallo `vb80 Welt."
                driver._set_voiceTags(True)
                until(lambda: Engine.instances[0].parameters[-1] == (0x100b, 1))
                assert driver._get_voiceTags() is True
                driver._set_voiceTags(False)
                until(lambda: Engine.instances[0].parameters[-1] == (0x100b, 0))
                assert list(driver.availablePausemodes) == ["0", "1", "2"]
                assert list(driver.availablePauseModes) == ["0", "1", "2"]
                assert driver.pauseMode == "0"
                for mode in ("1", "2", "0"):
                    driver.pauseMode = mode
                    until(lambda: Engine.instances[0].parameters[-1] == (0x100c, int(mode)))
                    assert driver.pauseMode == mode
                # Existing profile from before these settings were added.
                speech_config = sys.modules["config"].conf["speech"]
                assert type(speech_config).__name__ == "AggregatedSection"
                assert not hasattr(speech_config, "pop")
                assert not hasattr(type(speech_config), "__delitem__")
                speech_config[driver.name] = {"voice": "deu_experimental", "rate": 60,
                    "rateBoost": False, "pitch": 65, "inflection": 75, "volume": 85}
                driver.initSettings()
                assert driver.language == "de_DE" and driver.voice == "1"
                assert driver.pauseMode == "0" and driver.voiceTags is False
                driver.saveSettings()
                assert speech_config[driver.name]["voice"] == "1"
                assert "engineLanguage" not in speech_config[driver.name]
                driver.pauseMode = "2"
                driver.voiceTags = True
                driver.saveSettings()
                driver.pauseMode = "0"
                driver.voiceTags = False
                driver.loadSettings(onlyChanged=True)
                assert driver.pauseMode == "2" and driver.voiceTags is True
                until(lambda: worker.parameters[-2:] == [(0x100b, 1), (0x100c, 2)])
                driver.pauseMode = None
                assert driver.pauseMode == "0"
                driver.language = "unsupported"
                assert driver.language == "de_DE"
                normal_parts = driver_module._parts

                def delayed_parts(sequence, **kwargs):
                    entered.set()
                    assert release.wait(2)
                    return normal_parts(sequence, **kwargs)

                driver_module._parts = delayed_parts
                stale = threading.Thread(target=driver.speak, args=(["stale text"],))
                stale.start()
                assert entered.wait(2)
                driver.cancel()
                release.set()
                stale.join(2)
                assert not stale.is_alive() and len(worker.parts) == 4, (
                    "a cancelled old speech sequence re-entered the queue")
                driver_module._parts = normal_parts
                driver.voice = "2"
                assert driver.headSize == 50 and driver.breathiness == 50
                assert driver.pitch == 81 and driver.inflection == 30
                until(lambda: len(Engine.instances) >= 2)
                female_worker = Engine.instances[-1]
                until(lambda: dict(female_worker.parameters).get(0x1010) == 1)
                driver.headSize = 22
                driver.roughness = 27
                driver.breathiness = 61
                until(lambda: dict(female_worker.parameters).get(0x100f) == 61)
                driver.saveSettings()
                driver.headSize = 50
                driver.roughness = 0
                driver.breathiness = 0
                driver.loadSettings()
                assert (driver.headSize, driver.roughness, driver.breathiness) == (22, 27, 61)
                driver.voice = "1"
                assert driver.headSize == 50 and driver.breathiness == 0
                until(lambda: dict(worker.parameters).get(0x1010) == 0)
                driver.pitch = 43
                driver.inflection = 71
                driver.volume = 78
                driver.headSize = 54
                driver.roughness = 12
                driver.breathiness = 17
                snapshot = ((0x1008,43),(0x1009,71),(0x100a,78),
                            (0x100d,54),(0x100e,12),(0x100f,17))
                for locale, language in (("en_US",65536),("pt_BR",458752),("fi_FI",589824),("de_DE",262144),("en_US",65536),("de_DE",262144)):
                    driver.language = locale
                    driver.cancel()
                    until(lambda: driver._transport._engine._home == (int(language), "1") and
                                  all(all(dict(e.parameters).get(k)==v for k,v in snapshot)
                                      for e in Engine.instances if not e.closed))
                    assert (driver.pitch,driver.inflection,driver.volume,
                            driver.headSize,driver.roughness,driver.breathiness)==(43,71,78,54,12,17)
                driver.language = "el_GR"
                assert driver.language == "el_GR"
                driver.language = "en-US"
                assert driver.language == "en_US", "explicit selection must leave Greek's automatic English scope"
                driver.language = "zh_TW"
                assert driver.language == "zh_CN", "one Chinese selection also accepts old Taiwan locales"
                driver.language = "de_DE"
                assert list(driver.availableSamplerates) == ["0","1","3","2","4","5","6"]
                assert list(driver.availableSampleRates) == list(driver.availableSamplerates)
                for code,hz in (("3",16000),("5",44100),("6",48000),("1",11025)):
                    previous_player=driver._transport._player
                    driver.sampleRate=code
                    until(lambda: dict(worker.parameters).get(5)==int(code) and previous_player.closed)
                    assert driver._transport._player.sample_rate==hz
                driver.sampleRate="5"
                driver.saveSettings()
                driver.sampleRate="1"
                driver.loadSettings(onlyChanged=True)
                until(lambda: driver._transport._player.sample_rate==44100)
                assert driver.sampleRate=="5"
                driver.terminate()
                assert worker.closed and players[0].closed
                # A test-only second module proves profile restoration selects
                # language before validating a voice unavailable in German.
                language_api = importlib.import_module("synthDrivers.reloqC._reloq_languages")
                english = replace(english, lexicon=None, voices=(language_api.Voice("9", "Test voice"),))
                with mock.patch.object(driver_module, "LANGUAGES", {german.id: german, english.id: english}):
                    engines_before_profile = len(Engine.instances)
                    speech_config[driver.name] = {"engineLanguage": str(english.id),
                        "voice": "9", "rate": 60, "pitch": 65, "inflection": 75, "volume": 85,
                        "rateBoost": False, "voiceTags": True, "pauseMode": "2"}
                    multilingual = driver_module.SynthDriver()
                    multilingual.initSettings()
                    assert multilingual.language == english.locale
                    assert speech_config[driver.name]["engineLanguage"] == str(english.id)
                    assert speech_config[driver.name]["language"] == english.locale
                    assert multilingual.voice == "9" and multilingual.language == "en_GB"
                    speech_config[driver.name]["engineLanguage"] = str(german.id)
                    multilingual.loadSettings(onlyChanged=True)
                    assert multilingual.language == "en_GB", "standard setting must win over a stale legacy selector"
                    assert speech_config[driver.name]["engineLanguage"] == str(german.id)
                    assert list(multilingual.availableVoices) == ["9"]
                    assert set(multilingual.availableLanguages) == {"de_DE", "en_GB"}
                    until(lambda: len(Engine.instances) == engines_before_profile + 2 and
                        Engine.instances[-1].parameters[-1:] == [(0x100c, 2)])
                    assert Engine.instances[-1].dialect == english.id
                    multilingual.language = german.locale
                    assert multilingual.voice == "1" and multilingual.language == "de_DE"
                    multilingual.loadSettings(onlyChanged=True)
                    assert multilingual.voice == "9" and multilingual.language == "en_GB"
                    # Loading/saving an active profile must not erase an
                    # inherited legacy entry or rewrite the base profile.
                    config_api = sys.modules["config"]
                    original_conf = config_api.conf
                    base_profile = {"speech": {driver.name: {
                        "language": "en_GB", "engineLanguage": str(english.id), "voice": "9",
                        "communityDictionaries": True}}}
                    active_profile = {"speech": {driver.name: {"language": "de_DE", "voice": "1"}}}
                    manager = configuration_manager(base_profile, active_profile)
                    try:
                        config_api.conf = manager.rootSection
                        multilingual.dictionary_manager().fetch = offline
                        multilingual.initSettings()
                        assert multilingual.language == "de_DE" and multilingual.voice == "1"
                        assert multilingual.communityDictionaries and multilingual._dictionaryCommunity
                        multilingual.saveSettings()
                        assert base_profile["speech"][driver.name] == {
                            "language": "en_GB", "engineLanguage": str(english.id), "voice": "9",
                            "communityDictionaries": True}
                        assert active_profile["speech"][driver.name]["_dictionaryCommunity"] is True
                        multilingual._dictionaryCommunity = False
                        multilingual.saveSettings()
                        multilingual.loadSettings(onlyChanged=True)
                        assert not multilingual.communityDictionaries, "private setting must win over stale legacy data"
                        assert "engineLanguage" not in active_profile["speech"][driver.name]
                    finally:
                        config_api.conf = original_conf
                    multilingual.terminate()
                    assert all(instance.closed for instance in Engine.instances)
                if inspect_adapter is not None:
                    inspect_adapter(driver_module)
        finally:
            sys.path.remove(str(path))
            for name in tuple(sys.modules):
                if name.startswith("synthDrivers") or name in (
                    "config", "logHandler", "nvwave", "speech", "addonHandler",
                    "speech.commands", "synthDriverHandler"):
                    del sys.modules[name]
            if original_translate is None:
                del builtins._
            else:
                builtins._ = original_translate
    print(f"NVDA synth package ({ui_language} UI): gettext, selectable, x64, FIFO, index/done, cancel/close OK")


if __name__ == "__main__":
    main()
