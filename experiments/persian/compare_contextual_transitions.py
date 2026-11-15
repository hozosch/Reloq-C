"""Narrated listening comparison of cross-language transition candidates."""
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
from contextual_continuation import contextual_compare
from espeak_ng_probe import EspeakPhonemeProbe
from listening import RATE, render
from shared_sentence_contour import FRAME_TRACE, OVERRIDE, TRACE
from test_selected_probe import KEYS, engine, set_confirmed_persian_probe, set_probe
from text_frontend import translate_text


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()
    german, fa, en = (engine(args.probe, lang) for lang in (0x40000, 0x90000, 0x10000))
    audio = bytearray()
    clips = 0
    try:
        for voice in (german, fa, en):
            for parameter, value in ((SAMPLE_RATE, 3), (GENDER, 0), (NATIVE_PITCH, 65),
                                     (NATIVE_INFLECTION, 30)):
                voice.set_parameter(parameter, value)

        def append(pcm: bytes) -> None:
            nonlocal clips
            samples = np.frombuffer(pcm, '<i2').astype(np.int32)
            if not len(samples) or not np.any(samples) or np.max(np.abs(samples)) >= 32767:
                raise RuntimeError('empty or clipped audio segment')
            audio.extend(pcm)
            audio.extend(bytes(RATE * 2 // 3))
            clips += 1

        def label(text: str) -> None:
            set_probe()
            append(render(german, text))

        label('Persisch und Englisch. Vergleich der Sprachübergänge.')
        label('Variante eins verwendet lautnäheren Satzkontext. Variante zwei ergänzt einen sanften Übergang.')
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            sentences = (
                ('Erstens. Ich mag NVDA.', 'من NVDA را دوست دارم.', True),
                ('Zweitens. Ist NVDA gut? Eine Frage.', 'آیا NVDA خوب است؟', True),
                ('Drittens. Ich habe einen Laptop.', 'من یک laptop دارم.', False),
                ('Viertens. Ich mag Windows und NVDA.', 'من Windows و NVDA را دوست دارم.', True),
            )
            for caption, sentence, compare_bridge in sentences:
                label(caption)
                set_confirmed_persian_probe()
                parts = translate_text(api, sentence)
                old = contextual_compare(parts, fa, en, args.probe, RATE)
                near = contextual_compare(parts, fa, en, args.probe, RATE, phone_context=True)
                bridge = (contextual_compare(parts, fa, en, args.probe, RATE,
                                             phone_context=True, bridge_ms=25)
                          if compare_bridge else None)
                label('Bisher.'); append(old.connected)
                label('Variante eins.'); append(near.connected)
                if bridge is not None:
                    label('Variante zwei.'); append(bridge.connected)
        label('Ende der Hörprobe.')
    finally:
        for key in (*KEYS, TRACE, OVERRIDE, FRAME_TRACE, 'RELOQ_FA_TRACE_E'):
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
