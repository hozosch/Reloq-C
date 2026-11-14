"""Narrated listening sample of the selected private Persian sound probes."""
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
from test_selected_probe import KEYS, set_probe
from test_question_rise import PHRASE


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    references = {name: reference_audio(args.reference_dir / f'{name}.wav')
                  for name in ('fa_iran', 'fa_qalam', 'fa_noqte', 'fa_statement',
                               'fa_question', 'fa_stop_sequence')}
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    spanish = DirectEciEngine(args.library.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    audio = bytearray()
    items = 0
    try:
        for engine in (german, finnish, spanish):
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)
        for engine, tag in ((finnish, IRAN), (finnish, '`[.1Ga]'), (german, '`[.1xa]'),
                            (finnish, '`[.1pa]'), (finnish, '`[.1ta]'), (finnish, '`[.1ka]'),
                            (finnish, '`[.0sa.1lam]'), (finnish, '`[.1do.0nja]')):
            if tag not in engine._phonemes_for_probe(tag):
                raise RuntimeError('Native phoneme tag rejected: ' + tag)

        def append(pcm: bytes) -> None:
            nonlocal items
            if not pcm:
                raise RuntimeError('empty audio item')
            audio.extend(pcm)
            audio.extend(bytes(RATE * 2 // 3))
            items += 1

        def label(text: str) -> None:
            set_probe()
            append(render(german, text))

        def native(title: str, engine: DirectEciEngine, text: str, *,
                   a: int = 0, q: int = 0, x: int = 0, rise: int = 0,
                   stops: int = 0) -> None:
            label(title)
            set_probe(a=a, q=q, x=x, rise=rise, stops=stops)
            append(render(engine, text))

        def ref(title: str, name: str) -> None:
            label(title)
            append(references[name])

        label('Persisch. Hörprobe der ausgewählten Laute. '
              'Das ist noch keine fertige persische Stimme.')
        label('Erstens. Das lange A in Iran.')
        ref('Persisch, eSpeak.', 'fa_iran')
        native('Finnische Basis, bisher.', finnish, IRAN)
        native('Finnische Basis, das neue A.', finnish, IRAN, a=2)

        label('Zweitens. Das Q im Wort für Stift.')
        ref('Persisch, eSpeak.', 'fa_qalam')
        native('Ausgewählte Variante, bisheriger Pegel.', finnish, 'qalam', a=2, q=4)
        native('Ausgewählte Variante, um drei Dezibel leiser.', finnish, 'qalam', a=2, q=5)

        label('Drittens. Q vor T im Wort für Punkt.')
        ref('Persisch, eSpeak.', 'fa_noqte')
        native('Finnische Versuchsbasis, mit eingefügter Q-Reibung.', finnish, 'noqte', a=2, q=5)

        label('Viertens. Ach-Laut mit folgendem A.')
        native('Deutsche Stimme.', german, '`[.1xa]')
        native('Finnische Basis mit übertragenem Ach-Laut und neuem A.',
               finnish, '`[.1Ga]', a=2, x=1)

        label('Fünftens. Behauchtes P, T und K vor einem Vokal.')
        ref('Persisch, eSpeak.', 'fa_stop_sequence')
        stops = '`[.1pa] `[.1ta] `[.1ka]'
        native('Finnische Basis, bisher.', finnish, stops, a=2)
        native('Finnische Basis mit Behauchung nach deutschem Vorbild.', finnish, stops, a=2, stops=1)

        label('Sechstens. Satzmelodie. Aussage und Frage bedeuten jeweils Hallo Welt.')
        ref('Persisch, eSpeak. Aussage.', 'fa_statement')
        ref('Persisch, eSpeak. Frage.', 'fa_question')
        native('Finnische Basis, Aussage.', finnish, PHRASE + '.', a=2, rise=1)
        native('Finnische Basis, bisherige Frage.', finnish, PHRASE + '?', a=2)
        native('Finnische Basis mit spanischem Frageanstieg.', finnish, PHRASE + '?', a=2, rise=1)
        native('Spanische Stimme zum Vergleich.', spanish, 'salam donya?')
        label('Ende der Hörprobe.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)
        german.close()
        finnish.close()
        spanish.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote {items} labelled excerpts, {len(audio) / (2 * RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
