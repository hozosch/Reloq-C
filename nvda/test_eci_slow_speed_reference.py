"""The established slow voice keeps its fine control interpolation."""
import argparse
from array import array
import json
import math
from pathlib import Path
import sys
import tempfile
import wave

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from observe_high_speed_reference import capture


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library
    reference = json.loads((ROOT/'tests/data/slow_clock_before_dev3.json').read_text())
    size = reference['window_samples']
    with tempfile.TemporaryDirectory(prefix='reloq-slow-clock-') as folder:
        output = Path(folder)/'sample.wav'
        for case in reference['cases']:
            capture(library, False, case['text'], case['speed'], output,
                    want_phones=False, dialect=case['dialect'], gender=case['gender'])
            with wave.open(str(output), 'rb') as speech:
                assert speech.getframerate() == reference['sample_rate']
                samples = array('h', speech.readframes(speech.getnframes()))
                if sys.byteorder != 'little':
                    samples.byteswap()
            assert len(samples) == case['samples'], (case['text'], len(samples), case['samples'])
            actual = [math.sqrt(sum(v*v for v in samples[i:i+size])/size)
                      for i in range(0, len(samples)-size+1, size)]
            expected = case['rms_10ms']
            assert len(actual) == len(expected)
            difference = sum((a-b)**2 for a,b in zip(actual,expected))
            energy = sum(v*v for v in expected)
            error = math.sqrt(difference/energy)
            assert error < 0.01, (case['text'], case['speed'], case['gender'], error)
    print(f"Slow acoustic timing: {len(reference['cases'])} previous-voice envelopes match")


if __name__ == '__main__':
    main()
