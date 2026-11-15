"""Compare S noise, Salam-based A and explicit eSpeak vowel lengths."""
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
                           SALAM, TA_LONG, TA_SHORT)
from test_selected_probe import KEYS, set_probe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--previous', required=True, type=Path)
    parser.add_argument('--probe', required=True, type=Path)
    parser.add_argument('--target', required=True)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    german = DirectEciEngine(args.probe.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    old = DirectEciEngine(args.previous.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    new = DirectEciEngine(args.probe.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    audio = bytearray()
    clips = 0
    try:
        for engine in (german, old, new):
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

        def native(title: str, text: str, *, engine: DirectEciEngine = new,
                   s: int = 1, anchored: bool = True, q: int = 5) -> None:
            label(title)
            set_probe(a=1, q=q, x=1, h=5, l=4, rise=2, vowel=1, s=s,
                      a_target=args.target if anchored else '')
            phones = engine._phonemes_for_probe(text)
            for group in text.split():
                if group.startswith('`[') and group.rstrip('.?') not in phones:
                    raise RuntimeError('phoneme tag rejected: ' + group)
            append(render(engine, text))

        def reference(title: str, name: str) -> None:
            label(title)
            append(reference_audio(args.reference_dir / (name + '.wav')))

        label('Persisch. Hörvergleich für das S-Rauschen und die Vokale. '
              'Das ist eine Lautprobe, noch keine fertige NVDA Stimme.')
        label('Finnisch. Viisi, das Wort für fünf.')
        set_probe()
        append(render(new, 'viisi'))
        native('Salam. Letzter Versuch mit veränderten S Übergängen.', SALAM, engine=old, anchored=False)
        native('Salam. Neu: nur das S Rauschen angeglichen.', SALAM)
        native('S nach U. Bisherige Rauschwerte.', '`[.1us]', s=0)
        native('S nach U. Rauschen von Viisi.', '`[.1us]')
        label('Das zweite A von Salam bleibt die Klangvorlage.')
        reference('Iran. Espeak.', 'fa_iran')
        native('Iran. Frühere Länge und bisherige Klangfarbe.', IRAN_PREFERRED, engine=old, anchored=False)
        native('Iran. Frühere Länge, an Salam angenäherte Klangfarbe.', IRAN_PREFERRED)
        reference('Salam Donyâ. Espeak.', 'fa_statement')
        native('Donyâ. Bisherige Klangfarbe.', DONYA_ESPEAK, anchored=False)
        native('Donyâ. An Salam angenäherte Klangfarbe.', DONYA_ESPEAK)
        label('Finale Vokale.')
        reference('Nokte, Espeak.', 'fa_noqte')
        # Do not approximate the uvular cluster as Finnish /k/ here.  The
        # explicit test tag has no source-spelling q for the Q release hook.
        label('Lautfolge no-te, ohne Zäpfchenlaut.')
        native('Kurzes Schluss-E.', NO_TE_SHORT)
        native('Langes Schluss-E, wie in Espeaks Nokte.', NO_TE_LONG)
        reference('Ta, Espeak.', 'fa_ta')
        native('Ta. Kürzeres A.', TA_SHORT)
        native('Ta. Langes A wie in der Espeak-Lautfolge.', TA_LONG)
        native('Sefr. Das E innerhalb des Wortes.', '`[.1sefr]')
        label('Ende des Hörvergleichs.')
    finally:
        for key in KEYS:
            os.environ.pop(key, None)
        german.close()
        old.close()
        new.close()
    args.wav.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(args.wav), 'wb') as out:
        out.setnchannels(1)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(audio)
    print(f'Wrote {clips} spoken clips, {len(audio)/(2*RATE):.1f} seconds: {args.wav}')


if __name__ == '__main__':
    main()
