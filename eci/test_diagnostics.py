#!/usr/bin/env python3
"""Native ECI failure diagnostics and recovery on a real C text engine.

This exercises the same DirectEciEngine cleanup that an NVDA transport uses:
eciClearInput must not erase the last synthesis result before it can be read.
"""

from __future__ import annotations

import ctypes
from pathlib import Path
import sys


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "nvda"))
from _reloq_eci_direct import DirectEciEngine, EciIndex  # noqa: E402


def main() -> None:
    library = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("libreloq_eci.so")
    engine = DirectEciEngine(library)
    get_param = engine._dll.eciGetParam
    get_param.argtypes = (ctypes.c_void_p, ctypes.c_int)
    get_param.restype = ctypes.c_int

    def status() -> tuple[int, int, int]:
        return tuple(get_param(engine._handle, code) for code in
                     (0x1000, 0x1001, 0x1002))

    def speak(parts, succeeds: bool) -> None:
        blocks = []
        try:
            assert engine.speak(parts, blocks.append, lambda _: None)
        except RuntimeError as error:
            assert not succeeds and "eciSynthesize failed" in str(error), error
            assert not blocks, "unsupported job leaked partial audio"
        else:
            assert succeeds and blocks, parts

    try:
        speak(("Pack. Tempo.",), False)
        assert status() == (-2, 6, 2), status()  # lexicon or text analysis
        speak(("Pack. Maus.",), False)
        assert status() == (-2, 6, 3), status()  # recognized spelling, no audio rule
        speak(("Pa", EciIndex(1), "ck."), False)
        assert status() == (-2, 2, 4), status()  # unverified inside-word index
        speak(("Bett",), True)
        assert status() == (0, -1, 0), status()  # next job clears diagnosis
    finally:
        engine.close()
    print("Native ECI diagnostics: text/acoustic/index offset and recovery OK")


if __name__ == "__main__":
    main()
