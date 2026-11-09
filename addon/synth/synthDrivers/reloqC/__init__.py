"""OpenEVV-style NVDA settings and speech interface for the C-only engine.

The packaged ECI DLL is loaded in-process on the transport's persistent
worker. Speech and playback are queued without blocking NVDA's UI thread.
"""

from __future__ import annotations

from collections import OrderedDict
import ctypes
import json
import math
from pathlib import Path
import threading
import re

import addonHandler
addonHandler.initTranslation()

from autoSettingsUtils.driverSetting import BooleanDriverSetting, DriverSetting, NumericDriverSetting
from autoSettingsUtils.utils import StringParameterInfo

import config
import languageHandler
from logHandler import log
import nvwave
from speech.commands import (BreakCommand, CharacterModeCommand, IndexCommand, LangChangeCommand,
                             PhonemeCommand, PitchCommand, RateCommand, VolumeCommand)
from speech.extensions import filter_speechSequence
from synthDriverHandler import (SynthDriver as BaseSynthDriver, VoiceInfo, LanguageInfo,
                                synthDoneSpeaking, synthIndexReached)

from ._reloq_eci_direct import (DirectEciEngine, EciIndex, EciPitch, EciPitchHz, EciProsody, EciLanguage, EciText, NATIVE_VOLUME,
                                NATIVE_SPEED, NATIVE_PITCH, NATIVE_INFLECTION, PROCESS_TAGS,
                                PAUSE_MODE, HEAD_SIZE, ROUGHNESS, BREATHINESS, GENDER, SAMPLE_RATES,
                                DICTIONARY, PHRASE_PREDICTION, ORIGINAL_QUESTIONS, MELLOW_MODE, PITCH_HZ)
from ._reloq_eci_transport import DictionaryStartupError, EciTextTransport
from ._reloq_dictionary_manager import DictionaryManager as CommunityDictionaries
from ._reloq_languages import GERMAN, LANGUAGES, LANGUAGE_ALIASES, LANGUAGE_DISPLAY_ORDER, LanguageEngine, language_for
from ._speech_tags import EciTagCommand, protect_sequence


DLL = Path(__file__).with_name("eci.dll")


