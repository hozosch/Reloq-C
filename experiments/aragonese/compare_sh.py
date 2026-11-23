"""Create a labelled listening comparison for Aragonese x/ix research.

These are manually selected phoneme probes, not an Aragonese voice, a general
grapheme-to-phoneme rule, or a modified consonant derived from the digit 8.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE

# Contrasting /tʃ/ and /ʃ/ in otherwise identical Spanish-Mexican contexts.
# The spellings xada and caixa motivate the contexts, but these explicit
# phoneme tags are manual probes; they must never become dictionary rules.
COMPARISONS = (
    ('Probe eins: Tsch, wie in ocho.', '`[.1Ca.0da]'),
    ('Probe zwei: Sch, wie in show.', '`[.1Sa.0da]'),
    ('Im Wortinneren. Probe eins.', '`[.1kay.0Ca]'),
    ('Probe zwei.', '`[.1kay.0Sa]'),
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    spanish = DirectEciEngine(args.library.resolve(), dialect=0x20001, encoding='utf8', native_voice=True)
    audio = bytearray()
    try:
        for engine in (german, spanish):
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)
        def say(engine, text):
            chunk = bytearray()
            if not engine.speak([text], chunk.extend, lambda _index: None) or not chunk:
                raise RuntimeError('No audio for ' + text)
            audio.extend(chunk)
            audio.extend(bytes(16000 // 3 * 2))
        say(german, 'Vergleich für Aragonesisch. Zuerst acht auf Spanisch.')
        say(spanish, 'ocho')
        for label, phonemes in COMPARISONS:
            say(german, label)
            if phonemes not in spanish._phonemes_for_probe(phonemes):
                raise RuntimeError('Spanish phoneme probe was not accepted: ' + phonemes)
            say(spanish, phonemes)
        say(german, 'Zum Vergleich: acht auf Deutsch.')
        say(german, 'acht')
    finally:
        german.close()
        spanish.close()
    with wave.open(str(args.wav), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(16000)
        output.writeframes(audio)


if __name__ == '__main__':
    main()
