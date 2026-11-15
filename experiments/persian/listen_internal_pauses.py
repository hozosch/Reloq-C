"""Render a short labelled listening probe for Persian comma boundaries."""
from __future__ import annotations

import argparse
from pathlib import Path
import wave

import numpy as np

from direct_fa_sequence import ScopedDirectPersianEngine
from hybrid_fa_frontend import DirectFirstPersian
from listening import render
from test_selected_probe import engine, set_confirmed_persian_probe, set_probe
from _reloq_eci_direct import SAMPLE_RATE

RATE = 16000


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--scope-probe', type=Path, required=True)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--worker', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()
    examples = (
        ('Erstes Beispiel.', 'من بدبخت آمد.'),
        ('Zweites Beispiel.', 'آبگوشت،سلام.'),
        ('Drittes Beispiel, zwei innere Pausen.', 'بنابراین آمد.'),
        ('Viertes Beispiel, eine geschriebene und eine innere Pause treffen zusammen.', 'سلام،آیلار.'),
    )
    audio = bytearray()
    de = engine(args.reference, 0x40000)
    reference = engine(args.reference, 0x90000)
    with DirectFirstPersian(args.native_library, args.worker,
                            args.espeak_library, args.data_parent) as frontend:
        voice = ScopedDirectPersianEngine(args.scope_probe, frontend, allow_fallback=True)
        try:
            for target in (de, reference, voice):
                target.set_parameter(SAMPLE_RATE, 3)

            def append(pcm):
                samples = np.frombuffer(pcm, dtype='<i2').astype(np.int32)
                if not len(samples) or not np.any(samples) or np.max(np.abs(samples)) >= 32767:
                    raise RuntimeError('empty or clipped listening segment')
                audio.extend(pcm)
                audio.extend(bytes(RATE))

            def label(text):
                set_probe()
                append(render(de, text))

            label('Persisch. Kurze innere Pausen als Kommapausen. Vier Beispiele.')
            for caption, text in examples:
                label(caption)
                analysis = frontend.analyze(text)
                if len(analysis.parts) != 1 or analysis.parts[0].language != 'fa':
                    raise RuntimeError('sample is not a Persian-only utterance')
                tagged = analysis.parts[0].text
                if ',' not in tagged:
                    raise RuntimeError('sample lacks its comma boundary')
                pcm = bytearray()
                if not voice.speak((text,), pcm.extend, lambda _: None):
                    raise RuntimeError('sample synthesis was cancelled')
                set_confirmed_persian_probe()
                if bytes(pcm) != render(reference, tagged):
                    raise RuntimeError('scoped voice differs from selected reference')
                append(pcm)
        finally:
            voice.close()
            de.close()
            reference.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote {len(examples)} checked examples, {len(audio) / (2 * RATE):.1f} seconds.')


if __name__ == '__main__':
    main()
