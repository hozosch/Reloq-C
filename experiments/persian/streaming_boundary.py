"""Experimental streaming silence handoff for separate language handles.

Only an actually quiet tail is buffered. Audio that contains speech is
forwarded immediately, and index callbacks are kept in order with the held
audio. This is a probe, not yet connected to NVDA or the Persian frontend.
"""
from __future__ import annotations

from collections.abc import Callable
import threading

import numpy as np


class StreamingBoundary:
    def __init__(self, rate: int, on_pcm: Callable[[bytes], None],
                 on_index: Callable[[int], None]) -> None:
        if not isinstance(rate, int) or rate <= 0:
            raise ValueError('sample rate must be positive')
        self.rate = rate
        self.on_pcm, self.on_index = on_pcm, on_index
        self._quiet = bytearray()
        self._indexes: list[tuple[int, int]] = []  # sample offset within quiet
        self._sounded = False
        self._long_silence = False
        self._cancelled = threading.Event()
        self._lock = threading.RLock()

    def _discard(self) -> None:
        self._quiet.clear()
        self._indexes.clear()

    def _flush(self, samples: int) -> None:
        cursor = 0
        pending = []
        for offset, index in self._indexes:
            if offset <= samples:
                if offset > cursor:
                    self.on_pcm(bytes(self._quiet[2 * cursor:2 * offset]))
                    if self._cancelled.is_set():
                        self._discard()
                        return
                self.on_index(index)
                if self._cancelled.is_set():
                    self._discard()
                    return
                cursor = offset
            else:
                pending.append((offset - samples, index))
        if samples > cursor:
            self.on_pcm(bytes(self._quiet[2 * cursor:2 * samples]))
            if self._cancelled.is_set():
                self._discard()
                return
        del self._quiet[:2 * samples]
        self._indexes = pending

    def audio(self, pcm: bytes) -> None:
        with self._lock:
            if self._cancelled.is_set():
                self._discard()
                return
            if len(pcm) % 2:
                raise ValueError('expected 16-bit mono PCM')
            if not pcm:
                return
            samples = np.frombuffer(pcm, dtype='<i2').astype(np.int32)
            audible = np.flatnonzero(np.abs(samples) > 4)
            if len(audible):
                self._flush(len(self._quiet) // 2)
                if self._cancelled.is_set():
                    self._discard()
                    return
                last = int(audible[-1]) + 1
                self.on_pcm(pcm[:2 * last])
                if self._cancelled.is_set():
                    self._discard()
                    return
                self._quiet.extend(pcm[2 * last:])
                self._sounded = True
                self._long_silence = False
            else:
                self._quiet.extend(pcm)
                # Once a pause exceeds 500 ms it is treated as intentional;
                # forward it instead of retaining an unbounded stream.
                if self._long_silence or len(self._quiet) // 2 > self.rate // 2:
                    self._long_silence = True
                    self._flush(len(self._quiet) // 2)

    def index(self, index: int) -> None:
        with self._lock:
            if self._cancelled.is_set():
                self._discard()
                return
            if self._quiet:
                self._indexes.append((len(self._quiet) // 2, index))
            else:
                self.on_index(index)

    def end_part(self, text: str, *, switch: bool, final_stop: bool = False) -> None:
        with self._lock:
            if self._cancelled.is_set():
                self._discard()
                return
            boundary = text.rstrip().endswith(('.', '?', '!', ',', ';', ':', '،', '؟', '؛'))
            if switch and self._sounded and not self._long_silence and not boundary:
                keep = min(len(self._quiet) // 2, round(self.rate * (.070 if final_stop else .025)))
                del self._quiet[2 * keep:]
                self._indexes = [(min(offset, keep), index) for offset, index in self._indexes]
            self._flush(len(self._quiet) // 2)
            self._sounded = False
            self._long_silence = False

    def cancel(self) -> None:
        # Do not wait for a potentially blocked audio callback. A callback
        # already in progress checks the event and discards the held suffix.
        self._cancelled.set()
        if self._lock.acquire(blocking=False):
            try:
                self._discard()
            finally:
                self._lock.release()
