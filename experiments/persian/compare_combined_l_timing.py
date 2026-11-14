"""Make a narrated Persian L-position and vowel-timing listening test."""
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
from salam_samples import (SALAM, QALAM, PERSIAN_PHRASE,
                           PERSIAN_PHRASE_FINAL_STRESS, DONYA_FINAL_STRESS)
from test_native_probe import IRAN
from test_selected_probe import KEYS, set_probe


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--library', required=True, type=Path)
    ap.add_argument('--reference-dir', required=True, type=Path)
    ap.add_argument('--wav', required=True, type=Path)
    args = ap.parse_args()
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    audio = bytearray()
    count = 0
    try:
        for engine in (german, finnish):
            engine.set_parameter(PROCESS_TAGS, 1)
            engine.set_parameter(SAMPLE_RATE, 3)

        def append(pcm: bytes):
            nonlocal count
            if not pcm:
                raise RuntimeError('empty speech sample')
            if np.max(np.abs(np.frombuffer(pcm, dtype='<i2').astype(np.int32))) >= 32767:
                raise RuntimeError('clipped sample')
            audio.extend(pcm)
            audio.extend(bytes(RATE * 2 // 3))
            count += 1

        def label(text: str):
            set_probe()
            append(render(german, text))

        def speak(title: str, text: str, *, l: int = 4, q: int = 5):
            label(title)
            set_probe(a=1, q=q, x=1, h=5, l=l, rise=2)
            append(render(finnish, text))

        def reference(title: str, name: str):
            label(title)
            append(reference_audio(args.reference_dir / (name + '.wav')))

        label('Persisch. Hörvergleich für das L und die Vokallänge. '
              'Das ist eine Lautprobe auf finnischer Basis, noch keine fertige persische Stimme.')
        reference('Espeak. Salam Donyâ.', 'fa_statement')
        speak('Bisher. Französisches L, und Donyâ auf der ersten Silbe betont.',
              PERSIAN_PHRASE + '.', l=3)
        speak('Neu. Deutsches L im Wort. Donyâ auf der letzten Silbe betont.',
              PERSIAN_PHRASE_FINAL_STRESS + '.')

        label('Donyâ einzeln. Vergleich der Betonung.')
        speak('Erste Silbe betont.', '`[.1do.0nja]')
        speak('Letzte Silbe betont.', DONYA_FINAL_STRESS)

        label('L im Wortinneren.')
        speak('Salam.', SALAM)
        reference('Kalam, Espeak.', 'fa_qalam')
        speak('Kalam. Deutsche L Regeln.', QALAM, q=6)

        label('L am Wortende.')
        speak('Saal einzeln.', '`[.1sal]')
        speak('Saal vor einem weiteren Wort.', '`[.1sal] `[.1sa]')
        speak('Iran. Das ausgewählte A.', IRAN)
        speak('Salam Donyâ als Frage.', PERSIAN_PHRASE_FINAL_STRESS + '?')
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
    print(f'Wrote {count} labelled excerpts, {len(audio)/(2*RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
