"""Compare native build size, unchanged speech and exploratory CPU cost.

PCM is hashed in separate processes. CPU timings alternate both libraries
in one process after warming both engines; they are Linux host observations,
not a Windows/Android device latency guarantee.
"""
from pathlib import Path
import argparse
import hashlib
import json
import statistics
import subprocess
import sys
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (DirectEciEngine, EciIndex, NATIVE_SPEED,
    SAMPLE_RATE, SAMPLE_RATES, GENDER, HEAD_SIZE, BREATHINESS)
from _reloq_languages import LANGUAGES

TEXTS = {
    0x40000: 'Die Sprachausgabe ist klar. Restaurant, Homepage und Germanistik.',
    0x10000: 'The community uses a computer. How does the voice sound?',
    0x10001: 'The community uses a computer. How does the voice sound?',
    0x20000: 'La comunidad utiliza un teléfono. ¡Buenos días!',
    0x20001: 'La comunidad utiliza un teléfono. ¡Buenos días!',
    0x30000: 'La communauté utilise un téléphone. Bonjour !',
    0x30001: 'La communauté utilise un téléphone. Bonjour !',
    0x50000: 'La comunità usa il telefono. Buongiorno!',
    0x70000: 'A informação está disponível. Olá, São Paulo!',
    0x90000: 'Hyvää päivää! Ääni, yö ja käyttäjät.',
    0x60000: '你好，这是中文。Hello world, OK Ok ok g 503 4004.',
    0x80000: 'こんにちは。Windowsとcomputerを使います。',
    0xa0000: '한국어로 읽습니다. chocolate jam watch Windows.',
    0x120000: 'Καλημέρα σας! clock shop Nevada why computer.',
}


def capture(engine, text):
    digest = hashlib.sha256()
    samples = 0
    marks = []
    def pcm(data):
        nonlocal samples
        digest.update(data)
        samples += len(data) // 2
    assert engine.speak([text, EciIndex(7)], pcm,
                        lambda n: marks.append((n, samples)))
    assert samples and marks == [(7, samples)]
    return {'samples': samples, 'pcm_sha256': digest.hexdigest(), 'indices': marks}


def worker(library):
    speech = []
    for language in LANGUAGES.values():
        engine = DirectEciEngine(library, dialect=language.id, encoding='utf8',
                                 experimental_generic=True)
        try:
            engine.set_parameter(NATIVE_SPEED, 98)
            text = TEXTS[language.id]
            for rate in SAMPLE_RATES:
                engine.set_parameter(SAMPLE_RATE, rate)
                for gender, head, breath in ((0, 50, 0), (1, 29, 60)):
                    for parameter, value in ((GENDER, gender), (HEAD_SIZE, head),
                                             (BREATHINESS, breath)):
                        engine.set_parameter(parameter, value)
                    result = capture(engine, text)
                    speech.append(dict(language=language.locale, rate=rate,
                        gender=gender, head=head, breath=breath, **result))
        finally:
            engine.close()
    return dict(version=engine.native_version, bytes=library.stat().st_size,
                speech=speech)


def paired_timings(baseline, library):
    timings = [[], []]
    for language in LANGUAGES.values():
        engines = [DirectEciEngine(path, dialect=language.id, encoding='utf8',
            experimental_generic=True) for path in (baseline, library)]
        try:
            for engine in engines:
                engine.set_parameter(NATIVE_SPEED, 98)
            for rate in (1, 6):
                text = TEXTS[language.id]
                for engine in engines:
                    engine.set_parameter(SAMPLE_RATE, rate)
                    for _ in range(3):
                        capture(engine, text)
                elapsed = [[], []]
                # Balanced order limits drift and predecessor/cache effects.
                for repeat in range(12):
                    for index in ((0, 1) if repeat % 2 == 0 else (1, 0)):
                        start = time.process_time_ns()
                        capture(engines[index], text)
                        elapsed[index].append((time.process_time_ns() - start) / 1e6)
                for index in (0, 1):
                    timings[index].append(dict(language=language.locale, rate=rate,
                        median_cpu_ms=statistics.median(elapsed[index]),
                        samples_ms=elapsed[index]))
        finally:
            for engine in engines:
                engine.close()
    return timings


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library', type=Path, default=ROOT / 'eci/libreloq_eci.so')
    p.add_argument('--baseline', type=Path, required=True)
    p.add_argument('--output', type=Path)
    p.add_argument('--worker', action='store_true', help=argparse.SUPPRESS)
    a = p.parse_args()
    if a.worker:
        print(json.dumps(worker(a.library.resolve())))
        return
    builds = []
    for library in (a.baseline, a.library):
        r = subprocess.run([sys.executable, str(Path(__file__).resolve()),
            '--worker', '--baseline', str(a.baseline.resolve()),
            '--library', str(library.resolve())], check=True, capture_output=True,
            text=True, timeout=180)
        builds.append(json.loads(r.stdout))
    assert builds[0]['speech'] == builds[1]['speech'], 'PCM/index regression'
    for build, timings in zip(builds, paired_timings(a.baseline.resolve(), a.library.resolve())):
        build['timings'] = timings
    report = dict(method=__doc__.strip(), no_audio_files=True,
                  identical_pcm_cases=len(builds[0]['speech']), builds=builds)
    if a.output:
        a.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + '\n')
    print(json.dumps({k: v for k, v in report.items() if k != 'builds'}, indent=2))
    for before, after in zip(builds[0]['timings'], builds[1]['timings']):
        print(before['language'], before['rate'],
              round(before['median_cpu_ms'], 3), round(after['median_cpu_ms'], 3))


if __name__ == '__main__':
    main()
