#!/usr/bin/env python3
"""Build the selectable NVDA synth-driver add-on from the C engine.

The Windows package carries eci.dll for NVDA. The Linux package carries the
same engine as libreloq_eci.so for Linda, the NVDA port to Linux; it has the
same add-on name, driver and settings.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import re
import zipfile
import shutil
import tempfile
import sys
from urllib.parse import urlsplit
from urllib.request import urlopen
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "nvda"))
from _reloq_dictionaries import FILES as DICTIONARY_FILES, fetch_master, validate_snapshot

from pe_format import assert_x64_pe, assert_x86_64_elf, glibc_floor
from build_translations import translation_entries


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "addon" / "synth"

def addon_version(manifest: Path = SOURCE / "manifest.ini") -> str:
    matches = re.findall(r'^version\s*=\s*"(\d+\.\d+\.\d+(?:-(?:dev|rc)\d+)?)"\s*$',
                         manifest.read_text(encoding="utf-8"), re.M)
    if len(matches) != 1:
        raise ValueError(f"invalid or ambiguous NVDA version: {manifest}")
    return matches[0]

# Package platform: (release asset tag, library entry name, library check, build).
PLATFORMS = {
    "windows": ("x64", "eci.dll", assert_x64_pe, "make -C eci windows-x64"),
    "linux": ("linux-x64", "libreloq_eci.so", assert_x86_64_elf, "make -C eci libreloq_eci.so"),
}
# Linux packages are built on Ubuntu 22.04 so that Linda on older distributions
# can load them; a library needing a newer C library is reported.
RELEASE_GLIBC = (2, 35)


def addon_output(version: str, platform: str = "windows") -> Path:
    if not re.fullmatch(r"\d+\.\d+\.\d+(?:-(?:dev|rc)\d+)?", version):
        raise ValueError("invalid NVDA package version")
    return ROOT / "dist" / f"reloq-c-synth-{PLATFORMS[platform][0]}-{version}.nvda-addon"

DEFAULT_OUTPUT = addon_output(addon_version())
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
    """Historical regression fixture; never included in current packages."""
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


def make_entries(dll_path: Path, platform: str = "windows") -> dict[str, bytes]:
    _tag, library_name, check_library, build_command = PLATFORMS[platform]
    dll = dll_path.read_bytes()
    check_library(dll)
    if platform == "linux" and glibc_floor(dll) > RELEASE_GLIBC:
        print("warning: libreloq_eci.so needs glibc " + ".".join(map(str, glibc_floor(dll))) +
              "; release packages are built against glibc 2.35 or older", file=sys.stderr)
    entries = {}
    revision = os.environ.get("GITHUB_SHA", "")
    if re.fullmatch(r"[0-9a-f]{40}", revision):
        metadata = {
            "source_commit": revision,
            "source_url": "https://github.com/hozosch/Reloq-C/tree/" + revision,
            "build": build_command,
        }
        stamp = os.environ.get("RELOQ_BUILD_EPOCH", "")
        if stamp:
            if not re.fullmatch(r"[1-9]\d{0,11}", stamp) or int(stamp) > 253402300799:
                raise ValueError("invalid build timestamp")
            metadata["build_epoch"] = int(stamp)
        entries["doc/source.json"] = (json.dumps(metadata, indent=2) + "\n").encode("utf-8")
    for name in (
        "manifest.ini",
        "doc/de/readme.html",
        "doc/en/readme.html",
        "synthDrivers/reloqC/__init__.py",
        "synthDrivers/reloqC/_speech_tags.py",
        "globalPlugins/reloqCDictionaries/__init__.py",
    ):
        entries[name] = (SOURCE / name).read_bytes()
    prefix = "synthDrivers/reloqC/"
    for module in ("_reloq_eci_direct.py", "_reloq_eci_transport.py",
                   "_reloq_languages.py", "_reloq_dictionaries.py", "_reloq_dictionary_manager.py",
                   "_reloq_updates.py", "_reloq_nvda_dicts.py", "_reloq_wordseg.py"):
        entries[prefix + module] = (ROOT / "nvda" / module).read_bytes()
    wordseg = ROOT / "nvda" / "wordseg-data"
    american_pieces = (wordseg / "enus-pieces").read_bytes()
    for name in ("enus-sayable", "engb-sayable"):
        entries[prefix + "wordseg-data/" + name] = (wordseg / name).read_bytes()
    entries[prefix + "wordseg-data/enus-pieces"] = american_pieces
    british_pieces = (wordseg / "engb-pieces").read_bytes()
    if british_pieces != american_pieces:
        entries[prefix + "wordseg-data/engb-pieces"] = british_pieces
    entries["doc/licenses/OpenEVV-wordseg-NOTICE.txt"] = (ROOT / "nvda" / "wordseg-data" / "NOTICE").read_bytes()
    entries["doc/licenses/OpenEVV-wordseg-LICENSE.txt"] = (ROOT / "nvda" / "wordseg-data" / "LICENSE").read_bytes()
    entries["doc/licenses/ReLoq-C-LICENSE.txt"] = (ROOT / "LICENSE").read_bytes()
    entries["doc/licenses/ReLoq-C-NOTICE.txt"] = (ROOT / "NOTICE").read_bytes()
    entries["doc/licenses/Dictionaries-CC0.txt"] = (ROOT / "data/community/LICENSE.md").read_bytes()
    entries["doc/licenses/Unicode-LICENSE.txt"] = (ROOT / "reference-apple/behavior-0.2.82/unicode-input/LICENSE.txt").read_bytes()
    entries["doc/licenses/SCOWL-Copyright.txt"] = (ROOT / "reference-apple/behavior-0.2.83/input/SCOWL-Copyright.txt").read_bytes()
    entries["doc/licenses/OpenEVV-LICENSE.txt"] = (ROOT / "vendor/openevv61/LICENSE").read_bytes()
    entries["doc/licenses/OpenEVV-NOTICE.txt"] = (ROOT / "vendor/openevv61/NOTICE").read_bytes()
    validate_snapshot(ROOT / "data/community")
    for name in (*DICTIONARY_FILES, "manifest.json"):
        entries[prefix + "communityDictionaries/" + name] = (ROOT / "data/community" / name).read_bytes()
    entries[prefix + library_name] = dll
    entries.update(translation_entries())
    if any(name.endswith((".so", ".dll", ".syn", ".dylib")) and name != prefix + library_name
           for name in entries):
        raise ValueError("unexpected binary or language module")
    return entries


def _build_dictionary_fetch(request, timeout):
    """Authenticate only the build's GitHub API requests, not raw blobs.

    Unredirected headers prevent a cross-host redirect from forwarding the
    short-lived workflow token. Ordinary installed dictionary updates do not
    use this build-only adapter.
    """
    token = os.environ.get("RELOQ_BUILD_GITHUB_TOKEN")
    location = urlsplit(request.full_url)
    if token and location.scheme == "https" and location.netloc == "api.github.com":
        request.add_unredirected_header("Authorization", "Bearer " + token)
    return urlopen(request, timeout=timeout)


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
        if os.environ.get("RELOQ_BUILD_GITHUB_TOKEN"):
            snapshot = fetch_master(cache, fetch=_build_dictionary_fetch)
        else:
            snapshot = fetch_master(cache)
        destination.mkdir(parents=True, exist_ok=True)
        for name in (*DICTIONARY_FILES, "manifest.json"):
            shutil.copyfile(snapshot / name, destination / name)


def build(output: Path, dll_path: Path, *, refresh_dictionaries: bool = True,
          platform: str = "windows") -> Path:
    library_entry = "synthDrivers/reloqC/" + PLATFORMS[platform][1]
    if refresh_dictionaries:
        refresh_community_dictionaries()
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in sorted(make_entries(dll_path, platform).items()):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            # NVDA 2026.1 and 2026.3.1 both extract ZIP-LZMA with their own
            # AddonBundle and can load the resulting DLL. Apply it only to
            # the large native library; extraction happens at install time.
            info.compress_type = (zipfile.ZIP_LZMA if name == library_entry
                                  else zipfile.ZIP_DEFLATED)
            archive.writestr(info, data, compresslevel=9 if info.compress_type == zipfile.ZIP_DEFLATED else None)
    return output


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--platform", choices=sorted(PLATFORMS), default="windows",
                        help="windows: NVDA with eci.dll; linux: Linda with libreloq_eci.so")
    parser.add_argument("--dll", type=Path,
                        help="the engine library (default: eci/eci.dll or eci/libreloq_eci.so)")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    library = args.dll or ROOT / "eci" / PLATFORMS[args.platform][1]
    output = args.output or addon_output(addon_version(), args.platform)
    print(build(output, library, platform=args.platform))


if __name__ == "__main__":
    main()
