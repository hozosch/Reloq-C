"""Bounded request-scoped native frame capture for Persian continuation.

This is a planning building block. It does not itself change the language
driver, synthesize a transition, or retain a process-wide environment setting.
"""
from __future__ import annotations

import ctypes
from dataclasses import dataclass


def _native_function(dll, public, legacy):
    """Prefer the shared ECI API, retaining older private test libraries."""
    function = getattr(dll, public, None)
    return function if function is not None else getattr(dll, legacy, None)


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
        self._begin = _native_function(dll, 'eciPersianCaptureBegin', 'reloq_fa_capture_begin')
        if self._begin is None:
            raise AttributeError('native frame capture is not available')
        self._begin.argtypes, self._begin.restype = (ctypes.c_size_t,), ctypes.c_int
        self._end = _native_function(dll, 'eciPersianCaptureEnd', 'reloq_fa_capture_end')
        self._end.argtypes = (ctypes.POINTER(ctypes.c_size_t), ctypes.POINTER(ctypes.c_int))
        self._end.restype = ctypes.POINTER(NativeFrame)
        self._release = _native_function(dll, 'eciPersianCaptureRelease', 'reloq_fa_capture_release')
        self._release.argtypes, self._release.restype = (), None
        self._controls_only = _native_function(dll, 'eciPersianCaptureSetControlsOnly',
                                               'reloq_fa_capture_set_controls_only')
        if self._controls_only is not None:
            self._controls_only.argtypes, self._controls_only.restype = (ctypes.c_int,), ctypes.c_int
        self._sample_count = _native_function(dll, 'eciPersianCaptureSampleCount',
                                              'reloq_fa_capture_sample_count')
        if self._sample_count is not None:
            self._sample_count.argtypes, self._sample_count.restype = (), ctypes.c_size_t
        self._store_pcm = _native_function(dll, 'eciPersianCaptureSetPcmStorage',
                                           'reloq_fa_capture_set_pcm_storage')
        self._stored_pcm = _native_function(dll, 'eciPersianCapturePcm', 'reloq_fa_capture_pcm')
        if self._store_pcm is not None:
            self._store_pcm.argtypes, self._store_pcm.restype = (ctypes.c_int,), ctypes.c_int
        if self._stored_pcm is not None:
            self._stored_pcm.argtypes = (ctypes.POINTER(ctypes.c_size_t),)
            self._stored_pcm.restype = ctypes.POINTER(ctypes.c_int16)
        self.dense_curve = _native_function(dll, 'eciPersianDenseCurve', 'reloq_fa_dense_curve')
        if self.dense_curve is not None:
            self.dense_curve.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.c_uint,
                                         ctypes.POINTER(ctypes.c_float), ctypes.c_size_t)
            self.dense_curve.restype = ctypes.c_int
        self.fit_pitch = _native_function(dll, 'eciPersianFitPitch', 'reloq_fa_fit_pitch')
        if self.fit_pitch is not None:
            self.fit_pitch.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                                       ctypes.POINTER(ctypes.c_float), ctypes.c_size_t,
                                       ctypes.c_int64, ctypes.c_int64, ctypes.c_int64, ctypes.c_int64)
            self.fit_pitch.restype = ctypes.c_int
        self.fit_pitch_edges = _native_function(dll, 'eciPersianFitPitchEdges', 'reloq_fa_fit_pitch_edges')
        if self.fit_pitch_edges is not None:
            self.fit_pitch_edges.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                ctypes.POINTER(ctypes.c_float), ctypes.c_size_t, ctypes.POINTER(ctypes.c_float),
                ctypes.c_int64, ctypes.c_int64, ctypes.c_int64, ctypes.c_int64,
                ctypes.c_uint, ctypes.c_int64)
            self.fit_pitch_edges.restype = ctypes.c_int
        self.transfer_korean_pitch = _native_function(
            dll, 'eciPersianTransferKoreanPitch', 'reloq_fa_transfer_korean_pitch')
        if self.transfer_korean_pitch is not None:
            self.transfer_korean_pitch.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.c_uint, ctypes.c_int64)
            self.transfer_korean_pitch.restype = ctypes.c_int
        self.transfer_korean_aligned = _native_function(
            dll, 'eciPersianTransferKoreanAligned', 'reloq_fa_transfer_korean_aligned')
        if self.transfer_korean_aligned is not None:
            self.transfer_korean_aligned.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                ctypes.POINTER(NativeFrame), ctypes.c_size_t, ctypes.POINTER(ctypes.c_int64),
                ctypes.POINTER(ctypes.c_uint32), ctypes.c_size_t)
            self.transfer_korean_aligned.restype = ctypes.c_int
        self.last_audible_sample = _native_function(dll, 'eciPersianLastAudibleSample', 'reloq_fa_last_audible_sample')
        if self.last_audible_sample is not None:
            self.last_audible_sample.argtypes = (ctypes.c_char_p, ctypes.c_size_t)
            self.last_audible_sample.restype = ctypes.c_int64
        self.frame_ticks = _native_function(dll, 'eciPersianFrameTicks', 'reloq_fa_frame_ticks')
        if self.frame_ticks is not None:
            self.frame_ticks.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                                         ctypes.POINTER(ctypes.c_int64))
            self.frame_ticks.restype = ctypes.c_int
        self.frame_timeline = _native_function(dll, 'eciPersianFrameTimeline', 'reloq_fa_frame_timeline')
        if self.frame_timeline is not None:
            self.frame_timeline.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                                            ctypes.POINTER(ctypes.c_double))
            self.frame_timeline.restype = ctypes.c_int
        self.recompute_frication = _native_function(dll, 'eciPersianRecomputeFrication', 'reloq_fa_recompute_frication')
        if self.recompute_frication is not None:
            self.recompute_frication.argtypes = (ctypes.POINTER(NativeFrame), ctypes.c_size_t)
            self.recompute_frication.restype = None
        self.bridge_continuation = _native_function(dll, 'eciPersianBridgeContinuation', 'reloq_fa_bridge_continuation')
        if self.bridge_continuation is not None:
            self.bridge_continuation.argtypes = (ctypes.POINTER(NativeFrame), ctypes.POINTER(NativeFrame),
                ctypes.POINTER(ctypes.c_double), ctypes.POINTER(ctypes.c_size_t),
                ctypes.POINTER(ctypes.c_ubyte), ctypes.c_size_t, ctypes.c_size_t, ctypes.c_double,
                ctypes.POINTER(ctypes.c_ubyte))
            self.bridge_continuation.restype = ctypes.c_int

    def capture(self, voice, parts, *, maximum=60000, audio=True):
        if type(maximum) is not int or not 1 <= maximum <= 120000:
            raise ValueError('invalid frame capture limit')
        pcm, indexes = bytearray(), []
        if self._begin(maximum):
            raise RuntimeError('cannot start nested or oversized frame capture')
        try:
            if not audio and self._controls_only is not None and self._controls_only(1):
                raise RuntimeError('cannot enable control-only frame capture')
            silent_clock = not audio and self._controls_only is not None and self._sample_count is not None
            native_pcm = bool(audio and self._store_pcm is not None and self._stored_pcm is not None and
                              self._sample_count is not None)
            if native_pcm and self._store_pcm(1):
                raise RuntimeError('cannot enable native PCM capture')
            native_clock = silent_clock or native_pcm
            completed = voice.speak(parts, pcm.extend, lambda value: indexes.append(
                (value, self._sample_count() if native_clock else len(pcm) // 2)))
            sample_count = self._sample_count() if silent_clock else 0
            count, failed = ctypes.c_size_t(), ctypes.c_int()
            pointer = self._end(ctypes.byref(count), ctypes.byref(failed))
            if failed.value:
                raise RuntimeError('native frame capture exceeded its limit or failed')
            if not completed:
                return None
            if count.value and not pointer:
                raise RuntimeError('native frame buffer is missing')
            data = ctypes.string_at(pointer, count.value * ctypes.sizeof(NativeFrame)) if count.value else b''
            if native_pcm:
                stored_count = ctypes.c_size_t()
                stored = self._stored_pcm(ctypes.byref(stored_count))
                if stored_count.value and not stored:
                    raise RuntimeError('native PCM capture is missing')
                sound = ctypes.string_at(stored, stored_count.value * 2) if stored_count.value else b''
            else:
                sound = bytes(sample_count * 2) if silent_clock else bytes(pcm)
            return NativeCapture(sound, tuple(indexes), data)
        finally:
            # release is valid after either a normal end or a failed synthesis.
            # An exception can leave capture active, but release clears it too.
            self._release()


class FrameReplay:
    """Replay planned rows through the configured ECI text session once."""

    @staticmethod
    def available(dll):
        return all(_native_function(dll, public, legacy) is not None for public, legacy in (
            ('eciPersianReplayEpoch', 'reloq_fa_replay_epoch'),
            ('eciPersianReplayFrames', 'reloq_fa_replay_frames')))

    def __init__(self, dll):
        self._epoch = _native_function(dll, 'eciPersianReplayEpoch', 'reloq_fa_replay_epoch')
        self._epoch.argtypes, self._epoch.restype = (ctypes.c_void_p,), ctypes.c_uint32
        self._play = _native_function(dll, 'eciPersianReplayFrames', 'reloq_fa_replay_frames')
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


class FrameStream:
    """Replay consecutive frame slices without resetting the native session."""

    @staticmethod
    def available(dll):
        return all(_native_function(dll, public, legacy) is not None for public, legacy in (
            ('eciPersianReplayBegin', 'reloq_fa_replay_begin'),
            ('eciPersianReplayAppend', 'reloq_fa_replay_append'),
            ('eciPersianReplayEnd', 'reloq_fa_replay_end')))

    def __init__(self, voice, on_pcm, on_index, *, cancelled=None):
        voice._assert_owner()
        self.voice, self.cancelled, self.state = voice, cancelled, ctypes.c_void_p()
        dll = voice._dll
        self._begin = _native_function(dll, 'eciPersianReplayBegin', 'reloq_fa_replay_begin')
        self._append = _native_function(dll, 'eciPersianReplayAppend', 'reloq_fa_replay_append')
        self._end = _native_function(dll, 'eciPersianReplayEnd', 'reloq_fa_replay_end')
        self._begin.argtypes = (ctypes.c_void_p, ctypes.c_uint32, ctypes.POINTER(ctypes.c_void_p))
        self._begin.restype = ctypes.c_int
        self._append.argtypes = (ctypes.c_void_p, ctypes.POINTER(NativeFrame), ctypes.c_size_t,
                                 ctypes.POINTER(FrameMark), ctypes.c_size_t)
        self._append.restype = ctypes.c_int
        self._end.argtypes, self._end.restype = (ctypes.c_void_p, ctypes.c_int), ctypes.c_int
        if not voice._handle or (cancelled is not None and cancelled.is_set()):
            raise InterruptedError('native frame stream was cancelled')
        voice._cancelled.clear()
        epoch = FrameReplay(dll)._epoch(voice._handle)
        voice._callback_error = None
        voice._on_pcm, voice._on_index = on_pcm, on_index
        try:
            status = self._begin(voice._handle, epoch, ctypes.byref(self.state))
            if status == 1 or voice._cancelled.is_set() or (cancelled is not None and cancelled.is_set()):
                raise InterruptedError('native frame stream was cancelled')
            if status or not self.state:
                raise RuntimeError(f'cannot start native frame stream: {status}')
        except BaseException:
            if self.state:
                self._end(self.state, 0)
                self.state = ctypes.c_void_p()
            voice._on_pcm = voice._on_index = None
            raise

    def append(self, frames, first, end, marks=()):
        if not self.state:
            raise RuntimeError('native frame stream is closed')
        if self.cancelled is not None and self.cancelled.is_set():
            raise InterruptedError('native frame stream was cancelled')
        if (not isinstance(frames, ctypes.Array) or getattr(frames, '_type_', None) is not NativeFrame
                or type(first) is not int or type(end) is not int or not 0 <= first <= end <= len(frames)):
            raise ValueError('invalid native frame slice')
        marks = tuple(marks)
        if (any(type(at) is not int or type(value) is not int or not 0 <= at <= end - first
                or not 0 <= value <= 0x7fffffff for at, value in marks) or
                any(a[0] > b[0] for a, b in zip(marks, marks[1:]))):
            raise ValueError('invalid native frame marks')
        native_marks = (FrameMark * len(marks))(*(FrameMark(*mark) for mark in marks)) if marks else None
        pointer = ctypes.cast(ctypes.byref(frames, first * ctypes.sizeof(NativeFrame)),
                              ctypes.POINTER(NativeFrame)) if first < end else None
        status = self._append(self.state, pointer, end - first, native_marks, len(marks))
        if self.voice._callback_error is not None:
            raise self.voice._callback_error
        if status == 1 or self.voice._cancelled.is_set() or (self.cancelled and self.cancelled.is_set()):
            raise InterruptedError('native frame stream was cancelled')
        if status:
            raise RuntimeError(f'native frame stream failed: {status}')

    def close(self, *, flush=False):
        if not self.state:
            return
        state, self.state = self.state, ctypes.c_void_p()
        try:
            status = self._end(state, int(bool(flush)))
            if self.voice._callback_error is not None:
                raise self.voice._callback_error
            if status == 1 or self.voice._cancelled.is_set() or (self.cancelled and self.cancelled.is_set()):
                raise InterruptedError('native frame stream was cancelled')
            if status:
                raise RuntimeError(f'native frame stream ended with error: {status}')
        finally:
            self.voice._on_pcm = self.voice._on_index = None
