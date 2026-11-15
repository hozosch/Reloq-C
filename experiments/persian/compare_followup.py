"""Labelled comparison of glottal, final H, K, back A and language switches."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import wave

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import EciLanguage, PROCESS_TAGS, SAMPLE_RATE
from _reloq_languages import LanguageEngine
from espeak_ng_probe import EspeakPhonemeProbe
from listening import RATE, reference_audio, render
from mixed_language_probe import last_phone_is_stop, splice
from test_selected_probe import KEYS, engine, set_probe
from text_frontend import native_tag_accepted, translate_text


def select(*, old: bool = False, nasal: int = 0) -> None:
    set_probe(a=1, q=7, x=1, rise=2, h=5, l=4, vowel=1, s=1,
              a_target='salam-fixed', final=1,
              glottal=1 if old else 2, end_h=0 if old else 1, nasal=nasal)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--espeak-binary', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()
    label_engine = engine(args.probe, 0x40000)
    validators = {'fa': engine(args.probe, 0x90000), 'en-us': engine(args.probe, 0x10000)}
    speakers = {old: LanguageEngine(path, language=0x90000)
                for old, path in ((True, args.previous), (False, args.probe))}
    audio = bytearray()
    clips = 0
    try:
        for item in (label_engine, *validators.values(), *speakers.values()):
            item.set_parameter(PROCESS_TAGS, 1)
            item.set_parameter(SAMPLE_RATE, 3)

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
            append(render(label_engine, text))

        def reference(text: str) -> None:
            with tempfile.TemporaryDirectory(prefix='reloq-fa-followup-') as folder:
                path = Path(folder) / 'reference.wav'
                env = dict(os.environ, ESPEAK_DATA_PATH=str(args.data_parent.resolve()))
                subprocess.run([str(args.espeak_binary.resolve()), '-v', 'fa', '-w', str(path), text],
                               check=True, env=env, capture_output=True)
                append(reference_audio(path))

        def speech(api: EspeakPhonemeProbe, text: str, *, old: bool = False,
                   nasal: int = 0, smooth: bool = False) -> None:
            select(old=old, nasal=nasal)
            parts = translate_text(api, text)
            buffers = []
            for part in parts:
                native = validators[part.language]
                if part.language == 'fa':
                    for tag in re.findall(r'`\[[^]]+\]', part.text):
                        if not native_tag_accepted(tag, native._phonemes_for_probe(tag)):
                            raise RuntimeError(f'native probe changed the tag: {tag}')
                pronunciation = native._phonemes_for_probe(part.text)
                language = 0x90000 if part.language == 'fa' else 0x10000
                pcm = bytearray()
                if not speakers[old].speak([EciLanguage(language, '1'), part.text], pcm.extend, lambda _: None):
                    raise RuntimeError('speech cancelled')
                buffers.append((bytes(pcm), part.text, last_phone_is_stop(pronunciation)))
            append(splice(buffers, RATE) if smooth else b''.join(pcm for pcm, _text, _stop in buffers))

        label('Persisch. Hörprobe für den Glottisschlag, das H, das K, den A Klang und den Sprachwechsel.')
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            label('Erstens. Glottisschlag. Saa at und Soal.')
            label('Espeak.'); reference('ساعت سؤال')
            label('Bisher.'); speech(api, 'ساعت سؤال', old=True)
            label('Neuer Versuch.'); speech(api, 'ساعت سؤال')

            label('Zweitens. End H. Dah, Dawazdah, Mehr.')
            label('Espeak.'); reference('ده ۱۲ مهر')
            label('Bisher.'); speech(api, 'ده ۱۲ مهر', old=True)
            label('Neuer Versuch.'); speech(api, 'ده ۱۲ مهر')

            label('Drittens. Das K am Wortanfang nach einem anderen Wort. Kalam Kaghaz.')
            label('Bisher.'); speech(api, 'قلم کاغذ', old=True)
            label('Mit Behauchung.'); speech(api, 'قلم کاغذ')

            label('Viertens. Das dunkle A, in Iran, Salam und Donja.')
            label('Bisher.'); speech(api, 'ایران سلام دنیا', nasal=0)
            label('Leicht nasaler.'); speech(api, 'ایران سلام دنیا', nasal=1)
            label('Etwas stärker.'); speech(api, 'ایران سلام دنیا', nasal=2)

            label('Fünftens. Sprachwechsel, Persisch und US Englisch, NVDA.')
            label('Bisher.'); speech(api, 'من NVDA را دوست دارم.', smooth=False)
            label('Mit verkürzten Pausen.'); speech(api, 'من NVDA را دوست دارم.', smooth=True)
        label('Ende der Hörprobe.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)
        for item in (label_engine, *validators.values(), *speakers.values()):
            item.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(RATE)
        output.writeframes(audio)
    print(f'Wrote {clips} clips, {len(audio) / (2 * RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
