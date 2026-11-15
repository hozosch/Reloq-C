"""Hear retained first-word and removed inter-word glottalization."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
import wave

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from contextual_continuation import accepted_persian_ach_compare
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
    de, fa, en = (engine(args.probe, d) for d in (0x40000, 0x90000, 0x10000))
    audio = bytearray()
    clips = 0
    try:
        for voice in (de, fa, en):
            for parameter, value in ((SAMPLE_RATE, 3), (GENDER, 0), (NATIVE_PITCH, 65),
                                     (NATIVE_INFLECTION, 30), (NATIVE_SPEED, 50)):
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
            append(render(de, text))

        label('Persisch. Erster Wortanfang wie bisher; später ohne zusätzlichen Glottisschlag.')
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            text = translate_text(api, 'ایران')[0].text
            set_confirmed_persian_probe(no_auto_glottal=0)
            before = render(fa, text)
            set_confirmed_persian_probe(no_auto_glottal=1)
            after = render(fa, text)
            if before != after:
                raise RuntimeError('isolated first-word onset changed')
            label('Erstens. Iran allein, der ursprüngliche Einsatz.'); append(after)
            for caption, sentence in (
                ('Zweitens. Iran steht am Anfang; danach folgt ein Wort mit Vokal.', 'ایران آمد.'),
                ('Drittens. Ich mag Iran.', 'من ایران را دوست دارم.'),
                ('Viertens. Er oder sie kam.', 'او آمد.'),
            ):
                label(caption)
                text = translate_text(api, sentence)[0].text
                set_confirmed_persian_probe(no_auto_glottal=0)
                old = render(fa, text)
                set_confirmed_persian_probe(no_auto_glottal=1)
                new = render(fa, text)
                if old == new:
                    raise RuntimeError('expected inter-word onset did not change')
                label('Bisher.'); append(old)
                label('Nur späterer Wortanfang geändert.'); append(new)
            label('Der Glottisschlag in Saat bleibt erhalten.')
            set_confirmed_persian_probe()
            append(render(fa, translate_text(api, 'من ساعت دارم.')[0].text))
            label('Und die gewählte zweite Variante: Ist NVDA gut?')
            set_confirmed_persian_probe()
            result = accepted_persian_ach_compare(translate_text(api, 'آیا NVDA خوب است؟'),
                                                   fa, en, de, args.probe, RATE)
            if not result.english_voice_releases or not result.english_a_extensions:
                raise RuntimeError('selected A-to-Ach transition did not activate')
            append(result.connected)
        label('Ende der Hörprobe.')
    finally:
        for key in (*KEYS, TRACE, OVERRIDE, FRAME_TRACE, 'RELOQ_FA_TRACE_E'):
            os.environ.pop(key, None)
        for voice in (de, fa, en):
            voice.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as sink:
        sink.setnchannels(1)
        sink.setsampwidth(2)
        sink.setframerate(RATE)
        sink.writeframes(audio)
    print(f'Wrote {clips} clips, {len(audio) / (2 * RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
