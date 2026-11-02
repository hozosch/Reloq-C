#!/usr/bin/env python3
"""Package the tested x64 DLL, public ECI headers and corresponding-source link."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def build(dll, output, version, revision):
    if not re.fullmatch(r"\d+\.\d+\.\d+(?:-rc\d+)?", version):
        raise ValueError("Invalid SDK version")
    if not re.fullmatch(r"[0-9a-f]{40}", revision):
        raise ValueError("A complete corresponding-source commit is required")
    source = "https://github.com/hozosch/Reloq-C/tree/" + revision
    entries = {
        "eci.dll": dll.read_bytes(),
        "eci.h": (ROOT / "include/eci.h").read_bytes(),
        "reloq_eci.h": (ROOT / "include/reloq_eci.h").read_bytes(),
        "LICENSE.txt": (ROOT / "LICENSE").read_bytes(),
        "NOTICE.txt": (ROOT / "NOTICE").read_bytes(),
        "ECI-COMPATIBILITY.md": (ROOT / "docs/ECI-COMPATIBILITY.md").read_bytes(),
        "licenses/OpenEVV-LICENSE.txt": (ROOT / "vendor/openevv61/LICENSE").read_bytes(),
        "licenses/OpenEVV-NOTICE.txt": (ROOT / "vendor/openevv61/NOTICE").read_bytes(),
        "licenses/Unicode-LICENSE.txt": (ROOT / "reference-apple/behavior-0.2.82/unicode-input/LICENSE.txt").read_bytes(),
        "licenses/SCOWL-Copyright.txt": (ROOT / "reference-apple/behavior-0.2.83/input/SCOWL-Copyright.txt").read_bytes(),
    }
    entries["source.json"] = (json.dumps({
        "version": version, "source_commit": revision, "source_url": source,
        "dll_sha256": hashlib.sha256(entries["eci.dll"]).hexdigest(),
    }, indent=2) + "\n").encode()
    entries["README.md"] = f"""# ReLoq C ECI x64 {version}

This is a development candidate for Windows x64 and x64 emulation on Windows
ARM. It contains the same eci.dll as the matching NVDA test add-on.

Use eci.h for the standard ECI names; reloq_eci.h also declares the ReLoq
extensions. The DLL is self-contained and exposes 49 functions. Speech and
phoneme generation are synchronous through the caller's ECI buffer/callback.
Register a callback and an output buffer before synthesizing. Standard
eciGetAvailableLanguages, eciSetVoiceParam, language changes and dictionary
handles are supported. The compatibility document lists remaining differences
and missing exports; full original-ECI parity is not claimed.

AddText snapshots the eight native voice parameters for PCM synthesis. A
subsequent parameter setter changes following queued text, including when
public text annotations are disabled. Each utterance remains one rule stream.

The source for this exact package is {source}.
Rebuild the DLL with `make -C eci windows-x64` using the documented MinGW
compiler options. Rebuild this archive with `python3 tools/build_eci_sdk.py
--dll eci/eci.dll --version {version} --source-commit {revision} --output sdk.zip`.

ReLoq C is GPL-3.0-or-later. Redistributing its DLL also requires providing
the corresponding source and preserving the included component notices.
See LICENSE.txt and NOTICE.txt. No proprietary reference DLL is included.
""".encode()
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        for name, data in sorted(entries.items()):
            item = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            item.compress_type = zipfile.ZIP_DEFLATED
            item.external_attr = 0o644 << 16
            archive.writestr(item, data, compresslevel=9)
    return output


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dll", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--version", required=True)
    parser.add_argument("--source-commit", required=True)
    args = parser.parse_args()
    print(build(args.dll, args.output, args.version, args.source_commit))
