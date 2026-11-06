"""Direct, in-process ECI binding used by the NVDA driver.

This adapter owns a native ECI handle and its PCM output buffer. It neither
starts a helper executable nor imports NVDA. Exact mode covers a measured
German subset; the native voice provides broader language coverage.

German and US English pass unmodified UTF-8 and character-mode flags to
eciAddTextUtf8. Unicode names, Greek and accent handling belong to the C core.
The legacy eciAddText byte interface remains available for direct callers.
"""

from __future__ import annotations

import ctypes
import codecs
from dataclasses import dataclass
import logging
from pathlib import Path
import re
import sys
import threading
import time
from typing import Callable, Sequence


SAMPLE_RATE = 5
SAMPLE_RATES = {0: 8000, 1: 11025, 3: 16000, 2: 22050, 4: 32000, 5: 44100, 6: 48000}
ENGLISH_DIALECT = 0x00010000
GERMAN_DIALECT = 0x00040000
NATIVE_VOICE_MODE = 0x1003
GENERIC_RATE = 0x1004
GENERIC_PITCH = 0x1005
GENERIC_VOLUME = 0x1006
NATIVE_SPEED = 0x1007
NATIVE_PITCH = 0x1008
NATIVE_INFLECTION = 0x1009
NATIVE_VOLUME = 0x100a
DICTIONARY = 3  # ECI: 0 enabled, 1 disabled
PHRASE_PREDICTION = 11
PROCESS_TAGS = 0x100b
PAUSE_MODE = 0x100c
HEAD_SIZE = 0x100d
ROUGHNESS = 0x100e
BREATHINESS = 0x100f
GENDER = 0x1010
HIGH_RATE_PROFILE = 0x1011
GERMAN_GLIDE_OPENING = 0x1012
GERMAN_GLIDE_BACKNESS = 0x1013
GERMAN_GLIDE_SOFTNESS = 0x1014
_log = logging.getLogger(__name__)
WAVEFORM_BUFFER = 0
INDEX_REPLY = 2
DATA_PROCESSED = 1
DATA_ABORT = 2

_CallbackType = (ctypes.WINFUNCTYPE if sys.platform == "win32" else
                 ctypes.CFUNCTYPE)(ctypes.c_int, ctypes.c_void_p,
                                    ctypes.c_int, ctypes.c_int, ctypes.c_void_p)


@dataclass(frozen=True)
class EciText:
    text: str
    character_mode: bool = False


@dataclass(frozen=True)
class _Utf8Text:
    data: bytes
    character_mode: bool


@dataclass(frozen=True)
class EciIndex:
    value: int


@dataclass(frozen=True)
class EciPitch:
    """Absolute native pitch from this point in the same ECI utterance."""

    value: int


@dataclass(frozen=True)
class EciProsody:
    """A standard ECI speed (6) or volume (7) change in queued text."""

    parameter: int
    value: int


@dataclass(frozen=True)
class EciLanguage:
    """A language boundary handled by LanguageEngine, not passed to a DLL."""

    language: int
    voice: str


class _BusyRejection(RuntimeError):
    """A submission was refused because the native handle stayed busy.

    On the owner thread no synthesis of this engine can legitimately be in
    flight before eciSynthesize, so a busy refusal here means the handle is
    poisoned: every later submission would be refused the same way until
    the handle is rebuilt.
    """

    def __init__(self, operation: str, code: int) -> None:
        self.operation = operation
        self.code = code
        super().__init__(f"{operation} rejected input (code {code}; native busy=1)")


class EciSynthesisError(RuntimeError):
    """Structured native failure, including transactional preflight rejection."""

    def __init__(self, status: int, stage: int, utf8_offset: int,
                 character_position: int | None) -> None:
        self.status = status
        self.stage = stage
        self.utf8_offset = utf8_offset
        self.character_position = character_position
        super().__init__("eciSynthesize failed: "
                         f"status={status} stage={stage} "
                         f"UTF-8 offset={utf8_offset}")


