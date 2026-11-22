"""Independent, cancellable US contour capture for a Persian request.

The worker owns one ordinary acoustic handle and no text or speech results
between requests. Its thread-local scope and capture are independent of the
main NVDA worker. A caller must collect each submitted capture before issuing
another request or closing the worker.
"""
from __future__ import annotations

from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from dataclasses import replace
import threading

if __package__:
    from .._reloq_eci_direct import DirectEciEngine, EciText, PROCESS_TAGS
    from .._reloq_persian_frames import FrameCapture
    from .._reloq_persian_projection import ProjectedJob
    from .direct_fa_sequence import ScopedDirectPersianEngine, _safe_native_punctuation
else:
    from _reloq_eci_direct import DirectEciEngine, EciText, PROCESS_TAGS
    from _reloq_persian_frames import FrameCapture
    from _reloq_persian_projection import ProjectedJob
    from direct_fa_sequence import ScopedDirectPersianEngine, _safe_native_punctuation


class _CaptureVoice:
    def __init__(self, engine, prefix, validate, cancelled, source=''):
        self.engine, self.prefix, self.validate, self.cancelled = engine, prefix, validate, cancelled
        self.source = source
        self._request_phones = {}
        raw = engine._voice if isinstance(engine, ScopedDirectPersianEngine) else engine
        self._dll, self._handle = raw._dll, raw._handle

    def _phonemes_for_probe(self, text):
        if self.cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        if text in self._request_phones:
            return self._request_phones[text]
        result = self.engine._phonemes_for_probe(text)
        if self.cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        self._request_phones[text] = result
        return result

    def speak(self, parts, on_pcm, on_index):
        if self.cancelled.is_set():
            return False
        parts = (*self.prefix, *parts)
        if not isinstance(self.engine, ScopedDirectPersianEngine):
            return self.engine.speak(parts, on_pcm, on_index) and not self.cancelled.is_set()
        def normalized_space(value):
            return ''.join(' ' if ord(char) > 127 and char.isspace() else char for char in value)
        tagged_parts = tuple(EciText(normalized_space(item), segmentation_guarded=True)
                             if isinstance(item, str) else
                             replace(item, text=normalized_space(item.text))
                             if isinstance(item, EciText) and not item.is_control else item for item in parts)
        tagged = ''.join(item.text for item in tagged_parts
                         if isinstance(item, EciText) and not item.is_control)
        prepared = _safe_native_punctuation(ProjectedJob(self.source, tagged, tagged_parts, self.validate))
        return self.engine._speak_prepared(prepared, on_pcm, on_index,
                                           neutralize_stress=False) and not self.cancelled.is_set()


class ParallelPilot:
    def __init__(self, library, *, persian=False):
        self._library = Path(library)
        self._persian = persian
        self._executor = ThreadPoolExecutor(max_workers=1, thread_name_prefix='reloq.fa-contour')
        self._guard = threading.Lock()
        self._engine = None
        self._closed = False
        self._parameters = {}
        self._dictionary = None
        # Load only the handle, never speech results, while the owning thread
        # prepares this first long request. The queued capture still verifies
        # cancellation and settings before using the handle.
        try:
            self._initializing = self._executor.submit(self._open)
        except BaseException:
            self._executor.shutdown(wait=False)
            raise

    def wait_ready(self):
        """Finish optional voice-selection warmup on the worker's thread."""
        self._initializing.result()

    def _open(self):
        if self._engine is not None:
            return
        engine = (ScopedDirectPersianEngine(self._library, None) if self._persian else
                  DirectEciEngine(self._library, dialect=0x10000, encoding='utf8', native_voice=True))
        engine.set_parameter(PROCESS_TAGS, 1)
        with self._guard:
            if self._closed:
                engine.close()
                raise InterruptedError('Persian contour worker closed')
            self._engine = engine

    def submit(self, *, parts, prefix, validate, parameters, dictionary, cancelled, maximum, audio=True):
        with self._guard:
            if self._closed:
                raise RuntimeError('Persian contour worker is closed')
            return self._executor.submit(self._capture, tuple(parts), tuple(prefix), validate,
                                         dict(parameters), dictionary, cancelled, maximum, audio)

    def submit_proxy(self, *, tagged, marks, prefix, parameters, dictionary, cancelled, maximum):
        """Build and capture a US pilot entirely on its independent handle."""
        with self._guard:
            if self._closed:
                raise RuntimeError('Persian contour worker is closed')
            return self._executor.submit(self._capture_proxy, tagged, tuple(marks), tuple(prefix),
                                         dict(parameters), dictionary, cancelled, maximum)

    def submit_snapshot(self, *, parts, prefix, validate, source, parameters, dictionary,
                        cancelled, maximum, snapshot_at, on_snapshot, audio=False):
        with self._guard:
            if self._closed:
                raise RuntimeError('Persian contour worker is closed')
            return self._executor.submit(self._capture, tuple(parts), tuple(prefix), validate,
                                         dict(parameters), dictionary, cancelled, maximum, audio,
                                         snapshot_at, on_snapshot, source)

    def submit_proxy_snapshot(self, *, tagged, marks, prefix, parameters, dictionary,
                              cancelled, maximum, snapshot_at, on_snapshot):
        with self._guard:
            if self._closed:
                raise RuntimeError('Persian contour worker is closed')
            return self._executor.submit(self._capture_proxy, tagged, tuple(marks), tuple(prefix),
                                         dict(parameters), dictionary, cancelled, maximum,
                                         snapshot_at, on_snapshot)

    def _prepare_engine(self, parameters, dictionary, cancelled):
        if cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        if self._engine is None:
            self._open()
        engine = self._engine
        engine._cancelled.clear()
        if isinstance(engine, ScopedDirectPersianEngine):
            engine._voice._cancelled.clear()
        if cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        if dictionary != self._dictionary:
            (engine._voice if isinstance(engine, ScopedDirectPersianEngine) else engine).set_dictionary_path(dictionary)
            self._dictionary = dictionary
        for parameter, value in parameters.items():
            if parameter == PROCESS_TAGS:
                continue  # The generated proxy always uses private phoneme tags.
            if self._parameters.get(parameter) != value:
                engine.set_parameter(parameter, value)
                self._parameters[parameter] = value
        if cancelled.is_set():
            raise InterruptedError('sentence planning was cancelled')
        return engine

    def _capture_proxy(self, tagged, marks, prefix, parameters, dictionary, cancelled, maximum,
                       snapshot_at=None, on_snapshot=None):
        engine = self._prepare_engine(parameters, dictionary, cancelled)
        if __package__:
            from .continuous_plan import indexed_us_proxy
        else:
            from continuous_plan import indexed_us_proxy
        voice = _CaptureVoice(engine, (), False, cancelled)
        proxy = indexed_us_proxy(tagged, voice, marks)
        return self._capture(proxy, prefix, False, parameters, dictionary, cancelled, maximum, False,
                             snapshot_at, on_snapshot)

    def _capture(self, parts, prefix, validate, parameters, dictionary, cancelled, maximum, audio,
                 snapshot_at=None, on_snapshot=None, source=''):
        engine = self._prepare_engine(parameters, dictionary, cancelled)
        voice = _CaptureVoice(engine, prefix, validate, cancelled, source)
        options = {'maximum': maximum, 'audio': audio}
        if snapshot_at is not None:
            options.update(snapshot_at=snapshot_at, on_snapshot=on_snapshot)
        result = FrameCapture(voice._dll).capture(voice, parts, **options)
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
