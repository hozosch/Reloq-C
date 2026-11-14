"""Narrated comparison of French L in the Finnish Persian probe."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from listening import RATE, reference_audio, render
from salam_samples import SALAM, QALAM, PERSIAN_PHRASE
from test_native_probe import IRAN
from test_selected_probe import KEYS, set_probe

# French E is the short front-vowel comparison for Finnish ä.
SALAM_FR = '`[.s0E.l1am]'
QALAM_FR = '`[.k0E.l1Em]'


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    refs = {name: reference_audio(args.reference_dir / f'{name}.wav')
            for name in ('fa_statement', 'fa_qalam')}
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    french = DirectEciEngine(args.library.resolve(), dialect=0x30000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    audio = bytearray()
    count = 0
    try:
        for engine in (german, french, finnish):
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)
        for engine, tags in ((french, (SALAM_FR, QALAM_FR)),
                             (finnish, (SALAM, QALAM, IRAN, '`[.1lA]', '`[.1Al]',
                                        '`[.1lu]', '`[.1ol]', '`[.1pA]', '`[.1tA]', '`[.1kA]'))):
            for tag in tags:
                if tag not in engine._phonemes_for_probe(tag):
                    raise RuntimeError('Native phoneme tag rejected: ' + tag)

        def append(pcm: bytes) -> None:
            nonlocal count
            if not pcm:
                raise RuntimeError('empty speech item')
            audio.extend(pcm)
            audio.extend(bytes(RATE * 2 // 3))
            count += 1

        def label(text: str) -> None:
            set_probe()
            append(render(german, text))

        def speak(title: str, engine: DirectEciEngine, text: str, **flags: int) -> None:
            label(title)
            set_probe(**flags)
            append(render(engine, text))

        def ref(title: str, name: str) -> None:
            label(title)
            append(refs[name])

        label('Persisch. Hörvergleich für das französische L.')
        label('Erstens. Salam.')
        ref('Persisch, eSpeak. Salam Dunya.', 'fa_statement')
        speak('Französische Stimme. L zum Vergleich.', french, SALAM_FR)
        speak('Finnische Versuchsstimme. Bisheriges deutsches L.', finnish, SALAM, a=1, h=5, l=2)
        speak('Finnische Versuchsstimme. Französisches L.', finnish, SALAM, a=1, h=5, l=3)

        label('Zweitens. Kalam, das Wort für Stift.')
        ref('Persisch, eSpeak.', 'fa_qalam')
        speak('Französische Stimme. L zum Vergleich.', french, QALAM_FR)
        speak('Finnische Versuchsstimme. Bisheriges deutsches L.', finnish, QALAM, a=1, q=6, h=5, l=2)
        speak('Finnische Versuchsstimme. Französisches L.', finnish, QALAM, a=1, q=6, h=5, l=3)

        label('Drittens. Weitere Silben mit L am Anfang und am Ende.')
        syllables = '`[.1lA] `[.1Al] `[.1lu] `[.1ol]'
        speak('Bisheriges deutsches L.', finnish, syllables, a=1, h=5, l=2)
        speak('Französisches L.', finnish, syllables, a=1, h=5, l=3)
        speak('Der H Impuls bei 60 Prozent.', finnish, '`[.1pA] `[.1tA] `[.1kA]', a=1, h=5, l=3)
        speak('Iran. Das ausgewählte A.', finnish, IRAN, a=1, h=5, l=3)
        speak('Salam Dunya als Frage.', finnish, PERSIAN_PHRASE + '?', a=1, h=5, l=3, rise=2)
        label('Ende des Hörvergleichs.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)
        german.close()
        french.close()
        finnish.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote {count} labelled excerpts, {len(audio)/(2*RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
