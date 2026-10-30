"""OpenEVV-style NVDA settings and speech interface for the C-only engine.

The packaged ECI DLL is loaded in-process on the transport's persistent
worker. Speech and playback are queued without blocking NVDA's UI thread.
The authored exact preview uses a separate ECI handle in its default mode.
"""

from __future__ import annotations

from collections import OrderedDict
import ctypes
import math
from pathlib import Path
import threading
import re

import addonHandler
addonHandler.initTranslation()

from autoSettingsUtils.driverSetting import BooleanDriverSetting, DriverSetting, NumericDriverSetting
from autoSettingsUtils.utils import StringParameterInfo

import config
from logHandler import log
import nvwave
from speech.commands import (BreakCommand, CharacterModeCommand, IndexCommand, LangChangeCommand,
                             PhonemeCommand, PitchCommand)
from synthDriverHandler import (SynthDriver as BaseSynthDriver, VoiceInfo, LanguageInfo,
                                synthDoneSpeaking, synthIndexReached)

from ._reloq_eci_direct import (DirectEciEngine, EciIndex, EciPitch, EciLanguage, EciText, NATIVE_VOLUME,
                                NATIVE_SPEED, NATIVE_PITCH, NATIVE_INFLECTION, PROCESS_TAGS,
                                PAUSE_MODE, HEAD_SIZE, ROUGHNESS, BREATHINESS, GENDER, SAMPLE_RATES,
                                DICTIONARY, PHRASE_PREDICTION)
from ._reloq_eci_transport import EciTextTransport
from ._reloq_dictionary_manager import DictionaryManager as CommunityDictionaries
from ._reloq_languages import GERMAN, LANGUAGES, LANGUAGE_ALIASES, LANGUAGE_DISPLAY_ORDER, LanguageEngine, language_for


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


def _parts(speech_sequence, *, base_pitch=50, voice_tags=False,
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
            parts.append(EciPitch(value))
            raised = value != base_pitch
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
        # Other unadvertised commands (rate/volume prosody)
        # have no native representation yet. Never pass their repr to ECI.
    flush()
    if not parts:
        parts.append("")
    return parts


class SynthDriver(BaseSynthDriver):
    name = "reloqC"
    # Translators: Name in NVDA's synthesizer selection dialog.
    description = _("Eloquence C (experimental)")
    supportedCommands = {IndexCommand, CharacterModeCommand, PitchCommand, LangChangeCommand}
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
        BooleanDriverSetting("voiceTags", _("Process &ECI tags"), defaultVal=False),
        BooleanDriverSetting("_dictionaryCommunity", _("Use &community pronunciation dictionaries"), defaultVal=False),
        DriverSetting("pauseMode", _("Shorten pa&uses"), defaultVal="0"),
        DriverSetting("sampleRate", _("&Sample rate"), defaultVal="1"),
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
        self._inflection = 30
        self._volume = 100
        self._headSize = 50
        self._roughness = 0
        self._breathiness = 0
        self._abbreviationDictionary = True
        self._phrasePrediction = False
        self._voiceTags = False
        self._communityDictionaries = False
        self._dictionary_updater = None
        self._pauseMode = "0"
        self._sampleRate = "1"
        self._sample_rate_revision = 0
        self._languages = dict(LANGUAGES)
        self._selectedLanguage = next(iter(self._languages))
        self._voice = self._languages[self._selectedLanguage].voice(None).id
        player = self._make_player(11025)
        self._transport = EciTextTransport(
            lambda: LanguageEngine(DLL, languages=self._languages,
                language=next(iter(self._languages)),
                voice=self._languages[next(iter(self._languages))].voice(None).id,
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
        manager = self.dictionary_manager()
        # Start monitoring the installed dictionaries and selected own files.
        manager.set_enabled(False)

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
        section = config.conf[self._configSection]
        if section.isSet(self.name):
            section[self.name].spec.update(self.getConfigSpec())
        super().initSettings()

    def loadSettings(self, onlyChanged=False):
        # NVDA restores voice before other settings. Select the module first
        # so a voice belonging to a non-default language can be validated.
        section = config.conf[self._configSection][self.name]
        selected = section.get("language")
        if not selected:
            try:
                legacy = int(str(section.get("engineLanguage")), 0)
                legacy = LANGUAGE_ALIASES.get(legacy, legacy)
                selected = self._languages[legacy].locale
            except (TypeError, ValueError, KeyError):
                selected = self._get_language()
        self._set_language(selected)
        section["language"] = self._get_language()
        if not section.isSet("_dictionaryCommunity") and section.isSet("communityDictionaries"):
            section["_dictionaryCommunity"] = section["communityDictionaries"]
        # AggregatedSection supports neither pop nor deletion. Leave any
        # legacy value intact; the canonical language now takes precedence.
        super().loadSettings(onlyChanged=onlyChanged)

    def _get_communityDictionaries(self):
        return self._communityDictionaries

    def _get__dictionaryCommunity(self):
        return self.communityDictionaries

    def _set__dictionaryCommunity(self, enabled):
        self.communityDictionaries = enabled

    def _apply_dictionary_snapshot(self, revision, path):
        with self._submit_lock:
            if (not self._terminating and self._dictionary_updater is not None
                    and self._dictionary_updater.current(revision)):
                self._transport.set_dictionary_path(path)

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
                    legacy_settings=profile / "reloqCommunityDictionaries" / "sources.json")
            return self._dictionary_updater

    def refresh_dictionaries(self, completed=None):
        return self.dictionary_manager().refresh(completed)

    def set_dictionary_sources(self, sources, completed=None):
        return self.dictionary_manager().set_sources(sources, completed)

    def configure_dictionaries(self, sources, enabled, completed=None):
        with self._submit_lock:
            revision = self.dictionary_manager().configure(sources, enabled, completed)
            self._communityDictionaries = enabled
            return revision

    def _set_communityDictionaries(self, enable):
        if not isinstance(enable, bool):
            raise ValueError("communityDictionaries must be boolean")
        with self._submit_lock:
            if self._terminating or self._communityDictionaries == enable:
                return
            self.dictionary_manager().set_enabled(enable)
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
            ))
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
            self._transport.select_voice(self._selectedLanguage, selected.id)
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
        self._set_setting("pitch", NATIVE_PITCH, value)

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

    def speak(self, speechSequence):
        with self._submit_lock:
            if self._terminating:
                return
            epoch = self._request_epoch
            pitch = self._pitch
            tags = self._voiceTags
            language = self._selectedLanguage
            voice = self._voice
        def make_parts():
            return _parts(speechSequence, base_pitch=pitch, voice_tags=tags,
                          language=self._languages[language], voice=voice, languages=self._languages)
        parts = make_parts()
        with self._submit_lock:
            if self._terminating or epoch != self._request_epoch:
                return
            # A settings change can arrive while a long sequence is being
            # normalized. Keep its inline relative commands paired with the
            # base pitch which will precede this job in the native queue.
            if (self._pitch, self._voiceTags, self._selectedLanguage, self._voice) != (pitch, tags, language, voice):
                pitch, tags, language, voice = self._pitch, self._voiceTags, self._selectedLanguage, self._voice
                parts = make_parts()
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
        with self._submit_lock:
            self._terminating = True
            if self._dictionary_updater is not None:
                self._dictionary_updater.close()
            transport, self._transport = self._transport, None
        if transport is not None and not transport.shutdown(timeout=1.5):
            # The worker still owns WavePlayer until native cancellation
            # completes. It must not be closed underneath a native callback.
            log.warning("Eloquence C worker did not stop within 1.5 seconds")

