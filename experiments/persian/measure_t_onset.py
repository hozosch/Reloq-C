"""Estimate the initial /t/ release-to-voicing delay in three audio probes.

These are rough waveform diagnostics, not electroglottographic measurements
of voice-onset time. Different voices and vowels affect the result; it is not
a calibration target for every Persian /t/ or other consonant.
"""
from __future__ import annotations

import argparse
from pathlib import Path
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from listening import RATE, reference_audio, render


def rough_onsets(pcm: bytes) -> tuple[int, int]:
    x = np.frombuffer(pcm, dtype='<i2').astype(np.float64)
    if len(x) < RATE // 5:
        raise ValueError('Audio is too short')
    peak = float(np.max(np.abs(x)))
    if peak == 0:
        raise ValueError('Silent audio')
    # Acoustic release: first 2-ms window with non-trivial sound energy.
    window = RATE * 2 // 1000
    release = next((i for i in range(0, len(x) - window, window)
                    if np.sqrt(np.mean(x[i:i + window] ** 2)) > peak * 0.005), None)
    if release is None:
        raise ValueError('No stop release found')
    # 20-ms windows at 5-ms steps. Require two successive, clearly periodic
    # windows to reduce false positives from the transient and frication.
    window = RATE * 20 // 1000
    step = RATE * 5 // 1000
    scores = []
    for i in range(0, min(len(x) - window, RATE // 2), step):
        z = x[i:i + window] - np.mean(x[i:i + window])
        rms = float(np.sqrt(np.mean(z * z)))
        if rms < peak * 0.10:
            scores.append((i, 0.0))
            continue
        correlations = [float(np.dot(z[:-lag], z[lag:]) /
                          (np.linalg.norm(z[:-lag]) * np.linalg.norm(z[lag:]) + 1e-12))
                        for lag in range(53, 190)]
        scores.append((i, max(correlations)))
    voice = next((i + window // 2 for (i, a), (_, b) in zip(scores, scores[1:])
                  if a >= 0.75 and b >= 0.75 and i + window // 2 > release), None)
    if voice is None:
        raise ValueError('No periodic vowel onset found')
    return round(release * 1000 / RATE), round(voice * 1000 / RATE)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    parser.add_argument('--reference-dir', required=True, type=Path)
    args = parser.parse_args()
    samples = {'Persian eSpeak': reference_audio(args.reference_dir / 'fa_ta.wav')}
    for label, dialect in (('Finnish ReLoq', 0x90000), ('German ReLoq', 0x40000)):
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect, encoding='utf8', native_voice=True)
        try:
            engine.set_parameter(PROCESS_TAGS, 1)
            engine.set_parameter(SAMPLE_RATE, 3)
            tag = '`[.1ta]'
            if tag not in engine._phonemes_for_probe(tag):
                raise RuntimeError('Native T probe tag rejected')
            samples[label] = render(engine, tag)
        finally:
            engine.close()
    for name, pcm in samples.items():
        release, vowel = rough_onsets(pcm)
        print(f'{name}: estimated release {release} ms, '
              f'periodic vowel {vowel} ms, difference about {vowel - release} ms')
    print('Approximate audio-only estimates for this one /ta/ context, not linguistic VOT measurements.')


if __name__ == '__main__':
    main()
