"""Direct text queue checks with a fake native ECI engine and WavePlayer.

Run: python3 -B nvda/test_eci_transport.py
"""

from __future__ import annotations

import threading
import time

from _reloq_eci_direct import EciIndex, PAUSE_MODE
from _reloq_eci_transport import EciTextTransport


class Player:
    def __init__(self):
        self.cv = threading.Condition()
        self.pcm = []
        self.callbacks = []
        self.stops = 0
        self.closed = False
        self.pause_state = None
        self.block_feed = False
        self.fail_next_feed = False
        self.feed_entered = threading.Event()
        self.release_feed = threading.Event()
        self.idle_calls = 0

    def feed(self, pcm, onDone=None):
        if self.fail_next_feed:
            self.fail_next_feed = False
            raise OSError("audio device rejected buffer")
        if self.block_feed:
            self.feed_entered.set()
            assert self.release_feed.wait(2), "test feed remained blocked"
        with self.cv:
            self.pcm.append(pcm)
            self.callbacks.append(onDone)
            self.cv.notify_all()

    def wait_for_pcm(self, count):
        with self.cv:
            return self.cv.wait_for(lambda: len(self.pcm) >= count, 2)

    def finish_next(self):
        with self.cv:
            callback = self.callbacks.pop(0)
            self.cv.notify_all()
        callback()

    def idle(self):
        with self.cv:
            self.idle_calls += 1
            assert self.cv.wait_for(lambda: not self.callbacks, 2), (
                "WavePlayer did not drain during idle")

    def stop(self):
        with self.cv:
            self.stops += 1
            self.callbacks.clear()
            self.cv.notify_all()

    def pause(self, switch):
        self.pause_state = switch

    def close(self):
        self.closed = True


def until(predicate, seconds=2):
    deadline = time.monotonic() + seconds
    while not predicate():
        assert time.monotonic() < deadline, "playback completion was not reported"
        time.sleep(.001)


class Engine:
    def __init__(self):
        self.cv = threading.Condition()
        self.started = []
        self.cancelled = threading.Event()
        self.wait_on_text = None
        self.closed = False
        self.parameters = []
        self.settings_at_start = {}

    def speak(self, parts, on_pcm, on_index):
        text = "".join(p for p in parts if isinstance(p, str))
        with self.cv:
            self.started.append(text)
            self.settings_at_start[text] = list(self.parameters)
            self.cv.notify_all()
        if text == self.wait_on_text:
            # Fill the low-latency first PCM batch before this fake stalls.
            # Real ECI emits hundreds of short frame callbacks here.
            on_pcm(b"old" * 1000)
            self.cancelled.wait(2)
            return False
        if text == "silent":
            on_index(300)
            return True
        on_pcm((text + "-a").encode())
        on_index(next(p.value for p in parts if isinstance(p, EciIndex)))
        on_pcm((text + "-b").encode())
        return True

    def wait_for_start(self, text):
        with self.cv:
            return self.cv.wait_for(lambda: text in self.started, 2)

    def cancel(self):
        self.cancelled.set()

    def set_parameter(self, parameter, value):
        self.parameters.append((parameter, value))

    def close(self):
        self.closed = True


def ordinary_boundary_test():
    engine, player = Engine(), Player()
    observed = []
    errors = []
    transport = EciTextTransport(lambda: engine, player,
                                 lambda index: observed.append(("index", index)),
                                 lambda job: observed.append(("done", job)),
                                 errors.append)
    try:
        first = transport.submit(["first", EciIndex(100)])
        second = transport.submit(["second", EciIndex(200)])
        third = transport.submit(["silent"])
        assert player.wait_for_pcm(2), errors
        assert not errors and not observed
        player.finish_next()
        assert observed == [("index", 100)]
        player.finish_next()
        assert player.wait_for_pcm(4), errors
        assert observed == [("index", 100), ("done", first)]
        assert player.pcm == [b"first-a", b"first-b", b"second-a", b"second-b"]
        player.finish_next()
        assert observed[-1] == ("index", 200)
        player.finish_next()
        assert engine.wait_for_start("silent"), errors
        until(lambda: len(observed) == 6)
        assert observed == [
            ("index", 100), ("done", first), ("index", 200),
            ("done", second), ("index", 300), ("done", third),
        ], observed
        assert player.idle_calls == 3
        assert player.stops == 0, "transport inserted stop at text boundary"
        transport.pause(True)
        assert player.pause_state is True
    finally:
        assert transport.shutdown()
    assert engine.closed and player.closed


