"""Measure VWPM duration from a supplied original Windows ECI DLL.

The original binary is an external reference, never included in the results.
Each case runs in a fresh process so one bad extreme cannot corrupt the rest.
"""
import argparse
import ctypes as C
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys


CASES = (
    (0x40000, "Hallo.", "cp1252"),
    (0x10000, "Hello.", "cp1252"),
    (0x10001, "Hello.", "cp1252"),
    (0x20000, "Hola.", "cp1252"),
    (0x20001, "Hola.", "cp1252"),
    (0x30000, "Bonjour.", "cp1252"),
    (0x30001, "Bonjour.", "cp1252"),
    (0x50000, "Ciao.", "cp1252"),
    (0x70000, "Olá.", "cp1252"),
    (0x90000, "Moi.", "cp1252"),
    (0x60000, "你好。", "cp936"),
    (0x80000, "こんにちは。", "cp932"),
    (0xa0000, "안녕하세요.", "cp949"),
)
VALUES = (1, 2, 3, 4, 5, 8, 10, 12, 16, 20, 30, 50, 150, 200)


def single(library, language, text, value, encoding):
    library = library.resolve()
    os.chdir(library.parent)
    os.environ['PATH'] = str(library.parent) + os.pathsep + os.environ.get('PATH', '')
    locale_id = {0x60000: 0x0804, 0x80000: 0x0411, 0xa0000: 0x0412}.get(language)
    if locale_id:
        C.windll.kernel32.SetThreadLocale(locale_id)
    directory = os.add_dll_directory(str(library.parent))
    dll = C.WinDLL(str(library))
    H, I = C.c_void_p, C.c_int
    CB = C.WINFUNCTYPE(I, H, I, I, H)
    for name, restype, args in (
        ("eciNewEx", H, (I,)), ("eciDelete", H, (H,)),
        ("eciSetParam", I, (H, I, I)), ("eciGetParam", I, (H, I)),
        ("eciSetVoiceParam", I, (H, I, I, I)),
        ("eciRegisterCallback", None, (H, CB, H)),
        ("eciSetOutputBuffer", I, (H, I, C.POINTER(C.c_short))),
        ("eciAddText", I, (H, C.c_char_p)),
        ("eciSynthesize", I, (H,)), ("eciSynchronize", I, (H,)),
    ):
        func = getattr(dll, name)
        func.restype, func.argtypes = restype, args
    engine = dll.eciNewEx(language)
    if not engine:
        # Some host versions initialize their Asian backend only after a
        # default handle has been created. Record the actual selected dialect.
        engine = dll.eciNewEx(0x10000)
        if not engine:
            raise RuntimeError("eciNewEx failed for both target and default")
        previous = dll.eciSetParam(engine, 9, language)
        selected = dll.eciGetParam(engine, 9)
        if selected != language:
            dll.eciDelete(engine)
            raise RuntimeError(f"target creation and language switch failed: previous={previous}, selected={selected}")
    samples = 0
    buffer = (C.c_short * 1100)()

    @CB
    def callback(handle, message, size, context):
        nonlocal samples
        if message == 0:
            if size < 0 or size > len(buffer):
                return 2
            samples += size
        return 1

    try:
        dll.eciRegisterCallback(engine, callback, None)
        assert dll.eciSetOutputBuffer(engine, len(buffer), buffer)
        assert dll.eciSetParam(engine, 1, 1) >= 0  # Input annotations.
        assert dll.eciSetParam(engine, 5, 1) >= 0  # 11025 Hz.
        assert dll.eciSetVoiceParam(engine, 0, 6, 50) >= 0
        prefix = f"`vwpm{value} " if value else "`vs50 "
        data = (prefix + text).encode(encoding)
        assert dll.eciAddText(engine, data)
        assert dll.eciSynthesize(engine)
        assert dll.eciSynchronize(engine)
        return {"language": language, "text": text, "wpm": value,
                "samples": samples, "sample_rate": 11025, "silent": samples == 0}
    finally:
        dll.eciDelete(engine)
        directory.close()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--library", type=Path, required=True)
    p.add_argument("--output", type=Path)
    p.add_argument("--child", action="store_true")
    p.add_argument("--language", type=int)
    p.add_argument("--text")
    p.add_argument("--encoding")
    p.add_argument("--value", type=int)
    args = p.parse_args()
    if args.child:
        print("WPM_RESULT=" + json.dumps(single(args.library, args.language,
                                                args.text, args.value, args.encoding)))
        return
    library = args.library.resolve()
    cases = []
    for language, text, encoding in CASES:
        for value in (0, *VALUES):
            command = [sys.executable, str(Path(__file__).resolve()), "--child",
                       "--library", str(library), "--language", str(language),
                       "--text", text, "--encoding", encoding, "--value", str(value)]
            try:
                process = subprocess.run(command, capture_output=True, text=True, timeout=40)
                lines = [line for line in process.stdout.splitlines() if line.startswith("WPM_RESULT=")]
                if process.returncode or len(lines) != 1:
                    record = {"language": language, "text": text, "wpm": value,
                              "error": "child failed", "returncode": process.returncode,
                              "stderr": process.stderr[-2000:]}
                else:
                    record = json.loads(lines[0].split("=", 1)[1])
            except subprocess.TimeoutExpired:
                record = {"language": language, "text": text, "wpm": value, "error": "timeout"}
            cases.append(record)
            print(json.dumps(record), flush=True)
    report = {"pointer_bits": C.sizeof(C.c_void_p) * 8,
              "dll_sha256": hashlib.sha256(library.read_bytes()).hexdigest(),
              "cases": cases}
    if args.output:
        args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if any("error" in record for record in cases):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
