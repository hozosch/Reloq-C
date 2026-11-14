"""Narrated comparison of the light L, 50% H and eSpeak-informed A."""
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
from salam_samples import SALAM, QALAM, PERSIAN_PHRASE
from test_native_probe import IRAN
from test_selected_probe import KEYS, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--swedish-reference', type=Path,
                        help='optional normal eSpeak Swedish jag WAV')
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    references = {name: reference_audio(args.reference_dir / f'{name}.wav')
                  for name in ('fa_iran', 'fa_statement', 'fa_qalam')}
    if args.swedish_reference:
        references['sv_jag'] = reference_audio(args.swedish_reference)
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    audio = bytearray()
    count = 0
    try:
        for engine in (german, finnish):
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)
        tags = (IRAN, SALAM, QALAM, '`[.1pA]', '`[.1tA]', '`[.1kA]',
                '`[.1lA]', '`[.1Al]', '`[.1lu]', '`[.1ol]')
        for tag in tags:
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

        def native(title: str, text: str, **flags: int) -> None:
            label(title)
            set_probe(**flags)
            append(render(finnish, text))

        def ref(title: str, name: str) -> None:
            label(title)
            append(references[name])

        label('Persisch. Vergleich für A, L und H im finnischen Versuchskern.')
        label('Erstens. Das lange A, jeweils in Iran und Salam.')
        ref('Persisch, eSpeak. Iran.', 'fa_iran')
        if args.swedish_reference:
            ref('Schwedisch, eSpeak. Jag.', 'sv_jag')
        native('Bisherige Mischung, finnisches A und off.', IRAN + ' ' + SALAM, a=5, l=1)
        native('Annäherung an drei Resonanzen von eSpeak.', IRAN + ' ' + SALAM, a=1, l=1)
        native('Neue Annäherung, zusätzlich mit der vierten Resonanz.', IRAN + ' ' + SALAM, a=7, l=1)

        label('Zweitens. Das L, mit den Ä Lauten in Salam und Kalam.')
        ref('Persisch, eSpeak. Salam Dunya.', 'fa_statement')
        native('Salam. Bisheriges finnisches L.', SALAM, a=5)
        native('Salam. Helleres L nach deutschem Vorbild.', SALAM, a=5, l=1)
        ref('Persisch, eSpeak. Kalam, das Wort für Stift.', 'fa_qalam')
        native('Kalam. Bisheriges finnisches L.', QALAM, q=6)
        native('Kalam. Helleres L nach deutschem Vorbild.', QALAM, q=6, l=1)
        syllables = '`[.1lA] `[.1Al] `[.1lu] `[.1ol]'
        native('Weitere Silben. Bisheriges L.', syllables)
        native('Dieselben Silben. Helleres L.', syllables, l=1)

        label('Drittens. P, T und K mit finnischem H.')
        stops = '`[.1pA] `[.1tA] `[.1kA]'
        native('Vorher, 70 Prozent der H Länge.', stops, h=3)
        native('Jetzt, 50 Prozent der H Länge.', stops, h=4)
        native('Salam Dunya als Frage, mit der bestätigten Fragemelodie.',
               PERSIAN_PHRASE + '?', a=5, h=4, l=1, rise=2)
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
