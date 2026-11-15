"""Compare the Finnish automatic onset with the no-extra-onset Persian probe."""
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
    de, fa = (engine(args.probe, d) for d in (0x40000, 0x90000))
    audio = bytearray()
    clips = 0
    try:
        for voice in (de, fa):
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

        label('Persisch. Wortanfänge. Bisher und ohne automatischen Glottisschlag.')
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            for caption, sentence in (
                ('Erstens. Ich mag Iran.', 'من ایران را دوست دارم.'),
                ('Zweitens. Er oder sie kam.', 'او آمد.'),
                ('Drittens. Ein Mensch.', 'یک آدم.'),
                ('Viertens. Iran allein.', 'ایران'),
            ):
                label(caption)
                part = translate_text(api, sentence)
                if len(part) != 1 or part[0].language != 'fa':
                    raise RuntimeError('expected one Persian-only segment')
                set_confirmed_persian_probe(no_auto_glottal=0)
                before = render(fa, part[0].text)
                set_confirmed_persian_probe(no_auto_glottal=1)
                after = render(fa, part[0].text)
                if before == after:
                    raise RuntimeError('automatic Finnish onset did not change')
                label('Bisher.'); append(before)
                label('Ohne zusätzlichen Glottisschlag.'); append(after)
            label('Kontrolle. Der Glottisschlag innerhalb von Saat bleibt erhalten.')
            tags = translate_text(api, 'من ساعت دارم.')[0].text
            set_confirmed_persian_probe(no_auto_glottal=0)
            before = render(fa, tags)
            set_confirmed_persian_probe(no_auto_glottal=1)
            after = render(fa, tags)
            if before != after:
                raise RuntimeError('explicit stop control unexpectedly changed')
            append(after)
        label('Ende der Hörprobe.')
    finally:
        for key in (*KEYS, TRACE, OVERRIDE, FRAME_TRACE, 'RELOQ_FA_TRACE_E'):
            os.environ.pop(key, None)
        for voice in (de, fa):
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
