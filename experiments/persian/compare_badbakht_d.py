"""Compare the native D release in 100 and at badbakht's short boundary."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import tempfile
import wave

import numpy as np
from listening import RATE, reference_audio, render
from test_selected_probe import engine, set_confirmed_persian_probe, set_probe
from text_frontend import native_sequence_accepted
from _reloq_eci_direct import SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference', type=Path, required=True)
    parser.add_argument('--espeak-binary', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()
    de, fa = engine(args.reference, 0x40000), engine(args.reference, 0x90000)
    audio = bytearray()
    try:
        for voice in (de, fa):
            voice.set_parameter(SAMPLE_RATE, 3)

        def append(pcm):
            samples = np.frombuffer(pcm, '<i2').astype(np.int32)
            if not len(samples) or not np.any(samples) or np.max(np.abs(samples)) >= 32767:
                raise RuntimeError('empty or clipped sample')
            audio.extend(pcm)
            audio.extend(bytes(RATE))

        def label(text):
            set_probe()
            append(render(de, text))

        def persian(text):
            set_confirmed_persian_probe()
            shown = fa._phonemes_for_probe(text)
            if not native_sequence_accepted(text, shown):
                raise RuntimeError('native parser changed a D comparison')
            append(render(fa, text))

        label('Persischer D Laut. Zuerst hundert, dann badbakht in drei Varianten.')
        label('Hundert mit Eloquence.')
        persian("`[.1s'A:'d]")
        label('Badbakht mit eSpeak.')
        with tempfile.TemporaryDirectory(prefix='reloq-fa-d-reference-') as name:
            reference = Path(name) / 'espeak.wav'
            env = dict(os.environ, ESPEAK_DATA_PATH=str(args.data_parent.resolve()),
                       LD_LIBRARY_PATH=str(args.espeak_library.resolve().parent))
            subprocess.run([str(args.espeak_binary.resolve()), '-v', 'fa', '-s', '175',
                            '-w', str(reference), 'بدبخت'], check=True, env=env, capture_output=True)
            append(reference_audio(reference))
        label('Bisher ohne Wortgrenze.')
        persian('`[.1bAd.0bAGt]')
        label('Neu mit normalem D vor dem nächsten Teil.')
        persian('`[.1bAd] `[.0bAGt]')
    finally:
        de.close()
        fa.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote D comparison, {len(audio) / (2 * RATE):.1f} seconds.')


if __name__ == '__main__':
    main()
