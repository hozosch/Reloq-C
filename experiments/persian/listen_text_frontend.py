"""Create a German-labelled listening sample from real Persian input text."""
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
from test_selected_probe import KEYS, engine, set_confirmed_persian_probe, set_probe
from text_frontend import native_tag_accepted, translate_text


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--probe', required=True, type=Path)
    parser.add_argument('--espeak-library', required=True, type=Path)
    parser.add_argument('--espeak-binary', required=True, type=Path)
    parser.add_argument('--data-parent', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    german = engine(args.probe, 0x40000)
    validator = engine(args.probe, 0x90000)
    speaker = LanguageEngine(args.probe, language=0x90000)
    audio = bytearray()
    items = 0
    try:
        for e in (german, validator, speaker):
            e.set_parameter(PROCESS_TAGS, 1)
            e.set_parameter(SAMPLE_RATE, 3)

        def append(samples: bytes) -> None:
            nonlocal items
            pcm = np.frombuffer(samples, dtype='<i2').astype(np.int32)
            if not len(pcm) or not np.any(pcm) or np.max(np.abs(pcm)) >= 32767:
                raise RuntimeError('empty or clipped audio sample')
            audio.extend(samples)
            audio.extend(bytes(RATE * 2 // 3))
            items += 1

        def label(message: str) -> None:
            set_probe()
            append(render(german, message))

        def native(api: EspeakPhonemeProbe, text: str, pinglish: bool = False) -> None:
            set_confirmed_persian_probe()
            parts = translate_text(api, text, pinglish=pinglish)
            queued = []
            current = 'fa'
            for part in parts:
                if part.language == 'fa':
                    for tag in re.findall(r'`\[[^]]+\]', part.text):
                        if not native_tag_accepted(tag, validator._phonemes_for_probe(tag)):
                            raise RuntimeError(f'native probe dropped a phone: {tag}')
                if part.language != current:
                    queued.append(EciLanguage(0x90000 if part.language == 'fa' else 0x10000, '1'))
                    current = part.language
                queued.append(part.text)
            pcm = bytearray()
            if not speaker.speak(queued, pcm.extend, lambda _index: None):
                raise RuntimeError('native language-switch sample cancelled')
            append(bytes(pcm))

        def reference(text: str, pinglish: bool = False) -> None:
            with tempfile.TemporaryDirectory(prefix='reloq-fa-reference-') as folder:
                path = Path(folder) / 'reference.wav'
                env = dict(os.environ, ESPEAK_DATA_PATH=str(args.data_parent.resolve()))
                subprocess.run([str(args.espeak_binary.resolve()), '-v',
                                'fa-latn' if pinglish else 'fa', '-w', str(path), text],
                               check=True, env=env, capture_output=True)
                append(reference_audio(path))

        label('Persische Textprobe. Die Beispiele werden aus dem Text erzeugt. '
              'Die Stimme ist noch ein Versuch und noch kein NVDA Add-on.')
        with EspeakPhonemeProbe(args.espeak_library, args.data_parent) as api:
            for title, text, ref, pinglish in (
                ('Saa at. Mit Stimmritzenverschluss.', 'ساعت', True, False),
                ('Dah, Dawazdah und Mehr. H am Wortende oder vor einem R.', 'ده ۱۲ مهر', True, False),
                ('Salam Donja, als Aussage.', 'سلام دنیا.', True, False),
                ('Salam Donja, als Frage.', 'سلام دنیا؟', False, False),
                ('Iran.', 'ایران', True, False),
                ('Nokte, Kalam und Kaghaz. Kaghaz enthält K und Zäpfchenlaut.',
                 'نقطه قلم کاغذ', True, False),
                ('Sefr und Chane.', 'صفر خانه', True, False),
                ('Persischer Satz mit dem englischen Namen NVDA.', 'من NVDA را دوست دارم.', False, False),
                ('Pinglisch. Eine getrennte lateinische Eingabe.', 'salam donya?', True, True),
            ):
                label(title)
                if ref:
                    label('Zuerst Espeak.')
                    reference(text, pinglish)
                label('Jetzt die Eloquence Versuchsstimme.')
                native(api, text, pinglish)
        label('Ende der Textprobe.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)
        german.close()
        validator.close()
        speaker.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as output:
        output.setnchannels(1)
        output.setsampwidth(2)
        output.setframerate(RATE)
        output.writeframes(audio)
    print(f'Wrote {items} clips, {len(audio) / (2 * RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
