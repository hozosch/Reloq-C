"""Run with: python3 -B nvda/test_native_bridge.py host/libreloq_host.so"""

from __future__ import annotations

import sys
from pathlib import Path

from _reloq_native import (
    CANCELLED, OK, GermanTextError, NativeFrameEngine,
)


FIRST_PCM = (
    0, 0, 0, 2, 11, 36, 95, 208, 393, 647,
    951, 1267, 1555, 1788, 1955, 2061, 2116, 2127, 2100, 2046,
    1982, 1925, 1888, 1872, 1872, 1880, 1884, 1881, 1869, 1848,
    1816, 1772, 1712, 1639, 1554, 1462, 1367, 1267, 1158, 1035,
)


def main() -> None:
    dll = Path(sys.argv[1]).resolve()
    frame = [0.0] * 62
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
        frame[index] = value
    for invalid_config in (
        {"sample_rate_hz": 0x100003E80},  # would wrap to 16000
        {"max_frame_samples": -1},
        {"initial_seed": 0x100000064},  # would wrap to 100
    ):
        try:
            NativeFrameEngine(dll, **invalid_config)
        except ValueError:
            pass
        else:
            raise AssertionError(f"accepted wrapped config: {invalid_config}")
    engine = NativeFrameEngine(
        dll, sample_rate_hz=16000, max_frame_samples=256,
        initial_seed=100,
    )
    try:
        analyzed = engine.analyze_german("Maus, Schnee! Bäume?")
        assert analyzed is not None
        assert analyzed.annotation == (
            b"`[.1m'aw's], `[.1Sne]! `[.1b'oj'.0m@]?"
        )
        assert len(analyzed.phones) == 10
        assert (analyzed.word_count, analyzed.syllable_count) == (3, 4)
        assert analyzed.phones[0].source_offset == 4
        assert analyzed.phones[0].stress == 1
        assert analyzed.phones[2].same_word_right == 0
        assert analyzed.phones[3].same_word_left == 0
        try:
            engine.analyze_german("Maus, Bach!")
        except GermanTextError as error:
            assert error.code == -6 and error.utf8_byte_offset == 6
        else:
            raise AssertionError("uncovered German word accepted")
        assert engine.analyze_german("").phones == ()

        received: list[bytes] = []
        try:
            engine.render_controls(frame, 0x100000001, received.append)
        except ValueError:
            pass
        else:
            raise AssertionError("accepted wrapped formant selector")
        assert engine.render_controls(frame, 8, received.append) == (OK, 1)
        import array
        pcm = array.array("h")
        pcm.frombytes(received[0])
        assert tuple(pcm) == FIRST_PCM

        engine.reset()
        copied: list[bytes] = []

        def cancel_from_callback(block: bytes) -> None:
            copied.append(block)
            engine.cancel()

        assert engine.render_controls(frame * 2, 8, cancel_from_callback) == (
            CANCELLED, 1,
        )
        assert copied == received
        engine.reset()

        def reject(_block: bytes) -> None:
            raise LookupError("PCM queue failed")

        try:
            engine.render_controls(frame, 8, reject)
        except LookupError as error:
            assert str(error) == "PCM queue failed"
        else:
            raise AssertionError("sink failure did not propagate")
        engine.reset()
        replay: list[bytes] = []
        assert engine.render_controls(frame, 8, replay.append) == (OK, 1)
        assert replay == received
        engine.cancel()
        assert engine.analyze_german("Maus") is None
        engine.reset()
        assert len(engine.analyze_german("Maus").phones) == 3
    finally:
        engine.close()


if __name__ == "__main__":
    main()
