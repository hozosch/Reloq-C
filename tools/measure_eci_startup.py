"""Measure native time to first PCM and the return from an interrupted analysis.

Audio stays in RAM. Each library runs in its own process, without a playback
queue or device. These are host observations, not NVDA device latency claims.
The first callback cancels first-PCM probes; timed interruptions also measure
the following short utterance, so cancellation cleanup is part of the result.
"""
from pathlib import Path
import argparse
import hashlib
import json
import statistics
import subprocess
import sys
import threading
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "nvda"))
from _reloq_eci_direct import DirectEciEngine, NATIVE_SPEED, SAMPLE_RATE

TEXTS = {
    "de": (0x40000, "Die Stimme spricht einen längeren Text mit mehreren vollständigen Sätzen. "),
    "en": (0x10000, "The voice reads a longer text with several complete sentences. "),
}


def first_pcm(engine, text):
    received = []
    def take(data):
        if not received:
            received.append((time.perf_counter_ns(), hashlib.sha256(data).hexdigest(), len(data)))
            engine.cancel()
    started = time.perf_counter_ns()
    assert not engine.speak([text], take, lambda n: None)
    assert received, "Cancelled before first PCM"
    return dict(ms=(received[0][0] - started) / 1e6,
                pcm_sha256=received[0][1], pcm_bytes=received[0][2])


def interrupt(engine, text, delay_ms):
    requested = []
    received_bytes = 0
    def stop():
        time.sleep(delay_ms / 1000)
        requested.append(time.perf_counter_ns())
        engine.cancel()
    def take(data):
        nonlocal received_bytes
        received_bytes += len(data)
    thread = threading.Thread(target=stop)
    thread.start()
    started = time.perf_counter_ns()
    try:
        completed = engine.speak([text], take, lambda n: None)
        returned = time.perf_counter_ns()
    finally:
        thread.join()
    assert not completed and requested[0] <= returned, "Probe finished before cancellation"
    return dict(total_ms=(returned - started) / 1e6,
                after_cancel_ms=(returned - requested[0]) / 1e6,
                pcm_bytes=received_bytes)


def worker(args):
    rows = []
    for language in args.languages:
        dialect, unit = TEXTS[language]
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect,
            encoding="utf8", experimental_generic=True, buffer_samples=1100)
        try:
            engine.set_parameter(NATIVE_SPEED, 75)
            if args.dictionary:
                engine.set_dictionary_path(args.dictionary.resolve())
            for rate in args.rates:
                engine.set_parameter(SAMPLE_RATE, rate)
                first_pcm(engine, unit)  # Warm initialization, no speech-result cache.
                for copies in args.copies:
                    probes = [first_pcm(engine, unit * copies) for _ in range(args.repeat)]
                    rows.append(dict(kind="first_pcm", language=language, rate=rate,
                        characters=len(unit) * copies, copies=copies,
                        median_ms=statistics.median(p["ms"] for p in probes), probes=probes))
                for copies in args.cancel_copies:
                    probes = []
                    for _ in range(args.repeat):
                        probe = interrupt(engine, unit * copies, args.cancel_delay_ms)
                        probe["next_first_pcm"] = first_pcm(engine, unit)
                        probes.append(probe)
                    rows.append(dict(kind="interrupted_analysis", language=language, rate=rate,
                        characters=len(unit) * copies, copies=copies,
                        median_after_cancel_ms=statistics.median(p["after_cancel_ms"] for p in probes),
                        median_handoff_ms=statistics.median(p["after_cancel_ms"] +
                            p["next_first_pcm"]["ms"] for p in probes), probes=probes))
        finally:
            engine.close()
    return dict(library=str(args.library.resolve()), version=engine.native_version,
                rows=rows, dictionary=str(args.dictionary.resolve()) if args.dictionary else None)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--baseline", type=Path)
    parser.add_argument("--dictionary", type=Path)
    parser.add_argument("--languages", nargs="+", choices=tuple(TEXTS), default=["de", "en"])
    parser.add_argument("--rates", nargs="+", type=int, choices=range(7), default=[1, 6])
    parser.add_argument("--copies", nargs="+", type=int, default=[1, 8, 32, 128])
    parser.add_argument("--cancel-copies", nargs="+", type=int, default=[128, 512])
    parser.add_argument("--cancel-delay-ms", type=float, default=25)
    parser.add_argument("--repeat", type=int, default=5)
    parser.add_argument("--output", type=Path)
    parser.add_argument("--worker", action="store_true", help=argparse.SUPPRESS)
    args = parser.parse_args()
    if args.repeat < 1 or args.cancel_delay_ms <= 0 or any(n < 1 for n in args.copies + args.cancel_copies):
        parser.error("Counts, repeats and cancellation delay must be positive")
    if args.worker:
        print(json.dumps(worker(args)))
        return
    common = ["--languages", *args.languages, "--rates", *map(str, args.rates),
        "--copies", *map(str, args.copies), "--cancel-copies", *map(str, args.cancel_copies),
        "--cancel-delay-ms", str(args.cancel_delay_ms), "--repeat", str(args.repeat)]
    if args.dictionary:
        common += ["--dictionary", str(args.dictionary.resolve())]
    builds = []
    for library in ([args.baseline] if args.baseline else []) + [args.library]:
        result = subprocess.run([sys.executable, __file__, "--worker", "--library",
            str(library.resolve()), *common], capture_output=True, text=True, check=True, timeout=180)
        builds.append(json.loads(result.stdout))
    if args.baseline:
        for before, after in zip(builds[0]["rows"], builds[1]["rows"]):
            assert (before["kind"], before["language"], before["rate"], before["copies"]) == (
                after["kind"], after["language"], after["rate"], after["copies"])
            if before["kind"] == "first_pcm":
                assert {p["pcm_sha256"] for p in before["probes"]} == {
                    p["pcm_sha256"] for p in after["probes"]}, "Initial PCM changed"
    report = dict(method=__doc__.strip(), no_audio_files=True, native_speed=75,
                  requested_cancel_ms=args.cancel_delay_ms, builds=builds)
    if args.output:
        args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    for build in builds:
        print(build["library"])
        for row in build["rows"]:
            value = row.get("median_ms", row.get("median_after_cancel_ms"))
            print(row["language"], row["rate"], row["characters"], row["kind"], f"{value:.3f} ms")


if __name__ == "__main__":
    main()
