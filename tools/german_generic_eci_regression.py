#!/usr/bin/env python3
"""Exercise the public ECI text/PCM path of the opt-in German generic voice.

This is a stability and coverage test, not an Eloquence similarity test. A
completed phoneme annotation is never treated as completed speech. The test
only uses the public ECI functions that an NVDA adapter can call.
"""

from __future__ import annotations

import argparse
from collections import Counter
import ctypes
import json
import math
from pathlib import Path
import shutil
import sys
import tempfile
import threading
from time import perf_counter


ROOT = Path(__file__).resolve().parents[1]
GERMAN = 0x00040000
VOICE_MODE = 0x1003
GENERIC_RATE = 0x1004
GENERIC_PITCH = 0x1005
GENERIC_VOLUME = 0x1006
NATIVE_SPEED = 0x1007
NATIVE_PITCH = 0x1008
NATIVE_INFLECTION = 0x1009
NATIVE_VOLUME = 0x100a
SAMPLE_RATE = 5
LAST_ERROR = 0x1000
FAILURE_OFFSET = 0x1001
FAILURE_STAGE = 0x1002
CALLBACK = (ctypes.WINFUNCTYPE if sys.platform == "win32" else ctypes.CFUNCTYPE)(
    ctypes.c_int, ctypes.c_void_p, ctypes.c_int, ctypes.c_int, ctypes.c_void_p)


