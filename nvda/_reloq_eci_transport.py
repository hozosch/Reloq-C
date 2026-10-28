"""Direct-ECI text and bounded playback queues for the NVDA SynthDriver.

It has no helper process, NVDA import, or bundled voice. The persistent worker
owns the ECI handle, while playback can overlap the next text job. Indices and
completion normally follow playback. Shortened-pause modes hand off the
next NVDA chunk after the final feed, as in the OpenEVV driver.
"""

from __future__ import annotations

from collections import deque
from dataclasses import dataclass, field
import logging
import threading
import time
from typing import Callable, Protocol, Sequence

if __package__:
    from ._reloq_eci_direct import EciIndex, EciPitch, EciLanguage, EciText, SAMPLE_RATE, SAMPLE_RATES, PAUSE_MODE
else:  # Standalone development tests run from this directory.
    from _reloq_eci_direct import EciIndex, EciPitch, EciLanguage, EciText, SAMPLE_RATE, SAMPLE_RATES, PAUSE_MODE


_log = logging.getLogger(__name__)


class _EciEngine(Protocol):
    def speak(self, parts: Sequence[str | EciIndex | EciPitch | EciLanguage],
              on_pcm: Callable[[bytes], None],
              on_index: Callable[[int], None]) -> bool: ...
    def cancel(self) -> None: ...
    def set_parameter(self, parameter: int, value: int) -> None: ...
    def select_voice(self, language: int, voice: str) -> None: ...
    def set_dictionary_path(self, path: str | None) -> None: ...
    def close(self) -> None: ...


class _Player(Protocol):
    def feed(self, pcm: bytes, onDone: Callable[[], None] | None = None) -> None: ...
    def idle(self) -> None: ...
    def stop(self) -> None: ...
    def pause(self, switch: bool) -> None: ...
    def close(self) -> None: ...


@dataclass(frozen=True)
class _Job:
    number: int
    generation: int
    parts: tuple[str | EciIndex | EciPitch | EciLanguage, ...]
    finish_sequence: bool


@dataclass(frozen=True)
class _Control:
    parameter: int
    value: int


@dataclass(frozen=True)
class _DictionaryPath:
    path: str | None


@dataclass(frozen=True)
class _Selection:
    language: int
    voice: str
    parameters: tuple[tuple[int, int], ...] = ()


@dataclass(frozen=True)
class _SampleRate:
    code: int
    factory: Callable[[int], _Player]
    completed: Callable[[int], None] | None = None


@dataclass(frozen=True)
class _Marker:
    generation: int
    audio_serial: int
    kind: str
    value: int


class _Discarded(Exception):
    pass


@dataclass
class _PlaybackBatch:
    generation: int
    pending: int = 0
    error: Exception | None = None


@dataclass(frozen=True)
class _Playback:
    batch: _PlaybackBatch
    action: Callable[[], None]


@dataclass
class _PlayerCall:
    generation: int
    operation: str
    player: _Player
    deadline: float
    guard: threading.Lock = field(default_factory=threading.Lock)
    finished: bool = False
    stopped: bool = False


