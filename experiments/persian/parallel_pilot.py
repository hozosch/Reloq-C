"""Independent, cancellable Persian contour capture for a single request.

The worker owns one ordinary acoustic handle and no text or speech results
between requests. Its thread-local scope and capture are independent of the
main NVDA worker. A caller must collect each submitted capture before issuing
another request or closing the worker.
"""
from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import threading

if __package__:
    from .._reloq_eci_direct import EciText
    from .._reloq_persian_frames import FrameCapture
    from .._reloq_persian_projection import ProjectedJob
    from .direct_fa_sequence import ScopedDirectPersianEngine, _safe_native_punctuation
else:
    from _reloq_eci_direct import EciText
    from _reloq_persian_frames import FrameCapture
    from _reloq_persian_projection import ProjectedJob
    from direct_fa_sequence import ScopedDirectPersianEngine, _safe_native_punctuation


class _CaptureVoice:
    def __init__(self, engine, prefix, validate, cancelled):
        self.engine, self.prefix, self.validate, self.cancelled = engine, prefix, validate, cancelled
        self._dll, self._handle = engine._voice._dll, engine._voice._handle

    def speak(self, parts, on_pcm, on_index):
        if self.cancelled.is_set():
            return False
        parts = (*self.prefix, *parts)
        tagged_parts = tuple(EciText(item, segmentation_guarded=True) if isinstance(item, str) else item
                             for item in parts)
        tagged = ''.join(item.text for item in tagged_parts
                         if isinstance(item, EciText) and not item.speed_control)
        prepared = _safe_native_punctuation(ProjectedJob('', tagged, tagged_parts, self.validate))
        return self.engine._speak_prepared(prepared, on_pcm, on_index) and not self.cancelled.is_set()


class ParallelPilot:
    def __init__(self, library):
        self._library = Path(library)
        self._executor = ThreadPoolExecutor(max_workers=1, thread_name_prefix='reloq.fa-contour')
        self._guard = threading.Lock()
        self._engine = None
        self._closed = False
        self._parameters = {}
        self._dictionary = None
        # Load only the handle, never speech results, while the owning thread
        # prepares this first long request. The queued capture still verifies
        # cancellation and settings before using the handle.
        self._executor.submit(self._open)

    def _open(self):
        if self._engine is not None:
            return
        engine = ScopedDirectPersianEngine(self._library, None)
        with self._guard:
            if self._closed:
                engine.close()
                raise InterruptedError('Persian contour worker closed')
            self._engine = engine

    def submit(self, *, parts, prefix, validate, parameters, dictionary, cancelled, maximum):
        with self._guard:
            if self._closed:
                raise RuntimeError('Persian contour worker is closed')
            return self._executor.submit(self._capture, tuple(parts), tuple(prefix), validate,
                                         dict(parameters), dictionary, cancelled, maximum)

    def _capture(self, parts, prefix, validate, parameters, dictionary, cancelled, maximum):
        if cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        if self._engine is None:
            self._open()
        engine = self._engine
        engine._cancelled.clear()
        engine._voice._cancelled.clear()
        if cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        if dictionary != self._dictionary:
            engine._voice.set_dictionary_path(dictionary)
            self._dictionary = dictionary
        for parameter, value in parameters.items():
            if self._parameters.get(parameter) != value:
                engine.set_parameter(parameter, value)
                self._parameters[parameter] = value
        if cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        voice = _CaptureVoice(engine, prefix, validate, cancelled)
        result = FrameCapture(voice._dll).capture(voice, parts, maximum=maximum, audio=True)
        if cancelled.is_set() or result is None:
            raise InterruptedError('sentence planning was cancelled')
        return result

    def cancel(self):
        with self._guard:
            engine = self._engine
            if engine is not None:
                engine.cancel()

    def close(self):
        with self._guard:
            if self._closed:
                return
            self._closed = True
            engine = self._engine
            if engine is not None:
                engine.cancel()
        def finish():
            with self._guard:
                engine, self._engine = self._engine, None
            if engine is not None:
                engine.close()
        try:
            self._executor.submit(finish).result()
        finally:
            self._executor.shutdown(wait=True)
