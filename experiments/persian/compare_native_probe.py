"""Create an accessible, narrated A and uvular K listening comparison."""
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


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    samples = {name: reference_audio(args.reference_dir / (name + '.wav'))
               for name in ('fa_iran', 'fa_qalam', 'fa_noqte', 'ar_qalam', 'ar_noqta')}
    german = DirectEciEngine(args.library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(args.library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    audio = bytearray()
    items = []
    try:
        for e in (german, finnish):
            e.set_parameter(SAMPLE_RATE, 3)
            e.set_parameter(PROCESS_TAGS, 1)
        phones = finnish._phonemes_for_probe(IRAN)
        if IRAN not in phones:
            raise RuntimeError('The Finnish Iran phoneme tag was rejected')

        def append(label: str, pcm: bytes) -> None:
            if not pcm:
                raise RuntimeError('empty listening item ' + label)
            audio.extend(pcm)
            audio.extend(bytes(RATE * 2 // 3))
            items.append(label)

        def label(text: str) -> None:
            append('label: ' + text, render(german, text))

        def reference(text: str, name: str) -> None:
            label(text)
            append(name, samples[name])

        def native(text: str, word: str, *, a: int = 0, q: int = 0) -> None:
            label(text)
            os.environ['RELOQ_FA_PROBE_A'] = str(a)
            os.environ['RELOQ_FA_PROBE_Q'] = str(q)
            append('Finnish ' + text, render(finnish, word))
            os.environ.pop('RELOQ_FA_PROBE_A', None)
            os.environ.pop('RELOQ_FA_PROBE_Q', None)

        label('Persisch. Hörvergleich für das lange A und das Zäpfchen K. '
              'Die Eloquence Proben sind Versuche mit der finnischen Basis, keine fertige persische Stimme.')
        label('Erstens. Das A in Iran.')
        reference('Persisch, eSpeak.', 'fa_iran')
        native('Finnische Basis.', IRAN)
        native('Finnische Basis, A anhand der eSpeak Daten verändert.', IRAN, a=1)
        label('Zweitens. Das Wort für Stift. Beachte den ersten Laut.')
        reference('Persisch, eSpeak.', 'fa_qalam')
        reference('Arabisch, eSpeak.', 'ar_qalam')
        native('Finnisches K.', 'kalam')
        native('Zäpfchen K, Variante eins.', 'qalam', q=1)
        native('Zäpfchen K, Variante zwei.', 'qalam', q=2)
        label('Drittens. Das Wort für Punkt. Beachte den Laut vor dem T.')
        reference('Persisch, eSpeak.', 'fa_noqte')
        reference('Arabisch, eSpeak.', 'ar_noqta')
        native('Finnisches K.', 'nokte')
        native('Zäpfchen K, Variante eins.', 'noqte', q=1)
        native('Zäpfchen K, Variante zwei.', 'noqte', q=2)
        label('Ende des Hörvergleichs.')
    finally:
        os.environ.pop('RELOQ_FA_PROBE_A', None)
        os.environ.pop('RELOQ_FA_PROBE_Q', None)
        german.close()
        finnish.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as sink:
        sink.setnchannels(1)
        sink.setsampwidth(2)
        sink.setframerate(RATE)
        sink.writeframes(audio)
    print(f'Wrote {len(items)} labelled items, {len(audio)//2/RATE:.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
