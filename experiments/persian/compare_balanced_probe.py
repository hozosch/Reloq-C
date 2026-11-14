"""Narrated comparison of Finnish/off A, shorter H, and Salam vowels."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from listening import RATE, reference_audio, render
from salam_samples import SALAM, SALAM_PREVIOUS, PERSIAN_PHRASE
from test_native_probe import IRAN
from test_selected_probe import KEYS, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    references = {name: reference_audio(args.reference_dir / f'{name}.wav')
                  for name in ('fa_iran', 'fa_statement', 'fa_question')}
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    audio = bytearray()
    count = 0
    try:
        for engine in (german, finnish):
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)
        for tag in (IRAN, SALAM, SALAM_PREVIOUS, '`[.1pa]', '`[.1ta]', '`[.1ka]', '`[.1do.0nja]'):
            if tag not in finnish._phonemes_for_probe(tag):
                raise RuntimeError('Native phoneme tag rejected: ' + tag)

        def append(pcm: bytes) -> None:
            nonlocal count
            if not pcm:
                raise RuntimeError('empty speech item')
            audio.extend(pcm)
            audio.extend(bytes(RATE * 2 // 3))
            count += 1

        def label(text: str) -> None:
            set_probe()
            append(render(german, text))

        def native(title: str, text: str, *, a: int = 0, h: int = 0, rise: int = 0) -> None:
            label(title)
            set_probe(a=a, h=h, rise=rise)
            append(render(finnish, text))

        def ref(title: str, name: str) -> None:
            label(title)
            append(references[name])

        label('Persisch. Hörvergleich für A, H und Salam. '
              'Die Beispiele verwenden einen finnischen Versuchskern.')
        label('Erstens. Das lange A in Iran.')
        ref('Persisch, eSpeak.', 'fa_iran')
        native('Finnisches A, unverändert.', IRAN)
        native('Bisherige Mischung aus father und off.', IRAN, a=3)
        native('Je zur Hälfte finnisches A und off.', IRAN, a=5)
        native('25 Prozent finnisches A, 75 Prozent off.', IRAN, a=6)

        label('Zweitens. Finnisches H nach P, T und K.')
        stops = '`[.1pa] `[.1ta] `[.1ka]'
        native('Bisher, 80 Prozent der H-Länge.', stops, a=5, h=2)
        native('Neu, 70 Prozent der H-Länge.', stops, a=5, h=3)

        label('Drittens. Das kurze A in Salam.')
        ref('Persisch, eSpeak. Salam Dunya.', 'fa_statement')
        native('Bisheriges Salam.', SALAM_PREVIOUS, a=5)
        native('Neu, erstes A wie finnisches Ä.', SALAM, a=5)
        native('Mit der bestätigten Satzmelodie. Aussage.', PERSIAN_PHRASE + '.', a=5, h=3, rise=2)
        native('Frage.', PERSIAN_PHRASE + '?', a=5, h=3, rise=2)
        ref('Persisch, eSpeak. Frage.', 'fa_question')
        label('Ende des Hörvergleichs.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)
        german.close()
        finnish.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote {count} labelled excerpts, {len(audio)/(2*RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
