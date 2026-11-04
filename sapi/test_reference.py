"""Match the real SAPI backend's RAM PCM to the existing direct ECI client."""
from pathlib import Path
import argparse
import array
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import (DirectEciEngine, EciIndex, SAMPLE_RATE, NATIVE_SPEED, NATIVE_PITCH,
    NATIVE_INFLECTION, HEAD_SIZE, NATIVE_VOLUME, GENDER, ROUGHNESS, BREATHINESS,
    PAUSE_MODE, PROCESS_TAGS)


def parts_with_events(text, offsets):
    """Insert the same source-position marks supplied to the SAPI backend."""
    parts, start = [], 0
    for value, offset in enumerate(offsets, 1):
        assert start <= offset <= len(text)
        if offset > start:
            parts.append(text[start:offset])
        parts.append(EciIndex(value))
        start = offset
    if start < len(text):
        parts.append(text[start:])
    return parts


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    args = parser.parse_args()
    report = json.loads(args.report.read_text())
    samples = {int(i, 16): json.loads('"' + sample + '"') for i, sample in
               re.findall(r'new Language\((0x[0-9a-f]+),"[^"]+","[^"]+","([^"]+)"\)',
                          (ROOT / 'android/java/org/reloq/tts/Catalog.java').read_text())}
    engines = {}
    try:
        for row in report['comparisons']:
            language = row['language']
            if language not in engines:
                engine = engines[language] = DirectEciEngine(args.library.resolve(), dialect=language,
                    encoding='utf8', native_voice=True)
                for key, value in ((NATIVE_SPEED, 60), (NATIVE_PITCH, 65), (NATIVE_INFLECTION, 30),
                    (HEAD_SIZE, 50), (NATIVE_VOLUME, 100), (GENDER, 0), (ROUGHNESS, 0), (BREATHINESS, 0),
                    (PAUSE_MODE, 0), (PROCESS_TAGS, 0)):
                    engine.set_parameter(key, value)
            engine = engines[language]
            engine.set_parameter(SAMPLE_RATE, row['rate'])
            pcm = bytearray()
            marks = []
            assert engine.speak(parts_with_events(samples[language], row['event_offsets']), pcm.extend,
                                lambda value: marks.append((value, len(pcm))))
            assert marks == list(enumerate(row['event_audio_offsets'], 1)), row
            values = array.array('h', pcm)
            digest = 1469598103934665603
            for value in values:
                digest = ((digest ^ (value & 0xffff)) * 1099511628211) & 0xffffffffffffffff
            assert (len(values), f'{digest:016x}') == (row['samples'], row['hash']), row
    finally:
        for engine in engines.values():
            engine.close()
    report['direct_eci_identical_pcm_cases'] = len(report['comparisons'])
    report['no_audio_files'] = True
    args.report.write_text(json.dumps(report, indent=2) + '\n')
    print(f'SAPI / direct ECI: {len(report["comparisons"])} identical PCM cases with the same event marks')


if __name__ == '__main__':
    main()
