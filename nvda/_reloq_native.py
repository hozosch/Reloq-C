"""Native C analysis and numeric PCM bridge; this is not a synth driver.

The restricted German frontend can provide SPR and phone context. It cannot
derive a complete voice schedule from text yet. Numeric PCM calls still need
62 already scheduled control values per frame. Audio synthesis stays in the
authored C library, with no Apple or OpenEVV runtime dependency.
"""

from __future__ import annotations

import ctypes
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Sequence


FRAME_CONTROLS = 62
ABI_VERSION = 1
OK = 0
CANCELLED = 1
CAPACITY = -7
TEXT_INVALID = -5
TEXT_UNCOVERED = -6


def _checked_int(name: str, value: int, minimum: int, maximum: int) -> int:
    if not isinstance(value, int) or not minimum <= value <= maximum:
        raise ValueError(f"{name} must be an integer in {minimum}..{maximum}")
    return value


class _Config(ctypes.Structure):
    _fields_ = [
        ("struct_size", ctypes.c_uint32),
        ("abi_version", ctypes.c_uint32),
        ("sample_rate_hz", ctypes.c_uint32),
        ("max_frame_samples", ctypes.c_uint32),
        ("duration_scale", ctypes.c_float),
        ("output_gain", ctypes.c_float),
        ("initial_seed", ctypes.c_int32),
        ("source_offset", ctypes.c_int32),
        ("av_trim", ctypes.c_int32),
        ("ah_trim", ctypes.c_int32),
        ("af_trim", ctypes.c_int32),
    ]


class _Phone(ctypes.Structure):
    _fields_ = [(name, ctypes.c_uint32) for name in (
        "phone", "left_phone", "right_phone", "source_offset", "source_length",
        "word_index", "syllable_index", "syllable_in_word", "stress",
        "stress_explicit", "same_word_left", "same_word_right",
        "same_syllable_left", "same_syllable_right",
    )]


class _TextInfo(ctypes.Structure):
    _fields_ = [(name, ctypes.c_uint32) for name in (
        "struct_size", "annotation_bytes", "phone_count", "word_count",
        "syllable_count", "error_offset",
    )]


@dataclass(frozen=True)
class GermanPhone:
    phone: int
    left_phone: int
    right_phone: int
    source_offset: int
    source_length: int
    word_index: int
    syllable_index: int
    syllable_in_word: int
    stress: int
    stress_explicit: int
    same_word_left: int
    same_word_right: int
    same_syllable_left: int
    same_syllable_right: int


@dataclass(frozen=True)
class GermanAnalysis:
    annotation: bytes
    phones: tuple[GermanPhone, ...]
    word_count: int
    syllable_count: int


class GermanTextError(ValueError):
    def __init__(self, code: int, utf8_byte_offset: int):
        self.code = code
        self.utf8_byte_offset = utf8_byte_offset
        super().__init__(f"German frontend error {code} at UTF-8 byte {utf8_byte_offset}")


# The C API explicitly uses __cdecl on Windows. CFUNCTYPE and CDLL select
# that convention on both supported host families.
_PCMCallback = ctypes.CFUNCTYPE(
    ctypes.c_int, ctypes.c_void_p, ctypes.POINTER(ctypes.c_int16),
    ctypes.c_uint32,
)


