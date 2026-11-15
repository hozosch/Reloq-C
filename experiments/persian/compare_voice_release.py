"""Listen to English A's voice release at the Persian Ach switch."""
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
from contextual_continuation import accepted_german_ach_compare
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

        label('Englisches A zum persischen Ach Laut. Ausklang der Stimme.')
        label('Eins: sanfter Stimmabfall. Zwei: der gleiche Abfall mit längerem A.')
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            for caption, sentence in (
                ('Erstens. Ist NVDA gut?', 'آیا NVDA خوب است؟'),
                ('Zweitens. NVDA ist sehr gut.', 'NVDA خیلی خوب است.'),
                ('Drittens. Ist NVDA kaputt?', 'آیا NVDA خراب است؟'),
            ):
                label(caption)
                set_confirmed_persian_probe(no_auto_glottal=1)
                parts = translate_text(api, sentence)
                base = accepted_german_ach_compare(parts, fa, en, de, args.probe, RATE)
                one = accepted_german_ach_compare(parts, fa, en, de, args.probe, RATE,
                                                   voice_release_ms=24)
                two = accepted_german_ach_compare(parts, fa, en, de, args.probe, RATE,
                                                   voice_release_ms=24, english_a_stretch=1.15)
                if (not one.english_voice_releases or not two.english_voice_releases or
                        not two.english_a_extensions or
                        base.connected == one.connected or one.connected == two.connected):
                    raise RuntimeError('expected voiced transition did not activate')
                label('Bisher.'); append(base.connected)
                label('Eins.'); append(one.connected)
                label('Zwei.'); append(two.connected)
            label('Kontrolle ohne Ach Laut. Ich mag NVDA.')
            set_confirmed_persian_probe(no_auto_glottal=1)
            control = accepted_german_ach_compare(translate_text(api, 'من NVDA را دوست دارم.'),
                                                  fa, en, de, args.probe, RATE,
                                                  voice_release_ms=24, english_a_stretch=1.15)
            if control.english_voice_releases or control.english_a_extensions:
                raise RuntimeError('unexpected transition in control sentence')
            append(control.connected)
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
