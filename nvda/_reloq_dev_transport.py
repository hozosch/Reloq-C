"""Development transport for precomputed C control frames; not a SynthDriver.

This class is deliberately not registered with NVDA. Its jobs already contain
the 62 acoustic controls per frame. Once the native text-to-PCM path exists,
the same FIFO, cancellation and playback ownership can carry UTF-8 text jobs.
"""

from __future__ import annotations

from collections import deque
from dataclasses import dataclass
import logging
import threading
import time
from typing import Callable, Protocol, Sequence


_OK = 0
_CANCELLED = 1
_CONTROLS_PER_FRAME = 62
_log = logging.getLogger(__name__)


class _NativeEngine(Protocol):
    def reset(self) -> None: ...
    def cancel(self) -> None: ...
    def render_controls(
        self, controls: Sequence[float], highest_formant: int,
        on_pcm: Callable[[bytes], None],
    ) -> tuple[int, int]: ...
    def close(self) -> None: ...


class _AudioPlayer(Protocol):
    def feed(self, pcm: bytes, onDone: Callable[[], None] | None = None) -> None: ...
    def stop(self) -> None: ...
    def pause(self, switch: bool) -> None: ...
    def close(self) -> None: ...


class _Discarded(Exception):
    pass


@dataclass(frozen=True)
class _Job:
    number: int
    generation: int
    controls: tuple[float, ...]
    highest_formant: int


class _PlaybackCompletion:
    """Notify once both C synthesis and the last queued PCM block finish."""

    def __init__(self, generation: int, number: int,
                 is_current: Callable[[int], bool],
                 notify_done: Callable[[int], None],
                 report_error: Callable[[Exception, int], None]) -> None:
        self._lock = threading.Lock()
        self._generation = generation
        self._number = number
        self._is_current = is_current
        self._notify_done = notify_done
        self._report_error = report_error
        self._rendered = False
        self._played = False
        self._announced = False

    def _finish_if_ready(self) -> None:
        with self._lock:
            if not self._played or not self._rendered or self._announced:
                return
            self._announced = True
        if self._is_current(self._generation):
            try:
                self._notify_done(self._number)
            except Exception as error:
                self._report_error(error, self._generation)

    def played(self) -> None:
        with self._lock:
            self._played = True
        self._finish_if_ready()

    def rendered(self) -> None:
        with self._lock:
            self._rendered = True
        self._finish_if_ready()


