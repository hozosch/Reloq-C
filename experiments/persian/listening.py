"""Make a labelled Persian phoneme and intonation listening comparison.

The Persian and Arabic words from eSpeak are references; the ReLoq excerpts
are explicit phoneme probes in existing German and Finnish voices.  No
Persian spelling analysis or language module is implied by this experiment.
"""
from __future__ import annotations

import argparse
from math import gcd
from pathlib import Path
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE

RATE = 16000
VOWELS = (
    ('Ä, finnisch.', '`[.1pA]'),
    ('A, finnisch.', '`[.1pa]'),
    ('E, finnisch.', '`[.1pe]'),
    ('I, finnisch.', '`[.1pi]'),
    ('O, finnisch.', '`[.1po]'),
    ('U, finnisch.', '`[.1pu]'),
)
REFERENCES = ('fa_noqte', 'fa_qalam', 'ar_noqta', 'ar_qalam',
              'fa_statement', 'fa_question')


def reference_audio(path: Path) -> bytes:
    # eSpeak writes 22.05 kHz PCM. Use a real low-pass resampler, not an
    # unfiltered change of playback speed or nearest-neighbour conversion.
    import numpy as np
    from scipy.signal import resample_poly
    with wave.open(str(path), 'rb') as source:
        if source.getnchannels() != 1 or source.getsampwidth() != 2:
            raise ValueError(f'expected mono 16-bit PCM: {path}')
        rate = source.getframerate()
        data = np.frombuffer(source.readframes(source.getnframes()), dtype='<i2').astype(np.float64)
    if not len(data) or not np.any(data):
        raise ValueError(f'empty reference: {path}')
    if rate != RATE:
        common = gcd(rate, RATE)
        data = resample_poly(data, RATE // common, rate // common)
    # Match the reference clips to a moderate listening level without a
    # limiter, and cap peaks so no supplied eSpeak recording can clip.
    rms = float(np.sqrt(np.mean(data * data)))
    peak = float(np.max(np.abs(data)))
    gain = min(0.115 * 32767 / rms, 0.8 * 32767 / peak, 3.0)
    return np.clip(np.rint(data * gain), -32768, 32767).astype('<i2').tobytes()


def render(engine: DirectEciEngine, text: str) -> bytes:
    out = bytearray()
    if not engine.speak([text], out.extend, lambda _index: None) or not out:
        raise RuntimeError(f'empty speech for {text!r}')
    return bytes(out)


def generate(library: Path, references: Path, output: Path) -> list[dict]:
    samples = {name: reference_audio(references / (name + '.wav')) for name in REFERENCES}
    german = DirectEciEngine(library.resolve(), dialect=0x40000, encoding='utf8', native_voice=True)
    finnish = DirectEciEngine(library.resolve(), dialect=0x90000, encoding='utf8', native_voice=True)
    audio = bytearray()
    items = []
    try:
        for engine in (german, finnish):
            engine.set_parameter(SAMPLE_RATE, 3)
            engine.set_parameter(PROCESS_TAGS, 1)
        def append(name: str, pcm: bytes):
            if not pcm:
                raise RuntimeError('empty listening item: ' + name)
            audio.extend(pcm)
            audio.extend(bytes(RATE // 3 * 2))
            items.append({'name': name, 'samples': len(pcm) // 2})
        def label(text: str):
            append('label: ' + text, render(german, text))
        def native(text: str, engine: DirectEciEngine):
            # Every explicitly selected native phoneme must actually be
            # accepted; otherwise the frontend may just read the tag aloud.
            phones = engine._phonemes_for_probe(text)
            for group in text.split():
                if group.startswith('`[') and group.rstrip('.?') not in phones:
                    raise RuntimeError('native phoneme tag rejected: ' + group)
            return render(engine, text)
        def probe(label_text: str, engine: DirectEciEngine, text: str):
            label(label_text)
            append('ReLoq: ' + label_text, native(text, engine))
        def ref(label_text: str, name: str):
            label(label_text)
            append('eSpeak: ' + label_text, samples[name])

        label('Persisch. Hörvergleich für Laute und Satzmelodie. '
              'Die ReLoq-Proben sind Annäherungen, noch keine fertige persische Stimme.')
        label('Erstens. Das Wort für Punkt.')
        ref('Persisch, eSpeak.', 'fa_noqte')
        probe('Deutsch, Ach-Laut.', german, '`[.0nOx.1te]')
        probe('Deutsch, gewöhnliches K.', german, '`[.0nOk.1te]')
        ref('Arabisches Qaf, eSpeak.', 'ar_noqta')
        label('Zweitens. Das Wort für Stift.')
        ref('Persisch, eSpeak.', 'fa_qalam')
        probe('Finnisch, gewöhnliches G.', finnish, '`[.0gA.1lAm]')
        probe('Deutsch, R als Reibelaut.', german, '`[.0rE.1lEm]')
        ref('Arabisches Qaf, eSpeak.', 'ar_qalam')
        label('Drittens. Finnische Vokale, jeweils nach einem P.')
        for caption, tag in VOWELS:
            probe(caption, finnish, tag)
        label('Viertens. Satzmelodie. Der Satz bedeutet Hallo Welt.')
        ref('Persisch, eSpeak: Aussage.', 'fa_statement')
        ref('Persisch, eSpeak: Frage.', 'fa_question')
        for title, engine, phrase in (
            ('Finnische Basis', finnish, '`[.0sA.1lam] `[.0do.1nja]'),
            ('Deutsche Basis', german, '`[.0sE.1lam] `[.0do.1nja]'),
        ):
            probe(title + ', Aussage.', engine, phrase + '.')
            probe(title + ', Frage.', engine, phrase + '?')
    finally:
        german.close()
        finnish.close()
    output.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(output), 'wb') as sink:
        sink.setnchannels(1)
        sink.setsampwidth(2)
        sink.setframerate(RATE)
        sink.writeframes(audio)
    return items


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    parser.add_argument('--wav', required=True, type=Path)
    args = parser.parse_args()
    items = generate(args.library, args.reference_dir, args.wav)
    print(f'Wrote {len(items)} labelled items to {args.wav}')


if __name__ == '__main__':
    main()
