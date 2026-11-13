"""Narrated comparison of US father A and a fricated Q release."""
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
from test_native_probe import IRAN as IRAN_FI
from test_refined_probe import IRAN_US, SETTINGS, settings


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    samples = {name: reference_audio(args.reference_dir / (name + '.wav'))
               for name in ('fa_iran', 'fa_qalam', 'fa_noqte')}
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    english = DirectEciEngine(args.library.resolve(), dialect=0x10000, encoding='utf8', native_voice=True)
    audio = bytearray()
    entries: list[str] = []
    try:
        for engine in (german, finnish, english):
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)
        for engine, parts in ((finnish, [IRAN_FI]), (english, [IRAN_US])):
            for part in parts:
                if part not in engine._phonemes_for_probe(part):
                    raise RuntimeError('Native phoneme tag rejected: ' + part)

        def append(name: str, pcm: bytes) -> None:
            if not pcm:
                raise RuntimeError('empty audio item: ' + name)
            audio.extend(pcm)
            audio.extend(bytes(RATE * 2 // 3))
            entries.append(name)

        def label(text: str) -> None:
            settings()
            append('label: ' + text, render(german, text))

        def ref(label_text: str, name: str) -> None:
            label(label_text)
            append(name, samples[name])

        def native(label_text: str, engine: DirectEciEngine, text: str, *,
                   q: int = 0, us_a: int = 0) -> None:
            label(label_text)
            settings(q=q, us_a=us_a)
            append(label_text, render(engine, text))
            settings()

        label('Persisch. Zweiter Hörvergleich. Erstens das lange A in Iran.')
        ref('Persisch, eSpeak.', 'fa_iran')
        native('Finnisch, bisherige Basis.', finnish, IRAN_FI)
        native('US Englisch, Wort father.', english, 'father')
        label('Nun Iran mit dem A aus father. Das R bleibt englisch.')
        native('US A, unverändert.', english, IRAN_US)
        native('US A, leicht geschlossen.', english, IRAN_US, us_a=1)
        native('US A, stärker geschlossen.', english, IRAN_US, us_a=2)

        label('Zweitens. Q im Wort für Stift. Die Varianten verwenden eine kurze Reibung.')
        ref('Persisch, eSpeak.', 'fa_qalam')
        native('Finnisches K.', finnish, 'kalam')
        native('Bisheriger Q Versuch.', finnish, 'qalam', q=1)
        native('Neue Variante eins, kurze Reibung.', finnish, 'qalam', q=3)
        native('Neue Variante zwei, längere Reibung.', finnish, 'qalam', q=4)

        label('Drittens. Q im Wort für Punkt.')
        ref('Persisch, eSpeak.', 'fa_noqte')
        native('Finnisches K.', finnish, 'nokte')
        native('Neue Variante eins, kurze Reibung.', finnish, 'noqte', q=3)
        native('Neue Variante zwei, längere Reibung.', finnish, 'noqte', q=4)

        label('Ende des Hörvergleichs.')
    finally:
        for key in SETTINGS:
            os.environ.pop(key, None)
        for engine in (german, finnish, english):
            engine.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as sink:
        sink.setnchannels(1)
        sink.setsampwidth(2)
        sink.setframerate(RATE)
        sink.writeframes(audio)
    print(f'Wrote {len(entries)} labelled items, {len(audio)//2/RATE:.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
