"""Narrated listening comparison of later Persian acoustic candidates."""
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
from test_native_probe import IRAN
from test_question_rise import PHRASE
from test_selected_probe import KEYS, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    references = {name: reference_audio(args.reference_dir / f'{name}.wav')
                  for name in ('fa_iran', 'fa_stop_sequence', 'fa_question')}
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    spanish = DirectEciEngine(args.library.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    audio = bytearray()
    count = 0
    try:
        for engine in (german, finnish, spanish):
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)
        for tag in (IRAN, '`[.1pa]', '`[.1ta]', '`[.1ka]', '`[.1pha]',
                    '`[.1tha]', '`[.1kha]', '`[.0sa.1lam]', '`[.1do.0nja]'):
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

        def native(title: str, engine: DirectEciEngine, text: str, *,
                   a: int = 0, q: int = 0, x: int = 0, rise: int = 0,
                   stops: int = 0, h: int = 0) -> None:
            label(title)
            set_probe(a=a, q=q, x=x, rise=rise, stops=stops, h=h)
            append(render(engine, text))

        def ref(title: str, name: str) -> None:
            label(title)
            append(references[name])

        label('Persisch. Neuer Hörvergleich für A, H und die Fragemelodie. '
              'Die Beispiele sind Lautversuche in der finnischen Basis.')
        label('Erstens. Das lange A in Iran.')
        ref('Persisch, eSpeak.', 'fa_iran')
        native('Bisher, 40 Prozent father und 60 Prozent off.', finnish, IRAN, a=2)
        native('Neu, 20 Prozent father und 80 Prozent off.', finnish, IRAN, a=3)
        native('Zum Vergleich, 100 Prozent off.', finnish, IRAN, a=4)

        label('Zweitens. P, T und K. Die H-Laute sind einzeln in die Lautfolge eingesetzt.')
        ref('Persisch, eSpeak.', 'fa_stop_sequence')
        bare = '`[.1pa] `[.1ta] `[.1ka]'
        with_h = '`[.1pha] `[.1tha] `[.1kha]'
        native('Bisheriger Behauchungsversuch.', finnish, bare, a=3, stops=1)
        native('Finnisches H, volle Länge.', finnish, with_h, a=3)
        native('Finnisches H, leicht verkürzt auf 80 Prozent.', finnish, with_h, a=3, h=1)

        label('Drittens. Frageanstieg. Der finnische und der spanische Vergleich verwenden dieselbe Lautfolge.')
        ref('Persisch, eSpeak.', 'fa_question')
        native('Spanischer Klangkern.', spanish, PHRASE + '?')
        native('Bisheriger finnischer Versuch.', finnish, PHRASE + '?', a=3, rise=1)
        native('Neuer finnischer Versuch mit höherem Anstieg.', finnish, PHRASE + '?', a=3, rise=2)
        label('Ende des Hörvergleichs.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)
        for engine in (german, finnish, spanish):
            engine.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote {count} labelled excerpts, {len(audio)/(2*RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