def cancellation_race_test():
    engine, player = Engine(), Player()
    engine.wait_on_text = "block"
    observed, errors = [], []
    transport = EciTextTransport(lambda: engine, player,
                                 lambda index: observed.append(("index", index)),
                                 lambda job: observed.append(("done", job)),
                                 errors.append)
    try:
        stale = transport.submit(["block"])
        assert player.wait_for_pcm(1)
        old_callback = player.callbacks[0]
        transport.set_parameter(0x1004, 63)
        transport.cancel()
        current = transport.submit(["fresh", EciIndex(9)])
        assert player.wait_for_pcm(3), errors
        assert engine.settings_at_start["fresh"] == [(0x1004, 63)], (
            "a cancelled speech generation dropped a queued voice setting")
        old_callback()  # delayed callback after WavePlayer.stop()
        assert observed == [], "stale completion escaped generation check"
        player.finish_next()
        assert observed == [("index", 9)]
        player.finish_next()
        until(lambda: ("done", current) in observed)
        assert observed == [("index", 9), ("done", current)]
        assert ("done", stale) not in observed and not errors
    finally:
        assert transport.shutdown()
    assert engine.closed and player.closed


def stop_during_feed_test():
    engine, player = Engine(), Player()
    player.block_feed = True
    observed, errors = [], []
    transport = EciTextTransport(lambda: engine, player,
                                 observed.append, observed.append, errors.append)
    try:
        transport.submit(["old", EciIndex(1)])
        assert player.feed_entered.wait(2)
        transport.cancel()
        player.block_feed = False
        player.release_feed.set()
        current = transport.submit(["new", EciIndex(2)])
        assert player.wait_for_pcm(3), errors
        # The late old feed can remain in the fake player's record but stop
        # discards its callback. Only the fresh job has two active callbacks.
        assert len(player.callbacks) == 2
        player.finish_next()
        player.finish_next()
        until(lambda: len(observed) == 2)
        assert observed == [2, current], observed
        assert not errors
        assert player.stops >= 2, "late feed was not removed after cancellation"
    finally:
        assert transport.shutdown()


def feed_failure_does_not_block_later_job():
    engine, player = Engine(), Player()
    player.fail_next_feed = True
    observed, errors = [], []
    transport = EciTextTransport(lambda: engine, player,
                                 lambda index: observed.append(("index", index)),
                                 lambda job: observed.append(("done", job)),
                                 errors.append)
    try:
        transport.submit(["broken", EciIndex(1)])
        assert engine.wait_for_start("broken")
        current = transport.submit(["next", EciIndex(2)])
        assert player.wait_for_pcm(2)
        assert errors and "audio device rejected" in str(errors[0])
        player.finish_next()
        player.finish_next()
        until(lambda: ("done", current) in observed)
        assert observed == [("index", 2), ("done", current)]
    finally:
        assert transport.shutdown()


def stop_precedes_native_cancel_test():
    """A slow ECI Stop cannot delay audible silence or deadlock the player."""
    player = Player()

    class StopRequiresSilence(Engine):
        def cancel(self):
            assert player.stops, "native cancellation preceded WavePlayer.stop"
            super().cancel()

    engine = StopRequiresSilence()
    engine.wait_on_text = "block"
    transport = EciTextTransport(lambda: engine, player, lambda value: None,
                                 lambda value: None)
    try:
        transport.submit(["block"])
        assert player.wait_for_pcm(1)
        transport.cancel()
        assert engine.cancelled.is_set()
    finally:
        assert transport.shutdown()


def empty_pcm_does_not_hold_index_test():
    """WavePlayer has no playback completion for a zero-length buffer."""
    player = Player()

    class EmptyBeforeIndex(Engine):
        def speak(self, parts, on_pcm, on_index):
            on_pcm(b"")
            on_index(77)
            return True

    observed = []
    finished = threading.Event()
    transport = EciTextTransport(EmptyBeforeIndex, player,
                                 lambda value: observed.append(("index", value)),
                                 lambda value: (observed.append(("done", value)),
                                                finished.set()))
    try:
        number = transport.submit(["silent"])
        assert finished.wait(2)
        assert observed == [("index", 77), ("done", number)]
        assert not player.pcm
    finally:
        assert transport.shutdown()


