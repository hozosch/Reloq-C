"""End-to-end numeric transport check, without NVDA or a Windows runtime.

Run from the project root after ``make -C host test``:

    python3 -B nvda/test_dev_transport_native.py [host/libreloq_host.so]

The frame is a numeric ABI fixture, not a speech-from-text sample.
"""

from __future__ import annotations

from pathlib import Path
import struct
import sys
import threading

from _reloq_dev_transport import FrameTransport
from _reloq_native import NativeFrameEngine
from test_native_bridge import FIRST_PCM


class CallbackPlayer:
    def __init__(self) -> None:
        self._condition = threading.Condition()
        self.pcm: list[bytes] = []
        self._callbacks: list[object] = []
        self.closed = False

    def feed(self, pcm: bytes, onDone=None) -> None:
        with self._condition:
            self.pcm.append(pcm)
            if onDone is not None:
                self._callbacks.append(onDone)
            self._condition.notify_all()

    def wait_for_pcm(self, count: int) -> bool:
        with self._condition:
            return self._condition.wait_for(lambda: len(self.pcm) >= count, 3)

    def finish_next(self) -> None:
        with self._condition:
            callback = self._callbacks.pop(0)
        callback()

    def stop(self) -> None:
        with self._condition:
            self._callbacks.clear()

    def pause(self, switch: bool) -> None:
        pass

    def close(self) -> None:
        self.closed = True


def numeric_frame() -> tuple[float, ...]:
    controls = [0.0] * 62
    for index, value in {
        0: 2.5, 1: 1200, 2: 50, 3: 50, 4: 7, 7: 18, 8: 25,
        9: 500, 10: 70, 13: 900, 14: 90, 15: 1300, 16: 100,
        17: 1800, 18: 110, 19: 2300, 20: 120, 21: 2700, 22: 130,
        23: 3100, 24: 140, 25: 3500, 26: 150, 27: 260, 28: 75,
        29: 360, 30: 90, 31: 2100, 32: 140, 33: 2000, 34: 150,
        35: 25, 36: 24, 37: 23, 38: 22, 39: 21, 40: 20, 41: 19,
        42: 18, 43: 35.5, 44: 80, 45: 85, 46: 90, 47: 95,
        48: 100, 49: 105, 50: 110, 51: 115,
    }.items():
        controls[index] = value
    return tuple(controls)


def main() -> None:
    source = (Path(sys.argv[1]) if len(sys.argv) > 1 else
              Path(__file__).resolve().parents[1] / "host/libreloq_host.so")
    if len(sys.argv) > 2 or not source.is_file():
        raise SystemExit("expected Linux host .so; first run: make -C host test")
    source = source.resolve()
    player = CallbackPlayer()
    completed: list[int] = []
    failures: list[Exception] = []
    transport = FrameTransport(
        lambda: NativeFrameEngine(
            source, sample_rate_hz=16000, max_frame_samples=256,
            initial_seed=100,
        ),
        player, completed.append, failures.append,
    )
    try:
        first = transport.submit_frames(numeric_frame(), 8)
        second = transport.submit_frames(numeric_frame(), 8)
        assert player.wait_for_pcm(2), failures
        assert not failures
        assert completed == [], "completion preceded actual playback"
        assert len(player.pcm) == 2
        for pcm in player.pcm:
            assert len(pcm) == 80
            assert struct.unpack("<40h", pcm) == FIRST_PCM
        player.finish_next()
        assert completed == [first]
        player.finish_next()
        assert completed == [first, second]
    finally:
        assert transport.shutdown(), "native worker did not stop"
    assert player.closed
    print("native C → worker → PCM: two FIFO jobs, playback-gated completion OK")


if __name__ == "__main__":
    main()
