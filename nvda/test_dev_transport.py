"""Run with: python3 -B nvda/test_dev_transport.py (no NVDA required)."""

from __future__ import annotations

import threading
import time
import unittest

from _reloq_dev_transport import FrameTransport


def frame(value: int) -> tuple[float, ...]:
    return (float(value),) + (0.0,) * 61


class FakeEngine:
    def __init__(self) -> None:
        self.started = threading.Event()
        self.gate = threading.Event()
        self.cancelled = threading.Event()
        self.closed = False
        self.reset_count = 0

    def reset(self) -> None:
        self.cancelled.clear()
        self.reset_count += 1

    def cancel(self) -> None:
        self.cancelled.set()
        self.gate.set()

    def render_controls(self, controls, highest_formant, on_pcm):
        assert highest_formant == 8
        value = int(controls[0])
        if value == 3:
            self.started.set()
            if not self.gate.wait(3):
                raise TimeoutError("test gate did not open")
        if self.cancelled.is_set():
            return 1, 0
        on_pcm(bytes((value, 0)))
        return 0, 1

    def close(self) -> None:
        self.closed = True


class FakePlayer:
    def __init__(self) -> None:
        self.pcm: list[bytes] = []
        self.completions = []
        self.stop_count = 0
        self.closed = False
        self.paused = False

    def feed(self, pcm: bytes, onDone=None) -> None:
        self.pcm.append(pcm)
        if onDone:
            self.completions.append(onDone)

    def play_next(self) -> None:
        self.completions.pop(0)()

    def stop(self) -> None:
        self.stop_count += 1
        self.completions.clear()

    def pause(self, switch: bool) -> None:
        self.paused = switch

    def close(self) -> None:
        self.closed = True


class LateFeedPlayer(FakePlayer):
    def __init__(self) -> None:
        super().__init__()
        self.entered = threading.Event()
        self.released = threading.Event()

    def feed(self, pcm: bytes, onDone=None) -> None:
        if pcm[0] == 8:
            self.entered.set()
            if not self.released.wait(3):
                raise TimeoutError("blocked feed was not released")
        super().feed(pcm, onDone=onDone)

    def stop(self) -> None:
        super().stop()
        self.pcm.clear()
        self.released.set()


class SlowStopPlayer(FakePlayer):
    def __init__(self) -> None:
        super().__init__()
        self.stop_entered = threading.Event()
        self.stop_released = threading.Event()

    def stop(self) -> None:
        self.stop_entered.set()
        if not self.stop_released.wait(3):
            raise TimeoutError("test stop was not released")
        super().stop()


class StuckFeedPlayer(FakePlayer):
    def __init__(self) -> None:
        super().__init__()
        self.feed_entered = threading.Event()
        self.release_feed = threading.Event()

    def feed(self, pcm: bytes, onDone=None) -> None:
        self.feed_entered.set()
        if not self.release_feed.wait(3):
            raise TimeoutError("test feed was not released")
        super().feed(pcm, onDone=onDone)


class ImmediatePlayer(FakePlayer):
    def feed(self, pcm: bytes, onDone=None) -> None:
        self.pcm.append(pcm)
        if onDone:
            onDone()  # model a completion callback re-entering the transport


def wait_until(test, timeout: float = 3.0) -> None:
    until = time.monotonic() + timeout
    while time.monotonic() < until:
        if test():
            return
        time.sleep(0.005)
    raise AssertionError("transport did not reach expected state")