class NativeFrameEngine:
    """Host-owned synchronous numeric renderer, not text-to-speech yet.

    Render and reset must stay on one worker thread. Cancel may be called
    concurrently. Join that worker before close; the C host cannot be
    destroyed while a callback or cancellation is active.
    """

    def __init__(
        self, dll_path: str | Path, *, sample_rate_hz: int = 11025,
        max_frame_samples: int = 8192, duration_scale: float = 1.0,
        output_gain: float = 1.0, initial_seed: int = 0,
        source_offset: int = 0, av_trim: int = 0, ah_trim: int = 0,
        af_trim: int = 0,
    ) -> None:
        sample_rate_hz = _checked_int(
            "sample_rate_hz", sample_rate_hz, 1, 0xFFFFFFFF,
        )
        max_frame_samples = _checked_int(
            "max_frame_samples", max_frame_samples, 1, 0xFFFFFFFF,
        )
        initial_seed = _checked_int(
            "initial_seed", initial_seed, -0x80000000, 0x7FFFFFFF,
        )
        source_offset = _checked_int(
            "source_offset", source_offset, -0x80000000, 0x7FFFFFFF,
        )
        av_trim = _checked_int("av_trim", av_trim, -0x80000000, 0x7FFFFFFF)
        ah_trim = _checked_int("ah_trim", ah_trim, -0x80000000, 0x7FFFFFFF)
        af_trim = _checked_int("af_trim", af_trim, -0x80000000, 0x7FFFFFFF)
        self._dll = ctypes.CDLL(str(dll_path))
        self._dll.reloq_host_abi_version.argtypes = ()
        self._dll.reloq_host_abi_version.restype = ctypes.c_uint32
        version = self._dll.reloq_host_abi_version()
        if version != ABI_VERSION:
            raise RuntimeError(f"unsupported native ABI version {version}")

        self._dll.reloq_host_create.argtypes = (
            ctypes.POINTER(_Config), ctypes.POINTER(ctypes.c_void_p),
        )
        self._dll.reloq_host_create.restype = ctypes.c_int
        self._dll.reloq_host_destroy.argtypes = (ctypes.c_void_p,)
        self._dll.reloq_host_destroy.restype = None
        self._dll.reloq_host_reset.argtypes = (ctypes.c_void_p,)
        self._dll.reloq_host_reset.restype = ctypes.c_int
        self._dll.reloq_host_cancel.argtypes = (ctypes.c_void_p,)
        self._dll.reloq_host_cancel.restype = None
        self._dll.reloq_host_render_controls.argtypes = (
            ctypes.c_void_p, ctypes.POINTER(ctypes.c_float),
            ctypes.c_uint32, ctypes.c_uint32, _PCMCallback,
            ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint32),
        )
        self._dll.reloq_host_render_controls.restype = ctypes.c_int
        try:
            analyze = self._dll.reloq_host_analyze_german
        except AttributeError:
            analyze = None  # An earlier ABI-1 DLL can still render frames.
        if analyze is not None:
            analyze.argtypes = (
                ctypes.c_void_p, ctypes.POINTER(ctypes.c_ubyte),
                ctypes.c_uint32, ctypes.POINTER(ctypes.c_ubyte),
                ctypes.c_uint32, ctypes.POINTER(_Phone), ctypes.c_uint32,
                ctypes.POINTER(_TextInfo),
            )
            analyze.restype = ctypes.c_int
        self._analyze = analyze
        config = _Config(
            struct_size=ctypes.sizeof(_Config), abi_version=ABI_VERSION,
            sample_rate_hz=sample_rate_hz,
            max_frame_samples=max_frame_samples,
            duration_scale=duration_scale, output_gain=output_gain,
            initial_seed=initial_seed, source_offset=source_offset,
            av_trim=av_trim, ah_trim=ah_trim, af_trim=af_trim,
        )
        host = ctypes.c_void_p()
        code = self._dll.reloq_host_create(ctypes.byref(config),
                                           ctypes.byref(host))
        if code != OK or not host.value:
            raise RuntimeError(f"cannot initialize numeric C renderer: {code}")
        self._host = host

    def reset(self) -> None:
        code = self._dll.reloq_host_reset(self._host)
        if code != OK:
            raise RuntimeError(f"cannot reset numeric C renderer: {code}")

    def cancel(self) -> None:
        self._dll.reloq_host_cancel(self._host)

    def analyze_german(self, text: str) -> GermanAnalysis | None:
        """Query and copy validated German SPR/phone context, without audio.

        Returns None for a cancelled generation. Raised GermanTextError
        carries the UTF-8 byte offset of an uncovered or invalid token.
        Render/analyze/reset/close belong to the same worker; only cancel
        may be called from another thread. Caller-owned lexicon entries are
        not yet exposed by this thin Python bridge.
        """
        if self._analyze is None:
            raise RuntimeError("this native DLL has no German text analysis export")
        encoded = text.encode("utf-8")
        if len(encoded) > 0xFFFFFFFF:
            raise ValueError("one native text call exceeds uint32 length")
        source = (ctypes.c_ubyte * (len(encoded) + 1)).from_buffer_copy(
            encoded + b"\0",
        )
        info = _TextInfo(struct_size=ctypes.sizeof(_TextInfo))
        native = self._analyze
        code = native(self._host, source, len(encoded), None, 0,
                      None, 0, ctypes.byref(info))
        if code == CANCELLED:
            return None
        if code in (TEXT_INVALID, TEXT_UNCOVERED):
            raise GermanTextError(code, info.error_offset)
        if code != CAPACITY:
            raise RuntimeError(f"German frontend query failed: {code}")
        annotation = (ctypes.c_ubyte * (info.annotation_bytes + 1))()
        phones = (_Phone * info.phone_count)()
        lengths = (info.annotation_bytes, info.phone_count,
                   info.word_count, info.syllable_count)
        info = _TextInfo(struct_size=ctypes.sizeof(_TextInfo))
        code = native(self._host, source, len(encoded), annotation,
                      len(annotation), phones, len(phones), ctypes.byref(info))
        if code == CANCELLED:
            return None
        if code != OK:
            raise RuntimeError(f"German frontend changed during copy: {code}")
        if lengths != (info.annotation_bytes, info.phone_count,
                       info.word_count, info.syllable_count):
            raise RuntimeError("German frontend counts changed during copy")
        return GermanAnalysis(
            annotation=bytes(annotation[:info.annotation_bytes]),
            phones=tuple(
                GermanPhone(*(getattr(phone, name) for name, _ in _Phone._fields_))
                for phone in phones
            ),
            word_count=info.word_count,
            syllable_count=info.syllable_count,
        )

    def render_controls(
        self, controls: Sequence[float], highest_formant: int,
        on_pcm: Callable[[bytes], None],
    ) -> tuple[int, int]:
        """Send flat arrays of 62 controls; copy each PCM block to on_pcm.

        Returns (native result, accepted frame count). The caller chooses
        whether CANCELLED is expected. A Python sink exception is re-raised
        after the native call, never swallowed by ctypes' callback boundary.
        """
        if len(controls) % FRAME_CONTROLS:
            raise ValueError("controls must contain full 62-value frames")
        highest_formant = _checked_int("highest_formant", highest_formant, 1, 8)
        count = len(controls) // FRAME_CONTROLS
        if count > 0xFFFFFFFF:
            raise ValueError("too many frames in one native call")
        values = (ctypes.c_float * len(controls))(*controls)
        error: list[BaseException] = []

        def deliver(_context: int, pcm: ctypes.POINTER(ctypes.c_int16),
                    samples: int) -> int:
            try:
                on_pcm(ctypes.string_at(pcm, samples * 2))
            except BaseException as exc:
                error.append(exc)
                return 1
            return 0

        callback = _PCMCallback(deliver)
        accepted = ctypes.c_uint32()
        code = self._dll.reloq_host_render_controls(
            self._host, values, count, highest_formant, callback, None,
            ctypes.byref(accepted),
        )
        if error:
            raise error[0]
        return code, accepted.value

    def close(self) -> None:
        host = self._host
        if host:
            self._host = None
            self._dll.reloq_host_destroy(host)


__all__ = [
    "NativeFrameEngine", "GermanAnalysis", "GermanPhone", "GermanTextError", "OK",
    "CANCELLED", "FRAME_CONTROLS",
]