class Eci:
    def __init__(self, dll: Path, *, generic: bool = True, buffer_samples: int = 733):
        self.dll = ctypes.WinDLL(str(dll)) if sys.platform == "win32" else ctypes.CDLL(str(dll))
        h = ctypes.c_void_p
        i = ctypes.c_int
        for name, args, ret in (
            ("eciNewEx", (i,), h), ("eciDelete", (h,), h),
            ("eciReset", (h,), i), ("eciClearInput", (h,), i),
            ("eciRegisterCallback", (h, CALLBACK, h), None),
            ("eciSetOutputBuffer", (h, i, ctypes.POINTER(ctypes.c_int16)), i),
            ("eciAddText", (h, ctypes.c_char_p), i),
            ("eciInsertIndex", (h, i), i), ("eciSynthesize", (h,), i),
            ("eciSynchronize", (h,), i), ("eciStop", (h,), i),
            ("eciGetParam", (h, i), i), ("eciSetParam", (h, i, i), i),
            ("eciLoadLexicon", (h, ctypes.c_char_p), i),
        ):
            f = getattr(self.dll, name)
            f.argtypes = args
            f.restype = ret
        self.handle = self.dll.eciNewEx(GERMAN)
        if not self.handle:
            raise RuntimeError("German eciNewEx failed")
        self.buffer = (ctypes.c_int16 * buffer_samples)()
        self.callback = CALLBACK(self._callback)
        self.pcm = bytearray()
        self.indices: list[tuple[int, int]] = []
        self.cancel_first_pcm = False
        self.did_cancel = False
        self.first_pcm_ms: float | None = None
        self._synth_start = 0.0
        self.buffer_samples = buffer_samples
        self.dll.eciRegisterCallback(self.handle, self.callback, None)
        assert self.dll.eciSetOutputBuffer(self.handle, buffer_samples, self.buffer) > 0
        assert self.dll.eciGetParam(self.handle, SAMPLE_RATE) == 1
        if generic:
            previous = self.dll.eciSetParam(self.handle, VOICE_MODE, 1)
            if previous < 0 or self.dll.eciGetParam(self.handle, VOICE_MODE) != 1:
                raise RuntimeError("the opt-in generic ECI mode is not available")

    def _callback(self, handle: int, message: int, value: int, _unused: int) -> int:
        if handle != self.handle:
            return 2
        if message == 0:
            if not 0 < value <= self.buffer_samples:
                return 2
            if self.first_pcm_ms is None:
                self.first_pcm_ms = round((perf_counter() - self._synth_start) * 1000, 1)
            self.pcm.extend(ctypes.string_at(self.buffer, value * 2))
            if self.cancel_first_pcm and not self.did_cancel:
                self.did_cancel = True
                self.dll.eciStop(self.handle)
        elif message == 2:
            self.indices.append((value, len(self.pcm) // 2))
        else:
            return 2
        return 1

    def speak(self, fragments: tuple[str | int, ...], *, end_index: int | None = 19,
              cancel_first_pcm: bool = False,
              before_synthesize=None) -> dict:
        self.pcm.clear()
        self.indices.clear()
        self.did_cancel = False
        self.first_pcm_ms = None
        self.cancel_first_pcm = cancel_first_pcm
        started = perf_counter()
        try:
            for fragment in fragments:
                if isinstance(fragment, int):
                    assert self.dll.eciInsertIndex(self.handle, fragment) > 0
                    continue
                # ECI's German input is CP1252, even though these Python
                # sources are UTF-8. Reject unrepresentable Unicode here.
                data = fragment.encode("cp1252")
                if b"\0" in data:
                    raise ValueError("NUL cannot occur in ECI text")
                assert self.dll.eciAddText(self.handle, data) > 0
            if end_index is not None:
                assert self.dll.eciInsertIndex(self.handle, end_index) > 0
            if before_synthesize is not None:
                before_synthesize()
            self._synth_start = perf_counter()
            started_ok = self.dll.eciSynthesize(self.handle)
            elapsed_ms = (perf_counter() - started) * 1000
            success = bool(started_ok) and bool(self.dll.eciSynchronize(self.handle))
            error_code = self.dll.eciGetParam(self.handle, LAST_ERROR)
            result = {
                "status": "pcm" if success else "cancelled" if error_code == 1 else "rejected",
                "samples": len(self.pcm) // 2,
                "index_positions": self.indices.copy(),
                "elapsed_ms": round(elapsed_ms, 1),
                "first_pcm_ms": self.first_pcm_ms,
                "eci_error": error_code,
                "stage": self.dll.eciGetParam(self.handle, FAILURE_STAGE),
                "utf8_offset": self.dll.eciGetParam(self.handle, FAILURE_OFFSET),
            }
            if success:
                assert result["samples"] > 0, result
                requested = [fragment for fragment in fragments
                             if isinstance(fragment, int)]
                if end_index is not None:
                    requested.append(end_index)
                assert [index for index, _ in self.indices] == requested, result
                assert all(0 <= at <= result["samples"]
                           for _, at in self.indices), result
                assert all(a <= b for (_, a), (_, b) in
                           zip(self.indices, self.indices[1:])), result
                if end_index is not None:
                    assert self.indices[-1] == (end_index, result["samples"]), result
                first_text = next((i for i, fragment in enumerate(fragments)
                                   if isinstance(fragment, str)), len(fragments))
                assert all(at == 0 for _, at in
                           self.indices[:first_text]), result
                assert len(self.pcm) % 2 == 0
                result["peak"] = max(abs(sample) for sample in
                                     memoryview(self.pcm).cast("h"))
                result["clipped_samples"] = sum(abs(sample) >= 32767 for sample in
                                                memoryview(self.pcm).cast("h"))
            elif result["status"] == "cancelled":
                assert not self.indices, result
            else:
                # Text and acoustic preflight rejection must not leak the
                # first half of a paragraph to the screen reader.
                assert not self.pcm and not self.indices, result
            return result
        finally:
            self.cancel_first_pcm = False
            assert self.dll.eciClearInput(self.handle) > 0

    def close(self) -> None:
        if self.handle:
            self.handle = self.dll.eciDelete(self.handle)


def corpus(path: Path) -> list[dict]:
    cases = [json.loads(line) for line in path.read_text(encoding="utf-8").splitlines()]
    if len(cases) != 30 or len({case["id"] for case in cases}) != 30:
        raise ValueError("the fixed 30-item German prose corpus changed")
    return cases


def run(dll: Path) -> dict:
    rows: list[dict] = []
    eci = Eci(dll)
    try:
        for item in corpus(ROOT / "tests/data/german_prose_coverage.jsonl"):
            rows.append({**item, "native": eci.speak((item["text"],))})

        # More than a collection of isolated words: paragraph boundaries,
        # typography, numerals, and a long navigation-like utterance.
        for ident, phrase in (
            ("paragraph", "Peter packt Pakete und trinkt Kaffee.\n"
             "Die Sprachausgabe ist klar."),
            ("typography", "Er sagte: „Hallo!“ (Ganz leise.)"),
            ("number", "Am 11. Oktober um 16 Uhr."),
            ("navigation", "Packte. Packten. Tippte. Kippte. " * 12),
            ("nvda_chrome", "Desktop Liste. Google Chrome."),
            ("nvda_control", "Kontrollfeld."),
            ("nvda_settings", "Datei öffnen und speichern."),
            ("nvda_stress", "Internet. Außerdem."),
            ("nvda_number", "42 Elemente."),
            ("nvda_email", "E-Mail-Adresse: Karl@example.de"),
        ):
            rows.append({"id": ident, "source": "extension", "text": phrase,
                         "native": eci.speak((phrase,))})

        # One whole 120-word paragraph through AddText, annotation, Klatt,
        # ECI callbacks. Capture first-PCM separately from full render time.
        long_text = "Peter packt Pakete und trinkt Kaffee. " * 20
        long_prose = eci.speak((long_text,))
        assert long_prose["status"] == "pcm" and long_prose["samples"] > 120 * 100
        assert long_prose["first_pcm_ms"] is not None
        assert long_prose["clipped_samples"] == 0

        # A split within words must not alter PCM or silently truncate them.
        unsplit = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        pcm = bytes(eci.pcm)
        split = eci.speak(("Pack", "te. Packten. Tip", "pte. Kippte."))
        assert unsplit["status"] == split["status"] == "pcm"
        assert unsplit["samples"] == split["samples"] and pcm == eci.pcm

        # A leading backtick in ordinary navigation text is punctuation;
        # only a complete `[...] group selects phonetic notation. Neither
        # case may leave a failed ECI handle that silences the next job.
        for phrase in ("`Code` ist markiert.", "`Code und `Sprache.",
                       "`[.1mEt].", "  `[.1mEt].",
                       "  Code ist markiert.", "„Code“ ist markiert.",
                       "[Code] ist markiert."):
            result = eci.speak((phrase,))
            assert result["status"] == "pcm" and result["samples"] > 0
        assert eci.speak(("Packte. Packten. Tippte. Kippte.",))["status"] == "pcm"
        assert bytes(eci.pcm) == pcm

        # Native inflection changes F0 contour, not elapsed speech duration.
        assert eci.dll.eciGetParam(eci.handle, NATIVE_INFLECTION) == 30
        assert eci.dll.eciSetParam(eci.handle, NATIVE_INFLECTION, 101) == -1
        assert eci.dll.eciSetParam(eci.handle, NATIVE_INFLECTION, 0) == 30
        flat = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        flat_pcm = bytes(eci.pcm)
        assert flat["samples"] == unsplit["samples"] and flat_pcm != pcm
        assert eci.dll.eciSetParam(eci.handle, NATIVE_INFLECTION, 100) == 0
        expressive = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert expressive["samples"] == unsplit["samples"]
        assert bytes(eci.pcm) != flat_pcm and bytes(eci.pcm) != pcm
        assert eci.dll.eciSetParam(eci.handle, NATIVE_INFLECTION, 30) == 100
        assert eci.speak(("Packte. Packten. Tippte. Kippte.",))["status"] == "pcm"
        assert bytes(eci.pcm) == pcm

        # Gain is measured after Klatt. These six independent natural
        # sentences must keep timing and follow the Apple callback ratio,
        # including non-muted volume 0 and exact restoration at 100.
        ratios = {0: 57.363 / 2768.533, 25: 144.450 / 2768.533,
                  50: 363.184 / 2768.533, 75: 813.358 / 2768.533,
                  100: 1.0}
        sentences = (
            "Peter packt Pakete.", "Heute sprechen wir von Deutschland.",
            "Die Sprachausgabe ist klar.", "Mett und Bett.",
            "Kaffee kocht im Kessel.",
            "Wir lesen einen längeren deutschen Satz mit vielen Silben.")
        assert eci.dll.eciGetParam(eci.handle, NATIVE_VOLUME) == 100
        assert eci.dll.eciSetParam(eci.handle, NATIVE_VOLUME, -1) == -1
        assert eci.dll.eciSetParam(eci.handle, NATIVE_VOLUME, 101) == -1
        for sentence in sentences:
            measured = {}
            for value in ratios:
                assert eci.dll.eciSetParam(eci.handle, NATIVE_VOLUME, value) >= 0
                result = eci.speak((sentence,))
                assert result["status"] == "pcm" and result["peak"] > 0
                pcm_samples = memoryview(eci.pcm).cast("h")
                measured[value] = (result["samples"],
                                   math.sqrt(sum(int(x) * x for x in pcm_samples) /
                                             len(pcm_samples)))
                del pcm_samples
            for value, target in ratios.items():
                assert measured[value][0] == measured[100][0]
                assert abs(measured[value][1] / measured[100][1] - target) < .002
        assert eci.dll.eciSetParam(eci.handle, NATIVE_VOLUME, 100) == 100

        # The exact preview must be byte-identical even if a generic voice
        # had last selected the lowest native output volume.
        assert eci.dll.eciSetParam(eci.handle, VOICE_MODE, 0) == 1
        assert eci.speak(("Mett",))["status"] == "pcm"
        exact_reference = bytes(eci.pcm)
        assert eci.dll.eciSetParam(eci.handle, NATIVE_VOLUME, 0) == 100
        assert eci.speak(("Mett",))["status"] == "pcm"
        assert bytes(eci.pcm) == exact_reference
        assert eci.dll.eciSetParam(eci.handle, NATIVE_VOLUME, 100) == 0
        assert eci.dll.eciSetParam(eci.handle, VOICE_MODE, 1) == 0

        # Public native controls: changes must reach the generic renderer,
        # and explicitly restoring defaults must restore identical PCM.
        for param, initial in ((GENERIC_RATE, 50), (GENERIC_PITCH, 50),
                               (GENERIC_VOLUME, 100)):
            assert eci.dll.eciGetParam(eci.handle, param) == initial
            assert eci.dll.eciSetParam(eci.handle, param, -1) == -1
            assert eci.dll.eciSetParam(eci.handle, param, 101) == -1
            assert eci.dll.eciGetParam(eci.handle, param) == initial
        assert eci.dll.eciSetParam(eci.handle, GENERIC_RATE, 80) == 50
        faster = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert faster["status"] == "pcm" and 0 < faster["samples"] < unsplit["samples"]
        assert eci.dll.eciSetParam(eci.handle, GENERIC_RATE, 50) == 80
        assert eci.dll.eciSetParam(eci.handle, GENERIC_PITCH, 65) == 50
        higher = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert higher["status"] == "pcm" and bytes(eci.pcm) != pcm
        assert eci.dll.eciSetParam(eci.handle, GENERIC_PITCH, 50) == 65
        assert eci.dll.eciSetParam(eci.handle, GENERIC_VOLUME, 0) == 100
        muted = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert muted["status"] == "pcm" and muted["samples"] == unsplit["samples"]
        assert muted["peak"] == 0
        assert eci.dll.eciSetParam(eci.handle, GENERIC_VOLUME, 100) == 0
        assert eci.dll.eciReset(eci.handle) > 0
        restored = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert restored["status"] == "pcm" and bytes(eci.pcm) == pcm

        # Separate Apple/OpenEVV-style speed and pitch controls leave the
        # developmental legacy route untouched until explicitly selected.
        assert eci.dll.eciGetParam(eci.handle, NATIVE_SPEED) == 50
        assert eci.dll.eciGetParam(eci.handle, NATIVE_PITCH) == 65
        for value in (0, 39, 251):
            assert eci.dll.eciSetParam(eci.handle, NATIVE_SPEED, value) == -1
        assert eci.dll.eciSetParam(eci.handle, NATIVE_PITCH, 101) == -1
        assert eci.dll.eciSetParam(eci.handle, NATIVE_SPEED, 50) == 50
        assert eci.dll.eciSetParam(eci.handle, NATIVE_PITCH, 65) == 65
        native_baseline = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert native_baseline["status"] == "pcm"
        assert native_baseline["samples"] == restored["samples"]
        assert bytes(eci.pcm) == pcm
        assert eci.dll.eciSetParam(eci.handle, NATIVE_SPEED, 40) == 50
        native_slow = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert native_slow["status"] == "pcm"
        assert native_slow["samples"] > native_baseline["samples"]
        assert eci.dll.eciSetParam(eci.handle, NATIVE_SPEED, 98) == 40
        native_medium = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert native_medium["status"] == "pcm"
        assert native_medium["samples"] < native_baseline["samples"]
        assert eci.dll.eciSetParam(eci.handle, NATIVE_SPEED, 156) == 98
        native_fast = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert native_fast["status"] == "pcm"
        assert native_fast["samples"] < native_medium["samples"]
        assert eci.dll.eciSetParam(eci.handle, NATIVE_SPEED, 50) == 156
        assert eci.dll.eciSetParam(eci.handle, NATIVE_PITCH, 0) == 65
        native_low = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        low_pcm = bytes(eci.pcm)
        assert native_low["status"] == "pcm"
        assert native_low["samples"] == native_baseline["samples"]
        assert eci.dll.eciSetParam(eci.handle, NATIVE_PITCH, 100) == 0
        native_high = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert native_high["status"] == "pcm"
        assert native_high["samples"] == native_baseline["samples"]
        assert bytes(eci.pcm) != low_pcm
        assert eci.dll.eciSetParam(eci.handle, GENERIC_PITCH, 50) == 50
        assert eci.dll.eciSetParam(eci.handle, GENERIC_RATE, 50) == 50
        assert bytes(eci.pcm) != pcm
        assert eci.speak(("Packte. Packten. Tippte. Kippte.",))["status"] == "pcm"
        assert bytes(eci.pcm) == pcm

        # An arbitrary sequence of NVDA fragments and indices now occupies
        # one phrase plan. Word-boundary indices divide *the same* PCM as an
        # unindexed utterance, including two marks on either side of a space.
        unindexed = eci.speak(("Packte. Packten.",), end_index=None)
        unindexed_pcm = bytes(eci.pcm)
        interior = eci.speak(("Packte.", 23, " ", 24, "Packten."), end_index=25)
        assert interior["status"] == "pcm" and bytes(eci.pcm) == unindexed_pcm
        assert interior["index_positions"][0][1] > 0
        assert interior["index_positions"][0][1] < interior["samples"]
        assert interior["index_positions"][0][1] == interior["index_positions"][1][1]
        # A split inside a word stays one acoustic word and moves its mark
        # to that word's end without synthesizing the pieces separately.
        inside = eci.speak(("Pack", 26, "te. Packten."), end_index=27)
        assert inside["status"] == "pcm" and bytes(eci.pcm) == unindexed_pcm
        assert inside["index_positions"][0][1] == interior["index_positions"][0][1]
        umlaut = eci.speak(("Fünf", 28, " ", 29, "Pakete."), end_index=30)
        assert umlaut["status"] == "pcm"
        assert 0 < umlaut["index_positions"][0][1] < umlaut["samples"]
        assert umlaut["index_positions"][0][1] == umlaut["index_positions"][1][1]
        at_start = eci.speak((22, "Packte."), end_index=24)
        assert at_start["status"] == "pcm"
        assert at_start["index_positions"] == [(22, 0), (24, at_start["samples"])]

        # The C queue must distinguish German CP1252 from arbitrary UTF-8.
        for phrase in ("日本語", "🙂"):
            try:
                eci.speak((phrase,))
            except UnicodeEncodeError:
                pass
            else:
                raise AssertionError("non-CP1252 text was silently altered")

        # Stop in a PCM callback, then immediately reuse the same ECI handle.
        stopped = eci.speak(("Packte. Packten. Tippte. Kippte.",),
                            cancel_first_pcm=True)
        assert stopped["status"] == "cancelled" and stopped["samples"] > 0
        recovered = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert recovered["status"] == "pcm" and recovered["samples"] == unsplit["samples"]
        assert pcm == eci.pcm

        # Simulate rapid navigation stopping frontend/planner work. The
        # concurrent eciStop is the sole cross-thread native operation.
        huge_text = "Peter packt Pakete und trinkt Kaffee. " * 1000
        started = threading.Event()
        completed = threading.Event()
        stop_sent = threading.Event()

        def stop_later() -> None:
            started.wait()
            if not completed.wait(0.005):
                eci.dll.eciStop(eci.handle)
                stop_sent.set()

        stopper = threading.Thread(target=stop_later)
        stopper.start()
        try:
            large_cancel = eci.speak((huge_text,), before_synthesize=started.set)
        finally:
            completed.set()
            stopper.join(timeout=2)
        assert not stopper.is_alive()
        assert stop_sent.is_set() and large_cancel["status"] == "cancelled", large_cancel
        assert large_cancel["elapsed_ms"] < 2000, large_cancel
        after_external_stop = eci.speak(("Packte. Packten. Tippte. Kippte.",))
        assert after_external_stop["status"] == "pcm" and pcm == eci.pcm

        # Explicit caller pronunciations still change a selected word.
        # A distinct fixture avoids depending on historical observations
        # differing from the now complete built-in dictionary. Load failures
        # are transactional; a UTF-8 filename works and NULL restores it.
        lex_text = "Dr. Schmidt zahlte 1.234,56 Euro am 3. Mai 2026 um 19:45 Uhr. Die Sprachausgabe ist klar."
        without_lex = eci.speak((lex_text,))
        assert without_lex["status"] == "pcm"
        plain_pcm = bytes(eci.pcm)
        with tempfile.TemporaryDirectory(prefix="reloq-ä-") as temp:
            companion = Path(temp) / "deutsch-ä.rlex"
            key=b"Sprachausgabe";spr=b"`[.1tEst]"
            companion.write_bytes(b"RELOQ-DE-SPR-1\n"+key.hex().upper().encode()+b"\t"+
                spr.hex().upper().encode()+b"\n")
            assert eci.dll.eciLoadLexicon(eci.handle, str(companion).encode("utf-8")) == 1
            with_lex = eci.speak((lex_text,))
            assert with_lex["status"] == "pcm" and bytes(eci.pcm) != plain_pcm
            observed_pcm = bytes(eci.pcm)
            assert eci.dll.eciSetParam(eci.handle, VOICE_MODE, 0) == 1
            exact_loaded = eci.speak(("Mett",))
            assert exact_loaded["status"] == "pcm" and exact_loaded["samples"] == 8338
            exact_loaded_pcm = bytes(eci.pcm)
            assert eci.dll.eciSetParam(eci.handle, VOICE_MODE, 1) == 0
            invalid = Path(temp) / "invalid.rlex"
            invalid.write_text("not a reloq lexicon\n", encoding="utf-8")
            assert eci.dll.eciLoadLexicon(eci.handle, str(invalid).encode("utf-8")) == 0
            assert eci.speak((lex_text,))["status"] == "pcm" and bytes(eci.pcm) == observed_pcm
            assert eci.dll.eciReset(eci.handle) > 0
            assert eci.speak((lex_text,))["status"] == "pcm" and bytes(eci.pcm) == observed_pcm
        assert eci.dll.eciLoadLexicon(eci.handle, None) == 1
        assert eci.speak((lex_text,))["status"] == "pcm" and bytes(eci.pcm) == plain_pcm
    finally:
        eci.close()

    # The whole stream should be reproducible in a newly created ECI handle.
    other = Eci(dll)
    try:
        again = other.speak(("Packte. Packten. Tippte. Kippte.",))
        assert again["status"] == "pcm" and pcm == other.pcm
    finally:
        other.close()

    strict = Eci(dll, generic=False)
    try:
        unchanged = strict.speak(("Mett",))
        assert unchanged["status"] == "pcm" and unchanged["samples"] == 8338
        assert bytes(strict.pcm) == exact_loaded_pcm
        no_default_fallback = strict.speak(("Peter packt Pakete und trinkt Kaffee.",))
        assert no_default_fallback["status"] == "rejected"
        assert no_default_fallback["samples"] == 0
    finally:
        strict.close()

    by_source = {source: dict(Counter(row["native"]["status"] for row in rows
                                    if row["source"] == source))
                 for source in sorted({row["source"] for row in rows})}
    assert all(row["native"].get("clipped_samples", 0) == 0 for row in rows)
    return {"suite": "German opt-in generic ECI prose, 11025 Hz",
            "by_source": by_source, "cases": rows,
            "split_text_pcm_identical": True,
            "interior_index_pcm_boundary_and_identical_audio": True,
            "start_and_end_index_delivered": True,
            "cancel_recovery_pcm_identical": True,
            "long_prose_120_words": long_prose,
            "large_preflight_stop": large_cancel,
            "external_stop_recovery_pcm_identical": True,
            "fresh_engine_pcm_identical": True,
            "native_controls_effective_and_reversible": True,
            "lexicon_load_invalid_retains_unload_restores": True,
            "strict_mode_still_opt_in": True,
            "exact_mode_identical_with_generic_lexicon_loaded": True,
            "unicode_non_cp1252_rejected": True}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--native-eci", type=Path,
                        default=ROOT / "eci/libreloq_eci.so")
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--quiet", action="store_true",
                        help="print only the grouped counts, for make test")
    args = parser.parse_args()
    report = run(args.native_eci.resolve(strict=True))
    body = json.dumps(report, ensure_ascii=False, indent=2) + "\n"
    if args.json_out:
        args.json_out.write_text(body, encoding="utf-8")
    elif not args.quiet:
        print(body)
    print("generic ECI prose:", report["by_source"], file=sys.stderr)


if __name__ == "__main__":
    main()
