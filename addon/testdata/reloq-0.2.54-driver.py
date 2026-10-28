"""Selectable NVDA driver for the experimental C-only German voice.

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

from autoSettingsUtils.driverSetting import BooleanDriverSetting, DriverSetting
from autoSettingsUtils.utils import StringParameterInfo

import config
from logHandler import log
import nvwave
from speech.commands import (BreakCommand, CharacterModeCommand, IndexCommand,
                             PhonemeCommand, PitchCommand)
from synthDriverHandler import (SynthDriver as BaseSynthDriver, VoiceInfo,
                                synthDoneSpeaking, synthIndexReached)

from ._reloq_eci_direct import (DirectEciEngine, EciIndex, EciPitch, NATIVE_VOLUME,
                                NATIVE_SPEED, NATIVE_PITCH, NATIVE_INFLECTION, PROCESS_TAGS,
                                PAUSE_MODE)
from ._reloq_eci_transport import EciTextTransport
from ._reloq_unicode import normalize_generic_text, normalize_spelling_text


DLL = Path(__file__).with_name("eci.dll")


def _native_speed(percent, boost=False):
    """OpenEVV's useful NVDA range: ECI 40..156, boost to at most 250."""
    if not isinstance(percent, int) or isinstance(percent, bool) or not 0 <= percent <= 100:
        raise ValueError("rate must be from 0 to 100")
    value = 40 + (116 * percent + 50) // 100
    return min(250, (8 * value + 2) // 5) if boost else value


def _ordinary_text(text, voice_tags=False):
    value = normalize_generic_text(text)
    if voice_tags:
        return value
    # Literal mode names the introducer and spells command letters, so
    # ordinary documents cannot accidentally execute voice instructions.
    return re.sub(r"`([A-Za-z]*)", lambda match:
                  " Gravis " + " ".join(match[1]) + (" " if match[1] else ""), value)


def _command_pitch(command, base_pitch):
    """NVDA's spelling pitch is relative to the configured native pitch."""
    if hasattr(command, "multiplier"):
        value = base_pitch * command.multiplier
    elif command.isDefault:
        value = base_pitch
    else:
        value = command.newValue
    if not isinstance(value, (int, float)) or not math.isfinite(value):
        return base_pitch
    return max(0, min(100, int(round(value))))


def _parts(speech_sequence, *, base_pitch=50, voice_tags=False):
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

    def flush():
        if pieces:
            runs = []
            for text, character_mode in pieces:
                if runs and runs[-1][1] == character_mode:
                    runs[-1] = (runs[-1][0] + text, character_mode)
                else:
                    runs.append((text, character_mode))
            parts.append("".join(normalize_spelling_text(text) if mode else
                _ordinary_text(text, voice_tags) for text, mode in runs))
            pieces.clear()

    for item in speech_sequence:
        if isinstance(item, str):
            said = item
            # Character mode is a sequence of letter tokens. Preserve their
            # boundaries in the one native utterance so an inline pitch mark
            # cannot accidentally change a later letter of the same word.
            if needs_letter_boundary and said and said[0].isalnum():
                pieces.append((" ", spelling))
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
        elif isinstance(item, BreakCommand):
            # Native timed breaks do not exist yet. Keep adjacent text from
            # turning into one word even when an unadvertised break reaches us.
            pieces.append((" ", spelling))
        elif isinstance(item, PhonemeCommand) and item.text:
            # Preserve the plain-text fallback; IPA is not understood by ECI.
            pieces.append((item.text, spelling))
        # Other unadvertised commands (language, rate/volume prosody)
        # have no native representation yet. Never pass their repr to ECI.
    flush()
    if not parts:
        parts.append("")
    return parts


class SynthDriver(BaseSynthDriver):
    name = "reloqC"
    description = "Eloquence C (experimentell, Deutsch)"
    supportedCommands = {IndexCommand, CharacterModeCommand, PitchCommand}
    supportedNotifications = {synthIndexReached, synthDoneSpeaking}
    supportedSettings = (
        BaseSynthDriver.VoiceSetting(),
        BaseSynthDriver.RateSetting(),
        BaseSynthDriver.RateBoostSetting(),
        BaseSynthDriver.PitchSetting(),
        BaseSynthDriver.InflectionSetting(),
        BaseSynthDriver.VolumeSetting(),
        BooleanDriverSetting("voiceTags", "ECI-&Tags verarbeiten", defaultVal=False),
        DriverSetting("pauseMode", "&Pausen verkürzen", defaultVal="0"),
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
        self._voiceTags = False
        self._pauseMode = "0"
        player = nvwave.WavePlayer(
            channels=1,
            samplesPerSec=11025,
            bitsPerSample=16,
            outputDevice=config.conf["audio"]["outputDevice"],
        )
        self._transport = EciTextTransport(
            lambda: DirectEciEngine(
                DLL, experimental_generic=True,
                lexicon_path=DLL.with_name("german_apple_observed.rlex")),
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

    def _get_availablePauseModes(self):
        return OrderedDict((value, StringParameterInfo(value, label)) for value, label in (
            ("0", "Nie"), ("1", "Nur am Textende"), ("2", "Immer")))

    def _get_pauseMode(self):
        return self._pauseMode

    def _set_pauseMode(self, mode):
        if mode not in ("0", "1", "2"):
            raise ValueError("pauseMode must be 0, 1 or 2")
        with self._submit_lock:
            if self._terminating or self._pauseMode == mode:
                return
            self._transport.set_parameter(PAUSE_MODE, int(mode))
            self._pauseMode = mode

    def _get_language(self):
        return "de_DE"

    def _get_voice(self):
        return "deu_experimental"

    def _set_voice(self, voice):
        if voice != "deu_experimental":
            raise ValueError("only the experimental German voice is available")

    def _get_availableVoices(self):
        return OrderedDict((
            ("deu_experimental", VoiceInfo(
                "deu_experimental", "Deutsch (experimentell)", "de_DE")),
        ))

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
        parts = _parts(speechSequence, base_pitch=pitch, voice_tags=tags)
        with self._submit_lock:
            if self._terminating or epoch != self._request_epoch:
                return
            # A settings change can arrive while a long sequence is being
            # normalized. Keep its inline relative commands paired with the
            # base pitch which will precede this job in the native queue.
            if self._pitch != pitch or self._voiceTags != tags:
                parts = _parts(speechSequence, base_pitch=self._pitch, voice_tags=self._voiceTags)
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
            transport, self._transport = self._transport, None
        if transport is not None and not transport.shutdown(timeout=1.5):
            # The worker still owns WavePlayer until native cancellation
            # completes. It must not be closed underneath a native callback.
            log.warning("Eloquence C worker did not stop within 1.5 seconds")

