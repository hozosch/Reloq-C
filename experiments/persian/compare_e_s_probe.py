"""Create a brief German-labelled E, S and stress comparison for Persian."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
import wave

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from listening import RATE, reference_audio, render
from salam_samples import IRAN_ESPEAK, PERSIAN_PHRASE, SALAM
from test_selected_probe import KEYS, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    audio = bytearray()
    clips = 0
    try:
        for engine in (german, finnish):
            engine.set_parameter(PROCESS_TAGS, 1)
            engine.set_parameter(SAMPLE_RATE, 3)

        def append(pcm: bytes) -> None:
            nonlocal clips
            if not pcm or np.max(np.abs(np.frombuffer(pcm, dtype='<i2').astype(np.int32))) >= 32767:
                raise RuntimeError('empty or clipped sample')
            audio.extend(pcm)
            audio.extend(bytes(RATE * 2 // 3))
            clips += 1

        def label(text: str) -> None:
            set_probe()
            append(render(german, text))

        def native(title: str, text: str, *, vowel: int = 1, s: int = 1) -> None:
            label(title)
            set_probe(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=vowel, s=s)
            phones = finnish._phonemes_for_probe(text)
            for group in text.split():
                if group.startswith('`[') and group.rstrip('.?') not in phones:
                    raise RuntimeError('phoneme tag rejected: ' + group)
            append(render(finnish, text))

        def reference(title: str, name: str) -> None:
            label(title)
            append(reference_audio(args.reference_dir / (name + '.wav')))

        label('Persisch. Kurzer Hörvergleich für S, E und Betonung. '
              'Die Beispiele sind Lautproben, noch keine auswählbare Stimme.')
        label('Finnisch. Viisi, das Wort für fünf.')
        set_probe()
        append(render(finnish, 'viisi'))
        native('Salam. Bisheriges S.', SALAM, s=0)
        native('Salam. S an Viisi angeglichen.', SALAM)
        native('S nach U. Bisher.', '`[.1us]', s=0)
        native('S nach U. Angeglichen.', '`[.1us]')
        label('E. Das Wort für Null, sefr.')
        native('Bisherige E Dauer.', '`[.1sefr]', vowel=0)
        native('E mit dem Grundverhältnis von Espeak.', '`[.1sefr]')
        reference('Iran. Espeak.', 'fa_iran')
        native('Iran. Unsere Klangfarbe und Espeak Betonung.', IRAN_ESPEAK)
        reference('Salam Donyâ. Espeak.', 'fa_statement')
        native('Unsere Probe. Aussage.', PERSIAN_PHRASE + '.')
        native('Unsere Probe. Frage.', PERSIAN_PHRASE + '?')
        label('Ende des Hörvergleichs.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)
        german.close()
        finnish.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote {clips} spoken clips, {len(audio)/(2*RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
