"""Bounded request-scoped native frame capture for Persian continuation.

This is a planning building block. It does not itself change the language
driver, synthesize a transition, or retain a process-wide environment setting.
"""
from __future__ import annotations

import ctypes
from dataclasses import dataclass


class NativeFrame(ctypes.Structure):
    _fields_ = (
        ('tick', ctypes.c_int64),
        ('controls', ctypes.c_float * 62),
        ('gain', ctypes.c_float),
        ('formants', ctypes.c_int32),
        ('head', ctypes.c_int32),
        ('gender', ctypes.c_int32),
        ('s_frication', ctypes.c_int32),
        ('limit_frication', ctypes.c_int32),
    )


class FrameMark(ctypes.Structure):
    _fields_ = (('frame', ctypes.c_uint32), ('value', ctypes.c_int32))


@dataclass(frozen=True)
class NativeCapture:
    pcm: bytes
    indexes: tuple[tuple[int, int], ...]
    frame_bytes: bytes

    def frames(self):
        """Return an owned, mutable native row array in its original order."""
        size = ctypes.sizeof(NativeFrame)
        if len(self.frame_bytes) % size:
            raise ValueError('invalid native frame buffer')
        return (NativeFrame * (len(self.frame_bytes) // size)).from_buffer_copy(self.frame_bytes)


class FrameCapture:
    """Capture one voice's synthesis without modifying another thread's plan."""

    def __init__(self, dll):
        self._begin = dll.reloq_fa_capture_begin
        self._begin.argtypes, self._begin.restype = (ctypes.c_size_t,), ctypes.c_int
        self._end = dll.reloq_fa_capture_end
        self._end.argtypes = (ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_int))
        self._end.restype = ctypes.POINTER(NativeFrame)
        self._release = dll.reloq_fa_capture_release
        self._release.argtypes, self._release.restype = (), None

    def capture(self, voice, parts, *, maximum=60000):
        if type(maximum) is not int or not 1 <= maximum <= 120000:
            raise ValueError('invalid frame capture limit')
        pcm, indexes = bytearray(), []
        if self._begin(maximum):
            raise RuntimeError('cannot start nested or oversized frame capture')
        try:
            completed = voice.speak(parts, pcm.extend,
                                    lambda value: indexes.append((value, len(pcm) // 2)))
            count, failed = ctypes.c_size_t(), ctypes.c_int()
            pointer = self._end(ctypes.byref(count), ctypes.byref(failed))
            if failed.value:
                raise RuntimeError('native frame capture exceeded its limit or failed')
            if not completed:
                return None
            if count.value and not pointer:
                raise RuntimeError('native frame buffer is missing')
            data = ctypes.string_at(pointer, count.value * ctypes.sizeof(NativeFrame)) if count.value else b''
            return NativeCapture(bytes(pcm), tuple(indexes), data)
        finally:
            # release is valid after either a normal end or a failed synthesis.
            # An exception can leave capture active, but release clears it too.
            self._release()


class FrameReplay:
    """Replay planned rows through the configured ECI text session once."""

    def __init__(self, dll):
        self._epoch = dll.reloq_fa_replay_epoch
        self._epoch.argtypes, self._epoch.restype = (ctypes.c_void_p,), ctypes.c_uint32
        self._play = dll.reloq_fa_replay_frames
        self._play.argtypes = (ctypes.c_void_p, ctypes.c_uint32, ctypes.POINTER(NativeFrame),
                               ctypes.c_size_t, ctypes.POINTER(FrameMark), ctypes.c_size_t)
        self._play.restype = ctypes.c_int

    def replay(self, voice, frames, marks, on_pcm, on_index, *, cancelled=None):
        voice._assert_owner()
        if not voice._handle:
            raise RuntimeError('ECI engine has been closed')
        if cancelled is not None and cancelled.is_set():
            return False
        if not isinstance(frames, ctypes.Array) or getattr(frames, '_type_', None) is not NativeFrame:
            raise TypeError('expected an owned native frame array')
        marks = tuple(marks)
        if any(type(at) is not int or type(value) is not int or not 0 <= at <= len(frames)
               or not 0 <= value <= 0x7fffffff for at, value in marks):
            raise ValueError('invalid replay mark')
        if any(a[0] > b[0] for a, b in zip(marks, marks[1:])):
            raise ValueError('replay marks are not ordered')
        native_marks = (FrameMark * len(marks))(*(FrameMark(*mark) for mark in marks)) if marks else None
        voice._cancelled.clear()
        epoch = self._epoch(voice._handle)
        if voice._cancelled.is_set() or (cancelled is not None and cancelled.is_set()):
            return False
        voice._callback_error = None
        voice._on_pcm, voice._on_index = on_pcm, on_index
        try:
            result = self._play(voice._handle, epoch, frames, len(frames), native_marks, len(marks))
            if voice._callback_error is not None:
                raise voice._callback_error
            if result == 1 or voice._cancelled.is_set() or (cancelled is not None and cancelled.is_set()):
                return False
            if result:
                raise RuntimeError(f'native continuous replay failed: {result}')
            return True
        finally:
            voice._on_pcm = voice._on_index = None
