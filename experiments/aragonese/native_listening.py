"""Produce a labelled listening comparison from isolated native probes.

The explicitly supplied experimental libraries are NOT installable builds or
normal language modules. The S/au modifications are only tested in a private
Castilian-based binary; the ordinary library supplies unmodified references.
"""
from __future__ import annotations

import argparse
from array import array
from pathlib import Path
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from castilian_sh_listening import graft, millis


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--native-sh', type=Path, required=True)
    parser.add_argument('--rounded-au', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()

    german = DirectEciEngine(args.baseline.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    castilian = DirectEciEngine(args.baseline.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    mexican = DirectEciEngine(args.baseline.resolve(), dialect=0x20001, encoding='utf8', native_voice=True)
    native = DirectEciEngine(args.native_sh.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    rounded = DirectEciEngine(args.rounded_au.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    engines = (german, castilian, mexican, native, rounded)
    samples = array('h')
    try:
        for engine in engines:
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)

        def render(engine, text):
            pcm = bytearray()
            if not engine.speak([text], pcm.extend, lambda _index: None) or not pcm:
                raise RuntimeError('No audio for ' + text)
            result = array('h')
            result.frombytes(pcm)
            if sys.byteorder != 'little':
                result.byteswap()
            return result

        def add(audio):
            samples.extend(audio)
            samples.extend([0] * millis(300))

        def say(text):
            add(render(german, text))

        examples = (('Erstes Wort.', '`[.1Sa.0da]', '`[.1Ca.0da]'),
                    ('Zweites Wort.', '`[.1Swe.0la]', '`[.1Cwe.0la]'))
        say('Aragonesisch. Vergleich des Sch und des Au.')
        for label, source, carrier in examples:
            assert source in native._phonemes_for_probe(source), source
            say(label + ' Bisheriger Hörversuch.')
            add(graft(render(mexican, source), render(castilian, carrier), 30))
            say('Sch direkt im kastilischen Kern, moderat verkürzt.')
            add(render(native, source))
        say('Das neue Sch in zwei weiteren Positionen.')
        for source in ('`[.0a.1Si.0na]', '`[.1peS]'):
            assert source in native._phonemes_for_probe(source), source
            add(render(native, source))

        say('Jetzt das Au. Zuerst bisher, dann mit stärker gerundetem Übergang.')
        for word in ('auca', 'pauto', 'cudiau'):
            before, after = render(castilian, word), render(rounded, word)
            assert castilian._phonemes_for_probe(word) == rounded._phonemes_for_probe(word)
            if before == after:
                raise RuntimeError('The rounded-au probe had no effect on ' + word)
            say('Bisher.')
            add(before)
            say('Gerundeter Versuch.')
            add(after)
    finally:
        for engine in engines:
            engine.close()

    if sys.byteorder != 'little':
        samples.byteswap()
    with wave.open(str(args.wav), 'wb') as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(16000)
        wav.writeframes(samples.tobytes())


if __name__ == '__main__':
    main()
