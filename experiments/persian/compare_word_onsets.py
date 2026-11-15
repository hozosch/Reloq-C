"""Narrated comparison of automatic Persian vowel-initial word onsets."""
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
from persian_word_onsets import compare_onsets
from shared_sentence_contour import FRAME_TRACE, OVERRIDE, TRACE
from test_selected_probe import KEYS, engine, set_confirmed_persian_probe, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()
    german, fa = (engine(args.probe, d) for d in (0x40000, 0x90000))
    audio = bytearray()
    clips = 0
    try:
        for voice in (german, fa):
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
            append(render(german, text))

        label('Persisch. Vokal am Wortanfang. Bisher und mit schwächerem Stimmeinsatz.')
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            for caption, sentence, count in (
                ('Erstens. Ich mag Iran.', 'من ایران را دوست دارم.', 1),
                ('Zweitens. Er oder sie kam.', 'او آمد.', 1),
                ('Drittens. Ich bin heute hier.', 'من امروز اینجا هستم.', 2),
                ('Viertens. Gut ist.', 'خوب است.', 1),
            ):
                label(caption)
                set_confirmed_persian_probe(no_auto_glottal=0)
                result = compare_onsets(api, fa, args.probe, sentence, RATE)
                if len(result.changed) != count or result.baseline == result.softer:
                    raise RuntimeError('word-onset comparison did not activate as expected')
                label('Bisher.'); append(result.baseline)
                label('Schwächer.'); append(result.softer)
            label('Kontrolle. Der Glottisschlag im Wort Saat bleibt unverändert.')
            set_confirmed_persian_probe(no_auto_glottal=0)
            control = compare_onsets(api, fa, args.probe, 'من ساعت دارم.', RATE)
            if control.changed or control.baseline != control.softer:
                raise RuntimeError('explicit lexical glottal stop changed')
            append(control.baseline)
        label('Ende der Hörprobe.')
    finally:
        for key in (*KEYS, TRACE, OVERRIDE, FRAME_TRACE, 'RELOQ_FA_TRACE_E'):
            os.environ.pop(key, None)
        for voice in (german, fa):
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
