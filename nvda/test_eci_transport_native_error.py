"""Real C ECI invalid SPR failure between two NVDA chunks on one handle.

Run after ``make -C eci test``:
    python3 -B nvda/test_eci_transport_native_error.py

WavePlayer is emulated so this can run outside Windows/NVDA. Native ECI,
callback PCM, the persistent transport, error completion and FIFO are real.
"""

from __future__ import annotations

from pathlib import Path
import sys
import threading
import time

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciSynthesisError
from _reloq_eci_transport import EciTextTransport

INVALID_SPR = "`[.1%%%]."


class Player:
    def __init__(self):
        self.condition = threading.Condition()
        self.buffers: list[bytes] = []
        self.callbacks = []
        self.stops = 0
        self.closed = False

    def feed(self, pcm, onDone=None):
        with self.condition:
            self.buffers.append(pcm)
            self.callbacks.append(onDone)
            self.condition.notify_all()

    def play_one(self):
        with self.condition:
            callback = self.callbacks.pop(0)
            self.condition.notify_all()
        callback()

    def idle(self):
        with self.condition:
            assert self.condition.wait_for(lambda: not self.callbacks, 2)

    def stop(self):
        with self.condition:
            self.stops += 1
            self.callbacks.clear()
            self.condition.notify_all()

    def pause(self, switch):
        pass

    def close(self):
        self.closed = True


def main() -> None:
    root = Path(__file__).resolve().parents[1]
    path = (Path(sys.argv[1]) if len(sys.argv) == 2 else
            root / "eci/libreloq_eci.so")
    if len(sys.argv) > 2 or not path.is_file():
        raise SystemExit("expected native ECI library; run make -C eci test")
    path = path.resolve()
    player = Player()
    created = []
    events = []
    errors = []
    jobs = {}
    bytes_at_done = {}
    transport = None

    def engine_factory():
        engine = DirectEciEngine(
            path, experimental_generic=True,
            lexicon_path=root / "data/german_apple_observed.rlex")
        created.append(engine)
        return engine

    def done(number):
        bytes_at_done[number] = sum(map(len, player.buffers))
        events.append(("done", number))
        if number == jobs["A"]:
            jobs["B"] = transport.submit([INVALID_SPR, EciIndex(2)])
        elif number == jobs["B"]:
            jobs["C"] = transport.submit(["Google Chrome.", EciIndex(3)])

    transport = EciTextTransport(engine_factory, player,
                                 lambda value: events.append(("index", value)),
                                 done, errors.append, complete_on_error=True)
    try:
        jobs["A"] = transport.submit(["Mett.", EciIndex(1)])
        deadline = time.monotonic() + 8
        while not ("C" in jobs and ("done", jobs["C"]) in events):
            with player.condition:
                ready = player.condition.wait_for(
                    lambda: bool(player.callbacks), .02)
            if ready:
                player.play_one()
            assert time.monotonic() < deadline, (events, errors, jobs)
        assert len(created) == 1, "transport recreated the native ECI handle"
        assert len(errors) == 1 and isinstance(errors[0], EciSynthesisError)
        assert errors[0].status == -6 and errors[0].stage == 0
        assert errors[0].utf8_offset == -1
        assert events == [("index", 1), ("done", jobs["A"]),
                          ("done", jobs["B"]),
                          ("index", 3), ("done", jobs["C"])], events
        assert bytes_at_done[jobs["A"]] > 0
        assert bytes_at_done[jobs["B"]] == bytes_at_done[jobs["A"]]
        assert bytes_at_done[jobs["C"]] > bytes_at_done[jobs["B"]]
        assert any(b"".join(player.buffers)[bytes_at_done[jobs["B"]]:])
        assert player.stops == 0, "a failed native sentence reset WavePlayer"
        print("native A/B/C: invalid SPR rejected without PCM; Google Chrome spoke on same ECI handle")
    finally:
        assert transport.shutdown()
        assert player.closed


if __name__ == "__main__":
    main()
