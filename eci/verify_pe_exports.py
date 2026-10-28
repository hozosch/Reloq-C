#!/usr/bin/env python3
"""Verify that the direct ECI DLL is PE32+ with precisely its declared ABI."""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import subprocess


DECLARATION = re.compile(
    r"\bECIAPI\s+[A-Za-z_][A-Za-z_0-9\s*]*?\s+ECICALL\s+(eci[A-Za-z_0-9]+)\s*\("
)
EXPORT_LINE = re.compile(
    r"^\s*\[\s*\d+\]\s+"
    r"(?:\+base\[\s*\d+\]\s+[0-9a-fA-F]+\s+)?"
    r"([A-Za-z_]\w*)\s*$"
)
IMPORT_LINE = re.compile(r"^\s*DLL Name:\s*(\S+)\s*$", re.MULTILINE)
SYSTEM_DLLS = {"kernel32.dll", "msvcrt.dll", "ucrtbase.dll", "user32.dll"}


def declared_exports(header: str) -> set[str]:
    names = DECLARATION.findall(header)
    if not names or len(names) != len(set(names)):
        raise ValueError("no public ECI declarations or duplicate names")
    return set(names)


def pe_exports(dump: str) -> set[str]:
    marker = "[Ordinal/Name Pointer] Table"
    if marker not in dump:
        raise ValueError("PE export name table missing")
    lines = dump.split(marker, 1)[1].splitlines()[1:]
    names: list[str] = []
    for line in lines:
        match = EXPORT_LINE.fullmatch(line)
        if match:
            names.append(match.group(1))
        elif names and line.strip():
            break
    if not names or len(names) != len(set(names)):
        raise ValueError("empty PE export table or duplicate names")
    return set(names)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dll", type=Path)
    parser.add_argument("--header", type=Path, required=True)
    parser.add_argument("--objdump", default="x86_64-w64-mingw32-objdump")
    args = parser.parse_args()
    header_dump = subprocess.check_output(
        [args.objdump, "-f", str(args.dll)], text=True
    )
    pe_dump = subprocess.check_output(
        [args.objdump, "-p", str(args.dll)], text=True
    )
    if ("file format pei-x86-64" not in header_dump or
            "architecture: i386:x86-64" not in header_dump):
        raise ValueError("expected Windows x64 PE DLL")
    expected = declared_exports(args.header.read_text(encoding="utf-8"))
    actual = pe_exports(pe_dump)
    if expected != actual:
        raise ValueError(
            f"ECI exports differ: missing={sorted(expected - actual)}, "
            f"unexpected={sorted(actual - expected)}"
        )
    imports = {name.lower() for name in IMPORT_LINE.findall(pe_dump)}
    if not imports:
        raise ValueError("PE import table unavailable")
    external = {name for name in imports if name not in SYSTEM_DLLS
                and not name.startswith("api-ms-win-")}
    if external:
        raise ValueError(f"unbundled native dependencies: {sorted(external)}")
    print(f"Windows x64 ECI ABI: {len(actual)} exports; "
          f"imports {', '.join(sorted(imports))}")


if __name__ == "__main__":
    main()
