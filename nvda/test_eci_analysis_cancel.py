"""Cancel long German/US English input and retain its loaded dictionary and next speech.

This checks recovery through public ECI, without a device or audio files.
Times are observations; the deterministic native rule test checks pre-output
cancellation without depending on thread scheduling.
"""
from pathlib import Path
import argparse
import hashlib
import tempfile
import threading
import time

from _reloq_eci_direct import DirectEciEngine, EciIndex, NATIVE_SPEED, SAMPLE_RATE

ROOT = Path(__file__).resolve().parents[1]
LANGUAGES = (
    (0x40000, "DEUmain.dic", "Banane",
     "Die Stimme spricht einen längeren Text mit mehreren vollständigen Sätzen. "),
    (0x10000, "ENUmain.dic", "banana",
     "The voice reads a longer text with several complete sentences. "),
)


def capture(engine):
    digest = hashlib.sha256()
    size = 0
    marks = []
    def take(data):
        nonlocal size
        digest.update(data)
        size += len(data)
    assert engine.speak(["CancelWord.", EciIndex(7)], take,
                        lambda n: marks.append((n, size)))
    assert size and marks == [(7, size)]
    return digest.digest(), size, marks


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, default=ROOT / "eci/libreloq_eci.so")
    args = parser.parse_args()
    after_cancel = []
    before_output = 0
    cases = 0
    with tempfile.TemporaryDirectory() as temporary:
        directory = Path(temporary)
        for language, filename, replacement, unit in LANGUAGES:
            source = directory / filename
            source.write_bytes(("CancelWord\t" + replacement + "\n").encode("ascii"))
            engine = DirectEciEngine(args.library.resolve(), dialect=language,
                encoding="utf8", experimental_generic=True)
            try:
                engine.set_parameter(NATIVE_SPEED, 75)
                engine.set_dictionary_path(directory)
                expected = {}
                for rate in (1, 6):
                    engine.set_parameter(SAMPLE_RATE, rate)
                    expected[rate] = capture(engine)
                source.unlink()  # A discarded dictionary could no longer reload.
                for rate in (1, 6):
                    engine.set_parameter(SAMPLE_RATE, rate)
                    for _ in range(8):
                        stopped = []
                        pcm = []
                        def stop():
                            time.sleep(.025)
                            stopped.append(time.perf_counter_ns())
                            engine.cancel()
                        thread = threading.Thread(target=stop)
                        thread.start()
                        try:
                            assert not engine.speak([unit * 512], lambda data: pcm.append(len(data)), lambda n: None)
                            returned = time.perf_counter_ns()
                        finally:
                            thread.join()
                        after_cancel.append((returned - stopped[0]) / 1e6)
                        before_output += not pcm
                        assert capture(engine) == expected[rate], "Cancel lost dictionary, PCM or index state"
                        cases += 1
            finally:
                engine.close()
    print(f"German/US analysis cancellation: {cases} stop/recovery cases, {before_output} before PCM, "
          f"exact dictionary/PCM/index recovery with source absent; maximum return "
          f"{max(after_cancel):.3f} ms after stop; no audio files")


if __name__ == "__main__":
    main()
