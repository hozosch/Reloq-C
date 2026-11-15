"""Hear U→A alternatives and a separate glottal-onset reference."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import wave

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_INFLECTION, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from espeak_ng_probe import EspeakPhonemeProbe
from listening import RATE, reference_audio, render
from persian_vowel_glide import compare_vowel_glide
from shared_sentence_contour import FRAME_TRACE, OVERRIDE, TRACE
from test_selected_probe import KEYS, engine, set_confirmed_persian_probe, set_probe
from text_frontend import translate_text


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--espeak-binary', type=Path, required=True)
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

        label('Persisch. Vom U zum A an der Wortgrenze.')
        label('Eins: sanftere Resonanzen. Zwei: zusätzlich eine sanftere Tonhöhe. Drei: Glottaleinsatz.')
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            for caption, sentence in (
                ('Erstens. Er oder sie kam.', 'او آمد.'),
                ('Zweitens. Er oder sie war ruhig.', 'او آرام بود.'),
                ('Drittens. Er oder sie war dort.', 'او آنجا بود.'),
            ):
                label(caption)
                set_confirmed_persian_probe()
                trial = compare_vowel_glide(api, fa, args.probe, sentence, RATE)
                if (len(trial.changed) != 1 or trial.changed[0][3] < 0 or
                        trial.baseline == trial.glide or trial.glide == trial.glide_pitch):
                    raise RuntimeError('vowel comparison did not activate')
                # These examples have no other noninitial vowel-initial word,
                # so restoring the native rule produces only the desired
                # inter-vocalic glottal-onset comparison.
                set_confirmed_persian_probe(no_auto_glottal=0)
                glottal = render(fa, translate_text(api, sentence)[0].text)
                if glottal == trial.baseline:
                    raise RuntimeError('glottal-onset reference did not activate')
                label('Bisher ohne Glottaleinsatz.'); append(trial.baseline)
                label('Eins.'); append(trial.glide)
                label('Zwei.'); append(trial.glide_pitch)
                label('Drei. Mit Glottaleinsatz.'); append(glottal)
            label('Zum Vergleich. Ein U A innerhalb eines Wortes, nach ESpeak Lautzuweisung.')
            set_confirmed_persian_probe()
            append(render(fa, translate_text(api, 'توالت')[0].text))
            label('ESpeak sagt: Er oder sie kam.')
            with tempfile.TemporaryDirectory(prefix='reloq-espeak-fa-glide-') as name:
                wav = Path(name) / 'reference.wav'
                subprocess.run([str(args.espeak_binary.resolve()),
                                '--path=' + str(args.data_parent.resolve()),
                                '-v', 'fa', '-w', str(wav), 'او آمد.'], check=True)
                append(reference_audio(wav))
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
