"""Slow speech retains the original ECI acoustic timing."""
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
    reference = json.loads((ROOT/'tests/data/original_slow_clock.json').read_text())
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
            assert abs(len(samples) - case['samples']) <= 55, (case['text'], len(samples), case['samples'])
            actual = [math.sqrt(sum(v*v for v in samples[i:i+size])/size)
                      for i in range(0, len(samples)-size+1, size)]
            expected = case['rms_5ms']
            assert abs(len(actual) - len(expected)) <= 1
            count=min(len(actual),len(expected))
            difference = sum((a-b)**2 for a,b in zip(actual[:count],expected[:count]))
            energy = sum(v*v for v in expected[:count])
            error = math.sqrt(difference/energy)
            assert error < 0.07, (case['text'], case['speed'], case['gender'], error)
    print(f"Slow acoustic timing: {len(reference['cases'])} original speech envelopes match")


if __name__ == '__main__':
    main()