class DirectEciEngine:
    """Synchronous caller API around ECI's callback-producing worker.

    Create, speak and close on one Python worker. Only ``cancel`` may run on
    another thread. The PCM and index hooks run during synthesis, may be on a
    native thread, and must return promptly. They cannot call ECI recursively.
    The caller must wait for ``speak`` to finish before closing this object.
    Playback completion is a separate event and is not implied by speak's
    return. The NVDA transport owns that audio queue and its generation.
    """

    def __init__(self, dll_path: str | Path, *, dialect: int = GERMAN_DIALECT,
                 buffer_samples: int = 1100, native_voice: bool = False,
                 lexicon_path: str | Path | None = None,
                 encoding: str = "cp1252",
                 _loader: Callable[[str], object] | None = None) -> None:
        if not isinstance(dialect, int) or isinstance(dialect, bool) or not -0x80000000 <= dialect <= 0x7fffffff:
            raise ValueError("dialect must fit an ECI int")
        if not isinstance(buffer_samples, int) or isinstance(buffer_samples, bool) or not 1 <= buffer_samples <= 0x7fffffff:
            raise ValueError("buffer_samples must be positive ECI int")
        self._encoding = codecs.lookup(encoding).name
        self._dialect = dialect
        self._native_voice = native_voice
        self._lexicon_path = lexicon_path
        self._parameters: dict[int, int] = {}
        self._dictionary_path: str | None = None
        self._dictionary_path_set = False
        self._handle_poisoned = False
        self._owner = threading.get_ident()
        self._cancelled = threading.Event()
        self._on_pcm: Callable[[bytes], None] | None = None
        self._on_index: Callable[[int], None] | None = None
        self._callback_error: BaseException | None = None
        self._first_pcm_diagnostic = None
        self._buffer_samples = buffer_samples
        self._buffer = (ctypes.c_int16 * buffer_samples)()
        self._callback = _CallbackType(self._receive)
        loader = _loader or (ctypes.WinDLL if sys.platform == "win32" else ctypes.CDLL)
        self._dll = loader(str(Path(dll_path).resolve()))
        self._bind()
        self.native_version = "unknown"
        version = getattr(self._dll, "eciVersion", None)
        if version is not None:
            version.argtypes, version.restype = (ctypes.c_void_p,), None
            label = ctypes.create_string_buffer(20)
            version(label)
            self.native_version = label.value.decode("ascii", errors="replace")
        _log.info("Eloquence C native version %s, dialect %#x, DLL %s",
                  self.native_version, dialect, Path(dll_path).resolve())
        self._handle = self._open_handle()

    def _open_handle(self):
        handle = self._dll.eciNewEx(self._dialect)
        if not handle:
            raise RuntimeError(f"eciNewEx failed for dialect 0x{self._dialect:x}")
        try:
            if not self._native_voice:
                self._dll.eciSetParam(handle, NATIVE_VOICE_MODE, 0)
            if self._native_voice:
                previous = self._dll.eciSetParam(
                    handle, NATIVE_VOICE_MODE, 1)
                if previous not in (0, 1) or self._dll.eciGetParam(
                        handle, NATIVE_VOICE_MODE) != 1:
                    raise RuntimeError("eciSetParam rejected the native voice")
                if self._lexicon_path is not None:
                    path = Path(self._lexicon_path)
                    if path.is_file():
                        load = getattr(self._dll, "eciLoadLexicon", None)
                        if load is not None:
                            load.argtypes = (ctypes.c_void_p, ctypes.c_char_p)
                            load.restype = ctypes.c_int
                            if load(handle, str(path.resolve()).encode("utf-8")) != 1:
                                _log.warning("optional generic lexicon was rejected: %s", path)
                        else:
                            _log.warning("ECI DLL lacks optional lexicon loader")
                    else:
                        _log.warning("optional generic lexicon is missing: %s", path)
            self._dll.eciRegisterCallback(handle, self._callback, None)
            if self._dll.eciSetOutputBuffer(handle, self._buffer_samples, self._buffer) <= 0:
                raise RuntimeError("eciSetOutputBuffer failed")
        except BaseException:
            self._dll.eciDelete(handle)
            raise
        return handle

    def _recreate_handle(self) -> None:
        """Replace a poisoned native handle and replay this engine's state.

        Only the owner thread calls this, between utterances, when no
        callback can be active. The old handle is stopped and deleted on a
        best-effort basis: a native handle that still claims busy may refuse
        deletion, but its replacement is independent, so speech can resume.
        """
        self._assert_owner()
        if self._on_pcm is not None:
            raise RuntimeError("cannot rebuild ECI handle while a callback may be active")
        old = self._handle
        self._handle = None
        if old:
            try:
                self._dll.eciStop(old)
            except Exception:
                _log.exception("ECI stop failed while rebuilding a busy handle")
            try:
                self._dll.eciDelete(old)
            except Exception:
                _log.exception("ECI delete failed while rebuilding a busy handle")
        _log.warning("Eloquence C rebuilt a native handle that stayed busy")
        self._handle = self._open_handle()
        if self._dictionary_path_set:
            self._apply_dictionary_path(self._handle, self._dictionary_path)
        for parameter, value in self._parameters.items():
            self._apply_parameter(self._handle, parameter, value)

    def _bind(self) -> None:
        def function(name: str, args: tuple[object, ...], result: object) -> None:
            target = getattr(self._dll, name)
            target.argtypes = args
            target.restype = result

        h = ctypes.c_void_p
        i = ctypes.c_int
        function("eciNewEx", (i,), h)
        function("eciDelete", (h,), h)
        function("eciRegisterCallback", (h, _CallbackType, h), None)
        function("eciSetOutputBuffer", (h, i, ctypes.POINTER(ctypes.c_int16)), i)
        function("eciAddText", (h, ctypes.c_char_p), i)
        if self._encoding == "utf-8":
            function("eciAddTextUtf8", (h, ctypes.c_char_p, ctypes.c_size_t, i), i)
        function("eciInsertIndex", (h, i), i)
        function("eciInsertPitch", (h, i), i)
        function("eciSynthesize", (h,), i)
        function("eciSynchronize", (h,), i)
        function("eciClearInput", (h,), i)
        function("eciStop", (h,), i)
        function("eciGetParam", (h, i), i)
        function("eciSetParam", (h, i, i), i)
        function("eciGetVoiceParam", (h, i, i), i)
        function("eciSetVoiceParam", (h, i, i, i), i)

    def _apply_dictionary_path(self, handle, path: str | None) -> None:
        target = self._dll.eciSetDictionaryPath
        target.argtypes = (ctypes.c_void_p, ctypes.c_char_p)
        target.restype = ctypes.c_int
        encoded = path.encode("utf-8") if path is not None else None
        if not target(handle, encoded):
            raise RuntimeError("native ECI dictionary volumes could not be loaded")

    def set_dictionary_path(self, path: str | Path | None) -> None:
        self._assert_owner()
        if not self._handle:
            raise RuntimeError("ECI engine has been closed")
        stored = str(path) if path is not None else None
        self._apply_dictionary_path(self._handle, stored)
        self._dictionary_path = stored
        self._dictionary_path_set = True

    def _apply_parameter(self, handle, parameter: int, value: int) -> None:
        previous = self._dll.eciSetParam(handle, parameter, value)
        if previous < 0 or self._dll.eciGetParam(handle, parameter) != value:
            raise RuntimeError(f"eciSetParam rejected generic parameter {parameter:#x}")

    def set_parameter(self, parameter: int, value: int) -> None:
        """Change a generic-voice setting on the ECI owner's worker thread."""
        self._assert_owner()
        if not self._handle:
            raise RuntimeError("ECI engine has been closed")
        if parameter not in (GENERIC_RATE, GENERIC_PITCH, GENERIC_VOLUME,
                             NATIVE_SPEED, NATIVE_PITCH, NATIVE_INFLECTION,
                             NATIVE_VOLUME, PROCESS_TAGS, DICTIONARY, PHRASE_PREDICTION, PAUSE_MODE, HEAD_SIZE, ROUGHNESS, BREATHINESS, GENDER, SAMPLE_RATE, HIGH_RATE_PROFILE, GERMAN_GLIDE_OPENING, GERMAN_GLIDE_BACKNESS, GERMAN_GLIDE_SOFTNESS):
            raise ValueError("unsupported generic voice parameter")
        low, high = ((0, 250) if parameter == NATIVE_SPEED else
                     (0, 1) if parameter in (PROCESS_TAGS, GENDER, DICTIONARY, PHRASE_PREDICTION) else
                     (0, 6) if parameter == SAMPLE_RATE else
                     (0, 3) if parameter == HIGH_RATE_PROFILE else
                     (0, 2) if parameter == PAUSE_MODE else (0, 100))
        if not isinstance(value, int) or isinstance(value, bool) or not low <= value <= high:
            raise ValueError(f"generic voice parameter must be {low}..{high}")
        self._apply_parameter(self._handle, parameter, value)
        self._parameters[parameter] = value
        if parameter in (SAMPLE_RATE, HIGH_RATE_PROFILE, PROCESS_TAGS):
            _log.info("Eloquence C native setting verified: parameter=%#x value=%d", parameter, value)

    def _assert_owner(self) -> None:
        if threading.get_ident() != self._owner:
            raise RuntimeError("ECI instance belongs to its creating worker")

    def _receive(self, _handle: int, message: int, value: int,
                 _context: int) -> int:
        if self._cancelled.is_set() or self._callback_error is not None:
            return DATA_ABORT
        try:
            if message == WAVEFORM_BUFFER:
                if not 0 <= value <= self._buffer_samples:
                    raise RuntimeError(f"ECI reported invalid PCM sample count {value}")
                assert self._on_pcm is not None
                if value and self._first_pcm_diagnostic is not None:
                    started, characters = self._first_pcm_diagnostic
                    self._first_pcm_diagnostic = None
                    _log.info("Eloquence C first native PCM: %.1f ms, characters=%d",
                        (time.perf_counter() - started) * 1000, characters)
                self._on_pcm(ctypes.string_at(self._buffer, value * 2))
            elif message == INDEX_REPLY:
                assert self._on_index is not None
                self._on_index(value)
        except BaseException as error:
            self._callback_error = error
            return DATA_ABORT
        return DATA_PROCESSED

    @staticmethod
    def _prepare(parts: Sequence[str | EciIndex | EciPitch], encoding: str = "cp1252") -> tuple[bytes | EciIndex | EciPitch, ...]:
        prepared: list[bytes | EciIndex | EciPitch] = []
        for part in parts:
            if isinstance(part, EciText) and codecs.lookup(encoding).name != "utf-8":
                raise ValueError("EciText requires the native UTF-8 interface")
            if isinstance(part, EciText) or isinstance(part, str) and codecs.lookup(encoding).name == "utf-8":
                text = part.text if isinstance(part, EciText) else part
                mode = part.character_mode if isinstance(part, EciText) else False
                if not isinstance(text, str) or not isinstance(mode, bool):
                    raise TypeError("invalid native Unicode text run")
                prepared.append(_Utf8Text(text.encode("utf-8", errors="strict"), mode))
            elif isinstance(part, str):
                encoded = part.encode(encoding, errors="strict")
                if b"\0" in encoded:
                    raise ValueError("ECI text cannot contain NUL bytes")
                prepared.append(encoded)
            elif isinstance(part, EciIndex):
                if isinstance(part.value, bool) or not isinstance(part.value, int) or not 0 <= part.value <= 0x7fffffff:
                    raise ValueError("index must fit nonnegative ECI int")
                prepared.append(part)
            elif isinstance(part, EciPitch):
                if isinstance(part.value, bool) or not isinstance(part.value, int) or not 0 <= part.value <= 100:
                    raise ValueError("pitch must be a native 0..100 value")
                prepared.append(part)
            elif isinstance(part, EciProsody):
                if (type(part.parameter) is not int or part.parameter not in (6, 7)
                    or type(part.value) is not int or not 0 <= part.value <= (250 if part.parameter == 6 else 100)):
                    raise ValueError("invalid ECI speed/volume change")
                prepared.append(part)
            else:
                raise TypeError("ECI parts must be text, EciIndex, EciPitch or EciProsody")
        return tuple(prepared)

    def _synthesis_error(self, parts: Sequence[str | EciIndex]) -> EciSynthesisError:
        # eciGetParam is public ECI. The three read-only parameter numbers
        # belong to this C port; no private DLL symbol is needed. The native
        # diagnosis survives eciClearInput in the finally block below.
        status = self._dll.eciGetParam(self._handle, 0x1000)
        offset = self._dll.eciGetParam(self._handle, 0x1001)
        stage = self._dll.eciGetParam(self._handle, 0x1002)
        position = None
        if offset >= 0:
            phrase = "".join(part.text if isinstance(part, EciText) else part for part in parts if isinstance(part, (str, EciText)))
            encoded = phrase.encode("utf-8")
            position = len(encoded[:min(offset, len(encoded))].decode(
                "utf-8", errors="ignore")) + 1
        return EciSynthesisError(status, stage, offset, position)

    def _native_busy_diagnostic(self) -> str:
        """Capture reentry state without modifying the queue or ECI status."""
        query = getattr(self._dll, "eciIsBeingReentered", None)
        if query is None or not self._handle:
            return "unknown"
        try:
            query.argtypes = (ctypes.c_void_p,)
            query.restype = ctypes.c_int
            return str(query(self._handle))
        except Exception:
            return "unknown"

    def speak(self, parts: Sequence[str | EciIndex | EciPitch],
              on_pcm: Callable[[bytes], None],
              on_index: Callable[[int], None]) -> bool:
        """Submit text/index order and wait for native synthesis completion.

        Returns False after cancellation, otherwise True. Callback failures
        are re-raised on the caller thread. This does not wait for audio played
        by NVDA's WavePlayer.
        """
        self._assert_owner()
        prepared = self._prepare(parts, self._encoding)  # Preflight before queue mutation.
        if not self._handle:
            raise RuntimeError("ECI engine has been closed")
        self._cancelled.clear()
        delivered = False

        def pcm_hook(data: bytes) -> None:
            nonlocal delivered
            delivered = True
            on_pcm(data)

        def index_hook(value: int) -> None:
            nonlocal delivered
            delivered = True
            on_index(value)

        for attempt in (0, 1):
            self._handle_poisoned = False
            try:
                result = self._speak_attempt(
                    prepared, parts, pcm_hook, index_hook)
            except _BusyRejection:
                if attempt == 0 and not delivered and not self._cancelled.is_set():
                    self._recreate_handle()
                    continue
                if self._handle_poisoned:
                    try:
                        self._recreate_handle()
                    except Exception:
                        _log.exception("ECI busy recovery failed")
                raise
            except BaseException:
                if self._handle_poisoned:
                    try:
                        self._recreate_handle()
                    except Exception:
                        _log.exception("ECI busy recovery failed")
                raise
            if self._handle_poisoned:
                try:
                    self._recreate_handle()
                except Exception:
                    _log.exception("ECI busy recovery failed")
            return result
        raise RuntimeError("ECI busy recovery did not run")  # pragma: no cover

    # One native phrase of URL-like text corrupts the vendored rules
    # engine and crashes the host process (upstream issue #19): a phrase
    # past a few hundred bytes carrying hard tokens (a long hyphenated
    # compound, a long mixed letter/digit token) is enough, while long
    # ordinary prose synthesizes safely as one phrase. Every extra
    # eciSynthesize round costs a phrase-final pause, so only risky
    # phrases are split, in word-boundary rounds of at most this many
    # text bytes; a single longer word keeps a round of its own. All
    # other text, however long, keeps the single-round path unchanged.
    _MAX_ROUND_TEXT_BYTES = 200
    # A phrase longer than this is split even without hard tokens, as a
    # safety net for planner loads this driver has not characterized.
    _LONG_PHRASE_TEXT_BYTES = 1024
    _LONG_PHRASE_ROUND_BYTES = 512

    @staticmethod
    def _is_hard_token(unit: bytes) -> bool:
        core = unit.strip(b" \t\r\n\"'()[]{}<>.,;:!?")
        if len(core) >= 48:
            return True
        if len(core) >= 20 and core.count(b"-") >= 2:
            return True
        return (len(core) >= 32 and any(48 <= byte <= 57 for byte in core)
                and any(65 <= byte <= 90 or 97 <= byte <= 122 for byte in core))

    @staticmethod
    def _ends_phrase(unit: bytes) -> bool:
        if b"\n" in unit:
            return True
        core = unit.rstrip(b" \t\r\n\"'()[]{}<>")
        return core.endswith((b".", b"!", b"?"))

    @classmethod
    def _rounds(cls, prepared):
        total = sum(len(p.data) if isinstance(p, _Utf8Text) else
                    len(p) if isinstance(p, bytes) else 0 for p in prepared)
        if total <= cls._MAX_ROUND_TEXT_BYTES:
            return (tuple(prepared),)
        # Flatten to word/space units tagged with their phrase, keeping
        # non-text parts in order between the units around them.
        items = []  # (output part, phrase id or None, is word unit)
        phrase = 0
        for part in prepared:
            if isinstance(part, (_Utf8Text, bytes)):
                data = part.data if isinstance(part, _Utf8Text) else part
                for unit in re.findall(rb"[^ ]+| +", data):
                    output = (_Utf8Text(unit, part.character_mode)
                              if isinstance(part, _Utf8Text) else unit)
                    items.append((output, phrase, bool(unit.strip())))
                    if unit.strip() and cls._ends_phrase(unit):
                        phrase += 1
            else:
                items.append((part, None, False))
        phrase_bytes: dict[int, int] = {}
        phrase_hard: dict[int, bool] = {}
        for output, owner, is_word in items:
            if owner is None:
                continue
            data = output.data if isinstance(output, _Utf8Text) else output
            phrase_bytes[owner] = phrase_bytes.get(owner, 0) + len(data)
            if is_word and cls._is_hard_token(data):
                phrase_hard[owner] = True

        def round_limit(owner: int) -> int | None:
            if phrase_bytes.get(owner, 0) <= cls._MAX_ROUND_TEXT_BYTES:
                return None
            if phrase_hard.get(owner):
                return cls._MAX_ROUND_TEXT_BYTES
            if phrase_bytes[owner] > cls._LONG_PHRASE_TEXT_BYTES:
                return cls._LONG_PHRASE_ROUND_BYTES
            return None

        rounds = []
        current = []
        current_phrase_bytes = 0
        active_phrase: int | None = None

        def flush():
            nonlocal current, current_phrase_bytes
            if current:
                rounds.append(tuple(current))
                current = []
                current_phrase_bytes = 0

        for output, owner, is_word in items:
            if owner is not None:
                data = output.data if isinstance(output, _Utf8Text) else output
                limit = round_limit(owner)
                if limit is not None and owner != active_phrase and current:
                    # A split phrase must start a fresh round: sharing a
                    # round with earlier text would resubmit it whole.
                    flush()
                if (limit is not None and is_word and current_phrase_bytes
                        and current_phrase_bytes + len(data) > limit):
                    flush()
                if owner != active_phrase:
                    active_phrase = owner
                    current_phrase_bytes = 0
                current_phrase_bytes += len(data)
            current.append(output)
        flush()
        return tuple(rounds) if rounds else (tuple(prepared),)

    def _synthesize_round(self, round_parts, parts, restore) -> bool:
        """Submit one round and synthesize it; False means cancelled."""
        for part in round_parts:
            if self._cancelled.is_set():
                return False
            if isinstance(part, _Utf8Text):
                ok = self._dll.eciAddTextUtf8(self._handle, part.data, len(part.data), int(part.character_mode))
                operation = "eciAddTextUtf8"
            elif isinstance(part, EciIndex):
                ok = self._dll.eciInsertIndex(self._handle, part.value)
                operation = "eciInsertIndex"
            elif isinstance(part, EciPitch):
                ok = self._dll.eciInsertPitch(self._handle, part.value)
                operation = "eciInsertPitch"
            elif isinstance(part, EciProsody):
                if part.parameter not in restore:
                    restore[part.parameter] = self._dll.eciGetVoiceParam(self._handle, 0, part.parameter)
                    if restore[part.parameter] < 0:
                        raise RuntimeError("eciGetVoiceParam failed")
                previous = self._dll.eciSetVoiceParam(self._handle, 0, part.parameter, part.value)
                ok = int(previous >= 0 and
                    self._dll.eciGetVoiceParam(self._handle, 0, part.parameter) == part.value)
                operation = "eciSetVoiceParam"
            else:
                ok = self._dll.eciAddText(self._handle, part)
                operation = "eciAddText"
            if ok <= 0:
                busy = self._native_busy_diagnostic()
                if busy == "1":
                    raise _BusyRejection(operation, ok)
                raise RuntimeError(f"{operation} rejected input (code {ok}; native busy={busy})")
        if self._cancelled.is_set():
            return False
        started = self._dll.eciSynthesize(self._handle)
        if started <= 0:
            if self._callback_error is not None:
                raise self._callback_error
            if self._cancelled.is_set():
                return False
            raise self._synthesis_error(parts)
        completed = self._dll.eciSynchronize(self._handle)
        if self._callback_error is not None:
            raise self._callback_error
        if self._cancelled.is_set():
            return False
        if completed <= 0:
            raise RuntimeError("eciSynchronize failed")
        return True

    def _speak_attempt(self, prepared, parts,
                       on_pcm: Callable[[bytes], None],
                       on_index: Callable[[int], None]) -> bool:
        started_at = time.perf_counter()
        self._callback_error = None
        self._on_pcm, self._on_index = on_pcm, on_index
        characters = sum(len(p.text) if isinstance(p, EciText) else len(p) if isinstance(p, str) else 0 for p in parts)
        self._first_pcm_diagnostic = (started_at, characters) if characters >= 1024 else None
        completed_successfully = False
        restore = {}
        try:
            for round_parts in self._rounds(prepared):
                if not self._synthesize_round(round_parts, parts, restore):
                    return False
            completed_successfully = True
            return True
        finally:
            # Cancellation can occur after one accepted AddText and before
            # eciSynthesize. Discard those fragments so the next utterance
            # cannot accidentally join them to its own text.
            if not completed_successfully and self._handle:
                if not self._dll.eciClearInput(self._handle):
                    busy = self._native_busy_diagnostic()
                    _log.error("ECI could not clear cancelled input; native busy=%s",
                               busy)
                    if busy == "1":
                        # The handle would refuse every later submission.
                        # Let speak() rebuild it before the next job runs.
                        self._handle_poisoned = True
            # These are job-local NVDA commands, unlike the persistent user
            # sliders. Restore after synthesis/cancellation, never from PCM
            # callbacks while the native engine is busy.
            if self._handle:
                for parameter, value in restore.items():
                    if self._dll.eciSetVoiceParam(self._handle, 0, parameter, value) < 0:
                        _log.error("ECI could not restore voice parameter %s", parameter)
            self._on_pcm = self._on_index = None
            self._first_pcm_diagnostic = None

    def cancel(self) -> None:
        """Cooperative ECI stop; may be called by a different thread."""
        self._cancelled.set()
        handle = self._handle
        if handle:
            self._dll.eciStop(handle)

    def close(self) -> None:
        self._assert_owner()
        if self._on_pcm is not None:
            raise RuntimeError("cannot close while ECI callback may be active")
        handle = self._handle
        if handle:
            self._handle = None
            self._dll.eciDelete(handle)


__all__ = ["EciText", "DirectEciEngine", "EciIndex", "EciPitch", "EciProsody", "EciLanguage", "EciSynthesisError",
           "GERMAN_DIALECT", "NATIVE_VOICE_MODE", "GENERIC_RATE",
           "GENERIC_PITCH", "GENERIC_VOLUME", "NATIVE_SPEED", "NATIVE_PITCH",
           "NATIVE_INFLECTION", "NATIVE_VOLUME", "DICTIONARY", "PHRASE_PREDICTION", "PROCESS_TAGS", "PAUSE_MODE", "HEAD_SIZE", "ROUGHNESS", "BREATHINESS", "GENDER", "HIGH_RATE_PROFILE", "GERMAN_GLIDE_OPENING", "GERMAN_GLIDE_BACKNESS", "GERMAN_GLIDE_SOFTNESS", "SAMPLE_RATE", "SAMPLE_RATES"]
