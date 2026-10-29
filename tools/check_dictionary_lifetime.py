"""Exercise native English dictionary replacement and teardown; PCM stays in RAM.

This checks the Linux development library. It does not establish that the
user-reported Windows/NVDA crash has been resolved.
"""
from pathlib import Path
import argparse
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "nvda"))
from _reloq_eci_direct import DirectEciEngine, EciIndex

TEXTS = (
    "This is Eloquence, and it should keep speaking.",
    "504 4004 50005, and some ordinary text.",
    "ETI-Eloquence and NVDA work together.",
    "This includes α β γ and Greek letters.",
    "Open the pronunciation dictionary settings.",
    "`ts2 AaBbGg `ts0 Done.",
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, default=ROOT / "eci/libreloq_eci.so")
    parser.add_argument("--dictionaries", type=Path, default=ROOT / "data/community")
    parser.add_argument("--cycles", type=int, default=3)
    parser.add_argument("--corpus", type=Path, help="Optional native CP1252 Dic file whose keys are spoken in context")
    parser.add_argument("--stride", type=int, default=1)
    args = parser.parse_args()
    if args.cycles < 1:
        parser.error("--cycles must be positive")
    if args.stride < 1:
        parser.error("--stride must be positive")
    if args.corpus:
        engine = DirectEciEngine(args.library, dialect=0x10000,
                                 experimental_generic=True, encoding="utf8")
        complete, rejected = 0, []
        try:
            engine.set_dictionary_path(args.dictionaries)
            for row, line in enumerate(args.corpus.read_bytes().splitlines(), 1):
                if b"\t" not in line or (row - 1) % args.stride:
                    continue
                key = line.split(b"\t", 1)[0].strip().decode("cp1252")
                size, indices = [0], []

                def receive(data):
                    size[0] += len(data)

                try:
                    ok = engine.speak([f"This is {key} and it works with 504.", EciIndex(42)],
                                      receive, indices.append)
                    if not ok or not size[0] or indices != [42]:
                        raise AssertionError("Missing PCM or completion index")
                    complete += 1
                except RuntimeError as error:
                    rejected.append((row, str(error)))
                    print(f"Rejected corpus row {row}: {error}", flush=True)
                if row % 100 == 0:
                    print(f"corpus row {row}, {complete} complete, {len(rejected)} rejected", flush=True)
        finally:
            engine.close()
        print(f"Corpus: {complete} complete, {len(rejected)} rejected; no audio files written.")
        if rejected:
            raise SystemExit(1)
        return
    count = 0
    for cycle in range(args.cycles):
        engine = DirectEciEngine(args.library, dialect=0x10000,
                                 experimental_generic=True, encoding="utf8")
        try:
            for directory in (args.dictionaries, None, args.dictionaries):
                engine.set_dictionary_path(directory)
                for text in TEXTS:
                    size, indices = [0], []

                    def receive(data):
                        size[0] += len(data)

                    if not engine.speak([text, EciIndex(42)], receive, indices.append):
                        raise AssertionError(f"Unexpected cancellation: {text!r}")
                    if not size[0] or indices != [42]:
                        raise AssertionError(f"Missing PCM or completion index: {text!r}")
                    count += 1
        finally:
            engine.close()
        print(f"cycle {cycle + 1}: {count} complete utterances", flush=True)
    print("Native English dictionary lifetime check passed; no audio files written.")


if __name__ == "__main__":
    main()
