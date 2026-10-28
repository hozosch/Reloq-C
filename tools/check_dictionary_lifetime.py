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
    args = parser.parse_args()
    if args.cycles < 1:
        parser.error("--cycles must be positive")
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
