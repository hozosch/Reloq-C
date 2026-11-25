#!/usr/bin/env python3
"""Package the Linux Speech Dispatcher module, library and dictionaries.

The result, dist/reloq-c-linux-x64-<version>.tar.gz, unpacks into one
directory with install.sh. The archive is reproducible: entries are sorted,
owned by root and dated RELOQ_BUILD_EPOCH (or 1 January 1980).

    make -C eci -j4 libreloq_eci.so sd_reloq
    python3 linux/package.py
"""

from __future__ import annotations

import argparse
import gzip
import io
import os
from pathlib import Path
import sys
import tarfile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "addon"))
sys.path.insert(0, str(ROOT / "nvda"))
from build_synth import addon_version  # noqa: E402
from pe_format import assert_x86_64_elf  # noqa: E402
from _reloq_dictionaries import FILES as DICTIONARY_FILES, validate_snapshot  # noqa: E402

LICENSES = {
    "ReLoq-C-LICENSE.txt": "LICENSE",
    "ReLoq-C-NOTICE.txt": "NOTICE",
    "OpenEVV-LICENSE.txt": "vendor/openevv61/LICENSE",
    "OpenEVV-NOTICE.txt": "vendor/openevv61/NOTICE",
    "SpeechDispatcher-module-LICENSE.txt": "vendor/speechd-module/LICENSE",
    "Dictionaries-CC0.txt": "data/community/LICENSE.md",
    "Unicode-LICENSE.txt": "reference-apple/behavior-0.2.82/unicode-input/LICENSE.txt",
    "SCOWL-Copyright.txt": "reference-apple/behavior-0.2.83/input/SCOWL-Copyright.txt",
}


def output_name(version: str) -> str:
    return f"reloq-c-linux-x64-{version}.tar.gz"


def entries(library: Path, module: Path) -> dict[str, tuple[bytes, int]]:
    """Archive member name -> (contents, mode), relative to the top directory."""
    library_data = library.read_bytes()
    module_data = module.read_bytes()
    assert_x86_64_elf(library_data)
    # ET_EXEC or, for a position-independent executable, ET_DYN; x86-64.
    if (len(module_data) < 64 or module_data[:4] != b"\x7fELF" or module_data[16] not in (2, 3)
            or module_data[18:20] != b"\x3e\x00"):
        raise ValueError("sd_reloq is not an x86-64 ELF executable")
    validate_snapshot(ROOT / "data/community")
    files = {
        "install.sh": ((ROOT / "linux/install.sh").read_bytes(), 0o755),
        "README.md": ((ROOT / "linux/README.md").read_bytes(), 0o644),
        "sd_reloq": (module_data, 0o755),
        "reloq.conf": ((ROOT / "speechd/reloq.conf").read_bytes(), 0o644),
        "reloq-speechd-enable": ((ROOT / "speechd/reloq-speechd-enable").read_bytes(), 0o755),
        "libreloq_eci.so": (library_data, 0o755),
        "include/eci.h": ((ROOT / "include/eci.h").read_bytes(), 0o644),
        "include/reloq_eci.h": ((ROOT / "include/reloq_eci.h").read_bytes(), 0o644),
    }
    for name in DICTIONARY_FILES:
        if name.endswith(".dic"):
            files["dictionaries/" + name] = ((ROOT / "data/community" / name).read_bytes(), 0o644)
    for name, source in LICENSES.items():
        files["licenses/" + name] = ((ROOT / source).read_bytes(), 0o644)
    return files


def build(output: Path, library: Path, module: Path, version: str | None = None) -> Path:
    version = version or addon_version()
    top = output_name(version).removesuffix(".tar.gz")
    stamp = int(os.environ.get("RELOQ_BUILD_EPOCH") or 315532800)
    output.parent.mkdir(parents=True, exist_ok=True)
    files = entries(library, module)
    directories = sorted({top} | {f"{top}/{name.rsplit('/', 1)[0]}" for name in files if "/" in name})
    # The gzip header would otherwise record the output name and the time.
    with output.open("wb") as raw, gzip.GzipFile(filename="", mode="wb", fileobj=raw,
                                                  compresslevel=9, mtime=stamp) as compressed, \
            tarfile.open(fileobj=compressed, mode="w", format=tarfile.PAX_FORMAT) as archive:
        def member(name, kind, mode, size=0):
            info = tarfile.TarInfo(name)
            info.type, info.mode, info.size, info.mtime = kind, mode, size, stamp
            info.uid = info.gid = 0
            info.uname = info.gname = "root"
            return info
        for directory in directories:
            archive.addfile(member(directory, tarfile.DIRTYPE, 0o755))
        for name, (data, mode) in sorted(files.items()):
            archive.addfile(member(f"{top}/{name}", tarfile.REGTYPE, mode, len(data)), io.BytesIO(data))
    return output


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--library", type=Path, default=ROOT / "eci/libreloq_eci.so")
    parser.add_argument("--module", type=Path, default=ROOT / "eci/sd_reloq")
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    version = addon_version()
    print(build(args.output or ROOT / "dist" / output_name(version), args.library, args.module, version))


if __name__ == "__main__":
    main()
