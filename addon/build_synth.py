#!/usr/bin/env python3
"""Build a separate, selectable NVDA synth-driver add-on from the C DLL."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import zipfile
import shutil
import tempfile
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "nvda"))
from _reloq_dictionaries import FILES as DICTIONARY_FILES, fetch_master, validate_snapshot

from build_preview import assert_x64_pe
from build_translations import translation_entries


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "addon" / "synth"
DEFAULT_OUTPUT = ROOT / "dist" / "reloq-c-synth-experimental-x64-0.2.192.nvda-addon"
RLEX_HEADER = b"RELOQ-DE-SPR-1\n"
HEX = re.compile(rb"[0-9A-F]+\Z")


def lexicon_rows(source: bytes) -> list[tuple[bytes, bytes]]:
    """Read exact rows while rejecting unsorted or duplicate input keys."""
    if not source.startswith(RLEX_HEADER) or not source.endswith(b"\n"):
        raise ValueError("invalid German lexicon header or final newline")
    rows = []
    previous = b""
    for line in source[len(RLEX_HEADER):].splitlines(keepends=True):
        halves = line[:-1].split(b"\t")
        if len(halves) != 2 or any(
            not part or len(part) % 2 or not HEX.fullmatch(part)
            for part in halves
        ):
            raise ValueError("invalid German lexicon hex record")
        key = bytes.fromhex(halves[0].decode("ascii"))
        if key <= previous:
            raise ValueError("unsorted or duplicate German lexicon key")
        rows.append((key, line))
        previous = key
    return rows


def merged_german_lexicon() -> bytes:
    combined = sorted(
        (row for name in ("german_apple_observed.rlex",
                          "german_apple_hunspell_context.rlex",
                          "german_apple_dictionary.rlex",
                          "german_apple_verb_paradigms.rlex", "german_apple_roots82.rlex")
         for row in lexicon_rows((ROOT / "data" / name).read_bytes())),
        key=lambda item: item[0])
    if any(combined[i - 1][0] == combined[i][0]
           for i in range(1, len(combined))):
        raise ValueError("German lexicon supplements overlap")
    return RLEX_HEADER + b"".join(line for _, line in combined)


def make_entries(dll_path: Path) -> dict[str, bytes]:
    dll = dll_path.read_bytes()
    assert_x64_pe(dll)
    entries = {}
    for name in (
        "manifest.ini",
        "doc/de/readme.html",
        "doc/en/readme.html",
        "synthDrivers/reloqC/__init__.py",
        "globalPlugins/reloqCDictionaries/__init__.py",
    ):
        entries[name] = (SOURCE / name).read_bytes()
    prefix = "synthDrivers/reloqC/"
    for module in ("_reloq_eci_direct.py", "_reloq_eci_transport.py",
                   "_reloq_languages.py", "_reloq_dictionaries.py", "_reloq_dictionary_manager.py"):
        entries[prefix + module] = (ROOT / "nvda" / module).read_bytes()
    entries["doc/licenses/Unicode-LICENSE.txt"] = (ROOT / "reference-apple/behavior-0.2.82/unicode-input/LICENSE.txt").read_bytes()
    entries["doc/licenses/SCOWL-Copyright.txt"] = (ROOT / "reference-apple/behavior-0.2.83/input/SCOWL-Copyright.txt").read_bytes()
    entries["doc/licenses/OpenEVV-LICENSE.txt"] = (ROOT / "vendor/openevv61/LICENSE").read_bytes()
    entries["doc/licenses/OpenEVV-NOTICE.txt"] = (ROOT / "vendor/openevv61/NOTICE").read_bytes()
    validate_snapshot(ROOT / "data/community")
    for name in (*DICTIONARY_FILES, "manifest.json"):
        entries[prefix + "communityDictionaries/" + name] = (ROOT / "data/community" / name).read_bytes()
    entries[prefix + "eci.dll"] = dll
    entries[prefix + "german_apple_observed.rlex"] = merged_german_lexicon()
    entries.update(translation_entries())
    if any(name.endswith((".so", ".syn", ".dylib")) for name in entries):
        raise ValueError("unexpected binary or language module")
    return entries


def refresh_community_dictionaries() -> None:
    """Resolve master for each platform build, reusing verified blob data."""
    with tempfile.TemporaryDirectory(prefix="reloq-dictionary-build-") as temporary:
        cache = Path(temporary)
        destination = ROOT / "data/community"
        # Existing verified Git blobs can be reused, but the master head
        # is always resolved anew. A different head downloads a new set.
        try:
            manifest = validate_snapshot(destination)
        except (OSError, ValueError, KeyError, TypeError):
            manifest = None
        if manifest:
            cached = cache / manifest["commit"]
            cached.mkdir()
            for name in (*DICTIONARY_FILES, "manifest.json"):
                shutil.copyfile(destination / name, cached / name)
        snapshot = fetch_master(cache)
        destination.mkdir(parents=True, exist_ok=True)
        for name in (*DICTIONARY_FILES, "manifest.json"):
            shutil.copyfile(snapshot / name, destination / name)


def build(output: Path, dll_path: Path, *, refresh_dictionaries: bool = True) -> Path:
    if refresh_dictionaries:
        refresh_community_dictionaries()
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in sorted(make_entries(dll_path).items()):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, data, compresslevel=9)
    return output


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dll", type=Path, default=ROOT / "eci" / "eci.dll")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()
    print(build(args.output, args.dll))


if __name__ == "__main__":
    main()
