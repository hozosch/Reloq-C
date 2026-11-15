"""Make a German-labelled listening comparison for D, geminate J, and Z."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import tempfile
import wave

import numpy as np

from hybrid_fa_frontend import DirectFirstPersian
from espeak_ng_probe import EspeakPhonemeProbe
from listening import RATE, reference_audio, render
from test_selected_probe import engine, set_confirmed_persian_probe, set_probe
from text_frontend import translate_text
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
    narrator, fa = engine(args.probe, 0x40000), engine(args.probe, 0x90000)
    audio = bytearray()
    try:
        for voice in (narrator, fa):
            voice.set_parameter(SAMPLE_RATE, 3)

        def append(pcm):
            values = np.frombuffer(pcm, '<i2').astype(np.int32)
            if not len(values) or not np.any(values) or np.max(np.abs(values)) >= 32767:
                raise RuntimeError('empty or clipped listening sample')
            audio.extend(pcm)
            audio.extend(bytes(RATE))

        def label(text):
            set_probe()
            append(render(narrator, text))

        def persian(tag, *, d=0, z=0):
            set_confirmed_persian_probe(d=d, z=z)
            append(render(fa, tag))

        with DirectFirstPersian(args.native_library, unused, unused, unused) as frontend, \
             EspeakPhonemeProbe(args.espeak_library, args.data_parent) as espeak, \
             tempfile.TemporaryDirectory(prefix='reloq-fa-listen-') as name:
            temp = Path(name) / 'espeak.wav'
            env = dict(os.environ, ESPEAK_DATA_PATH=str(args.data_parent.resolve()),
                       LD_LIBRARY_PATH=str(args.espeak_library.resolve().parent))

            def reference(text):
                subprocess.run([str(args.espeak_binary.resolve()), '-v', 'fa', '-s', '175',
                                '-w', str(temp), text], check=True, env=env, capture_output=True)
                append(reference_audio(temp))

            label('Badbacht. Zuerst eSpeak, danach Eloquence mit bisherigem D und zwei längeren Varianten.')
            reference('بدبخت')
            tag = frontend.analyze_direct('بدبخت').parts[0].text
            for mode in (0, 1, 2):
                label(('Bisher.', 'Variante eins.', 'Variante zwei.')[mode])
                persian(tag, d=mode)
            label('Tadschob. Zuerst eSpeak, dann das bisher doppelt angesetzte Dsch und der neue längere Verschluss.')
            reference('تعجب')
            label('Bisher.')
            persian('`[.0tA.1wAJ.0Job]')
            label('Neu.')
            persian('`[.0tA.1wAd.0Job]')
            label('Das stimmhafte S. Drei Wörter, jeweils erst eSpeak, dann bisher, Variante eins und Variante zwei.')
            for text in ('زمان', 'دوازده', 'روز'):
                reference(text)
                analysis = frontend.analyze_direct(text)
                parts = analysis.parts if analysis else translate_text(espeak, text)
                if len(parts) != 1 or parts[0].language != 'fa':
                    raise RuntimeError('missing direct comparison word: ' + text)
                for mode in (0, 1, 2):
                    label(('Bisher.', 'Variante eins.', 'Variante zwei.')[mode])
                    persian(parts[0].text, z=mode)
    finally:
        narrator.close()
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
