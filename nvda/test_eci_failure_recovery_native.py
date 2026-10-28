"""Native ECI plus transport keeps later NVDA jobs after a rejected utterance.

Run after ``make -C eci libreloq_eci.so``. The deliberately malformed SPR
input remains invalid even when ordinary German G2P gains broader coverage.
The player stub acknowledges each feed immediately; actual Windows WavePlayer
and NVDA speech-manager behavior require their own integration check.
"""

from __future__ import annotations

from pathlib import Path
import threading

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciSynthesisError
from _reloq_eci_transport import EciTextTransport


ROOT = Path(__file__).resolve().parents[1]


class Player:
    def __init__(self):
        self.pcm_bytes = 0

    def feed(self, pcm, onDone=None):
        self.pcm_bytes += len(pcm)
        if onDone:
            onDone()

    def idle(self):
        pass

    def stop(self):
        pass

    def pause(self, switch):
        pass

    def close(self):
        pass


def main():
    player = Player()
    observed = []
    errors = []
    finished = threading.Event()

    def done(number):
        observed.append(("done", number))
        if sum(kind == "done" for kind, _ in observed) == 3:
            finished.set()

    transport = EciTextTransport(
        lambda: DirectEciEngine(
            ROOT / "eci/libreloq_eci.so", experimental_generic=True,
            lexicon_path=ROOT / "data/german_apple_observed.rlex"),
        player, lambda index: observed.append(("index", index)),
        done, errors.append, complete_on_error=True)
    try:
        jobs = [transport.submit([text, EciIndex(mark)]) for text, mark in (
            ("`[.1???]", 9001), ("Mett", 9002), ("Bett", 9003))]
        assert finished.wait(5), (observed, errors)
        assert observed == [("done", jobs[0]),
                            ("index", 9002), ("done", jobs[1]),
                            ("index", 9003), ("done", jobs[2])], observed
        assert len(errors) == 1 and isinstance(errors[0], EciSynthesisError), errors
        assert player.pcm_bytes > 0
    finally:
        assert transport.shutdown()
    print("ECI native error recovery: rejected job completed, later two jobs audible")


if __name__ == "__main__":
    main()
