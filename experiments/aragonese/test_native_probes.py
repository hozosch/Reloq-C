"""Compare private Aragonese acoustic probes with the unmodified core."""
import argparse
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, GENDER, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE
from test_eci_german_clock_native import render


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', type=Path, required=True)
    parser.add_argument('--native-sh', type=Path)
    parser.add_argument('--rounded-au', type=Path)
    args = parser.parse_args()
    if not args.native_sh and not args.rounded_au:
        parser.error('supply at least one probe')
    baseline = DirectEciEngine(args.baseline.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
    native = (DirectEciEngine(args.native_sh.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
              if args.native_sh else None)
    rounded = (DirectEciEngine(args.rounded_au.resolve(), dialect=0x20000, encoding='utf8', native_voice=True)
               if args.rounded_au else None)
    engines = [baseline] + ([native] if native else []) + ([rounded] if rounded else [])
    checks = 0
    try:
        for engine in engines:
            engine.set_parameter(PROCESS_TAGS, 1)
        for rate in (1, 3, 6):
            for speed in (40, 80):
                for gender in (0, 1):
                    for engine in engines:
                        engine.set_parameter(SAMPLE_RATE, rate)
                        engine.set_parameter(NATIVE_SPEED, speed)
                        engine.set_parameter(GENDER, gender)
                    if native:
                        for text in ('`[.1Sa.0da]', '`[.1Swe.0la]', '`[.0a.1Si.0na]', '`[.1peS]'):
                            assert text in native._phonemes_for_probe(text), (rate, speed, gender, text)
                            assert render(native, [text])[0], (rate, speed, gender, text)
                            checks += 1
                        # Adding S in the private build must not rewrite other
                        # phonemes, including the Castilian interdental.
                        for text in ('`[.1Ta.0da]', '`[.1Ca.0da]', '`[.1a.0Ca]', '`[.1aC]', 'hola', 'casa'):
                            assert render(native, [text])[0] == render(baseline, [text])[0], (rate, speed, gender, text)
                            checks += 1
                    if rounded:
                        for word in ('auca', 'pauto', 'cudiau'):
                            assert baseline._phonemes_for_probe(word) == rounded._phonemes_for_probe(word)
                            before = render(baseline, [word])[0]
                            after = render(rounded, [word])[0]
                            assert len(before) == len(after) and before != after, (rate, speed, gender, word)
                            checks += 1
                        for word in ('puent', 'hola', 'casa', 'mocet'):
                            assert render(baseline, [word])[0] == render(rounded, [word])[0], (rate, speed, gender, word)
                            checks += 1
    finally:
        for engine in engines:
            engine.close()
    print(f'Isolated Aragonese native probes: {checks} rate, voice, speed and unchanged-parent checks passed')


if __name__ == '__main__':
    main()
