"""Player recovery must remain bounded, pause-aware and safe across calls."""
import threading
from unittest.mock import patch
from _reloq_eci_direct import EciIndex, PAUSE_MODE
from _reloq_eci_transport import EciTextTransport
from test_eci_transport import Engine, Player, until


class ImmediatePlayer(Player):
    def feed(self, pcm, onDone=None):
        self.pcm.append(pcm)
        if onDone:
            onDone()


def constant_threads_and_retired_deadlines():
    starts = []
    original = threading.Thread.start
    def start(thread):
        starts.append(thread)
        return original(thread)
    errors, done = [], []
    engine, player = Engine(), ImmediatePlayer()
    with patch.object(threading.Thread, 'start', start):
        transport = EciTextTransport(lambda: engine, player, lambda n: None,
                                    done.append, errors.append,
                                    player_stall_seconds=.04)
        try:
            for mode in (0, 1, 2):
                transport.set_parameter(PAUSE_MODE, mode)
                last = 0
                for i in range(40):
                    last = transport.submit([f'sentence {mode} {i}', EciIndex(i)])
                until(lambda: last in done)
            # Allow all retired deadlines to expire while the player is idle.
            assert not threading.Event().wait(.08)
            assert len(done) == 120 and not errors and player.stops == 0
            assert len(player.pcm) == 240
            assert len(starts) == 3, [t.name for t in starts]
            assert player.idle_calls == 40
        finally:
            assert transport.shutdown()
        assert not any(t.is_alive() for t in starts)


def paused_stall_then_recover():
    class StalledPlayer(Player):
        def stop(self):
            self.release_feed.set()
            super().stop()
    engine, player, errors, done = Engine(), StalledPlayer(), [], []
    player.block_feed = True
    transport = EciTextTransport(lambda: engine, player, lambda n: None,
                                done.append, errors.append,
                                complete_on_error=True, player_stall_seconds=.03)
    try:
        transport.pause(True)
        transport.submit(['paused', EciIndex(1)])
        assert player.feed_entered.wait(1)
        # Cross both the first deadline and a paused retry.
        threading.Event().wait(.62)
        assert player.stops == 0 and not errors and not done
        transport.pause(False)
        until(lambda: len(errors) == 1)
        assert len(done) == 1 and player.stops == 2
        player.block_feed = False
        transport.set_parameter(PAUSE_MODE, 1)
        last = transport.submit(['recovered', EciIndex(2)])
        until(lambda: last in done)
        assert len(errors) == 1
    finally:
        player.release_feed.set()
        assert transport.shutdown()
    assert not transport._watchdog_worker.is_alive()


def stop_must_finish_before_next_feed():
    class SlowStopPlayer(ImmediatePlayer):
        def __init__(self):
            super().__init__()
            self.stop_entered = threading.Event()
            self.release_stop = threading.Event()
            self.next_feed = threading.Event()
        def feed(self, pcm, onDone=None):
            if pcm.startswith(b'A'):
                self.feed_entered.set()
                assert self.release_feed.wait(2)
            if pcm.startswith(b'B'):
                self.next_feed.set()
            super().feed(pcm, onDone)
        def stop(self):
            super().stop()
            if self.stops == 1:
                self.stop_entered.set()
                self.release_feed.set()
                assert self.release_stop.wait(2)
    player, errors, done = SlowStopPlayer(), [], []
    transport = EciTextTransport(Engine, player, lambda n: None, done.append,
                                errors.append, complete_on_error=True,
                                player_stall_seconds=.03)
    try:
        transport.set_parameter(PAUSE_MODE, 1)
        transport.submit(['A', EciIndex(1)])
        last = transport.submit(['B', EciIndex(2)])
        assert player.stop_entered.wait(1)
        assert not player.next_feed.wait(.05)
        player.release_stop.set()
        until(lambda: last in done)
        assert player.next_feed.is_set() and len(errors) == 1
        assert player.stops == 2
    finally:
        player.release_stop.set()
        player.release_feed.set()
        assert transport.shutdown()


def failed_startup_releases_all_workers():
    errors, player = [], ImmediatePlayer()
    def fail():
        raise OSError('factory failed')
    transport = EciTextTransport(fail, player, lambda n: None, lambda n: None,
                                errors.append)
    until(lambda: len(errors) == 1)
    until(lambda: not transport._watchdog_worker.is_alive())
    until(lambda: not transport._audio_worker.is_alive())
    assert transport.shutdown() and player.closed


class ObservedTransport(EciTextTransport):
    def __init__(self, *args, **kwargs):
        self.watch_waits = []
        self.waiting = threading.Event()
        self.armed = threading.Event()
        super().__init__(*args, **kwargs)

    def _watch_player(self):
        original = self._watch_condition.wait
        def wait(timeout=None):
            self.watch_waits.append(timeout)
            if timeout is None:
                self.waiting.set()
            else:
                self.armed.set()
            return original(timeout)
        self._watch_condition.wait = wait
        super()._watch_player()


def ordinary_queue_changes_do_not_wake_watchdog():
    player = ImmediatePlayer()
    transport = ObservedTransport(Engine, player, lambda n: None, lambda n: None,
                                  player_stall_seconds=10)
    try:
        assert transport.waiting.wait(1)
        transport._player_call(0, 'feed', lambda: transport.armed.wait(1))
        assert transport.armed.is_set()
        for _ in range(2000):
            transport._player_call(0, 'feed', lambda: None)
            with transport._condition:
                transport._condition.notify_all()
        assert len(transport.watch_waits) == 2, transport.watch_waits
        assert transport._watched_call is None and player.stops == 0
    finally:
        # An obsolete ten-second deadline must not delay shutdown.
        assert transport.shutdown(timeout=1)


def earlier_deadline_rearms_sleeping_watchdog():
    class ReleasingPlayer(Player):
        def stop(self):
            self.release_feed.set()
            super().stop()
    player = ReleasingPlayer()
    transport = ObservedTransport(Engine, player, lambda n: None, lambda n: None,
                                  player_stall_seconds=10)
    try:
        assert transport.waiting.wait(1)
        transport._player_call(0, 'feed', lambda: transport.armed.wait(1))
        assert transport.armed.is_set()
        transport._player_stall_seconds = .03
        player.block_feed = True
        try:
            transport._player_call(0, 'feed', lambda: player.feed(b'A' * 882))
        except RuntimeError as error:
            assert 'stalled and dropped queued audio' in str(error)
        else:
            raise AssertionError('Earlier deadline did not stop the blocked call')
        assert player.stops == 2
    finally:
        player.release_feed.set()
        assert transport.shutdown()


if __name__ == '__main__':
    constant_threads_and_retired_deadlines()
    paused_stall_then_recover()
    stop_must_finish_before_next_feed()
    failed_startup_releases_all_workers()
    ordinary_queue_changes_do_not_wake_watchdog()
    earlier_deadline_rearms_sleeping_watchdog()
    print('Player watchdog: 120 jobs, 240 feeds, 40 drains, exactly 3 persistent threads; retired deadlines, paused recovery, stop/feed serialization, failed startup and shutdown passed')
