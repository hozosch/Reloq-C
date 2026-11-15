"""Spoken comparison using K before /ae/ and quieter final H as references."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile
import wave

import numpy as np

from espeak_ng_probe import EspeakPhonemeProbe
from listening import RATE, reference_audio, render
from test_selected_probe import KEYS, SAMPLE_RATE, engine, set_confirmed_persian_probe, set_probe
from text_frontend import native_tag_accepted, translate_text


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--espeak-binary', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()
    german = engine(args.probe, 0x40000)
    persian = engine(args.probe, 0x90000)
    audio = bytearray()
    clips = 0
    try:
        for voice in (german, persian):
            voice.set_parameter(SAMPLE_RATE, 3)

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

        def reference(text: str) -> None:
            with tempfile.TemporaryDirectory(prefix='reloq-fa-front-k-') as folder:
                path = Path(folder) / 'reference.wav'
                env = dict(os.environ, ESPEAK_DATA_PATH=str(args.data_parent.resolve()))
                subprocess.run([str(args.espeak_binary.resolve()), '-v', 'fa', '-w', str(path), text],
                               check=True, env=env, capture_output=True)
                append(reference_audio(path))

        def speech(api: EspeakPhonemeProbe, text: str, *, k_shape: int = 0,
                   end_h_db: int = 0) -> None:
            set_confirmed_persian_probe(k_shape=k_shape, end_h_db=end_h_db)
            parts = translate_text(api, text)
            if len(parts) != 1 or parts[0].language != 'fa':
                raise RuntimeError(f'expected one Persian part: {parts!r}')
            for tag in re.findall(r'`\[[^]]+\]', parts[0].text):
                if not native_tag_accepted(tag, persian._phonemes_for_probe(tag)):
                    raise RuntimeError(f'native probe changed the tag: {tag}')
            append(render(persian, parts[0].text))

        label('Persisch. K vor Ä und A. Und ein leiseres End H.')
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            label('Erstens. Käf, Kasch, Kaghaz. Das erste K vor Ä bleibt gleich.')
            label('Espeak.'); reference('کف کاش کاغذ')
            label('Bisher.'); speech(api, 'کف کاش کاغذ', k_shape=1)
            label('K, Variante eins.'); speech(api, 'کف کاش کاغذ', k_shape=3)
            label('K, Variante zwei.'); speech(api, 'کف کاش کاغذ', k_shape=4)
            label('K, Variante drei.'); speech(api, 'کف کاش کاغذ', k_shape=5)

            label('Zweitens. Dah, Dawazdah, Mah. Das End H.')
            label('Bisher.'); speech(api, 'ده ۱۲ ماه', end_h_db=3)
            label('H, minus sechs Dezibel.'); speech(api, 'ده ۱۲ ماه', end_h_db=6)
            label('H, minus neun Dezibel.'); speech(api, 'ده ۱۲ ماه', end_h_db=9)
            label('H, minus zwölf Dezibel.'); speech(api, 'ده ۱۲ ماه', end_h_db=12)
        label('Ende der Hörprobe.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)
        german.close()
        persian.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(RATE)
        output.writeframes(audio)
    print(f'Wrote {clips} clips, {len(audio) / (2 * RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
