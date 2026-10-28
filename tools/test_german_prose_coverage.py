#!/usr/bin/env python3
"""Exercise the fixed natural-prose benchmark and its diagnostic invariants."""

from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]


def main() -> None:
    with tempfile.TemporaryDirectory(prefix="reloq-prose-test-") as temp:
        path = Path(temp) / "coverage.json"
        subprocess.run([sys.executable, "-B", str(ROOT / "tools/german_prose_coverage.py"),
                        "--native-eci", str(ROOT / "eci/libreloq_eci.so"),
                        "--json-out", str(path)], check=True, timeout=120,
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        result = json.loads(path.read_text(encoding="utf-8"))
    rows = result["cases"]
    assert len(rows) == 30
    assert {row["source"] for row in rows} == {
        "user_capture", "general_prose_fixture", "calibration"}
    assert len({row["id"] for row in rows}) == len(rows)
    assert sum(row["source"] == "user_capture" for row in rows) == 19
    assert any(row["id"] == "dede_loanword" and "packt Pakete" in row["text"]
               for row in rows)
    assert any(row["id"] == "dede_stress_sentence" and "Praktikum" in row["text"]
               for row in rows)
    for row in rows:
        frontend = row["text_plain"]["status"]
        native = row["native"]["status"]
        if native in ("pcm_produced", "acoustic_unsupported"):
            assert frontend == "known", row
        if native == "text_uncovered":
            assert frontend in ("uncovered", "invalid"), row
        if row["reference"] == "exact":
            assert result["reference_checked"] and native == "pcm_produced"
        if native == "pcm_produced":
            assert row["native"]["samples"] > 0
    summary = result["summary"]
    assert summary["natural_word_occurrences"] == 135
    assert summary["lexicon_known_occurrences"] >= summary["plain_known_occurrences"]
    assert sum(group["utterances"] for group in summary["by_source"].values()) == len(rows)
    assert all(row["native"]["status"] == "pcm_produced"
               for row in rows if row["source"] == "calibration")
    print("German prose benchmark: fixed user corpus, ECI diagnosis,"
          " optional lexicon, and nonleaking PCM classification PASS")


if __name__ == "__main__":
    main()
