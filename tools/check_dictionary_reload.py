"""Time native US dictionary load/reload/disable; compare PCM only in RAM."""
from pathlib import Path
import argparse
import hashlib
import json
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "nvda"))
from _reloq_eci_direct import DirectEciEngine, EciIndex, NATIVE_SPEED, SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, default=ROOT / ("eci/eci.dll" if sys.platform == "win32" else "eci/libreloq_eci.so"))
    parser.add_argument("--dictionaries", type=Path, default=ROOT / "data/community")
    parser.add_argument("--cycles", type=int, default=3)
    parser.add_argument("--rate", type=int, choices=range(7), default=1)
    parser.add_argument("--output", type=Path, help="Optional timing JSON; never writes audio")
    args = parser.parse_args()
    if args.cycles < 1:
        parser.error("--cycles must be positive")
    rows = []
    for _ in range(args.cycles):
        row = {}
        engine = DirectEciEngine(args.library, dialect=0x10000, encoding="utf8", experimental_generic=True)

        def timed(label, action):
            begin = time.perf_counter()
            result = action()
            row[label] = time.perf_counter() - begin
            return result

        def capture():
            digest, length, indices = hashlib.sha256(), [0], []
            def receive(data):
                digest.update(data)
                length[0] += len(data)
            assert engine.speak(["This is Eloquence and NVDA, and 504 and 4004.", EciIndex(7)],
                                receive, lambda i: indices.append((i, length[0])))
            assert length[0] and indices == [(7, length[0])]
            return digest.hexdigest(), length[0], indices

        try:
            engine.set_parameter(NATIVE_SPEED, 98)
            engine.set_parameter(SAMPLE_RATE, args.rate)
            timed("load_seconds", lambda: engine.set_dictionary_path(args.dictionaries))
            before = capture()
            timed("reload_seconds", lambda: engine.set_dictionary_path(args.dictionaries))
            after = capture()
            assert before == after, "Reload changed PCM or indices"
            timed("disable_seconds", lambda: engine.set_dictionary_path(None))
            row.update(pcm_identical_after_reload=True, pcm_bytes=before[1], native_version=engine.native_version)
        finally:
            timed("close_seconds", engine.close)
        rows.append(row)
        print(json.dumps(row), flush=True)
    if args.output:
        args.output.write_text(json.dumps({"no_audio_files": True, "rows": rows}, indent=2) + "\n")


if __name__ == "__main__":
    main()
