"""Validate and optionally render the direct Persian number experiment."""
from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys
import wave

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import SAMPLE_RATE
from hybrid_fa_frontend import DirectFirstPersian
from listening import render
from test_selected_probe import engine, set_confirmed_persian_probe
from text_frontend import native_tag_accepted


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native-library', type=Path, required=True)
    parser.add_argument('--probe', type=Path, required=True)
    parser.add_argument('--wav', type=Path)
    args = parser.parse_args()
    examples = (
        ('Null.', '۰'), ('Zwölf.', '۱۲'), ('Einundzwanzig.', '۲۱'),
        ('Einhunderteins.', '۱۰۱'), ('Tausendeins.', '۱٬۰۰۱'),
        ('Zwölftausenddreihundertfünfundvierzig.', '۱۲٬۳۴۵'),
        ('Drei Komma eins vier.', '۳٫۱۴'), ('Null Komma null fünf.', '۰٫۰۵'),
        ('Minus drei Komma vier.', '−۳٫۴'),
        ('Tausendzweihundertvierunddreißig Komma fünf.', '۱٬۲۳۴٫۵'),
        ('Eine Frage mit einer Zahl.', 'سلام ۳٫۱۴ دنیا؟'),
    )
    equivalences = (('۳٫۱۴', '3.14'), ('۱۲٬۳۴۵', '12,345'),
                    ('۱٬۲۳۴٫۵', '1,234.5'), ('−۳٫۴', '-3.4'))
    # This script only exercises inputs fully covered by C; no reference
    # process can be started from the placeholder paths.
    unused = Path('/nonexistent/reloq-reference')
    audio = bytearray()
    fa, de = engine(args.probe, 0x90000), engine(args.probe, 0x40000)
    try:
        set_confirmed_persian_probe()
        fa.set_parameter(SAMPLE_RATE, 3)
        de.set_parameter(SAMPLE_RATE, 3)
        with DirectFirstPersian(args.native_library, unused, unused, unused) as frontend:
            def tagged(text):
                result = frontend.analyze(text)
                assert not result.used_fallback and len(result.parts) == 1
                assert result.parts[0].language == 'fa'
                return result.parts[0].text

            for left, right in equivalences:
                one, two = tagged(left), tagged(right)
                assert one == two and render(fa, one) == render(fa, two), (left, right)
            if args.wav:
                audio.extend(render(de, 'Persische Zahlen. Zuerst die deutsche Ansage, dann Persisch.'))
                audio.extend(bytes(16000))
            for label, source in examples:
                text = tagged(source)
                for tag in re.findall(r'`\[[^]]+\]', text):
                    assert native_tag_accepted(tag, fa._phonemes_for_probe(tag)), (source, tag)
                pcm = render(fa, text)
                signal = np.frombuffer(pcm, dtype='<i2').astype(np.int32)
                assert np.any(signal) and np.max(np.abs(signal)) < 32767, source
                if args.wav:
                    audio.extend(render(de, label))
                    audio.extend(bytes(8000))
                    audio.extend(pcm)
                    audio.extend(bytes(16000))
    finally:
        fa.close()
        de.close()
    if args.wav:
        args.wav.parent.mkdir(parents=True, exist_ok=True)
        with wave.open(str(args.wav), 'wb') as out:
            out.setnchannels(1)
            out.setsampwidth(2)
            out.setframerate(16000)
            out.writeframes(audio)
    print(f'Passed {len(examples)} selected-voice number samples and {len(equivalences)} script equivalences.')


if __name__ == '__main__':
    main()
