"""Per-formant listening filters for the sound laboratory.

Eloquence's voiced formants form a serial filter bank, not five independent
amplitude controls.  The optional editor gains are therefore peaking filters
on the rendered probe, following the captured formant centres.  They do not
change the synthesizer or its acoustic rules.
"""
from __future__ import annotations

from array import array
from math import cos, isfinite, pi, sin
import sys

CENTRES = (9, 13, 15, 17, 19)
WIDTHS = (10, 14, 16, 18, 20)


def _coefficients(frequency, width, gain_db, rate):
    # A peaking biquad gives the chosen dB change at the formant centre.
    # A bounded Q avoids a nearly unstable filter at extreme voice settings.
    frequency = max(30.0, min(float(frequency), rate * .475))
    width = max(50.0, float(width))
    quality = max(.5, min(12.0, frequency / width))
    omega = 2.0 * pi * frequency / rate
    alpha = sin(omega) / (2.0 * quality)
    a = 10.0 ** (gain_db / 40.0)
    den = 1.0 + alpha / a
    middle = -2.0 * cos(omega)
    return ((1.0 + alpha * a) / den, middle / den,
            (1.0 - alpha * a) / den, middle / den,
            (1.0 - alpha / a) / den)


def apply(pcm, rows, gains, rate, start_ms=0, end_ms=0):
    """Return (PCM, adjusted-filter count, peak-normalized).

    Gains are relative dB changes, not native formant amplitudes.  Filter
    centre and width follow the current output frame.  A short fade at a
    selected interval's edges protects neighbouring phone transitions.
    """
    if len(pcm) % 2 or rate not in (8000, 11025, 16000, 22050, 32000, 44100, 48000):
        raise ValueError('Ungültige Audiodaten oder Abtastrate.')
    if len(gains) != 5 or any(type(x) is not int or not -18 <= x <= 18 for x in gains):
        raise ValueError('Der Resonanzpegel muss zwischen -18 und +18 dB liegen.')
    if not rows or not any(gains):
        return bytes(pcm), 0, False
    samples = array('h')
    samples.frombytes(bytes(pcm))
    if sys.byteorder != 'little':
        samples.byteswap()
    values = [float(x) for x in samples]
    states = [[0.0, 0.0, 0.0, 0.0] for _ in range(5)]
    active = set()
    elapsed_ms = 0.0
    cursor = 0
    for row in rows:
        duration = float(row.controls[0])
        if not isfinite(duration) or duration <= 0:
            raise ValueError('Ungültiger Zeitraum der Lautprobe.')
        next_ms = elapsed_ms + duration
        stop = min(len(values), max(cursor, round(next_ms * rate / 1000.0)))
        centre_ms = (elapsed_ms + next_ms) / 2.0
        weight = 1.0
        if start_ms:
            weight = min(weight, max(0.0, (centre_ms - start_ms) / 8.0))
        if end_ms:
            weight = min(weight, max(0.0, (end_ms - centre_ms) / 8.0))
        coeffs = []
        for i, gain in enumerate(gains):
            f, b = float(row.controls[CENTRES[i]]), float(row.controls[WIDTHS[i]])
            if gain and weight > 0 and isfinite(f) and isfinite(b) and 30 <= f < rate * .475:
                coeffs.append((i, _coefficients(f, b, gain * weight, rate)))
                active.add(i)
        for pos in range(cursor, stop):
            x = values[pos]
            for i, (b0,b1,b2,a1,a2) in coeffs:
                x1,x2,y1,y2 = states[i]
                y = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2
                states[i] = [x,x1,y,y1]
                x = y
            values[pos] = x
        cursor = stop
        elapsed_ms = next_ms
        if cursor >= len(values):
            break
    # Do not hard-clip a boosted listening probe.  Normalize only if a peak
    # really exceeds PCM16 range, leaving all other comparisons unchanged.
    peak = max((abs(x) for x in values), default=0.0)
    if not isfinite(peak):
        raise ValueError('Die Resonanzänderung konnte nicht stabil berechnet werden.')
    scaled = peak > 32760
    factor = 32760.0 / peak if scaled else 1.0
    output = array('h', (max(-32768, min(32767, round(x * factor))) for x in values))
    if sys.byteorder != 'little':
        output.byteswap()
    return output.tobytes(), len(active), scaled
