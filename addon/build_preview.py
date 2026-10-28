#!/usr/bin/env python3
"""Build the explicit x64 NVDA listening preview from the current C DLL.

Usage: make -C eci windows-x64 && python3 addon/build_preview.py
The builder does not fetch or embed any reference engine.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import struct
import zipfile


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "addon" / "preview"
DEFAULT_OUTPUT = ROOT / "dist" / "reloq-c-preview-x64-0.1.5.nvda-addon"


def assert_x64_pe(dll: bytes) -> None:
    if len(dll) < 0x100 or dll[:2] != b"MZ":
        raise ValueError("eci.dll is not a Windows PE DLL")
    pe_offset = struct.unpack_from("<I", dll, 0x3C)[0]
    if pe_offset + 26 > len(dll) or dll[pe_offset:pe_offset + 4] != b"PE\0\0":
        raise ValueError("eci.dll has no valid PE header")
    machine, = struct.unpack_from("<H", dll, pe_offset + 4)
    magic, = struct.unpack_from("<H", dll, pe_offset + 24)
    if machine != 0x8664 or magic != 0x20B:
        raise ValueError("eci.dll must be PE32+ x86-64; ARM64 and 32-bit are not bundled")


def make_entries(dll_path: Path) -> dict[str, bytes]:
    dll = dll_path.read_bytes()
    assert_x64_pe(dll)
    entries: dict[str, bytes] = {}
    # An explicit allowlist keeps transient __pycache__ and test artifacts out
    # of user-installable packages, even after the local Python tests run.
    for name in (
        "manifest.ini",
        "doc/de/readme.html",
        "doc/en/readme.html",
        "globalPlugins/reloqPreview/__init__.py",
    ):
        entries[name] = (SOURCE / name).read_bytes()
    source_prefix = "globalPlugins/reloqPreview/"
    entries[source_prefix + "_reloq_eci_direct.py"] = (
        ROOT / "nvda" / "_reloq_eci_direct.py").read_bytes()
    entries[source_prefix + "_reloq_eci_transport.py"] = (
        ROOT / "nvda" / "_reloq_eci_transport.py").read_bytes()
    entries[source_prefix + "eci.dll"] = dll
    if any(name.endswith((".so", ".syn", ".dylib")) for name in entries):
        raise ValueError("preview contains an unexpected binary or language module")
    return entries


def build(output: Path, dll_path: Path) -> Path:
    entries = make_entries(dll_path)
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for name, data in sorted(entries.items()):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, data)
    return output


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dll", type=Path, default=ROOT / "eci" / "eci.dll")
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    args = parser.parse_args()
    print(build(args.output, args.dll))


if __name__ == "__main__":
    main()