def _native_speed(percent, boost=False):
    """OpenEVV's useful NVDA range: ECI 40..156, boost to at most 250."""
    if not isinstance(percent, int) or isinstance(percent, bool) or not 0 <= percent <= 100:
        raise ValueError("rate must be from 0 to 100")
    value = 40 + (116 * percent + 50) // 100
    return min(250, (8 * value + 2) // 5) if boost else value


def _command_pitch(command, base_pitch):
    """NVDA's spelling pitch is relative to the configured native pitch."""
    if getattr(command, "isDefault", False):
        value = base_pitch
    elif getattr(command, "_offset", 0):
        # NVDA's multiplier accessor divides by the saved default for an
        # offset command. At pitch zero that would divide by zero. Apply the
        # original offset to this job's configured pitch snapshot instead.
        value = base_pitch + command.offset
    elif hasattr(command, "multiplier"):
        value = base_pitch * command.multiplier
    elif command.isDefault:
        value = base_pitch
    else:
        value = command.newValue
    if not isinstance(value, (int, float)) or not math.isfinite(value):
        return base_pitch
    return max(0, min(100, int(round(value))))


def _parts(speech_sequence, *, base_pitch=50, base_pitch_hz=0, base_rate=50, base_volume=100, rate_boost=False, voice_tags=False,
           language=GERMAN, voice="1", languages=LANGUAGES):
    """Keep NVDA's text and indices together in one native utterance.

    ECI accepts interior index marks at word boundaries. Consecutive text
    runs are joined before Unicode admission so a mark neither ends a
    sentence nor starts an additional synthesis job. No length cap is used.
    """
    pieces = []
    parts = []
    spelling = False
    needs_letter_boundary = False
    raised = False
    last_text_alnum = False
    home_language = language.id

    def flush():
        if pieces:
            runs = []
            for text, character_mode in pieces:
                if runs and runs[-1][1] == character_mode:
                    runs[-1] = (runs[-1][0] + text, character_mode)
                else:
                    runs.append((text, character_mode))
            parts.extend(EciText(text, mode) for text, mode in runs)
            pieces.clear()

    for item in speech_sequence:
        if isinstance(item, str):
            said = item
            # Character mode is a sequence of letter tokens. Preserve their
            # boundaries in the one native utterance so an inline pitch mark
            # cannot accidentally change a later letter of the same word.
            if needs_letter_boundary and said and said[0].isalnum():
                # This boundary is metadata between letter tokens, not a
                # character supplied by NVDA. Spelling it would announce the
                # space name in languages such as Finnish and Korean.
                pieces.append((" ", False))
            if said:
                needs_letter_boundary = spelling or (raised and
                    len(said) == 1 and said.isalpha())
                last_text_alnum = said[-1].isalnum()
            pieces.append((said, spelling))
        elif isinstance(item, IndexCommand):
            flush()
            parts.append(EciIndex(item.index))
        elif isinstance(item, PitchCommand):
            flush()
            value = _command_pitch(item, base_pitch)
            if base_pitch_hz:
                # Preserve NVDA's relative capital-pitch behavior around the
                # explicitly selected physical baseline.
                hz_for = lambda v: 35.11023877 + 4.889761232 * 2 ** (.06306456 * v)
                frequency = round(base_pitch_hz * hz_for(value) / hz_for(base_pitch))
                parts.append(EciPitchHz(max(1, min(24000, frequency))))
            else:
                parts.append(EciPitch(value))
            raised = value != base_pitch
        elif isinstance(item, (RateCommand, VolumeCommand)):
            flush()
            rate = isinstance(item, RateCommand)
            value = _command_pitch(item, base_rate if rate else base_volume)
            parts.append(EciProsody(6 if rate else 7,
                _native_speed(value, rate_boost) if rate else value))
        elif isinstance(item, EciTagCommand):
            # A setting can change after the early NVDA filter. Keep its
            # original text even then; native tag/character-mode admission
            # decides whether it is a control or ordinary spoken text.
            speed = None
            if voice_tags and not spelling:
                if re.fullmatch(r"`(?:vwpm|vs)\d+", item.text):
                    speed = item.text
                elif re.fullmatch(r"`v[1-9]", item.text):
                    preset_speeds = (50, 50, 50, 50, 70, 70, 50, 50, 50)
                    speed = f"`vs{preset_speeds[int(item.text[-1]) - 1]}"
            if speed is None:
                pieces.append((item.text, spelling))
            else:
                flush()
                parts.append(EciText(item.text, False, speed))
        elif isinstance(item, CharacterModeCommand):
            next_spelling = bool(item.state)
            if next_spelling != spelling and last_text_alnum:
                needs_letter_boundary = True
            spelling = next_spelling
        elif isinstance(item, LangChangeCommand):
            selected = language_for(item.lang, home_language, languages)
            if selected is not None and selected != language.id:
                flush()
                language = languages[selected]
                try:
                    selected_voice = language.voice(voice).id
                except ValueError:
                    selected_voice = language.voice(None).id
                parts.append(EciLanguage(selected, selected_voice))
        elif isinstance(item, BreakCommand):
            # Timed NVDA breaks are not advertised by this adapter yet.
            # Keep adjacent text separate if such a command still reaches us.
            pieces.append((" ", spelling))
        elif isinstance(item, PhonemeCommand) and item.text:
            # Preserve the plain-text fallback; IPA is not understood by ECI.
            pieces.append((item.text, spelling))
    flush()
    if not parts:
        parts.append("")
    return parts


class SynthDriver(BaseSynthDriver):
    name = "reloqC"
    # Translators: Name in NVDA's synthesizer selection dialog.
    description = _("Eloquence C")
    supportedCommands = {IndexCommand, CharacterModeCommand, PitchCommand, RateCommand,
                         VolumeCommand, LangChangeCommand, EciTagCommand}
    supportedNotifications = {synthIndexReached, synthDoneSpeaking}
    supportedSettings = (
        BaseSynthDriver.LanguageSetting(),
        BaseSynthDriver.VoiceSetting(),
        BaseSynthDriver.RateSetting(),
        BaseSynthDriver.RateBoostSetting(),
        BaseSynthDriver.PitchSetting(),
        BaseSynthDriver.InflectionSetting(),
        BaseSynthDriver.VolumeSetting(),
        NumericDriverSetting("headSize", _("&Head size"), False, defaultVal=50),
        NumericDriverSetting("roughness", _("Rou&ghness"), False, defaultVal=0),
        NumericDriverSetting("breathiness", _("&Breathiness"), False, defaultVal=0),
        BooleanDriverSetting("abbreviationDictionary", _("Use &abbreviation dictionary"), defaultVal=True),
        BooleanDriverSetting("phrasePrediction", _("Phrase &prediction"), defaultVal=False),
        BooleanDriverSetting("originalQuestions", _("Use original &question intonation"), defaultVal=False),
        BooleanDriverSetting("voiceTags", _("Process &ECI tags"), defaultVal=False),
        BooleanDriverSetting("_dictionaryCommunity", _("Use &community pronunciation dictionaries"), defaultVal=False),
        DriverSetting("pauseMode", _("Shorten pa&uses"), defaultVal="0"),
        DriverSetting("sampleRate", _("&Sample rate"), defaultVal="1"),
        BooleanDriverSetting("mellowMode", _("&Mellow mode (16 kHz and above)"), defaultVal=False),
        # Hidden from NVDA's automatic slider panel. The dedicated Eloquence
        # panel uses a numeric input, while the standard pitch stays 0..100.
        NumericDriverSetting("_pitchHertz", _("Pitch in hertz"), defaultVal=150, minVal=1, maxVal=24000),
        BooleanDriverSetting("_pitchInHertz", _("Use pitch in hertz"), defaultVal=False),
    )

    @classmethod
    def check(cls):
        # ARM64EC can load x64 code although the native host is ARM64. The
        # Windows loader, not IsWow64Process2, decides compatibility.
        if ctypes.sizeof(ctypes.c_void_p) != 8 or not DLL.is_file():
            return False
        try:
            ctypes.WinDLL(str(DLL))
        except (AttributeError, OSError):
            return False
        return True

    def __init__(self):
        if not self.check():
            raise RuntimeError("The x64 ECI DLL cannot load in this NVDA process")
        self._terminating = False
        self._submit_lock = threading.RLock()
        self._request_epoch = 0
        self._rate = 50
        self._rateBoost = False
        self._pitch = 50
        self._pitchHertz = 150
        self._pitchInHertz = False
        self._applied_pitch_hz = 0
        self._loading_pitch_settings = False
        self._inflection = 30
        self._volume = 100
        self._headSize = 50
        self._roughness = 0
        self._breathiness = 0
        self._abbreviationDictionary = True
        self._phrasePrediction = False
        self._originalQuestions = False
        self._mellowMode = False
        self._voiceTags = False
        self._communityDictionaries = self._initial_dictionary_setting()
        self._dictionary_updater = None
        self._pauseMode = "0"
        self._sampleRate = "1"
        self._sample_rate_revision = 0
        self._languages = dict(LANGUAGES)
        self._selectedLanguage = self._initial_language()
        default_voice = self._languages[self._selectedLanguage].voice(None)
        self._voice = default_voice.id
        # On the first NVDA initialization, match the selected voice's ECI
        # preset. NVDA subsequently restores any existing saved pitch.
        self._pitch = dict(default_voice.parameters).get(NATIVE_PITCH, self._pitch)
        initial_language, initial_voice = self._selectedLanguage, self._voice
        player = self._make_player(11025)
        self._transport = EciTextTransport(
            lambda: LanguageEngine(DLL, languages=self._languages,
                language=initial_language, voice=initial_voice,
                engine_factory=DirectEciEngine),
            player,
            self._on_index,
            self._on_done,
            self._on_error,
            complete_on_error=True,
        )
        # Enqueue defaults before speech. NVDA may show a stored/default
        # setting without calling its setter on the first initialization.
        self._transport.set_parameter(NATIVE_SPEED, _native_speed(self._rate))
        self._transport.set_parameter(NATIVE_PITCH, self._pitch)
        self._transport.set_parameter(NATIVE_INFLECTION, self._inflection)
        self._transport.set_parameter(NATIVE_VOLUME, self._volume)
        self._transport.set_parameter(HEAD_SIZE, self._headSize)
        self._transport.set_parameter(ROUGHNESS, self._roughness)
        self._transport.set_parameter(BREATHINESS, self._breathiness)
        self._transport.set_parameter(ORIGINAL_QUESTIONS, 0)
        self._transport.set_parameter(MELLOW_MODE, 0)
        manager = self.dictionary_manager()
        # Start monitoring the installed dictionaries and selected own files.
        self._dictionary_request(lambda: manager.set_enabled(self._communityDictionaries))
        filter_speechSequence.register(self._protect_tags)
        try:
            source = json.loads((DLL.parents[2] / "doc/source.json").read_text(encoding="utf-8"))
            revision = source["source_commit"]
        except (OSError, ValueError, KeyError, TypeError):
            revision = "local"
        log.info("Eloquence C loaded driver %s, source %s, DLL %s", __file__, revision, DLL)

    def _protect_tags(self, sequence):
        # The extension runs before NVDA replaces the grave accent by its
        # spoken symbol name (or a space). Keep this scoped to our active
        # synthesizer and its explicit tag setting.
        if self._terminating or not self._voiceTags:
            return sequence
        from synthDriverHandler import getSynth
        if getSynth() is not self:
            return sequence
        return protect_sequence(sequence)

    @staticmethod
    def _make_player(sample_rate):
        return nvwave.WavePlayer(channels=1, samplesPerSec=sample_rate,
            bitsPerSample=16, outputDevice=config.conf["audio"]["outputDevice"])

    def _get_availableSamplerates(self):
        # Translators: Audio sample rates; localize the decimal separator.
        labels = {0: _("8 kHz"), 1: _("11.025 kHz"), 3: _("16 kHz"), 2: _("22.05 kHz"),
                  4: _("32 kHz"), 5: _("44.1 kHz"), 6: _("48 kHz")}
        return OrderedDict((str(code), StringParameterInfo(str(code), labels[code]))
                           for code in SAMPLE_RATES)

    def _get_availableSampleRates(self):
        return self._get_availableSamplerates()

    def _get_sampleRate(self):
        return self._sampleRate

    def _set_sampleRate(self, value):
        value = str(value)
        if value not in self._get_availableSamplerates():
            raise ValueError("unsupported sample rate")
        with self._submit_lock:
            if self._terminating or self._sampleRate == value:
                return
            self._request_epoch += 1
            self._transport.cancel()
            self._sample_rate_revision += 1
            revision = self._sample_rate_revision
            def completed(actual_code):
                with self._submit_lock:
                    if revision == self._sample_rate_revision:
                        self._sampleRate = str(actual_code)
            self._sampleRate = value
            self._transport.set_sample_rate(int(value), self._make_player, completed)

    def initSettings(self):
        # OpenEVV refreshes the schema before NVDA restores existing profiles.
        # This also covers NVDA releases whose base class did not do so yet.
        previous = self._loading_pitch_settings
        self._loading_pitch_settings = True
        try:
            section = config.conf[self._configSection]
            if section.isSet(self.name):
                section[self.name].spec.update(self.getConfigSpec())
            super().initSettings()
        finally:
            self._loading_pitch_settings = previous
        if not previous:
            self._sync_pitch_hertz()

    def _initial_language(self):
        # Schema defaults are not a saved choice. Use NVDA's resolved UI
        # language (also when General is set to "Windows") only if neither
        # the standard nor the legacy language selector has a valid value.
        speech = config.conf[self._configSection]
        if speech.isSet(self.name):
            section = speech[self.name]
            if section.isSet("language"):
                value = section["language"]
                selected = language_for(value, 0, self._languages) if isinstance(value, str) else None
                if selected in self._languages:
                    return selected
            if section.isSet("engineLanguage"):
                try:
                    selected = int(str(section["engineLanguage"]), 0)
                    selected = LANGUAGE_ALIASES.get(selected, selected)
                    if selected in self._languages:
                        return selected
                except (TypeError, ValueError):
                    pass
        value = languageHandler.getLanguage()
        selected = language_for(value, 0, self._languages) if isinstance(value, str) else None
        return selected if selected in self._languages else next(iter(self._languages))

    def _initial_dictionary_setting(self):
        # Read the active profile before starting the dictionary worker. NVDA
        # restores settings after constructing the driver; starting with the
        # default could otherwise briefly allow speech before a saved enabled
        # community dictionary has been installed.
        speech = config.conf[self._configSection]
        if speech.isSet(self.name):
            section = speech[self.name]
            for key in ("_dictionaryCommunity", "communityDictionaries"):
                if section.isSet(key):
                    # The driver schema has not yet been installed. Reading
                    # through [] here caches a raw ConfigObj string, and
                    # NVDA's later schema update does not invalidate it.
                    # Read without caching, then use NVDA's own validator.
                    value = section.__getitem__(key, checkValidity=False)
                    return self._dictionary_config_boolean(value, section)
        return False

    def _dictionary_config_boolean(self, value, section=None):
        if isinstance(value, bool):
            return value
        if section is None:
            section = config.conf[self._configSection][self.name]
        return section.manager.validator.check("boolean(default=False)", value)

    def loadSettings(self, onlyChanged=False):
        # NVDA restores voice before other settings. Select the module first
        # so a voice belonging to a non-default language can be validated.
        was_loading = self._loading_pitch_settings
        self._loading_pitch_settings = True
        try:
            section = config.conf[self._configSection][self.name]
            self._select_language(self._initial_language())
            section["language"] = self._get_language()
            if not section.isSet("_dictionaryCommunity") and section.isSet("communityDictionaries"):
                previous = section.__getitem__("communityDictionaries", checkValidity=False)
                section["_dictionaryCommunity"] = self._dictionary_config_boolean(previous, section)
            # AggregatedSection supports neither pop nor deletion. Leave any
            # legacy value intact; the canonical language now takes precedence.
            super().loadSettings(onlyChanged=onlyChanged)
        finally:
            self._loading_pitch_settings = was_loading
        if not was_loading:
            self._sync_pitch_hertz()

    def _get_communityDictionaries(self):
        return self._communityDictionaries

    def _get__dictionaryCommunity(self):
        return self.communityDictionaries

    def _set__dictionaryCommunity(self, enabled):
        # A different caller may have cached a raw value before the schema
        # was installed. Normalize that cached setting as well.
        self.communityDictionaries = self._dictionary_config_boolean(enabled)

    def _apply_dictionary_snapshot(self, revision, path):
        with self._submit_lock:
            if (not self._terminating and self._dictionary_updater is not None
                    and self._dictionary_updater.current(revision)):
                self._transport.set_dictionary_path(path)

    def _dictionary_snapshot_failed(self, revision):
        with self._submit_lock:
            if (not self._terminating and self._dictionary_updater is not None
                    and self._dictionary_updater.current(revision)):
                if not self._transport.fail_dictionary_load():
                    self._show_dictionary_startup_error()

    def _show_dictionary_startup_error(self):
        # The normal speech path is waiting, so expose the failure as an
        # accessible dialog too (for example on a braille display).
        import wx
        def show():
            with self._submit_lock:
                if self._terminating or not self._transport.dictionary_startup_blocked():
                    return
            import gui
            gui.messageBox(_("The selected pronunciation dictionaries could not be loaded. "
                             "Speech will resume when they are available. Check the dictionary "
                             "settings, or select another synthesizer."),
                           _("Pronunciation dictionaries"), wx.OK | wx.ICON_ERROR,
                           parent=getattr(gui, "mainFrame", None))
        wx.CallAfter(show)

    def _dictionary_request(self, start):
        # Speech jobs wait on their own worker until the complete local
        # snapshot has been selected and installed into the native engine.
        # The NVDA UI and background network update never wait on it.
        with self._submit_lock:
            if self._terminating:
                raise RuntimeError("synthesizer is terminating")
            self._transport.begin_dictionary_load()
            try:
                return start()
            except Exception:
                self._transport.fail_dictionary_load()
                raise

    def dictionary_manager(self):
        with self._submit_lock:
            if self._terminating:
                raise RuntimeError("synthesizer is terminating")
            if self._dictionary_updater is None:
                import globalVars
                profile = Path(globalVars.appArgs.configPath)
                dictionaries = DLL.with_name("communityDictionaries")
                self._dictionary_updater = CommunityDictionaries(
                    dictionaries, dictionaries / ".cache",
                    self._apply_dictionary_snapshot,
                    lambda error: log.warning("Community dictionary refresh failed: %s", error),
                    settings_path=profile / "reloqCDictionaries.json",
                    legacy_settings=profile / "reloqCommunityDictionaries" / "sources.json",
                    on_update=self._dictionary_update_notice,
                    on_load_error=self._dictionary_snapshot_failed)
            return self._dictionary_updater

    def _dictionary_update_notice(self, revision, details):
        import wx
        def show():
            with self._submit_lock:
                if (self._terminating or self._dictionary_updater is None or
                        not self._dictionary_updater.current(revision)):
                    return
            from globalPlugins.reloqCDictionaries import show_dictionary_update
            show_dictionary_update(details)
        wx.CallAfter(show)

    def refresh_dictionaries(self, completed=None):
        return self._dictionary_request(lambda: self.dictionary_manager().refresh(completed))

    def set_dictionary_sources(self, sources, completed=None):
        return self._dictionary_request(lambda: self.dictionary_manager().set_sources(sources, completed))

    def configure_dictionaries(self, sources, enabled, completed=None, repository=None):
        with self._submit_lock:
            revision = self._dictionary_request(lambda: self.dictionary_manager().configure(
                sources, enabled, completed, repository=repository))
            self._communityDictionaries = enabled
            return revision

    def _set_communityDictionaries(self, enable):
        if not isinstance(enable, bool):
            raise ValueError("communityDictionaries must be boolean")
        with self._submit_lock:
            if self._terminating or self._communityDictionaries == enable:
                return
            self._dictionary_request(lambda: self.dictionary_manager().set_enabled(enable))
            self._communityDictionaries = enable

    def _get_abbreviationDictionary(self):
        return self._abbreviationDictionary

    def _set_abbreviationDictionary(self, enable):
        if not isinstance(enable, bool):
            raise ValueError("abbreviationDictionary must be boolean")
        with self._submit_lock:
            if self._terminating or self._abbreviationDictionary == enable:
                return
            self._transport.set_parameter(DICTIONARY, int(not enable))
            self._abbreviationDictionary = enable

    def _get_phrasePrediction(self):
        return self._phrasePrediction

    def _set_phrasePrediction(self, enable):
        if not isinstance(enable, bool):
            raise ValueError("phrasePrediction must be boolean")
        with self._submit_lock:
            if self._terminating or self._phrasePrediction == enable:
                return
            self._transport.set_parameter(PHRASE_PREDICTION, int(enable))
            self._phrasePrediction = enable

    def _get_originalQuestions(self):
        return self._originalQuestions

    def _set_originalQuestions(self, enable):
        if not isinstance(enable, bool):
            raise ValueError("originalQuestions must be boolean")
        with self._submit_lock:
            if self._terminating or self._originalQuestions == enable:
                return
            self._transport.set_parameter(ORIGINAL_QUESTIONS, int(enable))
            self._originalQuestions = enable

    def _get_mellowMode(self):
        return self._mellowMode

    def _set_mellowMode(self, enable):
        if not isinstance(enable, bool):
            raise ValueError("mellowMode must be boolean")
        with self._submit_lock:
            if self._terminating or self._mellowMode == enable:
                return
            self._transport.set_parameter(MELLOW_MODE, int(enable))
            self._mellowMode = enable

    def _get_voiceTags(self):
        return self._voiceTags

    def _set_voiceTags(self, enable):
        if not isinstance(enable, bool):
            raise ValueError("voiceTags must be boolean")
        with self._submit_lock:
            if self._terminating or self._voiceTags == enable:
                return
            self._transport.set_parameter(PROCESS_TAGS, int(enable))
            self._voiceTags = enable

    def _get_availablePausemodes(self):
        return OrderedDict((value, StringParameterInfo(value, label)) for value, label in (
            ("0", _("Never")), ("1", _("Only at text end")), ("2", _("Always"))))

    # NVDA uses setting.id.capitalize(), which lowercases the internal M.
    # Keep the old spelling as an alias for external callers.
    def _get_availablePauseModes(self):
        return self._get_availablePausemodes()

    def _get_pauseMode(self):
        return self._pauseMode

    def _set_pauseMode(self, mode):
        mode = str(mode)
        if mode not in ("0", "1", "2"):
            mode = "0"
        with self._submit_lock:
            if self._terminating or self._pauseMode == mode:
                return
            self._transport.set_parameter(PAUSE_MODE, int(mode))
            self._pauseMode = mode

    def _get_language(self):
        return self._languages[self._selectedLanguage].locale

    def _get_availableLanguages(self):
        result = OrderedDict()
        order = {ident: position for position, ident in enumerate(LANGUAGE_DISPLAY_ORDER)}
        for language in sorted(self._languages.values(), key=lambda item: order.get(item.id, len(order))):
            info = LanguageInfo(language.locale)
            info.displayName = language.name
            result[language.locale] = info
        return result

    def _set_language(self, value):
        if not isinstance(value, str) or not value:
            return
        # An explicit settings selection is independent of automatic speech
        # language commands, including Greek's deliberate English reading.
        selected = language_for(value, 0, self._languages)
        if selected is not None:
            self._select_language(selected)

    def _select_language(self, selected):
        with self._submit_lock:
            if self._terminating or selected not in self._languages or selected == self._selectedLanguage:
                return
            language = self._languages[selected]
            try:
                voice = language.voice(self._voice).id
            except ValueError:
                voice = language.voice(None).id
            self._transport.select_voice(selected, voice, parameters=(
                (NATIVE_SPEED, _native_speed(self._rate, self._rateBoost)),
                (NATIVE_PITCH, self._pitch), (NATIVE_INFLECTION, self._inflection),
                (NATIVE_VOLUME, self._volume), (HEAD_SIZE, self._headSize),
                (ROUGHNESS, self._roughness), (BREATHINESS, self._breathiness),
                (PROCESS_TAGS, int(self._voiceTags)), (PAUSE_MODE, int(self._pauseMode)),
                (DICTIONARY, int(not self._abbreviationDictionary)),
                (PHRASE_PREDICTION, int(self._phrasePrediction)),
                (ORIGINAL_QUESTIONS, int(self._originalQuestions)),
                (MELLOW_MODE, int(self._mellowMode)),
            ) + (((PITCH_HZ, self._pitchHertz),) if self._pitchInHertz else ()))
            self._selectedLanguage, self._voice = selected, voice

    def _get_voice(self):
        return self._voice

    def _set_voice(self, voice):
        with self._submit_lock:
            if self._terminating:
                return
            selected = self._languages[self._selectedLanguage].voice(voice)
            if selected.id == self._voice:
                return
            self._transport.select_voice(self._selectedLanguage, selected.id,
                parameters=((PITCH_HZ, self._pitchHertz),) if self._pitchInHertz else ())
            self._voice = selected.id
            # A module may define presets using the controls supported by C.
            for parameter, value in selected.parameters:
                name = {NATIVE_PITCH: "pitch", NATIVE_VOLUME: "volume",
                        NATIVE_INFLECTION: "inflection", HEAD_SIZE: "headSize",
                        ROUGHNESS: "roughness", BREATHINESS: "breathiness"}.get(parameter)
                if name is not None:
                    setattr(self, "_" + name, value)

    def _get_availableVoices(self):
        language = self._languages[self._selectedLanguage]
        return OrderedDict((voice.id, VoiceInfo(voice.id, voice.name, language.locale))
                           for voice in language.voices)

    def _set_setting(self, name, parameter, value, native_value=None):
        if not isinstance(value, int) or isinstance(value, bool) or not 0 <= value <= 100:
            raise ValueError(f"{name} must be from 0 to 100")
        with self._submit_lock:
            if self._terminating:
                return
            self._transport.set_parameter(
                parameter, value if native_value is None else native_value)
            setattr(self, "_" + name, value)

    def _get_rate(self):
        return self._rate

    def _set_rate(self, value):
        self._set_setting("rate", NATIVE_SPEED, value,
                          _native_speed(value, self._rateBoost))

    def _get_rateBoost(self):
        return self._rateBoost

    def _set_rateBoost(self, enable):
        if not isinstance(enable, bool):
            raise ValueError("rateBoost must be boolean")
        with self._submit_lock:
            if self._terminating or self._rateBoost == enable:
                return
            self._rateBoost = enable
            self._transport.set_parameter(
                NATIVE_SPEED, _native_speed(self._rate, enable))

    def _get_pitch(self):
        return self._pitch

    def _set_pitch(self, value):
        switched = False
        with self._submit_lock:
            self._set_setting("pitch", NATIVE_PITCH, value)
            if self._applied_pitch_hz:
                self._transport.set_parameter(PITCH_HZ, 0)
                self._applied_pitch_hz = 0
            if self._pitchInHertz and not self._loading_pitch_settings:
                self._pitchInHertz = False
                switched = True
        if switched:
            import ui
            ui.message(_("Hertz pitch off; using the ECI pitch scale."))

    def _sync_pitch_hertz(self):
        with self._submit_lock:
            if self._terminating:
                return
            wanted = self._pitchHertz if self._pitchInHertz else 0
            if wanted == self._applied_pitch_hz:
                return
            if wanted:
                self._transport.set_parameter(PITCH_HZ, wanted)
            else:
                self._transport.set_parameter(PITCH_HZ, 0)
                self._transport.set_parameter(NATIVE_PITCH, self._pitch)
            self._applied_pitch_hz = wanted

    def configure_pitch_hertz(self, enabled, value):
        if type(enabled) is not bool or type(value) is not int or not 1 <= value <= 24000:
            raise ValueError("invalid Hertz pitch setting")
        with self._submit_lock:
            self._pitchHertz, self._pitchInHertz = value, enabled
            self._sync_pitch_hertz()

    def _get_volume(self):
        return self._volume

    def _set_volume(self, value):
        self._set_setting("volume", NATIVE_VOLUME, value)

    def _get_inflection(self):
        return self._inflection

    def _set_inflection(self, value):
        self._set_setting("inflection", NATIVE_INFLECTION, value)

    def _get_headSize(self):
        return self._headSize

    def _set_headSize(self, value):
        self._set_setting("headSize", HEAD_SIZE, value)

    def _get_roughness(self):
        return self._roughness

    def _set_roughness(self, value):
        self._set_setting("roughness", ROUGHNESS, value)

    def _get_breathiness(self):
        return self._breathiness

    def _set_breathiness(self, value):
        self._set_setting("breathiness", BREATHINESS, value)

    def _on_index(self, value):
        if not self._terminating:
            synthIndexReached.notify(synth=self, index=value)

    def _on_done(self, _job_number):
        if not self._terminating:
            synthDoneSpeaking.notify(synth=self)

    def _on_error(self, error):
        log.error("Eloquence C synthesis failed", exc_info=(
            type(error), error, error.__traceback__))
        if isinstance(error, DictionaryStartupError):
            self._show_dictionary_startup_error()

    def speak(self, speechSequence):
        with self._submit_lock:
            if self._terminating:
                return
            epoch = self._request_epoch
            pitch = self._pitch
            pitch_hz = self._pitchHertz if self._pitchInHertz else 0
            tags = self._voiceTags
            language = self._selectedLanguage
            voice = self._voice
            rate, boost, volume = self._rate, self._rateBoost, self._volume
        def make_parts():
            return _parts(speechSequence, base_pitch=pitch, base_pitch_hz=pitch_hz, voice_tags=tags,
                          base_rate=rate, rate_boost=boost, base_volume=volume,
                          language=self._languages[language], voice=voice, languages=self._languages)
        parts = make_parts()
        with self._submit_lock:
            if self._terminating or epoch != self._request_epoch:
                return
            # A settings change can arrive while a long sequence is being
            # normalized. Keep its inline relative commands paired with the
            # base pitch which will precede this job in the native queue.
            if (self._pitch, self._pitchHertz if self._pitchInHertz else 0, self._voiceTags, self._selectedLanguage, self._voice,
                self._rate, self._rateBoost, self._volume) != (pitch, pitch_hz, tags, language, voice, rate, boost, volume):
                pitch, tags, language, voice = self._pitch, self._voiceTags, self._selectedLanguage, self._voice
                pitch_hz = self._pitchHertz if self._pitchInHertz else 0
                rate, boost, volume = self._rate, self._rateBoost, self._volume
                parts = make_parts()
            characters = sum(len(p.text) if isinstance(p, EciText) else len(p) if isinstance(p, str) else 0 for p in parts)
            protected_tags = sum(isinstance(p, EciTagCommand) for p in speechSequence)
            if characters >= 1024 or protected_tags:
                log.info("Eloquence C queued speech: characters=%d, protected_tags=%d, rate=%d Hz, tags=%s",
                    characters, protected_tags, SAMPLE_RATES[int(self._sampleRate)], tags)
            self._transport.submit(parts)

    def cancel(self):
        with self._submit_lock:
            if not self._terminating:
                self._request_epoch += 1
                self._transport.cancel()

    def pause(self, switch):
        if not self._terminating:
            self._transport.pause(switch)

    def terminate(self):
        filter_speechSequence.unregister(self._protect_tags)
        with self._submit_lock:
            self._terminating = True
            if self._dictionary_updater is not None:
                self._dictionary_updater.close()
            transport, self._transport = self._transport, None
        if transport is not None and not transport.shutdown(timeout=1.5):
            # The worker still owns WavePlayer until native cancellation
            # completes. It must not be closed underneath a native callback.
            log.warning("Eloquence C worker did not stop within 1.5 seconds")

