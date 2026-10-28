#!/usr/bin/env python3
"""Check Linux development ECI symbols and reject leakage of internal API."""

from __future__ import annotations

import argparse
import ctypes
from pathlib import Path

from verify_pe_exports import declared_exports


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("library", type=Path)
    parser.add_argument("--header", type=Path, required=True)
    args = parser.parse_args()
    public = declared_exports(args.header.read_text(encoding="utf-8"))
    library = ctypes.CDLL(str(args.library.resolve()))
    for name in sorted(public):
        getattr(library, name)
    for private in ("reloq_eci_render_controls", "reloq_eci_last_error",
                    "reloq_host_render_controls", "reloq_host_create"):
        try:
            getattr(library, private)
        except AttributeError:
            continue
        raise AssertionError(f"private or host symbol unexpectedly exported: {private}")
    print(f"Linux development ABI: {len(public)} ECI symbols, private API hidden")


if __name__ == "__main__":
    main()
