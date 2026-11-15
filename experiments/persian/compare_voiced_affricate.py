"""Compare voiced geminate affricate candidates without a separate D."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile
import wave

import numpy as np

from hybrid_fa_frontend import DirectFirstPersian
from listening import RATE, reference_audio, render
from test_selected_probe import engine, set_confirmed_persian_probe, set_probe
from _reloq_eci_direct import SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--espeak-binary', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()
    unused = Path('/nonexistent/reloq-reference')
    de, fa = engine(args.probe, 0x40000), engine(args.probe, 0x90000)
    audio = bytearray()
    try:
        for voice in (de, fa):
            voice.set_parameter(SAMPLE_RATE, 3)

        def append(pcm):
            values = np.frombuffer(pcm, '<i2').astype(np.int32)
            if not len(values) or not np.any(values) or np.max(np.abs(values)) >= 32767:
                raise RuntimeError('empty or clipped listening sample')
            audio.extend(pcm)
            audio.extend(bytes(RATE))

        def label(text):
            set_probe()
            append(render(de, text))

        def say(tag, mode=0):
            set_confirmed_persian_probe(d=2, z=2, gem=mode)
            append(render(fa, tag))

        with DirectFirstPersian(args.native_library, unused, unused, unused) as frontend, \
             tempfile.TemporaryDirectory(prefix='reloq-fa-j-') as name:
            path = Path(name) / 'espeak.wav'
            env = dict(os.environ, ESPEAK_DATA_PATH=str(args.data_parent.resolve()),
                       LD_LIBRARY_PATH=str(args.espeak_library.resolve().parent))
            label('Ein zusammenhängendes stimmhaftes Dsch. Je Wort zuerst eSpeak, '
                  'dann bisher, und drei Versuche mit einem verlängerten Dsch.')
            for text, caption in (('تعجب', 'Erstes Wort.'), ('توجه', 'Zweites Wort.')):
                label(caption)
                subprocess.run([str(args.espeak_binary.resolve()), '-v', 'fa', '-s', '175',
                                '-w', str(path), text], check=True, env=env, capture_output=True)
                append(reference_audio(path))
                analysis = frontend.analyze_direct(text)
                if not analysis or len(analysis.parts) != 1:
                    raise RuntimeError('missing direct word: ' + text)
                original = analysis.parts[0].text
                single, count = re.subn(r'J(?=(?:\.[012])?J)', '', original)
                if count != 1:
                    raise RuntimeError('expected one double affricate: ' + text)
                label('Bisher.')
                say(original)
                for mode in (1, 2, 3):
                    label(('Variante eins.', 'Variante zwei.', 'Variante drei.')[mode - 1])
                    say(single, mode)
    finally:
        de.close()
        fa.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote {len(audio) / (2 * RATE):.1f} seconds of comparisons.')


if __name__ == '__main__':
    main()
