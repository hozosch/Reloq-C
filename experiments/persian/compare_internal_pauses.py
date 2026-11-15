"""Compare eSpeak's short pauses with the revised Persian glottal onset."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import tempfile
import wave

import numpy as np

from direct_fa_sequence import ScopedDirectPersianEngine
from hybrid_fa_frontend import DirectFirstPersian
from listening import RATE, reference_audio, render
from test_selected_probe import engine, set_probe
from _reloq_eci_direct import SAMPLE_RATE


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--scope-probe', type=Path, required=True)
    parser.add_argument('--narrator', type=Path, required=True)
    parser.add_argument('--espeak-binary', type=Path, required=True)
    parser.add_argument('--espeak-library', type=Path, required=True)
    parser.add_argument('--data-parent', type=Path, required=True)
    parser.add_argument('--wav', type=Path, required=True)
    args = parser.parse_args()
    samples = (('Erstes Wort, Konsonant vor Konsonant.', 'بدبخت', False),
               ('Zweites Wort, Vokal vor Vokal.', 'سوال', True),
               ('Drittes Wort, Konsonant vor Vokal.', 'بنابراین', True))
    audio = bytearray()
    narrator = engine(args.narrator, 0x40000)
    unused = Path('/nonexistent/reloq-reference')
    try:
        narrator.set_parameter(SAMPLE_RATE, 3)
        with DirectFirstPersian(args.native_library, unused, unused, unused) as frontend:
            voice = ScopedDirectPersianEngine(args.scope_probe, frontend)
            voice.set_parameter(SAMPLE_RATE, 3)
            try:
                def append(pcm):
                    values = np.frombuffer(pcm, '<i2').astype(np.int32)
                    if not len(values) or not np.any(values) or np.max(np.abs(values)) >= 32767:
                        raise RuntimeError('empty or clipped sample')
                    audio.extend(pcm)
                    audio.extend(bytes(RATE))

                def label(text):
                    set_probe()
                    append(render(narrator, text))

                label('Kurze Pausen auf Persisch. Erst eSpeak, dann Eloquence mit neuem Übergang.')
                with tempfile.TemporaryDirectory(prefix='reloq-fa-pause-reference-') as name:
                    reference = Path(name) / 'espeak.wav'
                    env = dict(os.environ, ESPEAK_DATA_PATH=str(args.data_parent.resolve()),
                               LD_LIBRARY_PATH=str(args.espeak_library.resolve().parent))
                    for caption, text, glottal in samples:
                        label(caption)
                        label('eSpeak.')
                        subprocess.run([str(args.espeak_binary.resolve()), '-v', 'fa', '-s', '175',
                                        '-w', str(reference), text],
                                       check=True, env=env, capture_output=True)
                        append(reference_audio(reference))
                        label('Eloquence mit neuem Übergang.')
                        analysis = frontend.analyze_direct(text)
                        if (not analysis or ',' in analysis.parts[0].text or
                                ('w' in analysis.parts[0].text) != glottal):
                            raise RuntimeError('sample does not have the selected pause behavior')
                        pcm = bytearray()
                        if not voice.speak((text,), pcm.extend, lambda _: None):
                            raise RuntimeError('Persian sample was cancelled')
                        append(pcm)
                assert frontend._worker is None
            finally:
                voice.close()
    finally:
        narrator.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote {len(samples)} paired examples, {len(audio) / (2 * RATE):.1f} seconds.')


if __name__ == '__main__':
    main()
