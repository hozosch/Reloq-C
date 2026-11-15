"""Spoken A/B sample of separate and shared Persian/English sentence melody."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
import wave

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, SAMPLE_RATE
from espeak_ng_probe import EspeakPhonemeProbe
from listening import RATE, render
from shared_sentence_contour import OVERRIDE, TRACE, compare
from test_selected_probe import KEYS, engine, set_confirmed_persian_probe, set_probe
from text_frontend import translate_text


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()
    german = engine(args.probe, 0x40000)
    fa = engine(args.probe, 0x90000)
    en = engine(args.probe, 0x10000)
    audio = bytearray()
    clips = 0
    try:
        for voice in (german, fa, en):
            voice.set_parameter(SAMPLE_RATE, 3)
            voice.set_parameter(GENDER, 0)
            voice.set_parameter(NATIVE_PITCH, 65)
            voice.set_parameter(NATIVE_INFLECTION, 30)

        def append(pcm: bytes) -> None:
            nonlocal clips
            samples = np.frombuffer(pcm, dtype='<i2').astype(np.int32)
            if not len(samples) or not np.any(samples) or np.max(np.abs(samples)) >= 32767:
                raise RuntimeError('empty or clipped audio segment')
            audio.extend(pcm)
            audio.extend(bytes(RATE * 2 // 3))
            clips += 1

        def label(text: str) -> None:
            set_probe()
            append(render(german, text))

        label('Persisch und Englisch. Vergleich der Satzmelodie beim Sprachwechsel.')
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            for caption, sentence in (
                ('Erstens. Ich mag NVDA. Aussage.', 'من NVDA را دوست دارم.'),
                ('Zweitens. Ist NVDA gut? Eine Frage.', 'آیا NVDA خوب است؟'),
                ('Drittens. Ich mag Windows und NVDA.', 'من Windows و NVDA را دوست دارم.'),
            ):
                label(caption)
                set_confirmed_persian_probe()
                outcome = compare(translate_text(api, sentence), fa, en, RATE)
                label('Bisher. Kurze Pausen, getrennte Melodie.'); append(outcome.baseline)
                label('Versuch. Gemeinsame Satzmelodie.'); append(outcome.short_pause)
        label('Ende der Hörprobe.')
    finally:
        for key in (*KEYS, TRACE, OVERRIDE):
            os.environ.pop(key, None)
        for voice in (german, fa, en):
            voice.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(RATE)
        output.writeframes(audio)
    print(f'Wrote {clips} clips, {len(audio) / (2 * RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
