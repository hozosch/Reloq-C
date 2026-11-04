"""Castilian S/T retains Classic Presence with a restrained optional upper band.

Observe real words and both models. The 6.5--7.5 kHz / 1.2--5 kHz spectral
ratio must remain below -20 dB; the 16-kHz foundation stays below -27 dB.
A pre-exception baseline can additionally verify its exact consonant onsets.
All audio remains in RAM; no analysis dependency beyond Python is required.
"""
import argparse
import array
import math
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, GENDER, SAMPLE_RATE,
                              SAMPLE_RATES, HIGH_RATE_PROFILE)


def capture(engine, text, profile):
    engine.set_parameter(HIGH_RATE_PROFILE, profile)
    chunks = []
    assert engine.speak([text], chunks.append, lambda n: None)
    return b"".join(chunks)


def onset(data, rate, milliseconds):
    samples = array.array("h", data)
    start = next(i for i, x in enumerate(samples) if abs(x) > 8)
    end = start + int(rate * milliseconds / 1000)
    assert len(samples) >= end
    return samples[:end], samples[start:end]


def band_power(samples, rate, low, high):
    window = [x * (.5 - .5 * math.cos(2 * math.pi * i / (len(samples) - 1)))
              for i, x in enumerate(samples)]
    total = 0
    for hz in range(low, high + 1, 100):
        coefficient = 2 * math.cos(2 * math.pi * hz / rate)
        previous = older = 0
        for x in window:
            current = x + coefficient * previous - older
            older, previous = previous, current
        total += previous * previous + older * older - coefficient * previous * older
    return total / ((high - low) // 100 + 1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--baseline", type=Path,
                        help="Previous build with accepted C as profile 3")
    args = parser.parse_args()
    spectra = peaks = identities = maximum = 0
    worst_ratio = 0
    texts = ("seseo.", "teta.", "Seis sacerdotes están sentados en siete sitios.")
    for dialect in (0x20000, 0x20001) if args.baseline else (0x20000,):
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect,
                                 encoding="utf8", experimental_generic=True)
        baseline = (DirectEciEngine(args.baseline.resolve(), dialect=dialect,
                    encoding="utf8", experimental_generic=True) if args.baseline else None)
        try:
            for gender in (0, 1):
                foundation_ratio = None
                engine.set_parameter(GENDER, gender)
                if baseline: baseline.set_parameter(GENDER, gender)
                for rate in (3, 2, 4, 5, 6):
                    hz = SAMPLE_RATES[rate]
                    engine.set_parameter(SAMPLE_RATE, rate)
                    if baseline: baseline.set_parameter(SAMPLE_RATE, rate)
                    for text in texts:
                        b = capture(engine, text, 3)
                        if dialect == 0x20000:
                            peak = max(abs(x) for x in array.array("h", b))
                            assert peak < 32767, (gender, rate, text, peak)
                            maximum = max(maximum, peak); peaks += 1
                            if text == texts[0]:
                                _, frication = onset(b, hz, 60)
                                ratio = (band_power(frication, hz, 6500, 7500) /
                                         band_power(frication, hz, 1200, 5000))
                                assert 0 < ratio < .01, (gender, hz, ratio)
                                if hz == 16000:
                                    assert ratio < .002, (gender, hz, ratio)
                                    foundation_ratio = ratio
                                else:
                                    assert ratio > foundation_ratio, (gender, hz, ratio)
                                worst_ratio = max(worst_ratio, ratio); spectra += 1
                        if baseline:
                            assert b == capture(baseline, text, 3), (
                                hex(dialect), gender, hz, text, "accepted C changed")
                            identities += 1
        finally:
            engine.close()
            if baseline: baseline.close()
    print(f"Castilian S/T: {spectra} upper-band bounds (worst {worst_ratio:.7f}), "
          f"{peaks} peak checks "
          f"(maximum {maximum}), {identities} accepted C identities; audio in RAM")


if __name__ == "__main__": main()
