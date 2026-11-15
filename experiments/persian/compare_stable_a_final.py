"""Make a short, narrated stable-A and word-final-duration comparison."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
import wave

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from listening import RATE, reference_audio, render
from salam_samples import (DONYA_ESPEAK, IRAN_PREFERRED, NO_TE_LONG, NO_TE_SHORT,
                           PERSIAN_PHRASE, SALAM, TA_LONG, TA_SHORT)
from test_selected_probe import KEYS, set_persian_probe, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    persian = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    audio = bytearray()
    clips = 0
    try:
        for engine in (german, persian):
            engine.set_parameter(PROCESS_TAGS, 1)
            engine.set_parameter(SAMPLE_RATE, 3)

        def append(pcm: bytes) -> None:
            nonlocal clips
            if not pcm or np.max(np.abs(np.frombuffer(pcm, dtype='<i2').astype(np.int32))) >= 32767:
                raise RuntimeError('empty or clipped sample')
            audio.extend(pcm)
            audio.extend(bytes(RATE * 2 // 3))
            clips += 1

        def label(text: str) -> None:
            set_probe()
            append(render(german, text))

        def native(title: str, text: str, *, final: int = 1, old_a: bool = False) -> None:
            label(title)
            if old_a:
                set_probe(a=1, q=5, x=1, h=5, l=4, rise=2, vowel=1, s=1,
                          a_target='656,1000,2620', final=final)
            else:
                set_persian_probe(final=final)
            phones = persian._phonemes_for_probe(text)
            for group in text.split():
                if group.startswith('`[') and group.rstrip('.?') not in phones:
                    raise RuntimeError('phoneme tag rejected: ' + group)
            append(render(persian, text))

        def reference(title: str, name: str) -> None:
            label(title)
            append(reference_audio(args.reference_dir / (name + '.wav')))

        label('Persisch. Hörvergleich für den A Klang und mittellange Endvokale. '
              'Das bestätigte S Rauschen ist in allen persischen Proben aktiv.')
        native('Salam. Das zweite A ist die Vorlage.', SALAM)
        reference('Iran. Espeak.', 'fa_iran')
        native('Iran. Bisherige A Angleichung.', IRAN_PREFERRED, final=0, old_a=True)
        native('Iran. Stabilerer A Klang, mit derselben Länge.', IRAN_PREFERRED, final=0)
        native('Donyâ. Bisherige A Angleichung.', DONYA_ESPEAK, final=0, old_a=True)
        native('Donyâ. Stabilerer A Klang, mit derselben Länge.', DONYA_ESPEAK, final=0)
        label('E am Wortende.')
        reference('Nokte. Espeak.', 'fa_noqte')
        label('Lautfolge no-te, ohne Zäpfchenlaut.')
        native('Kurz.', NO_TE_SHORT, final=0)
        native('Mittel.', NO_TE_SHORT)
        native('Lang.', NO_TE_LONG, final=0)
        label('A am Wortende.')
        reference('Ta. Espeak.', 'fa_ta')
        native('Kurz.', TA_SHORT, final=0)
        native('Mittel.', TA_SHORT)
        native('Lang.', TA_LONG, final=0)
        reference('Salam Donyâ. Espeak.', 'fa_statement')
        native('Unsere Probe, mit mittlerem Schluss-A.', PERSIAN_PHRASE + '.')
        native('Sefr. E im Wortinneren.', '`[.1sefr]')
        label('Ende des Hörvergleichs.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)
        german.close()
        persian.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote {clips} spoken clips, {len(audio)/(2*RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
