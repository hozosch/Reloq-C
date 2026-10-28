"""Export independent ECI phoneme observations to a portable German lexicon.

The original bulk collection is restricted to isolated ASCII words with one
complete SPR bracket: its non-ASCII labels were sent with the wrong encoding.
An optional, separately captured CP1252 correction file can provide German
letters when its rows explicitly record that encoding. Each candidate is
checked again through Apple's public ECI API. No runtime code or bytecode is
read or reproduced. The resulting .rlex is *pronunciation* data only; a phone
sequence, duration, prosody and PCM still require separate C synthesis code.

Format v1 is ASCII, one row per line. It begins with a literal
``RELOQ-DE-SPR-1`` line, followed by uppercase-hex(CP1252 word), TAB,
uppercase-hex(ASCII SPR group), LF records.

The byte-ordered keys permit binary search after native C validation. Hex
encoding keeps file I/O independent of the host's locale or text encoding.
"""

from __future__ import annotations

import argparse
import ast
import collections
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

WORD = re.compile(r"[A-Za-z]+\Z")
CP1252_WORD = re.compile(r"[A-Za-zÄÖÜäöüß]+\Z")
ONE_GROUP = re.compile(r"\s*(`\[[^\[\]\x00]+\])\x00\Z")


def gather(source: Path) -> tuple[list[tuple[bytes, bytes]], collections.Counter[str]]:
    entries: dict[bytes, bytes] = {}
    reasons: collections.Counter[str] = collections.Counter()
    with source.open("r", encoding="utf-8") as stream:
        for line_number, line in enumerate(stream, 1):
            try:
                item = json.loads(line)
            except json.JSONDecodeError as error:
                raise ValueError(f"invalid JSON at line {line_number}") from error
            reasons["input_rows"] += 1
            if not isinstance(item, dict) or item.get("lang") != "dede":
                reasons["other_language"] += 1
                continue
            if item.get("status") != "ok":
                reasons["non_success"] += 1
                continue
            word = item.get("text")
            phonemes = item.get("phonemes")
            if not isinstance(word, str) or not WORD.fullmatch(word):
                reasons["context_or_non_ascii"] += 1
                continue
            if not isinstance(phonemes, str) or not (match := ONE_GROUP.fullmatch(phonemes)):
                reasons["not_single_group"] += 1
                continue
            key = word.encode("ascii")
            value = match.group(1).encode("ascii")
            if len(key) > 127 or len(value) > 4096:
                reasons["too_long"] += 1
                continue
            if key in entries:
                if entries[key] != value:
                    raise ValueError(f"conflicting observations for {word!r}")
                reasons["duplicate_identical"] += 1
                continue
            entries[key] = value
            reasons["accepted"] += 1
    return sorted(entries.items()), reasons


def gather_cp1252_corrections(
    source: Path, existing: list[tuple[bytes, bytes]],
) -> tuple[list[tuple[bytes, bytes]], collections.Counter[str]]:
    """Accept explicitly CP1252-captured, single-word public observations."""
    entries = dict(existing)
    reasons: collections.Counter[str] = collections.Counter()
    with source.open("r", encoding="utf-8") as stream:
        for line_number, line in enumerate(stream, 1):
            try:
                item = json.loads(line)
            except json.JSONDecodeError as error:
                raise ValueError(f"invalid correction JSON at line {line_number}") from error
            reasons["correction_rows"] += 1
            if (not isinstance(item, dict) or item.get("lang") != "dede" or
                    item.get("input_encoding") != "cp1252" or
                    item.get("reference_status") != "ok"):
                reasons["correction_unverified"] += 1
                continue
            word = item.get("text")
            phonemes = item.get("phonemes")
            if not isinstance(word, str) or not CP1252_WORD.fullmatch(word):
                reasons["correction_not_word"] += 1
                continue
            if not isinstance(phonemes, str) or not (match := ONE_GROUP.fullmatch(phonemes)):
                reasons["correction_not_single_group"] += 1
                continue
            key = word.encode("cp1252")
            value = match.group(1).encode("ascii")
            if len(key) > 127 or len(value) > 4096:
                reasons["correction_too_long"] += 1
                continue
            if key in entries:
                if entries[key] != value:
                    raise ValueError(f"conflicting correction for {word!r}")
                reasons["correction_duplicate"] += 1
                continue
            entries[key] = value
            reasons["correction_candidate"] += 1
    return sorted(entries.items()), reasons


def keep_exact_apple(
    rows: list[tuple[bytes, bytes]], probe: Path, eci: Path, workdir: Path,
) -> tuple[list[tuple[bytes, bytes]], collections.Counter[str]]:
    """Use public API outputs in new processes; never read binary internals."""
    kept = []
    reasons: collections.Counter[str] = collections.Counter()
    for key, spr in rows:
        result = subprocess.run(
            # POSIX subprocess accepts a byte argv item, preserving the ECI
            # CP1252 input instead of silently recoding it as UTF-8.
            (str(probe), str(eci), key, "0"),
            cwd=workdir, env=os.environ.copy(), capture_output=True,
            timeout=10, check=False,
        )
        lines = result.stdout.decode("ascii", errors="replace").splitlines()
        if result.returncode or len(lines) < 2 or not lines[-1].startswith('"'):
            reasons["apple_error"] += 1
            continue
        try:
            apple = ast.literal_eval(lines[-1])
            if not isinstance(apple, str):
                raise ValueError("not a quoted string")
            output = apple.encode("latin1")
        except (ValueError, SyntaxError, UnicodeError):
            reasons["apple_error"] += 1
            continue
        if output == b" " + spr + b"\x00":
            kept.append((key, spr))
            reasons["exact_apple"] += 1
        else:
            reasons["apple_differs"] += 1
    return kept, reasons


def render(rows: list[tuple[bytes, bytes]]) -> bytes:
    return b"RELOQ-DE-SPR-1\n" + b"".join(
        key.hex().upper().encode("ascii") + b"\t" +
        spr.hex().upper().encode("ascii") + b"\n"
        for key, spr in rows
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="user-collected ECI JSONL")
    parser.add_argument("output", type=Path, help="portable .rlex output")
    parser.add_argument("--apple-probe", type=Path, required=True)
    parser.add_argument("--apple-eci", type=Path, required=True)
    parser.add_argument("--apple-workdir", type=Path, required=True)
    parser.add_argument("--cp1252-corrections", type=Path,
                        help="separately captured, explicitly CP1252 German rows")
    parser.add_argument("--check", action="store_true", help="compare without writing")
    args = parser.parse_args()
    rows, reasons = gather(args.input)
    if args.cp1252_corrections:
        rows, correction_reasons = gather_cp1252_corrections(
            args.cp1252_corrections, rows)
        reasons.update(correction_reasons)
    rows, comparison = keep_exact_apple(
        rows, args.apple_probe.resolve(), args.apple_eci.resolve(),
        args.apple_workdir.resolve(),
    )
    reasons.update(comparison)
    data = render(rows)
    if args.check:
        if args.output.read_bytes() != data:
            raise SystemExit("checked-in lexicon differs from source observations")
    else:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_bytes(data)
    print(json.dumps({"counts": reasons, "entries": len(rows),
                      "sha256": hashlib.sha256(data).hexdigest()}, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
