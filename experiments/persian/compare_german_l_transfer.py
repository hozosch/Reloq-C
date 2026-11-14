"""Narrated comparison of German L transferred to the Finnish probe."""
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

# German /E/ is a short front comparison vowel, Finnish /A/ is ä.
SALAM_DE = '`[.0sE.1lam]'
QALAM_DE = '`[.0kE.1lEm]'


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    refs = {n: reference_audio(args.reference_dir / f'{n}.wav')
            for n in ('fa_statement', 'fa_qalam')}
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    audio = bytearray()
    count = 0
    try:
        for engine in (german, finnish):
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)
        for engine, tags in ((german, (SALAM_DE, QALAM_DE)),
                             (finnish, (SALAM, QALAM, IRAN, '`[.1lA]', '`[.1Al]',
                                        '`[.1lu]', '`[.1ol]', '`[.1pA]', '`[.1tA]', '`[.1kA]'))):
            for tag in tags:
                if tag not in engine._phonemes_for_probe(tag):
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

        def speak(title: str, engine: DirectEciEngine, text: str, **flags: int) -> None:
            label(title)
            set_probe(**flags)
            append(render(engine, text))

        def ref(title: str, name: str) -> None:
            label(title)
            append(refs[name])

        label('Persisch. Vergleich des deutschen L mit der finnischen Versuchsstimme.')
        label('Erstens. Salam.')
        ref('eSpeak, Persisch. Salam Dunya.', 'fa_statement')
        speak('Deutscher Kern. L zum Vergleich.', german, SALAM_DE)
        speak('Finnischer Kern. Bisheriges finnisches L.', finnish, SALAM, a=1)
        speak('Finnischer Kern. Angepasste deutsche L Regel.', finnish, SALAM, a=1, l=2)

        label('Zweitens. Kalam, das Wort für Stift.')
        ref('eSpeak, Persisch.', 'fa_qalam')
        speak('Deutscher Kern. L zum Vergleich.', german, QALAM_DE)
        speak('Finnischer Kern. Bisheriges finnisches L.', finnish, QALAM, a=1, q=6)
        speak('Finnischer Kern. Angepasste deutsche L Regel.', finnish, QALAM, a=1, q=6, l=2)

        syllables = '`[.1lA] `[.1Al] `[.1lu] `[.1ol]'
        label('Drittens. L am Silbenanfang und Silbenende.')
        speak('Finnisches L.', finnish, syllables, a=1)
        speak('Angepasste deutsche L Regel.', finnish, syllables, a=1, l=2)

        stops = '`[.1pA] `[.1tA] `[.1kA]'
        label('Viertens. Der finnische H Impuls nach P, T und K.')
        speak('Vorher, 50 Prozent.', finnish, stops, a=1, h=4)
        speak('Jetzt, 60 Prozent.', finnish, stops, a=1, h=5)
        speak('Iran. Die ausgewählte A Variante.', finnish, IRAN, a=1, h=5, l=2)
        speak('Salam Dunya als Frage.', finnish, PERSIAN_PHRASE + '?', a=1, h=5, l=2, rise=2)
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
