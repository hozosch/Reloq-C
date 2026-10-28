#!/usr/bin/env python3
"""End-to-end regression gate for sentence PCM, symbols, Unicode and indices.

Run after ``make -C eci libreloq_eci.so``. The measured generic mode is
experimental; successful PCM does not assert correct pronunciation.
"""

from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parents[1]
MATRIX = ROOT / "tools/german_eci_sentence_matrix.py"


def measure(extra: Path, category: str | None, output: Path) -> list[dict]:
    command = [sys.executable, "-B", str(MATRIX), "--eci",
               str(ROOT / "eci/libreloq_eci.so"), "--mode", "generic",
               "--addon-admission", "--lexicon",
               str(ROOT / "data/german_apple_observed.rlex"),
               "--extra", str(extra), "--json-out", str(output)]
    if category:
        command.extend(("--category", category))
    subprocess.run(command, capture_output=True, check=True, timeout=90)
    cases = json.loads(output.read_text(encoding="utf-8"))["cases"]
    for case in cases:
        result = case["result"]
        assert result["status"] == "pcm_with_index", case
        assert result["samples"] > 0 and result["nonzero_samples"] > 0, case
        assert result["index"] == [{"value": 9101,
                                     "after_samples": result["samples"]}], case
        assert [follow["status"] for follow in result["following_controls"]] == [
            "pcm_with_index", "pcm_with_index"], case
    return cases


def main() -> None:
    with tempfile.TemporaryDirectory(prefix="reloq-sentence-matrix-") as temp:
        output = Path(temp) / "matrix.json"
        regular = measure(ROOT / "tests/data/german_natural_sentence_holdout.jsonl",
                          None, output)
        unicode = measure(ROOT / "tests/data/german_unicode_admission.jsonl",
                          "unicode_admission", output)
    assert len(regular) == 122 and len(unicode) == 8
    assert {row["category"] for row in regular if row["id"].startswith("symbols_")} == {
        "cp1252_symbols"}
    print("German ECI sentence matrix: 122 whole texts, eight Unicode admissions,"
          " PCM/index/recovery PASS")


if __name__ == "__main__":
    main()
