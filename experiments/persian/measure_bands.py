"""Print how a recording's energy is spread over frequency bands, over time.

Used to compare the murmur of M and N with a reference voice: one row per
band, one column per 10 ms, in dB. Reads 16-bit mono WAV files.
"""
from __future__ import annotations

import argparse
import sys
import wave

import numpy as np

EDGES = (0, 700, 1500, 2500, 3500, 4500, 5500)


def bands(samples, rate, step=0.010, window=0.012):
    size, hop = int(rate * window), int(rate * step)
    frequencies = np.fft.rfftfreq(size, 1 / rate)
    rows = []
    for start in range(0, len(samples) - size, hop):
        power = np.abs(np.fft.rfft(samples[start:start + size] * np.hanning(size))) ** 2
        rows.append([10 * np.log10(power[(frequencies >= low) & (frequencies < high)].sum() + 1)
                     for low, high in zip(EDGES, EDGES[1:])])
    return np.array(rows)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('files', nargs='+')
    args = parser.parse_args()
    for name in args.files:
        with wave.open(name, 'rb') as source:
            if source.getsampwidth() != 2 or source.getnchannels() != 1:
                sys.exit(name + ': expected 16-bit mono')
            rate = source.getframerate()
            samples = np.frombuffer(source.readframes(source.getnframes()), dtype=np.int16).astype(float)
        # Scale every recording to the same peak so the columns compare.
        samples *= 20000 / max(1.0, np.abs(samples).max())
        table = bands(samples, rate)
        print(f'{name} ({rate} Hz, {len(samples) / rate:.2f} s)')
        for band, (low, high) in enumerate(zip(EDGES, EDGES[1:])):
            print(f'{low:5d}-{high:<5d}', ' '.join(f'{value:3.0f}' for value in table[:, band]))


if __name__ == '__main__':
    main()