def lost_playback_callback_still_starts_next_nvda_chunk_test():
    """NVDA can accept a feed without registering its onDone callback."""
    class MissingCallbackPlayer(Player):
        drop_callbacks = True

        def feed(self, pcm, onDone=None):
            if not self.drop_callbacks:
                return super().feed(pcm, onDone)
            with self.cv:
                self.pcm.append(pcm)
                self.cv.notify_all()

    engine, player = Engine(), MissingCallbackPlayer()
    observed, errors = [], []
    transport = None

    def done(job):
        observed.append(("done", job))
        if len([event for event in observed if event[0] == "done"]) == 1:
            player.drop_callbacks = False
            transport.submit(["Google Chrome.", EciIndex(202)])

    transport = EciTextTransport(lambda: engine, player,
                                 lambda index: observed.append(("index", index)),
                                 done, errors.append)
    try:
        first = transport.submit(["Desktop liste.", EciIndex(101)])
        assert engine.wait_for_start("Google Chrome."), (
            "NVDA never requested its second chunk after the first done")
        assert player.wait_for_pcm(4), errors
        assert observed == [("index", 101), ("done", first)], observed
        player.finish_next()
        player.finish_next()
        until(lambda: len(observed) == 4)
        assert observed[2:] == [("index", 202), ("done", first + 1)]
        assert engine.started == ["Desktop liste.", "Google Chrome."]
        assert not errors and player.idle_calls == 2
    finally:
        assert transport.shutdown()


def separate_chunks_preserve_native_terminal_pause_test():
    """A period's native PCM tail must play before NVDA requests the next job.

    Neither WavePlayer.idle nor a playback marker may drop or overlap the
    phrase-final quiet samples. The generic C voice, not this queue, owns the
    actual length of that pause.
    """
    class PauseEngine(Engine):
        def speak(self, parts, on_pcm, on_index):
            text = "".join(p for p in parts if isinstance(p, str))
            with self.cv:
                self.started.append(text)
                self.cv.notify_all()
            on_pcm(b"\x10\x00" * 220)
            on_pcm(b"\x00\x00" * 4400)  # 399 ms at native 11,025 Hz.
            on_index(next(p.value for p in parts if isinstance(p, EciIndex)))
            return True

    engine, player = PauseEngine(), Player()
    observed, errors = [], []
    transport = None

    def done(job):
        observed.append(("done", job))
        if job == first:
            transport.submit(["Google Chrome.", EciIndex(202)])

    transport = EciTextTransport(lambda: engine, player,
                                 lambda index: observed.append(("index", index)),
                                 done, errors.append)
    try:
        first = transport.submit(["Desktop liste.", EciIndex(101)])
        assert player.wait_for_pcm(2)
        assert engine.started == ["Desktop liste."]
        deadline = time.monotonic() + 2
        while len(observed) < 4:
            with player.cv:
                has_callback = player.cv.wait_for(
                    lambda: bool(player.callbacks), .02)
            if has_callback:
                player.finish_next()
            assert time.monotonic() < deadline, (engine.started, observed, errors)
        assert engine.started == ["Desktop liste.", "Google Chrome."]
        assert observed == [
            ("index", 101), ("done", first),
            ("index", 202), ("done", first + 1),
        ], observed
        expected = b"\x10\x00" * 220 + b"\x00\x00" * 4400
        assert b"".join(player.pcm) == expected * 2
        assert player.idle_calls == 2 and player.stops == 0 and not errors
    finally:
        assert transport.shutdown()


def stuck_player_idle_releases_nvda_test():
    """A watchdog must report dropped audio and let the next chunk start."""
    class StalledPlayer(Player):
        def __init__(self):
            super().__init__()
            self.entered_idle = threading.Event()
            self.released = threading.Event()

        def idle(self):
            self.entered_idle.set()
            assert self.released.wait(2), "watchdog did not release idle"
            super().idle()

        def stop(self):
            super().stop()
            self.released.set()

    player = StalledPlayer()
    engine = Engine()
    observed, errors = [], []
    transport = EciTextTransport(lambda: engine, player,
                                 lambda index: None,
                                 lambda job: observed.append(job), errors.append,
                                 complete_on_error=True,
                                 player_stall_seconds=.05)
    try:
        first = transport.submit(["Desktop liste.", EciIndex(1)])
        assert player.entered_idle.wait(2)
        until(lambda: first in observed)
        assert player.stops == 2 and len(errors) == 1
        assert "dropped queued audio" in str(errors[0])
        # A watchdog failure must not trap subsequent NVDA utterances.
        second = transport.submit(["Google Chrome.", EciIndex(2)])
        assert player.wait_for_pcm(4)
        player.finish_next()
        player.finish_next()
        until(lambda: second in observed)
        assert observed == [first, second] and len(errors) == 1
    finally:
        assert transport.shutdown()