class EciTextTransport:
    """FIFO across ECI synthesis and audio playback, with generation cancel.

    The owner supplies an in-process ECI engine factory, a WavePlayer-like
    object, and index/done hooks. Each PCM buffer gets a playback completion
    callback; an index or utterance completion is deferred until all earlier
    buffers across job boundaries have played. Pause modes 1/2 instead report
    outstanding indices followed by completion after the last feed returns;
    NVDA can prepare the next chunk while the final player buffer plays.
    The player is owned by this transport after construction.
    """

    def __init__(self, engine_factory: Callable[[], _EciEngine], player: _Player,
                 notify_index: Callable[[int], None],
                 notify_done: Callable[[int], None],
                 notify_error: Callable[[Exception], None] | None = None,
                 *, complete_on_error: bool = False,
                 first_pcm_bytes: int = 882,
                 pcm_batch_bytes: int = 1764,
                 final_pcm_min_bytes: int = 882,
                 player_stall_seconds: float = 5.0,
                 playback_queue_size: int = 64) -> None:
        if (first_pcm_bytes <= 0 or pcm_batch_bytes < first_pcm_bytes or
                final_pcm_min_bytes <= 0 or
                final_pcm_min_bytes > pcm_batch_bytes or
                player_stall_seconds <= 0 or playback_queue_size < 1):
            raise ValueError("PCM batch sizes must be positive and ordered")
        self._factory = engine_factory
        self._player = player
        self._notify_index = notify_index
        self._notify_done = notify_done
        self._notify_error = notify_error or self._log_error
        self._complete_on_error = complete_on_error
        # Native ECI emits one small callback per acoustic frame (often just
        # 11 samples). Feeding each frame to WavePlayer can leave over a
        # thousand pending completion callbacks for one short sentence. Keep
        # a 40-ms initial reserve and 80-ms subsequent batches. As in
        # OpenEVV, a bounded playback worker lets ECI prepare the following
        # buffers while WavePlayer waits for room. Early indices retain
        # their PCM positions without starting playback with tiny frames.
        self._base_batch_sizes = (first_pcm_bytes, pcm_batch_bytes, final_pcm_min_bytes)
        self._sample_rate_code = 1
        self._first_pcm_bytes = first_pcm_bytes
        self._pcm_batch_bytes = pcm_batch_bytes
        # Keep enough PCM for a substantial final feed. NVDA's done event
        # depends on the final WavePlayer onDone callback; attaching it to a
        # last few milliseconds of audio is fragile on some output devices.
        self._final_pcm_min_bytes = final_pcm_min_bytes
        self._player_stall_seconds = player_stall_seconds
        state_lock = threading.RLock()
        self._condition = threading.Condition(state_lock)
        # Share the state lock, not queue notifications. The watchdog has no
        # work on ordinary text/audio queue changes or completed player calls.
        self._watch_condition = threading.Condition(state_lock)
        self._watch_deadline: float | None = None
        self._notification_lock = threading.RLock()
        self._pause_mode = 0
        self._jobs: deque[_Job | _Control | _Selection | _SampleRate | _DictionaryPath] = deque()
        self._audio: deque[_Playback] = deque()
        self._active_playback: _PlaybackBatch | None = None
        self._audio_limit = playback_queue_size
        self._markers: deque[_Marker] = deque()
        self._generation = 0
        self._number = 0
        self._last_audio_serial = 0
        self._played_serial = 0
        self._played_out_of_order: set[int] = set()
        self._engine: _EciEngine | None = None
        self._closed = False
        self._player_closed = False
        self._paused = False
        self._stops_in_flight = 0
        self._watched_call: _PlayerCall | None = None
        self._watchdog_worker = threading.Thread(target=self._watch_player,
                                                 name="reloq.eci-watch", daemon=True)
        self._audio_worker = threading.Thread(target=self._play_audio,
                                              name="reloq.eci-audio", daemon=True)
        self._worker = threading.Thread(target=self._run, name="reloq.eci-text",
                                        daemon=True)
        self._watchdog_worker.start()
        self._audio_worker.start()
        self._worker.start()

    def _queue_playback(self, batch: _PlaybackBatch,
                        action: Callable[[], None]) -> None:
        with self._condition:
            while (len(self._audio) >= self._audio_limit and
                   not self._closed and batch.generation == self._generation and
                   batch.error is None):
                self._condition.wait()
            if self._closed or batch.generation != self._generation:
                raise _Discarded()
            if batch.error is not None:
                raise batch.error
            batch.pending += 1
            self._audio.append(_Playback(batch, action))
            self._condition.notify_all()

    def _wait_playback(self, batch: _PlaybackBatch, *, errors: bool = True) -> None:
        # This is the OpenEVV ready barrier: completion in shortened-pause
        # modes must follow the last real feed, not merely queue insertion.
        with self._condition:
            while (batch.pending and not self._closed and
                   batch.generation == self._generation):
                self._condition.wait()
            if errors:
                if self._closed or batch.generation != self._generation:
                    raise _Discarded()
                if batch.error is not None:
                    raise batch.error

    def _clear_audio_locked(self) -> None:
        while self._audio:
            self._audio.popleft().batch.pending -= 1
        self._condition.notify_all()

    def _play_audio(self) -> None:
        while True:
            with self._condition:
                while not self._audio and not self._closed:
                    self._condition.wait()
                if self._closed:
                    return
                item = self._audio.popleft()
                self._active_playback = item.batch
                self._condition.notify_all()
            try:
                if (self._is_current(item.batch.generation) and
                        item.batch.error is None):
                    item.action()
            except _Discarded:
                pass
            except Exception as error:
                with self._condition:
                    if item.batch.error is None:
                        item.batch.error = error
            finally:
                with self._condition:
                    item.batch.pending -= 1
                    self._active_playback = None
                    self._condition.notify_all()

    @staticmethod
    def _log_error(error: Exception) -> None:
        _log.error("direct ECI transport failed", exc_info=error)

    def submit(self, parts: Sequence[str | EciIndex | EciPitch | EciLanguage], *,
               finish_sequence: bool = True) -> int:
        snapshot = tuple(parts)
        if not all(isinstance(item, (str, EciIndex, EciPitch, EciLanguage, EciText)) for item in snapshot):
            raise TypeError("expected text, index, pitch or language parts")
        with self._condition:
            if self._closed:
                raise RuntimeError("transport has shut down")
            self._number += 1
            number = self._number
            self._jobs.append(_Job(number, self._generation, snapshot,
                                   finish_sequence))
            self._condition.notify_all()
            return number

    def select_voice(self, language: int, voice: str, *, parameters=()) -> None:
        """Serialize the home voice with speech and other persistent controls."""
        parameters = tuple(parameters)
        with self._condition:
            if self._closed:
                raise RuntimeError("transport has shut down")
            self._jobs.append(_Selection(language, voice, parameters))
            self._condition.notify_all()

    def set_parameter(self, parameter: int, value: int) -> None:
        """Apply a real ECI setting on the worker, in queue order.

        Control items survive cancellation so NVDA's visible setting still
        reaches ECI during rapid navigation or a speech interruption.
        """
        with self._condition:
            if self._closed:
                raise RuntimeError("transport has shut down")
            self._jobs.append(_Control(parameter, value))
            self._condition.notify_all()

    def set_dictionary_path(self, path) -> None:
        with self._condition:
            if self._closed:
                raise RuntimeError("transport has shut down")
            self._jobs.append(_DictionaryPath(str(path) if path is not None else None))
            self._condition.notify_all()

    def set_sample_rate(self, code: int, player_factory: Callable[[int], _Player],
                        completed: Callable[[int], None] | None = None) -> None:
        """Switch both the native clock and playback format on their owner worker."""
        if code not in SAMPLE_RATES:
            raise ValueError("unsupported sample rate")
        with self._condition:
            if self._closed:
                raise RuntimeError("transport has shut down")
            self._jobs.append(_SampleRate(code, player_factory, completed))
            self._condition.notify_all()

    def _change_sample_rate(self, engine: _EciEngine, job: _SampleRate) -> None:
        if job.code == self._sample_rate_code:
            return
        hz = SAMPLE_RATES[job.code]
        next_player = job.factory(hz)
        try:
            with self._condition:
                # cancel/shutdown capture their player while acquiring this
                # lock. Never close it until their external stop has returned.
                while self._stops_in_flight or self._active_playback is not None:
                    self._condition.wait()
                if self._closed:
                    return
                next_player.pause(self._paused)
                engine.set_parameter(SAMPLE_RATE, job.code)
                previous, self._player = self._player, next_player
                next_player = None
                self._sample_rate_code = job.code
                sizes = tuple(max(2, 2 * round(size * hz / 22050))
                              for size in self._base_batch_sizes)
                (self._first_pcm_bytes, self._pcm_batch_bytes,
                 self._final_pcm_min_bytes) = sizes
            previous.close()
        finally:
            if next_player is not None:
                next_player.close()

    def _is_current(self, generation: int) -> bool:
        with self._condition:
            return not self._closed and generation == self._generation

    def _report_error(self, error: Exception, generation: int) -> None:
        if self._is_current(generation):
            try:
                self._notify_error(error)
            except Exception:
                _log.exception("direct ECI error hook failed")

    def _ready_markers_locked(self) -> list[_Marker]:
        ready = []
        while self._markers and self._markers[0].audio_serial <= self._played_serial:
            marker = self._markers.popleft()
            if marker.generation == self._generation and not self._closed:
                ready.append(marker)
        return ready

    def _emit_markers(self, markers: list[_Marker]) -> None:
        for marker in markers:
            if not self._is_current(marker.generation):
                continue
            try:
                if marker.kind == "index":
                    self._notify_index(marker.value)
                else:
                    self._notify_done(marker.value)
            except Exception as error:
                self._report_error(error, marker.generation)

    def _add_marker(self, generation: int, kind: str, value: int) -> None:
        with self._notification_lock:
            with self._condition:
                if self._closed or generation != self._generation:
                    return
                self._markers.append(_Marker(generation, self._last_audio_serial,
                                             kind, value))
                ready = self._ready_markers_locked()
            self._emit_markers(ready)

    def _mark_played(self, generation: int, serial: int) -> None:
        with self._notification_lock:
            with self._condition:
                if self._closed or generation != self._generation:
                    return
                if serial <= self._played_serial:
                    return
                self._played_out_of_order.add(serial)
                while self._played_serial + 1 in self._played_out_of_order:
                    self._played_serial += 1
                    self._played_out_of_order.remove(self._played_serial)
                ready = self._ready_markers_locked()
            self._emit_markers(ready)

    def _settle_played_through(self, generation: int, serial: int) -> None:
        """A drained player can release marks when a last onDone is missed."""
        with self._notification_lock:
            with self._condition:
                if self._closed or generation != self._generation:
                    return
                if serial > self._played_serial:
                    self._played_serial = serial
                    self._played_out_of_order.difference_update(
                        {candidate for candidate in self._played_out_of_order
                         if candidate <= serial})
                ready = self._ready_markers_locked()
            self._emit_markers(ready)

    def _handoff(self, job: _Job) -> None:
        """All PCM is in the player. Announce marks once, then the next chunk.

        Do not advance the played serial: callbacks still track real playback
        for subsequent jobs and for a queued return to the original pause mode.
        Serializing extraction and notification prevents a playback callback
        from overtaking this final index/done pair.
        """
        with self._notification_lock:
            with self._condition:
                if self._closed or job.generation != self._generation:
                    return
                ready = list(self._markers)
                self._markers.clear()
                ready.append(_Marker(job.generation, self._last_audio_serial,
                                     "done", job.number))
            self._emit_markers(ready)

    def _discard_queued_audio(self, generation: int) -> None:
        """After WavePlayer.stop, retire serials without announcing their marks."""
        with self._condition:
            if self._closed or generation != self._generation:
                return
            # A playback watchdog has thrown away the remaining buffers.
            # Pending indices cannot be reported as though they were heard.
            self._markers = deque(marker for marker in self._markers
                                  if marker.generation != generation)
            self._played_serial = self._last_audio_serial
            self._played_out_of_order.clear()

    def _watch_player(self) -> None:
        """One sleeper per transport, shared by all serial feed/idle calls."""
        while True:
            with self._condition:
                while not self._closed:
                    call = self._watched_call
                    if call is None:
                        self._watch_deadline = None
                        self._watch_condition.wait()
                        continue
                    remaining = call.deadline - time.monotonic()
                    if remaining > 0:
                        self._watch_deadline = call.deadline
                        self._watch_condition.wait(remaining)
                        continue
                    self._watched_call = None
                    self._watch_deadline = None
                    break
                else:
                    return
            # Never acquire a call guard while holding the queue condition.
            # Completion waits for an in-progress stop before the next feed.
            with call.guard:
                with self._condition:
                    if (call.finished or self._closed or
                            call.generation != self._generation):
                        continue
                    if self._paused:
                        call.deadline = time.monotonic() + .5
                        self._watched_call = call
                        continue
                _log.error("WavePlayer.%s stalled; stopping its queued audio",
                           call.operation)
                call.stopped = True
                try:
                    call.player.stop()
                except Exception:
                    _log.exception("could not release stalled WavePlayer.%s",
                                   call.operation)

    def _player_call(self, generation: int, operation: str,
                     action: Callable[[], None]) -> None:
        """Arm the persistent player watchdog without starting a thread."""
        call = _PlayerCall(generation, operation, self._player,
                           time.monotonic() + self._player_stall_seconds)
        with self._condition:
            self._watched_call = call
            # A later deadline can use the existing earlier wakeup. Once it
            # expires, the watcher reads the current call under the same lock.
            if self._watch_deadline is None or call.deadline < self._watch_deadline:
                self._watch_condition.notify()
        try:
            action()
        finally:
            with call.guard:
                call.finished = True
                with self._condition:
                    if self._watched_call is call:
                        self._watched_call = None
        if call.stopped:
            # stop() discarded audio. Let the ordinary per-job error path
            # release NVDA's speech manager and report the missing playback;
            # a dropped utterance must not be reported as played successfully.
            # A blocked feed can return by enqueuing a late buffer *after*
            # the watchdog's first stop. Clear that buffer before the next
            # native sentence is allowed to enter WavePlayer.
            try:
                call.player.stop()
            except Exception:
                _log.exception("could not clear late WavePlayer.%s feed",
                               operation)
            self._discard_queued_audio(generation)
            raise RuntimeError(f"WavePlayer.{operation} stalled and dropped queued audio")

    def _feed(self, batch: _PlaybackBatch, pcm: bytes) -> None:
        # A zero-sample native callback contains no audio for WavePlayer to
        # complete. Do not assign it a serial that would hold later marks.
        if not pcm:
            return
        generation = batch.generation
        with self._condition:
            if self._closed or generation != self._generation:
                raise _Discarded()
            self._last_audio_serial += 1
            serial = self._last_audio_serial
        def send() -> None:
            try:
                self._player_call(generation, "feed", lambda: self._player.feed(
                    pcm, onDone=lambda: self._mark_played(generation, serial)))
            except Exception:
                self._discard_queued_audio(generation)
                raise
            if not self._is_current(generation):
                # A blocked feed may return after cancellation's stop.
                self._player.stop()
                raise _Discarded()
        self._queue_playback(batch, send)

    def _run_job(self, engine: _EciEngine, job: _Job) -> None:
        if not self._is_current(job.generation):
            return
        pending = bytearray()
        batch = _PlaybackBatch(job.generation)
        first_sent = False
        startup_indices = deque()

        def flush(count: int | None = None) -> None:
            nonlocal first_sent
            if not pending:
                return
            if count is None:
                count = len(pending)
            # Index callbacks may arrive before the first useful audio
            # reserve. Preserve their exact byte positions while filling
            # that reserve; do not let an early word mark start playback
            # with only a few milliseconds of PCM.
            consumed = 0
            while startup_indices and startup_indices[0][0] <= count:
                at, index = startup_indices.popleft()
                self._feed(batch, bytes(pending[consumed:at]))
                self._add_marker(job.generation, "index", index)
                consumed = at
            self._feed(batch, bytes(pending[consumed:count]))
            del pending[:count]
            if startup_indices:
                shifted = [(at - count, index) for at, index in startup_indices]
                startup_indices.clear()
                startup_indices.extend(shifted)
            first_sent = True

        def on_pcm(pcm: bytes) -> None:
            if not pcm:
                return
            pending.extend(pcm)
            startup_bytes = self._first_pcm_bytes
            if not first_sent and len(pending) >= startup_bytes:
                flush(startup_bytes)
            while len(pending) >= (self._pcm_batch_bytes +
                                   self._final_pcm_min_bytes):
                flush(self._pcm_batch_bytes)

        def on_index(index: int) -> None:
            # Index playback must follow every sample before its native word
            # boundary. Push the partial batch before recording its serial.
            if not first_sent:
                startup_indices.append((len(pending), index))
            else:
                flush()
                self._add_marker(job.generation, "index", index)

        try:
            completed = engine.speak(job.parts, on_pcm, on_index)
            if not self._is_current(job.generation):
                return
            if not completed:
                raise RuntimeError("ECI cancelled a current utterance")
            flush()
            while startup_indices:
                _, index = startup_indices.popleft()
                self._add_marker(job.generation, "index", index)
            self._wait_playback(batch)
            if self._pause_mode in (1, 2):
                if job.finish_sequence:
                    self._handoff(job)
                return
            # OpenEVV drains WavePlayer before reporting utterance completion.
            # Its idle() result also covers a device missing the final onDone
            # callback; indices inside the audio remain playback gated.
            with self._condition:
                last_serial = self._last_audio_serial
            self._queue_playback(batch, lambda: self._player_call(
                job.generation, "idle", self._player.idle))
            self._wait_playback(batch)
            if not self._is_current(job.generation):
                return
            self._settle_played_through(job.generation, last_serial)
            if job.finish_sequence:
                self._add_marker(job.generation, "done", job.number)
        except _Discarded:
            pass
        except Exception as error:
            # A failed final fragment must still release NVDA's speech
            # manager. Earlier PCM, if any, gates this completion marker.
            # Discard an unplayed partial batch on failure; no index from it
            # can have been announced yet.
            self._wait_playback(batch, errors=False)
            if batch.error is not None:
                self._discard_queued_audio(job.generation)
            if job.finish_sequence and self._complete_on_error:
                self._add_marker(job.generation, "done", job.number)
            self._report_error(error, job.generation)

    def _run(self) -> None:
        try:
            engine = self._factory()
        except Exception as error:
            with self._condition:
                self._closed = True
                self._jobs.clear()
                self._clear_audio_locked()
                self._condition.notify_all()
                self._watch_condition.notify_all()
            try:
                self._notify_error(error)
            except Exception:
                _log.exception("direct ECI startup error hook failed")
            return
        with self._condition:
            self._engine = engine
        try:
            while True:
                with self._condition:
                    while (not self._jobs or self._stops_in_flight) and not self._closed:
                        self._condition.wait()
                    if self._closed:
                        break
                    job = self._jobs.popleft()
                if isinstance(job, (_Control, _Selection, _SampleRate, _DictionaryPath)):
                    try:
                        if isinstance(job, _SampleRate):
                            try:
                                self._change_sample_rate(engine, job)
                            finally:
                                if job.completed:
                                    job.completed(self._sample_rate_code)
                        elif isinstance(job, _DictionaryPath):
                            engine.set_dictionary_path(job.path)
                        elif isinstance(job, _Selection):
                            engine.select_voice(job.language, job.voice)
                            # A language-only choice keeps NVDA's displayed
                            # controls. Native preset selection resets them;
                            # restore the captured controls in this same queue
                            # item, including after cancellation.
                            for parameter, value in job.parameters:
                                engine.set_parameter(parameter, value)
                                if parameter == PAUSE_MODE:
                                    self._pause_mode = value
                        else:
                            engine.set_parameter(job.parameter, job.value)
                            if job.parameter == PAUSE_MODE:
                                self._pause_mode = job.value
                    except Exception as error:
                        self._report_error(error, self._generation)
                else:
                    self._run_job(engine, job)
        finally:
            with self._condition:
                self._engine = None
                while self._stops_in_flight:
                    self._condition.wait()
            engine.close()

    def cancel(self) -> None:
        with self._condition:
            if self._closed:
                return
            self._generation += 1
            self._jobs = deque(item for item in self._jobs
                               if isinstance(item, (_Control, _Selection, _SampleRate, _DictionaryPath)))
            self._markers.clear()
            self._clear_audio_locked()
            self._played_out_of_order.clear()
            self._played_serial = self._last_audio_serial
            engine = self._engine
            player = self._player
            self._stops_in_flight += 1
        try:
            # Silence the user's audio first. A native Stop can be delayed
            # by a render reset or a callback waiting for WavePlayer room;
            # stopping playback releases that callback before entering ECI.
            try:
                player.stop()
            finally:
                if engine is not None:
                    engine.cancel()
        finally:
            with self._condition:
                self._stops_in_flight -= 1
                self._condition.notify_all()

    def pause(self, switch: bool) -> None:
        with self._condition:
            self._paused = switch
            if not self._player_closed:
                self._player.pause(switch)

    def shutdown(self, timeout: float = 3.0) -> bool:
        if threading.current_thread() in (self._worker, self._audio_worker,
                                          self._watchdog_worker):
            raise RuntimeError("cannot join transport from its worker")
        deadline = time.monotonic() + timeout
        with self._condition:
            if self._player_closed:
                return True
            self._closed = True
            self._generation += 1
            self._jobs.clear()
            self._markers.clear()
            self._clear_audio_locked()
            self._stops_in_flight += 1
            engine = self._engine
            player = self._player
            self._condition.notify_all()
            self._watch_condition.notify_all()
        try:
            try:
                player.stop()
            finally:
                if engine is not None:
                    engine.cancel()
        finally:
            with self._condition:
                self._stops_in_flight -= 1
                self._condition.notify_all()
        self._worker.join(max(0.0, deadline - time.monotonic()))
        if self._worker.is_alive():
            return False
        self._audio_worker.join(max(0.0, deadline - time.monotonic()))
        if self._audio_worker.is_alive():
            return False
        self._watchdog_worker.join(max(0.0, deadline - time.monotonic()))
        if self._watchdog_worker.is_alive():
            return False
        with self._condition:
            while self._stops_in_flight:
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    return False
                self._condition.wait(remaining)
            self._player_closed = True
        self._player.close()
        return True


__all__ = ["EciTextTransport"]
