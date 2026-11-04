"""Greek uses the Castilian upper-band restraint for /s/ and /t/.

Use the public ECI stream with both voice models and all wide rates. The
16-kHz foundation stays identical to the prior build; at higher rates the
additional /s/ bandwidth is bounded rather than removed. Audio stays in RAM.
"""
import argparse
import array
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, GENDER, SAMPLE_RATE, SAMPLE_RATES
from test_eci_castilian_filter import band_power, capture, onset


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    parser.add_argument('--baseline', type=Path)
    args = parser.parse_args()
    engine = DirectEciEngine(args.library.resolve(), dialect=0x120000,
                             encoding='utf8', native_voice=True)
    old = (DirectEciEngine(args.baseline.resolve(), dialect=0x120000,
                           encoding='utf8', native_voice=True)
           if args.baseline else None)
    ratios = identities = 0
    try:
        for gender in (0, 1):
            for e in (engine, old):
                if e: e.set_parameter(GENDER, gender)
            foundation = None
            for rate in (3, 2, 4, 5, 6):
                hz = SAMPLE_RATES[rate]
                for e in (engine, old):
                    if e: e.set_parameter(SAMPLE_RATE, rate)
                for text in ('Σάσα.', 'Σήμερα.', 'τότε.'):
                    audio = capture(engine, text, 3)
                    assert max(map(abs, array.array('h', audio))) < 32767
                    if old and hz == 16000:
                        assert audio == capture(old, text, 3), (gender, hz, text)
                        identities += 1
                    if text == 'Σάσα.':
                        _, samples = onset(audio, hz, 60)
                        ratio = band_power(samples, hz, 6500, 7500) / band_power(samples, hz, 1200, 5000)
                        assert 0 < ratio < .01, (gender, hz, ratio)
                        if hz == 16000:
                            foundation = ratio
                        else:
                            assert ratio > foundation, (gender, hz, ratio, foundation)
                            if old:
                                _, previous = onset(capture(old, text, 3), hz, 60)
                                old_ratio = (band_power(previous, hz, 6500, 7500) /
                                             band_power(previous, hz, 1200, 5000))
                                assert .1 < ratio / old_ratio < .6, (gender, hz, ratio, old_ratio)
                        ratios += 1
    finally:
        engine.close()
        if old: old.close()
    print(f'Greek S/T: {ratios} spectral checks, {identities} unchanged 16-kHz outputs; audio in RAM')


if __name__ == '__main__':
    main()
