"""Offline Aragonese/Spanish pronunciation probe; this is not a voice."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import sys
import unicodedata
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))

# Small orthographic probes from the 2023 Academia Aragonesa de la Lengua
# orthography. Deliberately not a pronunciation dictionary or a corpus.
SAMPLES = ('sinyal', 'cabanyera', 'nyapir', 'bony', 'Espanya', 'cigüenya',
           'choven', 'puent', 'aixina', 'xada', 'fixar', 'mocez')
_NY = re.compile('ny', re.IGNORECASE | re.ASCII)


def spanish_probe_input(text: str) -> str:
    """Try the /ɲ/ digraph via the Spanish backend, for this experiment only.

    The rule is intentionally not hooked into normal speech: proper names,
    other languages, text offsets, dictionaries and character mode still
    require explicit handling before such a transformation can be used.
    """
    normalized = unicodedata.normalize('NFC', text)
    return _NY.sub(lambda match: 'Ñ' if match[0][0].isupper() else 'ñ', normalized)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--wav', type=Path, help='write a local listening sample')
    args = parser.parse_args()
    from _reloq_eci_direct import DirectEciEngine, SAMPLE_RATE
    engine = DirectEciEngine(args.library.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    report, samples = [], bytearray()
    try:
        engine.set_parameter(SAMPLE_RATE, 1)
        for text in SAMPLES:
            adapted = spanish_probe_input(text)
            phones = engine._phonemes_for_probe(adapted)
            if not phones:
                raise RuntimeError('No phonemes for ' + text)
            report.append({'aragonese': text, 'spanish_probe_input': adapted, 'phonemes': phones})
            if args.wav:
                pcm = bytearray()
                if not engine.speak([adapted], pcm.extend, lambda _index: None) or not pcm:
                    raise RuntimeError('No audio for ' + text)
                samples.extend(pcm)
                samples.extend(bytes(11025 // 3 * 2))
    finally:
        engine.close()
    if args.wav:
        with wave.open(str(args.wav), 'wb') as output:
            output.setnchannels(1)
            output.setsampwidth(2)
            output.setframerate(11025)
            output.writeframes(samples)
    print(json.dumps(report, ensure_ascii=False, indent=2))


if __name__ == '__main__':
    main()
