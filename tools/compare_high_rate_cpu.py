"""Compare unchanged PCM and warmed CPU cost of high-rate filters.

The two ECI libraries are warmed and measured in alternating order with the
same voice and profile. Linux process CPU time excludes NVDA/device latency;
audio and indices are compared in RAM and no audio files are written.
"""
import argparse
import hashlib
import json
import platform
import statistics
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (DirectEciEngine, GENDER, HEAD_SIZE, BREATHINESS,
    HIGH_RATE_PROFILE, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES)
from _reloq_languages import LANGUAGES
from compare_eci_builds import TEXTS, capture


def pair(paths, language):
    return [DirectEciEngine(path.resolve(), dialect=language, encoding='utf8',
                            native_voice=True) for path in paths]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--output', type=Path)
    parser.add_argument('--repeats', type=int, default=15)
    args = parser.parse_args()
    assert args.repeats >= 3
    paths = (args.baseline, args.library)
    checks = 0
    for language in LANGUAGES.values():
        engines = pair(paths, language.id)
        try:
            for engine in engines:
                engine.set_parameter(NATIVE_SPEED, 98)
            for rate in SAMPLE_RATES:
                for gender, head, breath in ((0, 50, 0), (1, 29, 60)):
                    for engine in engines:
                        for parameter, value in ((HIGH_RATE_PROFILE, 3), (SAMPLE_RATE, rate),
                                (GENDER, gender), (HEAD_SIZE, head), (BREATHINESS, breath)):
                            engine.set_parameter(parameter, value)
                    a, b = (capture(engine, TEXTS[language.id]) for engine in engines)
                    assert a == b, (language.locale, rate, gender, 'PCM/index changed')
                    checks += 1
        finally:
            for engine in engines: engine.close()
    # Old profile selectors must all retain the accepted profile-3 sound.
    for dialect in (0x10000, 0x40000, 0x20000, 0x120000):
        engines = pair(paths, dialect)
        try:
            for rate in (3, 2, 4, 5, 6):
                for profile in (0, 1):
                    for engine in engines: engine.set_parameter(SAMPLE_RATE, rate)
                    engines[0].set_parameter(HIGH_RATE_PROFILE, 3)
                    engines[1].set_parameter(HIGH_RATE_PROFILE, profile)
                    assert capture(engines[0], TEXTS[dialect]) == capture(engines[1], TEXTS[dialect]), (
                        hex(dialect), rate, profile, 'control PCM/index changed')
                    checks += 1
        finally:
            for engine in engines: engine.close()
    rows = []
    for dialect in (0x40000, 0x10000, 0x120000):
        engines = pair(paths, dialect)
        try:
            for engine in engines:
                engine.set_parameter(HIGH_RATE_PROFILE, 3)
                engine.set_parameter(NATIVE_SPEED, 98)
            for rate in (3, 2, 4, 6):
                for engine in engines:
                    engine.set_parameter(SAMPLE_RATE, rate)
                    for _ in range(3): capture(engine, TEXTS[dialect])
                measurements = [[], []]
                for repeat in range(args.repeats):
                    for i in ((0, 1) if repeat % 2 == 0 else (1, 0)):
                        start = time.process_time_ns()
                        capture(engines[i], TEXTS[dialect])
                        measurements[i].append((time.process_time_ns() - start) / 1e6)
                medians = [statistics.median(values) for values in measurements]
                rows.append(dict(language=hex(dialect), hz=SAMPLE_RATES[rate],
                                 baseline_cpu_ms=medians[0], candidate_cpu_ms=medians[1],
                                 ratio=medians[1] / medians[0]))
        finally:
            for engine in engines: engine.close()
    report = dict(method=__doc__.strip(), platform=platform.platform(),
                  baseline_sha256=hashlib.sha256(args.baseline.read_bytes()).hexdigest(),
                  candidate_sha256=hashlib.sha256(args.library.read_bytes()).hexdigest(),
                  profile=3, pcm_and_index_checks=checks, no_audio_files=True, timings=rows)
    if args.output:
        args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
    print(json.dumps(report, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
