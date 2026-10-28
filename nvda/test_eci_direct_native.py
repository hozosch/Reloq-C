"""Check native C text→PCM, index order and explicit coverage errors.

Run after ``make -C eci test``:
    python3 -B nvda/test_eci_direct_native.py [eci/libreloq_eci.so]

The authored voice covers only a tiny German 11,025-Hz prototype subset.
"""

from __future__ import annotations

import ctypes
from pathlib import Path
import sys
import threading

from _reloq_eci_direct import DirectEciEngine, EciIndex, EciSynthesisError


def main() -> None:
    path = (Path(sys.argv[1]) if len(sys.argv) == 2 else
            Path(__file__).resolve().parents[1] / "eci/libreloq_eci.so")
    if len(sys.argv) > 2 or not path.is_file():
        raise SystemExit("expected native ECI library; run make -C eci test")
    path = path.resolve()

    dll = ctypes.CDLL(str(path))
    dll.eciGetAvailableLanguages.argtypes = (
        ctypes.POINTER(ctypes.c_uint), ctypes.POINTER(ctypes.c_int),
    )
    dll.eciGetAvailableLanguages.restype = ctypes.c_int
    count = ctypes.c_int(0)
    assert dll.eciGetAvailableLanguages(None, ctypes.byref(count)) == 0
    assert count.value == 0, "partial C voice advertised a complete language"

    engine = DirectEciEngine(path)
    pcm: list[bytes] = []
    indexes: list[tuple[int, int]] = []
    try:
        assert engine.speak(["Hieb.", EciIndex(71)], pcm.append,
                            lambda index: indexes.append(
                                (index, sum(map(len, pcm)))))
        pcm_bytes = b"".join(pcm)
        assert pcm_bytes and len(pcm_bytes) % 2 == 0
        assert any(pcm_bytes), "covered German text rendered only silence"
        assert indexes == [(71, len(pcm_bytes))], "index preceded PCM"
        pcm.clear()
        indexes.clear()

        # A manual NVDA preview commonly sends a word without typing a
        # period. Apple ECI gives these exactly the period-final PCM. Test
        # the same direct binding and native text path used by the addon.
        for text, expected_samples in (("Mett", 8338), ("Bett", 8426)):
            assert engine.speak([text + "."], pcm.append, indexes.append)
            marked = b"".join(pcm)
            assert len(marked) == expected_samples * 2
            pcm.clear()
            assert engine.speak([text, EciIndex(90)], pcm.append,
                                lambda index: indexes.append(
                                    (index, sum(map(len, pcm)))))
            assert b"".join(pcm) == marked and indexes[-1] == (90, len(marked))
            pcm.clear()
            indexes.clear()

        assert engine.speak(["Pa", "ck.", EciIndex(81), " ",
                             EciIndex(82), "Tipp.", EciIndex(83)],
                            pcm.append,
                            lambda index: indexes.append(
                                (index, sum(map(len, pcm)))))
        assert len(indexes) == 3
        assert [value for value, _ in indexes] == [81, 82, 83]
        assert 0 < indexes[0][1] == indexes[1][1] < indexes[2][1]
        assert indexes[2][1] == sum(map(len, pcm))
        pcm.clear()
        indexes.clear()

        # An index inside a single sentence has no measured PCM location yet.
        try:
            engine.speak(["Pa", EciIndex(84), "ck."],
                         pcm.append, lambda index: indexes.append(
                             (index, sum(map(len, pcm)))))
        except EciSynthesisError as error:
            assert "eciSynthesize failed" in str(error), error
            assert (error.status, error.stage, error.utf8_offset,
                    error.character_position) == (-2, 4, 2, 3)
        else:
            raise AssertionError("intra-sentence index was guessed")
        assert not pcm and not indexes

        # A later unknown word invalidates the entire queue in preflight.
        try:
            engine.speak(["Pack. ", EciIndex(72), "Hallo."], pcm.append,
                         lambda index: indexes.append(
                             (index, sum(map(len, pcm)))))
        except EciSynthesisError as error:
            assert "eciSynthesize failed" in str(error), error
            assert error.status == -2 and error.stage in (2, 3)
        else:
            raise AssertionError("uncovered text was accepted")
        assert not pcm and not indexes

        def broken_pcm(_chunk: bytes) -> None:
            raise LookupError("audio consumer failed")

        try:
            engine.speak(["Hieb."], broken_pcm, indexes.append)
        except LookupError as error:
            assert str(error) == "audio consumer failed"
        else:
            raise AssertionError("native callback failure was swallowed")

        cancelled_pcm: list[bytes] = []
        cancelled_indices: list[int] = []
        def cancel_from_another_thread(chunk: bytes) -> None:
            cancelled_pcm.append(chunk)
            worker = threading.Thread(target=engine.cancel)
            worker.start()
            worker.join(timeout=2)
            assert not worker.is_alive(), "native eciStop blocked on callback"

        assert engine.speak(["Hieb.", EciIndex(73)],
                            cancel_from_another_thread,
                            cancelled_indices.append) is False
        assert cancelled_pcm and not cancelled_indices

        # The same persistent handle must recover after Stop.
        pcm.clear()
        indexes.clear()
        assert engine.speak(["Hieb.", EciIndex(74)], pcm.append,
                            lambda index: indexes.append(
                                (index, sum(map(len, pcm)))))
        assert pcm and indexes == [(74, sum(map(len, pcm)))]
    finally:
        engine.close()
    print("direct ECI native: covered German PCM, index, Stop, preflight OK")


if __name__ == "__main__":
    main()
