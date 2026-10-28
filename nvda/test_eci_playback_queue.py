"""Playback backpressure must not block ECI until the bounded queue fills."""
import threading
from _reloq_eci_direct import EciIndex, PAUSE_MODE
from _reloq_eci_transport import EciTextTransport
from test_eci_transport import Engine, Player, until


class StreamingEngine(Engine):
    def __init__(self, chunks):
        super().__init__()
        self.chunks = chunks
        self.generated = 0
        self.finished = threading.Event()
        self.owner = None

    def speak(self, parts, on_pcm, on_index):
        self.owner = threading.get_ident()
        for i in range(self.chunks):
            on_pcm(bytes([i % 256, 0]) * 441)
            self.generated += 1
        on_index(71)
        self.finished.set()
        return True


class BlockedPlayer(Player):
    def __init__(self):
        super().__init__()
        self.block_feed = True
        self.owner = None

    def feed(self, pcm, onDone=None):
        self.owner = threading.get_ident()
        super().feed(pcm, onDone)

    def stop(self):
        self.release_feed.set()
        super().stop()


def overlap_and_handoff():
    engine, player = StreamingEngine(8), BlockedPlayer()
    events = []
    transport = EciTextTransport(lambda: engine, player,
        lambda n: events.append(('index', n)),
        lambda n: events.append(('done', n)), playback_queue_size=8)
    try:
        transport.set_parameter(PAUSE_MODE, 1)
        job = transport.submit(['speech'])
        assert player.feed_entered.wait(1)
        assert engine.finished.wait(1), 'ECI waited for a blocked WavePlayer'
        assert not events, 'handoff preceded the actual final feed'
        assert engine.owner != player.owner
        player.release_feed.set()
        until(lambda: ('done', job) in events)
        assert events == [('index', 71), ('done', job)]
        assert len(player.pcm[0]) == 882, 'first audio must start after 40 ms'
        assert b''.join(player.pcm) == b''.join(bytes([i, 0])*441 for i in range(8))
        while player.callbacks:player.finish_next()
        assert events == [('index', 71), ('done', job)], 'duplicate playback marks'
    finally:
        assert transport.shutdown()


def bounded_cancel():
    engine, player = StreamingEngine(1000), BlockedPlayer()
    events = []
    transport = EciTextTransport(lambda: engine, player, events.append,
                                  events.append, playback_queue_size=2)
    try:
        transport.submit(['long speech'])
        assert player.feed_entered.wait(1)
        # One in-flight feed, two queued and at most one producer batch.
        until(lambda: len(transport._audio) == 2)
        assert not engine.finished.is_set() and engine.generated < 12
        stale = transport._generation
        transport.cancel()
        until(lambda: not transport._audio)
        assert transport._generation != stale and not events
    finally:
        assert transport.shutdown()
    assert not transport._audio_worker.is_alive()


def cancelled_feed_before_format_switch():
    class LatePlayer(BlockedPlayer):
        def stop(self):
            # The real device may return from feed after stop has returned.
            Player.stop(self)
    engine, old, new = StreamingEngine(8), LatePlayer(), Player()
    created, switched = threading.Event(), threading.Event()
    def factory(rate):
        assert rate == 16000
        created.set()
        return new
    transport = EciTextTransport(lambda: engine, old, lambda n: None,
                                lambda n: None)
    try:
        transport.submit(['old rate'])
        assert old.feed_entered.wait(1)
        transport.cancel()
        transport.set_sample_rate(3, factory, lambda code: switched.set())
        assert created.wait(1)
        assert not switched.is_set() and not old.closed
        old.release_feed.set()
        assert switched.wait(1)
        assert old.closed and new.stops == 0
        assert not old.callbacks
    finally:
        old.release_feed.set()
        assert transport.shutdown()


if __name__ == '__main__':
    overlap_and_handoff()
    bounded_cancel()
    cancelled_feed_before_format_switch()
    print('Playback queue: concurrent ECI, bounded lookahead, 40-ms start, final-feed barrier, cancellation, late-feed/rate race and single index/done passed')