def stalled_feed_late_buffer_does_not_replace_next_sentence_test():
    """A feed released after the watchdog stop may have queued stale PCM."""
    class LateFeedPlayer(Player):
        def __init__(self):
            super().__init__()
            self.first = True
            self.entered = threading.Event()
            self.released = threading.Event()

        def feed(self, pcm, onDone=None):
            if self.first:
                self.first = False
                self.entered.set()
                assert self.released.wait(2)
            super().feed(pcm, onDone)

        def stop(self):
            super().stop()
            self.released.set()

    engine, player = Engine(), LateFeedPlayer()
    observed, errors, numbers = [], [], {}
    transport = None

    def done(job):
        observed.append(("done", job))
        if job == numbers["B"]:
            numbers["C"] = transport.submit(["C", EciIndex(3)])

    transport = EciTextTransport(lambda: engine, player,
                                 lambda index: observed.append(("index", index)),
                                 done, errors.append, complete_on_error=True,
                                 player_stall_seconds=.05)
    try:
        numbers["B"] = transport.submit(["B", EciIndex(2)])
        assert player.entered.wait(2)
        assert player.wait_for_pcm(3)
        assert player.pcm == [b"B-a", b"C-a", b"C-b"]
        assert player.stops == 2 and len(player.callbacks) == 2
        assert len(errors) == 1 and "dropped queued audio" in str(errors[0])
        player.finish_next()
        player.finish_next()
        until(lambda: ("done", numbers["C"]) in observed)
        assert observed == [("done", numbers["B"]), ("index", 3),
                            ("done", numbers["C"])]
    finally:
        assert transport.shutdown()


def three_chunk_native_error_and_silence_test():
    """NVDA's done-triggered next chunk survives a failed or empty middle job."""
    for middle in ("native failure", "empty native PCM"):
        class ScriptedEngine(Engine):
            def speak(self, parts, on_pcm, on_index):
                text = "".join(p for p in parts if isinstance(p, str))
                with self.cv:
                    self.started.append(text)
                    self.cv.notify_all()
                if text == middle:
                    if middle == "native failure":
                        raise RuntimeError("native synthesis rejected B")
                    return True
                on_pcm((text + "-audible").encode())
                on_index(next(p.value for p in parts if isinstance(p, EciIndex)))
                return True

        engine, player = ScriptedEngine(), Player()
        observed, errors, numbers = [], [], {}
        transport = None

        def done(job):
            observed.append(("done", job))
            if job == numbers["A"]:
                numbers["B"] = transport.submit([middle])
            elif job == numbers["B"]:
                numbers["C"] = transport.submit(["C", EciIndex(3)])

        transport = EciTextTransport(lambda: engine, player,
                                     lambda index: observed.append(("index", index)),
                                     done, errors.append, complete_on_error=True)
        try:
            numbers["A"] = transport.submit(["A", EciIndex(1)])
            deadline = time.monotonic() + 2
            while not ("C" in numbers and ("done", numbers["C"]) in observed):
                with player.cv:
                    ready = player.cv.wait_for(lambda: bool(player.callbacks), .02)
                if ready:
                    player.finish_next()
                assert time.monotonic() < deadline, (middle, engine.started,
                                                     observed, errors)
            assert engine.started == ["A", middle, "C"]
            assert observed == [("index", 1), ("done", numbers["A"]),
                                ("done", numbers["B"]),
                                ("index", 3), ("done", numbers["C"])]
            assert player.pcm == [b"A-audible", b"C-audible"]
            assert len(errors) == (1 if middle == "native failure" else 0)
            assert not player.stops
        finally:
            assert transport.shutdown()