class TestFrameTransport(unittest.TestCase):
    def test_fifo_chunks_and_cancel_pending_utterances(self) -> None:
        engine = FakeEngine()
        player = FakePlayer()
        done: list[int] = []
        errors: list[Exception] = []
        transport = FrameTransport(lambda: engine, player, done.append,
                                   errors.append)
        try:
            a = transport.submit_frames(frame(1), 8)
            b = transport.submit_frames(frame(2), 8)
            wait_until(lambda: len(player.pcm) == 2)
            self.assertEqual(done, [])
            player.play_next()
            player.play_next()
            wait_until(lambda: len(done) == 2)
            self.assertEqual(done, [a, b])
            self.assertEqual(player.pcm, [b"\x01\x00", b"\x02\x00"])
            transport.pause(True)
            self.assertTrue(player.paused)
            c = transport.submit_frames(frame(3), 8)
            self.assertTrue(engine.started.wait(3))
            d = transport.submit_frames(frame(4), 8)
            transport.cancel()
            e = transport.submit_frames(frame(5), 8)
            wait_until(lambda: player.pcm[-1] == b"\x05\x00")
            player.play_next()
            wait_until(lambda: len(done) == 3)
            self.assertEqual(done, [a, b, e])
            self.assertNotIn(c, done)
            self.assertNotIn(d, done)
            self.assertEqual(player.pcm[-1], b"\x05\x00")
            self.assertEqual(errors, [])
        finally:
            self.assertTrue(transport.shutdown())
        self.assertTrue(engine.closed)
        self.assertTrue(player.closed)
        with self.assertRaises(RuntimeError):
            transport.submit_frames(frame(6), 8)

    def test_cancel_during_blocked_audio_feed_removes_late_pcm(self) -> None:
        engine = FakeEngine()
        player = LateFeedPlayer()
        done: list[int] = []
        errors: list[Exception] = []
        transport = FrameTransport(lambda: engine, player, done.append,
                                   errors.append)
        try:
            old = transport.submit_frames(frame(8), 8)
            self.assertTrue(player.entered.wait(3))
            transport.cancel()
            current = transport.submit_frames(frame(9), 8)
            wait_until(lambda: player.pcm == [b"\x09\x00"])
            player.play_next()
            wait_until(lambda: done == [current])
            self.assertNotIn(old, done)
            self.assertEqual(player.pcm, [b"\x09\x00"])
            self.assertGreaterEqual(player.stop_count, 2)
            self.assertEqual(errors, [])
        finally:
            self.assertTrue(transport.shutdown())

    def test_new_chunk_waits_until_cancel_stop_finishes(self) -> None:
        engine = FakeEngine()
        player = SlowStopPlayer()
        done: list[int] = []
        transport = FrameTransport(lambda: engine, player, done.append)
        canceller = threading.Thread(target=transport.cancel)
        try:
            canceller.start()
            self.assertTrue(player.stop_entered.wait(3))
            current = transport.submit_frames(frame(7), 8)
            time.sleep(0.03)
            self.assertEqual(player.pcm, [])
            player.stop_released.set()
            canceller.join(3)
            self.assertFalse(canceller.is_alive())
            wait_until(lambda: player.pcm == [b"\x07\x00"])
            player.play_next()
            wait_until(lambda: done == [current])
        finally:
            player.stop_released.set()
            canceller.join(3)
            self.assertTrue(transport.shutdown())

    def test_shutdown_leaves_player_open_while_worker_is_blocked(self) -> None:
        engine = FakeEngine()
        player = StuckFeedPlayer()
        done: list[int] = []
        transport = FrameTransport(lambda: engine, player, done.append)
        try:
            transport.submit_frames(frame(6), 8)
            self.assertTrue(player.feed_entered.wait(3))
            self.assertFalse(transport.shutdown(timeout=0.01))
            self.assertFalse(player.closed)
            player.release_feed.set()
            self.assertTrue(transport.shutdown())
            self.assertTrue(engine.closed)
            self.assertTrue(player.closed)
            self.assertEqual(done, [])
        finally:
            player.release_feed.set()
            self.assertTrue(transport.shutdown())

    def test_immediate_playback_callback_can_cancel_and_enqueue(self) -> None:
        engine = FakeEngine()
        player = ImmediatePlayer()
        done: list[int] = []
        errors: list[Exception] = []
        next_job: list[int] = []

        def on_done(number: int) -> None:
            done.append(number)
            if len(done) == 1:
                transport.cancel()
                next_job.append(transport.submit_frames(frame(12), 8))

        transport = FrameTransport(lambda: engine, player, on_done,
                                   errors.append)
        try:
            first = transport.submit_frames(frame(11), 8)
            wait_until(lambda: len(done) == 2)
            self.assertEqual(done, [first, next_job[0]])
            self.assertEqual(player.pcm[-1], b"\x0c\x00")
            self.assertEqual(errors, [])
        finally:
            self.assertTrue(transport.shutdown())


if __name__ == "__main__":
    unittest.main()
