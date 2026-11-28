"""Check that shortening an English /s/ does not lengthen the stop closure."""
import argparse
from array import array
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, EciIndex, NATIVE_WPM, PROCESS_TAGS, SAMPLE_RATE
from test_eci_german_clock_native import phonemes, render


def s_and_release(pcm):
    """Find the first s-to-stop silence and the following noise onset."""
    samples = array("h")
    samples.frombytes(pcm)
    size = 11  # approximately one millisecond at 11025 Hz
    powers = [sum(int(x) * int(x) for x in samples[i:i + size]) / size
              for i in range(0, min(len(samples), 2205) - size, size)]
    threshold = max(100.0, max(powers[:100]) * 0.0004)
    end = next((i for i in range(15, len(powers) - 4)
                if all(power < threshold for power in powers[i:i + 5])), None)
    assert end is not None, "no closure after the initial s"
    release = next((i for i in range(end + 5, len(powers) - 2)
                    if all(power >= threshold for power in powers[i:i + 3])), None)
    assert release is not None, "no stop release after the closure"
    return end, release


def make_engine(path, dialect):
    e = DirectEciEngine(path.resolve(), dialect=dialect, encoding="utf8", native_voice=True)
    e.set_parameter(SAMPLE_RATE, 1)
    e.set_parameter(PROCESS_TAGS, 1)
    return e


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--library", type=Path, required=True)
    p.add_argument("--reference", type=Path, required=True)
    p.add_argument("--previous-dev4", type=Path)
    p.add_argument("--dictionary", type=Path)
    args = p.parse_args()
    changed = ("ski", "scout", "school", "stop", "spill", "transcribe")
    unchanged = ("see", "say", "ask", "display", "rescue", "good dog",
                 "`[.1skhWt]", "`[.1trAnsh.2krIpt]")
    checks = 0
    for dialect in (0x10000, 0x10001):
        current, before = make_engine(args.library, dialect), make_engine(args.reference, dialect)
        dev4 = make_engine(args.previous_dev4, dialect) if args.previous_dev4 else None
        try:
            for rate in (200, 320):
                for engine in (current, before):
                    engine.set_parameter(NATIVE_WPM, rate)
                if dev4:
                    dev4.set_parameter(NATIVE_WPM, rate)
                for word in changed:
                    assert phonemes(current, word) == phonemes(before, word), (dialect, word)
                    audio = render(current, [word + "."])[0]
                    old = render(before, [word + "."])[0]
                    assert len(audio) < len(old), (dialect, rate, word, len(audio), len(old))
                    if word in ("ski", "scout", "school", "stop", "spill"):
                        end, release = s_and_release(audio)
                        old_end, old_release = s_and_release(old)
                        assert end + 3 <= old_end, (dialect, rate, word, end, old_end)
                        assert abs((release - end) - (old_release - old_end)) <= 3, \
                            (dialect, rate, word, end, release, old_end, old_release)
                        assert abs((old_release - release) - (old_end - end)) <= 3
                        if dev4:
                            old_experiment = render(dev4, [word + "."])[0]
                            end4, release4 = s_and_release(old_experiment)
                            assert abs(end - end4) <= 3, (dialect, rate, word, end, end4)
                            assert (release4 - end4) - (release - end) >= 5, \
                                (dialect, rate, word, end, release, end4, release4)
                    checks += 1
                for word in unchanged:
                    assert render(current, [word]) == render(before, [word]), (dialect, rate, word)
                    checks += 1
                parts = ["Please ", EciIndex(1), "transcribe ", EciIndex(2),
                         "the scout, ", EciIndex(3), "stop and spill.", EciIndex(4)]
                new_audio, new_marks = render(current, parts)
                old_audio, old_marks = render(before, parts)
                assert len(new_audio) < len(old_audio) and new_marks[0] == old_marks[0]
                assert [mark for mark, _ in new_marks] == [mark for mark, _ in old_marks]
                assert all(new <= old for (_, new), (_, old) in zip(new_marks, old_marks))
                assert new_marks[-1][1] == len(new_audio)
                checks += 1
        finally:
            if dev4:
                dev4.close()
            current.close()
            before.close()
    if args.dictionary:
        current, before = make_engine(args.library, 0x10000), make_engine(args.reference, 0x10000)
        try:
            for engine in (current, before):
                engine.set_dictionary_path(args.dictionary.resolve())
            assert render(current, ["transcript"]) == render(before, ["transcript"])
            assert len(render(current, ["transcribe"])[0]) < len(render(before, ["transcribe"])[0])
            checks += 2
        finally:
            current.close()
            before.close()
    for dialect, word in ((0x40000, "Du wachst, wenn ihr wachst."),
                          (0x120000, "Καλημέρα σας!")):
        current, before = make_engine(args.library, dialect), make_engine(args.reference, dialect)
        try:
            assert render(current, [word]) == render(before, [word]), dialect
            checks += 1
        finally:
            current.close()
            before.close()
    print(f"English s-stop timing: {checks} speech, closure and isolation checks")


if __name__ == "__main__":
    main()