class FrameTransport:
    """FIFO for diagnostic numeric frames with one persistent native worker.

    ``submit_frames`` preserves separate utterances. ``cancel`` drops every
    pending utterance and stops the active renderer and player; a later submit
    starts fresh. ``notify_done`` runs after the last PCM block has played,
    never for a cancelled job. No playback sync is inserted between jobs.
    The engine is created, reset, rendered and
    destroyed on the worker thread. Only its atomic cancel runs elsewhere.

    The caller owns the player until ``shutdown`` returns True; the transport
    then closes it. A failed shutdown leaves the player open under the still
    running worker. This stage has no text synthesis or index timing.
    """

    def __init__(
        self, engine_factory: Callable[[], _NativeEngine],
        player: _AudioPlayer,
        notify_done: Callable[[int], None],
        notify_error: Callable[[Exception], None] | None = None,
    ) -> None:
        self._engine_factory = engine_factory
        self._player = player
        self._notify_done = notify_done
        self._notify_error = notify_error or self._log_error
        self._condition = threading.Condition()
        self._pending: deque[_Job] = deque()
        self._generation = 0
        self._next_number = 1
        self._closed = False
        self._stops_in_flight = 0
        self._player_closed = False
        self._engine: _NativeEngine | None = None
        self._thread = threading.Thread(
            target=self._run, name="reloq.dev-frames", daemon=True,
        )
        self._thread.start()

    @staticmethod
    def _log_error(error: Exception) -> None:
        _log.error("native frame transport failed", exc_info=error)

    def submit_frames(self, controls: Sequence[float], highest_formant: int) -> int:
        if len(controls) % _CONTROLS_PER_FRAME:
            raise ValueError("expected complete 62-control frames")
        if not 1 <= highest_formant <= 8:
            raise ValueError("highest_formant must be in 1..8")
        # Own the values before the caller can modify or release its buffer.
        values = tuple(controls)
        with self._condition:
            if self._closed:
                raise RuntimeError("transport has shut down")
            number = self._next_number
            self._next_number += 1
            self._pending.append(_Job(number, self._generation,
                                      values, highest_formant))
            self._condition.notify()
        return number

    def cancel(self) -> None:
        with self._condition:
            if self._closed:
                return
            self._generation += 1
            self._pending.clear()
            if self._engine is not None:
                self._engine.cancel()
            self._stops_in_flight += 1
        # stop may unblock a feed or idle call; never hold the queue lock here.
        try:
            self._player.stop()
        finally:
            with self._condition:
                self._stops_in_flight -= 1
                self._condition.notify_all()

    def pause(self, switch: bool) -> None:
        self._player.pause(switch)

    def shutdown(self, timeout: float = 3.0) -> bool:
        if threading.current_thread() is self._thread:
            raise RuntimeError("cannot join transport from its worker")
        deadline = time.monotonic() + timeout
        with self._condition:
            if self._player_closed:
                return True
            self._closed = True
            self._generation += 1
            self._pending.clear()
            if self._engine is not None:
                self._engine.cancel()
            self._condition.notify_all()
        self._player.stop()
        self._thread.join(max(0, deadline - time.monotonic()))
        if self._thread.is_alive():
            return False
        with self._condition:
            while self._stops_in_flight:
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    return False
                self._condition.wait(remaining)
            close_player = not self._player_closed
            self._player_closed = True
        if close_player:
            self._player.close()
        return True

    def _is_current(self, generation: int) -> bool:
        with self._condition:
            return not self._closed and generation == self._generation

    def _report_error(self, error: Exception, generation: int) -> None:
        if self._is_current(generation):
            try:
                self._notify_error(error)
            except Exception:
                _log.exception("error handler failed")

    def _run_job(self, engine: _NativeEngine, job: _Job) -> None:
        if not self._is_current(job.generation):
            return
        frame_count = len(job.controls) // _CONTROLS_PER_FRAME
        completion = _PlaybackCompletion(
            job.generation, job.number, self._is_current,
            self._notify_done, self._report_error,
        )
        fed = 0
        try:
            engine.reset()
            if not self._is_current(job.generation):
                return

            def on_pcm(pcm: bytes) -> None:
                nonlocal fed
                if not self._is_current(job.generation):
                    raise _Discarded()
                fed += 1
                if fed > frame_count:
                    raise RuntimeError("native renderer emitted too many PCM blocks")
                self._player.feed(
                    pcm, onDone=completion.played if fed == frame_count else None,
                )
                if not self._is_current(job.generation):
                    # cancel may race with the short interval between the
                    # pre-feed check and feed; remove any late old audio.
                    self._player.stop()
                    raise _Discarded()

            code, _completed = engine.render_controls(
                job.controls, job.highest_formant, on_pcm,
            )
            if not self._is_current(job.generation):
                return
            if code == _CANCELLED:
                raise RuntimeError("native renderer cancelled a current job")
            if code != _OK:
                raise RuntimeError(f"native renderer failed with {code}")
            if fed != frame_count:
                raise RuntimeError("native renderer emitted too few PCM blocks")
            completion.rendered()
            if frame_count == 0:
                completion.played()
        except _Discarded:
            pass
        except Exception as error:
            self._report_error(error, job.generation)

    def _run(self) -> None:
        try:
            engine = self._engine_factory()
        except Exception as error:
            with self._condition:
                self._closed = True
                self._pending.clear()
            try:
                self._notify_error(error)
            except Exception:
                _log.exception("error handler failed during startup")
            return
        with self._condition:
            self._engine = engine
        try:
            while True:
                with self._condition:
                    while (not self._pending or self._stops_in_flight) and not self._closed:
                        self._condition.wait()
                    if self._closed:
                        break
                    job = self._pending.popleft()
                self._run_job(engine, job)
        finally:
            with self._condition:
                self._engine = None
                engine.close()


__all__ = ["FrameTransport"]