def three_chunk_fast_navigation_test():
    """A plays; B is interrupted; its delayed callback cannot suppress C."""
    engine, player = Engine(), Player()
    engine.wait_on_text = "B"
    observed, errors = [], []
    transport = EciTextTransport(lambda: engine, player,
                                 lambda index: observed.append(("index", index)),
                                 lambda job: observed.append(("done", job)),
                                 errors.append, complete_on_error=True)
    try:
        first = transport.submit(["A", EciIndex(1)])
        assert player.wait_for_pcm(2)
        player.finish_next()
        player.finish_next()
        until(lambda: ("done", first) in observed)
        middle = transport.submit(["B", EciIndex(2)])
        assert player.wait_for_pcm(3)
        delayed = player.callbacks[0]
        transport.cancel()
        last = transport.submit(["C", EciIndex(3)])
        assert player.wait_for_pcm(5)
        delayed()
        player.finish_next()
        player.finish_next()
        until(lambda: ("done", last) in observed)
        assert engine.started == ["A", "B", "C"]
        assert observed == [("index", 1), ("done", first),
                            ("index", 3), ("done", last)]
        assert ("done", middle) not in observed and not errors
        assert player.pcm[3:] == [b"C-a", b"C-b"]
    finally:
        assert transport.shutdown()


def shortened_pause_handoff_test(mode):
    """NVDA's done hook supplies B while A's final audio is still queued."""
    engine, player = Engine(), Player()
    observed, errors, jobs = [], [], []

    def done(job):
        observed.append(("done", job))
        if len(jobs) == 1:
            jobs.append(transport.submit(["B", EciIndex(2)]))

    transport = EciTextTransport(lambda: engine, player,
                                 lambda index: observed.append(("index", index)),
                                 done, errors.append)
    try:
        transport.set_parameter(PAUSE_MODE, mode)
        # Prevent completion racing with the test's assignment of A's id.
        player.block_feed = True
        jobs.append(transport.submit(["A", EciIndex(1)]))
        assert player.feed_entered.wait(2)
        player.block_feed = False
        player.release_feed.set()
        assert player.wait_for_pcm(4), errors
        until(lambda: len(observed) == 4)
        expected = [("index", 1), ("done", jobs[0]),
                    ("index", 2), ("done", jobs[1])]
        assert observed == expected and not errors
        assert player.pcm == [b"A-a", b"A-b", b"B-a", b"B-b"]
        assert player.idle_calls == 0 and player.stops == 0
        assert len(player.callbacks) == 4, "handoff waited for playback"
        # A return to original pauses must still await the actual old tail.
        transport.set_parameter(PAUSE_MODE, 0)
        last = transport.submit(["C", EciIndex(3)])
        assert player.wait_for_pcm(6)
        for _ in range(4):
            player.finish_next()
        assert observed == expected, "old tail duplicated a notification"
        player.finish_next()
        player.finish_next()
        until(lambda: ("done", last) in observed)
        assert observed == expected + [("index", 3), ("done", last)]
    finally:
        assert transport.shutdown()


def shortened_pause_cancel_tail_test():
    engine, player = Engine(), Player()
    observed, errors = [], []
    transport = EciTextTransport(lambda: engine, player,
                                 lambda index: observed.append(("index", index)),
                                 lambda job: observed.append(("done", job)), errors.append)
    try:
        transport.set_parameter(PAUSE_MODE, 1)
        old = transport.submit(["A", EciIndex(1)])
        until(lambda: ("done", old) in observed)
        late = list(player.callbacks)
        transport.cancel()
        fresh = transport.submit(["B", EciIndex(2)])
        until(lambda: ("done", fresh) in observed)
        for callback in late:
            callback()
        while player.callbacks:
            player.finish_next()
        assert observed == [("index", 1), ("done", old),
                            ("index", 2), ("done", fresh)]
        assert not errors and player.stops == 1
    finally:
        assert transport.shutdown()


if __name__ == "__main__":
    ordinary_boundary_test()
    cancellation_race_test()
    stop_during_feed_test()
    feed_failure_does_not_block_later_job()
    stop_precedes_native_cancel_test()
    empty_pcm_does_not_hold_index_test()
    lost_playback_callback_still_starts_next_nvda_chunk_test()
    separate_chunks_preserve_native_terminal_pause_test()
    stuck_player_idle_releases_nvda_test()
    stalled_feed_late_buffer_does_not_replace_next_sentence_test()
    three_chunk_native_error_and_silence_test()
    three_chunk_fast_navigation_test()
    shortened_pause_handoff_test(1)
    shortened_pause_handoff_test(2)
    shortened_pause_cancel_tail_test()
    print("direct ECI queue: FIFO, played indices, native terminal PCM, three-chunk failure/navigation, dropped-playback report, stop/feed race OK")
