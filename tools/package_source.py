#!/usr/bin/env python3
"""Create a reproducible source-only archive of the independent C port.

The archive contains authored code, tests, documents and portable text data.
It deliberately excludes binaries, captured audio and local build products.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import zipfile


ROOT = Path(__file__).resolve().parents[1]
SOURCE_SUFFIXES = {
    ".xml", ".c", ".h", ".inc", ".py", ".md", ".txt", ".html",
    ".ini", ".tsv", ".csv", ".rlex", ".jsonl", ".patch", ".json", ".log", ".sh", ".cmd", ".po", ".pot",
}
SOURCE_NAMES = {"Makefile", "Makefile.native", "CMakeLists.txt", "LICENSE"}
EXCLUDED_PARTS = {
    "dist", "__pycache__", ".git", ".pytest_cache", "build",
    "cmake-build-debug", "cmake-build-release",
}


def source_files() -> list[Path]:
    result = []
    for path in ROOT.rglob("*"):
        if not path.is_file() or any(part in EXCLUDED_PARTS for part in path.parts):
            continue
        if (path.suffix.lower() in SOURCE_SUFFIXES or path.name in SOURCE_NAMES or
                path.name.endswith(".json.gz")):
            result.append(path)
    return sorted(result, key=lambda path: path.relative_to(ROOT).as_posix())


def package(output: Path) -> tuple[int, int]:
    output.parent.mkdir(parents=True, exist_ok=True)
    size = 0
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for path in source_files():
            content = path.read_bytes()
            name = "eloquence-c/" + path.relative_to(ROOT).as_posix()
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            info.external_attr = 0o644 << 16
            archive.writestr(info, content)
            size += len(content)
    return len(source_files()), size


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    count, size = package(args.output)
    print(f"{count} source files ({size} uncompressed bytes): {args.output}")


if __name__ == "__main__":
    main()
