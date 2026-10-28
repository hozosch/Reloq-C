#!/usr/bin/env python3
"""Measure whole German sentences through the same native ECI API as NVDA.

Each case/mode runs in a separate subprocess: a native crash or hang becomes
one observable result rather than hiding the rest of the sample. The worker
queues one text plus a final ECI index and then says a supported control on
the *same handle*, so silence after a rejection is also observable. No
Apple/OpenEVV library or proprietary data is loaded. Generic is an explicitly
opted-in approximation, and PCM production does not imply correct speech.
"""

from __future__ import annotations

import argparse
from array import array
from collections import Counter, defaultdict
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import time


ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "nvda"))
from _reloq_eci_direct import DirectEciEngine, EciIndex, EciSynthesisError  # noqa: E402
from _reloq_unicode import normalize_generic_text  # noqa: E402

REFERENCES = ("Mett", "Bett")
WORD = re.compile(r"[A-Za-zÄÖÜäöüß]+")


def load_cases(extra: Path) -> list[dict[str, str]]:
    paths = (ROOT / "tests/data/german_prose_coverage.jsonl", extra)
    rows = []
    ids = set()
    for path in paths:
        for line_no, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
            if not line.strip():
                continue
            row = json.loads(line)
            if path == paths[0]:
                row["category"] = "baseline_" + row.pop("source")
            required = {"id", "category", "text"}
            optional = {"left", "right", "joiner"}
            if not required <= set(row) or set(row) - required - optional:
                raise ValueError(f"{path}:{line_no}: expected id/category/text")
            if row["category"] == "compound_positive":
                valid = (optional <= set(row) and
                         all(isinstance(row[key], str) for key in optional) and
                         row["left"] + row["joiner"] + row["right"] == row["text"])
                if not valid:
                    raise ValueError(f"{path}:{line_no}: invalid compound components")
            elif set(row) & optional:
                raise ValueError(f"{path}:{line_no}: unexpected component fields")
            if (not all(isinstance(row[key], str) and row[key] for key in required) or
                    row["id"] in ids or "\0" in row["text"] or "\n" in row["text"]):
                raise ValueError(f"{path}:{line_no}: invalid or duplicate row")
            if row["category"] != "unicode_admission":
                row["text"].encode("cp1252")
            ids.add(row["id"])
            rows.append(row)
    return rows


def audio_stats(blocks: list[bytes]) -> dict:
    pcm = b"".join(blocks)
    if len(pcm) & 1:
        raise RuntimeError("odd-length PCM callback")
    samples = array("h")
    samples.frombytes(pcm)
    if sys.byteorder != "little":
        samples.byteswap()
    active = sum(sample != 0 for sample in samples)
    return {"samples": len(samples), "callbacks": len(blocks),
            "nonzero_samples": active,
            "peak_abs": max((abs(v) for v in samples), default=0),
            "pcm_sha256": hashlib.sha256(pcm).hexdigest()}


