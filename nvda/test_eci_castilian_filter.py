"""Actual Castilian S/T onsets retain A's PCM before voicing begins.

Compare the independently selected hearing profiles, including both models,
all extended clocks and real words. No inferred phoneme/SNR threshold is
reimplemented here, and all audio remains in RAM.
"""
import argparse
import array
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, GENDER, SAMPLE_RATE,
                              SAMPLE_RATES, HIGH_RATE_PROFILE)


def capture(engine, text, profile):
    engine.set_parameter(HIGH_RATE_PROFILE, profile)
    chunks = []
    assert engine.speak([text], chunks.append, lambda n: None)
    return b"".join(chunks)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library", type=Path, required=True)
    parser.add_argument("--baseline", type=Path)
    args = parser.parse_args()
    prefixes = peaks = identities = maximum = 0
    texts = ("seseo.", "teta.", "Seis sacerdotes están sentados en siete sitios.")
    for dialect in (0x20000, 0x20001) if args.baseline else (0x20000,):
        engine = DirectEciEngine(args.library.resolve(), dialect=dialect,
                                 encoding="utf8", experimental_generic=True)
        baseline = (DirectEciEngine(args.baseline.resolve(), dialect=dialect,
                    encoding="utf8", experimental_generic=True) if args.baseline else None)
        try:
            for gender in (0, 1):
                engine.set_parameter(GENDER, gender)
                if baseline: baseline.set_parameter(GENDER, gender)
                for rate in (3, 2, 4, 5, 6):
                    engine.set_parameter(SAMPLE_RATE, rate)
                    if baseline: baseline.set_parameter(SAMPLE_RATE, rate)
                    for text in texts:
                        a = capture(engine, text, 0)
                        b = capture(engine, text, 1)
                        if dialect == 0x20000:
                            samples = array.array("h", b)
                            peak = max(abs(x) for x in samples)
                            assert peak < 32767, (gender, rate, text, peak)
                            maximum = max(maximum, peak); peaks += 1
                            if text in texts[:2]:
                                reference = array.array("h", a)
                                first = next(i for i, x in enumerate(reference) if abs(x) > 8)
                                ms = 60 if text == texts[0] else 8
                                end = first + ms * (SAMPLE_RATES[rate] // 1000)
                                assert len(samples) >= end and len(reference) >= end
                                assert samples[:end] == reference[:end], (gender, rate, text)
                                prefixes += 1
                        if baseline:
                            profile = 0 if dialect == 0x20000 else 1
                            assert (a if not profile else b) == capture(baseline, text, profile), (
                                hex(dialect), gender, rate, text, "control changed")
                            identities += 1
        finally:
            engine.close()
            if baseline: baseline.close()
    print(f"Castilian S/T: {prefixes} real A-onset identities, {peaks} peak checks "
          f"(maximum {maximum}), {identities} unchanged A/Mexican controls; audio in RAM")


if __name__ == "__main__": main()
