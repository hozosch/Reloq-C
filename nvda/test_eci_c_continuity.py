"""Check accepted C audio against the preceding build using public ECI.

The intentional Greek S/T change begins above 16 kHz and has its own test.
Every compared PCM stream and the final source index remain in memory.
"""
import argparse
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from compare_eci_builds import TEXTS, capture
from _reloq_eci_direct import (DirectEciEngine, GENDER, HEAD_SIZE, BREATHINESS,
    HIGH_RATE_PROFILE, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES)
from _reloq_languages import LANGUAGES


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--library', type=Path, required=True)
    args = parser.parse_args(); checks = 0
    for language in LANGUAGES.values():
        engines = [DirectEciEngine(path.resolve(), dialect=language.id,
                                   encoding='utf8', experimental_generic=True)
                   for path in (args.baseline, args.library)]
        try:
            for engine in engines:
                engine.set_parameter(NATIVE_SPEED, 98)
                engine.set_parameter(HIGH_RATE_PROFILE, 3)
            for rate, hz in SAMPLE_RATES.items():
                if language.id == 0x120000 and hz > 16000: continue
                for gender, head, breath in ((0, 50, 0), (1, 29, 60)):
                    for engine in engines:
                        for parameter, value in ((SAMPLE_RATE, rate), (GENDER, gender),
                                                 (HEAD_SIZE, head), (BREATHINESS, breath)):
                            engine.set_parameter(parameter, value)
                    assert capture(engines[0], TEXTS[language.id]) == capture(engines[1], TEXTS[language.id]), (
                        language.locale, hz, gender)
                    checks += 1
        finally:
            for engine in engines: engine.close()
    print(f'Accepted C continuity: {checks} actual speech/PCM/index comparisons match; audio in RAM')


if __name__ == '__main__':
    main()