def speak_one(engine: DirectEciEngine, text: str, final_index: int) -> dict:
    blocks: list[bytes] = []
    index: list[dict[str, int]] = []

    def on_index(value: int) -> None:
        index.append({"value": value, "after_samples": sum(len(b) for b in blocks) // 2})

    started = time.monotonic()
    try:
        accepted = engine.speak((text, EciIndex(final_index)), blocks.append, on_index)
    except EciSynthesisError as error:
        result = {"status": "eci_rejected", "native_status": error.status,
                  "stage": error.stage, "utf8_offset": error.utf8_offset,
                  "character_position": error.character_position}
    except Exception as error:
        result = {"status": "python_error", "error": repr(error)}
    else:
        result = {"status": "accepted" if accepted else "cancelled"}
    result.update(audio_stats(blocks))
    result["index"] = index
    result["elapsed_ms"] = round((time.monotonic() - started) * 1000, 1)
    if result["status"] == "accepted":
        if result["samples"] == 0:
            result["status"] = "accepted_without_pcm"
        elif result["nonzero_samples"] == 0:
            result["status"] = "all_zero_pcm"
        elif index != [{"value": final_index, "after_samples": result["samples"]}]:
            result["status"] = "index_mismatch"
        else:
            result["status"] = "pcm_with_index"
    return result


def worker(eci: Path, mode: str, phrase: str, lexicon: Path | None) -> dict:
    engine = DirectEciEngine(eci, experimental_generic=(mode == "generic"),
                             lexicon_path=lexicon if mode == "generic" else None)
    try:
        result = speak_one(engine, phrase, 9101)
        # Recovery probes the same ECI handle twice. These controls are
        # excluded from the sentence coverage numerator.
        result["following_controls"] = [
            speak_one(engine, phrase, 9102 + offset)
            for offset, phrase in enumerate(REFERENCES)]
    finally:
        engine.close()
    return result


def diagnostic_token(text: str, result: dict) -> str | None:
    byte = result.get("utf8_offset")
    if not isinstance(byte, int) or byte < 0:
        return None
    encoded = text.encode("utf-8")
    at = len(encoded[:min(byte, len(encoded))].decode("utf-8", "ignore"))
    for match in WORD.finditer(text):
        if match.start() <= at < match.end():
            return match.group()
    return text[at] if at < len(text) else "<end>"


def frontend_rows(probe: Path, cases: list[dict], lexicon: Path | None,
                  addon_admission: bool) -> dict[str, dict]:
    texts = [normalize_generic_text(row["text"]).replace("`", " ")
             if addon_admission else row["text"] for row in cases]
    completed = subprocess.run([str(probe)] + ([str(lexicon)] if lexicon else []),
                               input=("\n".join(texts) + "\n").encode("utf-8"),
                               capture_output=True, check=True, timeout=30)
    lines = completed.stdout.decode("ascii").splitlines()
    if len(lines) != len(cases):
        raise RuntimeError(f"frontend gave {len(lines)} rows for {len(cases)} cases")
    result = {}
    for case, line in zip(cases, lines):
        status, bad, guessed, groups, length = map(int, line.split("\t"))
        result[case["id"]] = {"status": status, "encoded_error_offset": bad,
                               "approximate_groups": guessed, "spr_groups": groups,
                               "annotation_bytes": length}
    return result


def aggregate(rows: list[dict]) -> dict:
    classes: dict[str, list[dict]] = defaultdict(list)
    by_mode: dict[str, list[dict]] = defaultdict(list)
    for row in rows:
        classes[row["category"]].append(row)
        by_mode[row["mode"]].append(row)

    def counts(subset: list[dict]) -> dict:
        outcome = Counter(row["result"]["status"] for row in subset)
        stages = Counter(str(row["result"].get("stage", "none"))
                         for row in subset if row["result"]["status"] == "eci_rejected")
        recoveries = Counter(
            "/".join(control["status"] for control in
                     row["result"].get("following_controls", [])) or "none"
            for row in subset)
        frontend = [row["frontend"] for row in subset if row.get("frontend")]
        return {"total": len(subset), "outcomes": dict(sorted(outcome.items())),
                "rejection_stage": dict(sorted(stages.items())),
                "following_controls": dict(sorted(recoveries.items())),
                "frontend_approximate_groups": sum(row["approximate_groups"] for row in frontend)
                if frontend else None,
                "frontend_spr_groups": sum(row["spr_groups"] for row in frontend)
                if frontend else None}

    return {"by_mode": {name: counts(value) for name, value in sorted(by_mode.items())},
            "by_category_mode": {
                f"{category}/{mode}": counts([row for row in values if row["mode"] == mode])
                for category, values in sorted(classes.items())
                for mode in sorted(by_mode)}}


def run_all(args: argparse.Namespace) -> dict:
    cases = load_cases(args.extra)
    if args.category:
        cases = [case for case in cases if case["category"] == args.category]
    frontend = (frontend_rows(args.frontend_probe, cases, args.lexicon,
                              args.addon_admission) if args.frontend_probe else {})
    rows = []
    modes = ("exact", "generic") if args.mode == "both" else (args.mode,)
    for case in cases:
        for mode in modes:
            delivered = (normalize_generic_text(case["text"]).replace("`", " ")
                         if args.addon_admission else case["text"])
            command = [sys.executable, "-B", str(Path(__file__).resolve()),
                       "--worker", "--eci", str(args.eci), "--mode", mode]
            if args.lexicon:
                command.extend(("--lexicon", str(args.lexicon)))
            try:
                completed = subprocess.run(command, input=json.dumps(delivered),
                                           encoding="utf-8", capture_output=True,
                                           timeout=args.timeout)
            except subprocess.TimeoutExpired:
                result = {"status": "timeout", "seconds": args.timeout}
            else:
                if completed.returncode:
                    result = {"status": "worker_failed", "exit_code": completed.returncode,
                              "stderr": completed.stderr[-1000:]}
                else:
                    try:
                        result = json.loads(completed.stdout)
                    except json.JSONDecodeError:
                        result = {"status": "malformed_worker_output",
                                  "stdout": completed.stdout[-1000:]}
            row = {**case, "mode": mode, "delivered_text": delivered, "result": result}
            row["failure_token"] = diagnostic_token(delivered, result)
            if frontend:
                row["frontend"] = frontend[case["id"]]
            rows.append(row)
            if args.verbose:
                print(f"{case['id']} {mode}: {result['status']} "
                      f"stage={result.get('stage')} token={row['failure_token']}",
                      file=sys.stderr, flush=True)
    return {"description": "whole ECI text request, isolated worker per case/mode",
            "note": "generic speech is approximate; PCM success does not imply correct pronunciation",
            "eci": str(args.eci), "lexicon": str(args.lexicon) if args.lexicon else None,
            "addon_admission": args.addon_admission,
            "summary": aggregate(rows), "cases": rows}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--eci", type=Path,
                        default=ROOT / "eci/libreloq_eci.so")
    parser.add_argument("--extra", type=Path,
                        default=ROOT / "tests/data/german_natural_sentence_holdout.jsonl")
    parser.add_argument("--lexicon", type=Path,
                        help="optional .rlex, same generic lexicon as the packaged add-on")
    parser.add_argument("--frontend-probe", type=Path,
                        help="optional compiled reloq_best_effort_coverage diagnostic")
    parser.add_argument("--mode", choices=("exact", "generic", "both"), default="both")
    parser.add_argument("--category", help="limit to one named corpus category")
    parser.add_argument("--addon-admission", action="store_true",
                        help="apply the add-on's Unicode normalization and backtick policy")
    parser.add_argument("--timeout", type=float, default=10)
    parser.add_argument("--json-out", type=Path)
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--worker", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()
    args.eci = args.eci.resolve(strict=True)
    if args.lexicon:
        args.lexicon = args.lexicon.resolve(strict=True)
    if args.frontend_probe:
        args.frontend_probe = args.frontend_probe.resolve(strict=True)
    if args.worker:
        try:
            phrase = json.load(sys.stdin)
            if not isinstance(phrase, str):
                raise ValueError("worker expects a JSON string")
            print(json.dumps(worker(args.eci, args.mode, phrase, args.lexicon), ensure_ascii=False))
            return 0
        except Exception as error:
            print(f"worker exception: {error!r}", file=sys.stderr)
            return 2
    result = run_all(args)
    body = json.dumps(result, ensure_ascii=False, indent=2) + "\n"
    if args.json_out:
        args.json_out.write_text(body, encoding="utf-8")
    else:
        print(body)
    print(json.dumps(result["summary"], ensure_ascii=False), file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
