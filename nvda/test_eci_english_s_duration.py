"""Check the English s-stop listening experiment through the public ECI path."""
import argparse
from array import array
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, NATIVE_WPM, PROCESS_TAGS, SAMPLE_RATE
from test_eci_german_clock_native import phonemes, render


def initial_s_end(pcm):
    """Find the first sustained silence after an initial unvoiced /s/."""
    samples = array("h")
    samples.frombytes(pcm)
    size = 11  # approximately one millisecond at 11025 Hz
    powers = [sum(int(x) * int(x) for x in samples[i:i + size]) / size
              for i in range(0, min(len(samples), 2205) - size, size)]
    peak = max(powers[:100])
    threshold = max(100.0, peak * 0.0004)
    for i in range(15, len(powers) - 4):
        if all(power < threshold for power in powers[i:i + 5]):
            return i
    raise AssertionError("no stop closure after the initial s")


def make_engine(path, dialect):
    e = DirectEciEngine(path.resolve(), dialect=dialect, encoding="utf8", native_voice=True)
    e.set_parameter(SAMPLE_RATE, 1)
    e.set_parameter(PROCESS_TAGS, 1)
    return e


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--library", type=Path, required=True)
    p.add_argument("--reference", type=Path, required=True)
    p.add_argument("--dictionary", type=Path)
    args = p.parse_args()
    changed = ("ski", "scout", "school", "stop", "spill", "transcribe")
    unchanged = ("see", "say", "ask", "display", "rescue", "good dog",
                 "`[.1skhWt]", "`[.1trAnsh.2krIpt]")
    checks = 0
    for dialect in (0x10000, 0x10001):
        current, before = make_engine(args.library, dialect), make_engine(args.reference, dialect)
        try:
            for rate in (200, 320):
                for engine in (current, before):
                    engine.set_parameter(NATIVE_WPM, rate)
                for word in changed:
                    assert phonemes(current, word) == phonemes(before, word), (dialect, word)
                    audio = render(current, [word + "."])[0]
                    old = render(before, [word + "."])[0]
                    assert len(audio) == len(old) and audio != old, (dialect, rate, word)
                    if word in ("ski", "scout", "school", "stop", "spill"):
                        assert initial_s_end(audio) + 3 <= initial_s_end(old), (dialect, rate, word)
                    checks += 1
                for word in unchanged:
                    assert render(current, [word]) == render(before, [word]), (dialect, rate, word)
                    checks += 1
        finally:
            current.close()
            before.close()
    if args.dictionary:
        current, before = make_engine(args.library, 0x10000), make_engine(args.reference, 0x10000)
        try:
            for engine in (current, before):
                engine.set_dictionary_path(args.dictionary.resolve())
            assert render(current, ["transcript"]) == render(before, ["transcript"])
            assert render(current, ["transcribe"]) != render(before, ["transcribe"])
            checks += 2
        finally:
            current.close()
            before.close()
    for dialect, word in ((0x40000, "Du wachst, wenn ihr wachst."),
                          (0x80000, "Sortez de cette situation.")):
        current, before = make_engine(args.library, dialect), make_engine(args.reference, dialect)
        try:
            assert render(current, [word]) == render(before, [word]), dialect
            checks += 1
        finally:
            current.close()
            before.close()
    print(f"English s-stop listening variant: {checks} speech and isolation checks")


if __name__ == "__main__":
    main()
