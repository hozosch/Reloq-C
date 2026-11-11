"""Check short speech transitions against the original ECI timing."""
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
    data = json.loads((ROOT / 'tests/data/original_transition_clock.json').read_text(encoding='utf8'))
    assert data['sample_rate'] == 11025
    checks = 0
    with tempfile.TemporaryDirectory(prefix='reloq-transitions-') as folder:
        output = Path(folder) / 'sample.wav'
        for case in data['cases']:
            capture(library, False, case['text'], case['eci_rate'], output,
                    buffer_samples=1100, want_phones=False,
                    dialect=case['dialect'], gender=case['gender'], acoustic_clock=1)
            with wave.open(str(output), 'rb') as speech:
                samples = array('h', speech.readframes(speech.getnframes()))
                if sys.byteorder != 'little':
                    samples.byteswap()
            assert abs(len(samples) - case['samples']) <= 55, case
            size = data['window_samples']
            actual = [math.sqrt(sum(x*x for x in samples[i:i+size]) / size)
                      for i in range(0, len(samples)-size+1, size)]
            expected = case['rms_5ms']
            count = min(len(actual), len(expected))
            assert count and abs(len(actual)-len(expected)) <= 1, case
            energy = sum(x*x for x in expected[:count])
            error = math.sqrt(sum((a-b)**2 for a, b in zip(actual[:count], expected[:count])) / energy)
            assert error < .07, (case['text'], case['nvda_rate'], case['gender'], error)
            checks += 1
    print(f'Original speech transitions: {checks} five-millisecond envelopes match')


if __name__ == '__main__':
    main()
