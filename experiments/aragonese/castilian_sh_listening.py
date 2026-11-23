"""Audition a Mexican /ʃ/ grafted onto two Castilian phoneme probes.

This is an OFFLINE EDIT of rendered PCM, not a language or voice. The splice
and two shortened variants let a listener judge the idea before any native
acoustic-rule integration. Fixed boundaries were measured only for these two
explicit probes, one voice and 16 kHz; do not apply them to arbitrary speech.
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

RATE = 16000
WORDS = (
    ('Erstes Beispiel.', '`[.1Sa.0da]', '`[.1Ca.0da]'),  # xada-like
    ('Zweites Beispiel.', '`[.1Swe.0la]', '`[.1Cwe.0la]'),  # xuela-like
)


def millis(ms):
    return round(RATE * ms / 1000)


def splice(left, right, overlap_ms=6):
    overlap = millis(overlap_ms)
    if overlap < 2 or len(left) < overlap or len(right) < overlap:
        raise ValueError('short or invalid crossfade')
    middle = array('h', (round(left[-overlap+i] * (1 - i/(overlap-1)) +
                               right[i] * i/(overlap-1)) for i in range(overlap)))
    return left[:-overlap] + middle + right[overlap:]


def shorten_frication(source, remove_ms):
    # Preserve attack and end of the measured unvoiced region. The removed
    # material is turbulent noise, so a short crossfade avoids a hard splice.
    begin, end, last = millis(30), millis(30 + remove_ms), millis(120)
    if not (0 < begin < end < last < len(source)):
        raise ValueError('invalid measured fricative window')
    return splice(source[:begin], source[end:last])


def graft(donor, carrier, remove_ms):
    # The Castilian vowel begins around 50-60 ms in these test items. Crossfade
    # just before that onset; the donor's vowels and following speech are not
    # used. This is only a listening sketch, not synthesis-time coarticulation.
    return splice(shorten_frication(donor, remove_ms), carrier[millis(50):], 8)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    castilian = DirectEciEngine(args.library.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    mexican = DirectEciEngine(args.library.resolve(), dialect=0x20001, encoding='utf8', native_voice=True)
    out = array('h')
    try:
        for engine in (german, castilian, mexican):
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)
        def render(engine, text):
            pcm = bytearray()
            if not engine.speak([text], pcm.extend, lambda _index: None) or not pcm:
                raise RuntimeError('No audio for ' + text)
            result = array('h'); result.frombytes(pcm)
            if sys.byteorder != 'little': result.byteswap()
            return result
        def add(samples):
            out.extend(samples)
            out.extend([0] * millis(250))
        def say(engine, text):
            add(render(engine, text))
        say(german, 'Aragonesisch. Hörversuch mit kastilischer Grundlage.')
        for title, donor_tag, carrier_tag in WORDS:
            if donor_tag not in mexican._phonemes_for_probe(donor_tag):
                raise RuntimeError('Mexican sh phoneme was not accepted')
            if carrier_tag not in castilian._phonemes_for_probe(carrier_tag):
                raise RuntimeError('Castilian carrier phoneme was not accepted')
            donor, carrier = render(mexican, donor_tag), render(castilian, carrier_tag)
            say(german, title + ' Probe eins: mexikanisches Sch, unverändert.')
            add(donor)
            say(german, 'Probe zwei: kürzeres Sch mit kastilischer Stimme.')
            add(graft(donor, carrier, 30))
            say(german, 'Probe drei: noch kürzeres Sch mit kastilischer Stimme.')
            add(graft(donor, carrier, 50))
        say(german, 'Kastilische Grundlage: erst S, dann der Th-Laut.')
        say(castilian, 'saso')
        say(castilian, 'caza')
    finally:
        german.close(); castilian.close(); mexican.close()
    if sys.byteorder != 'little': out.byteswap()
    with wave.open(str(args.wav), 'wb') as wav:
        wav.setnchannels(1); wav.setsampwidth(2); wav.setframerate(RATE)
        wav.writeframes(out.tobytes())


if __name__ == '__main__':
    main()
