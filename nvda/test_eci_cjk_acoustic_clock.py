"""Native voices keep the slow and accepted fast clocks at the right rates."""
import argparse
from array import array
import json
import math
from pathlib import Path

from test_eci_inline_queue import Engine

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library
    checks = 0
    for filename in ('slow_clock_cjk_5ms.json', 'fast_clock_cjk_dev3.json',
                     'slow_clock_cjk_wpm_5ms.json', 'fast_clock_cjk_wpm.json',
                     'fast_clock_cjk_transition.json',
                     'clock_cjk_wpm434_5ms.json', 'clock_cjk_wpm435_5ms.json',
                     'clock_western_wpm434_5ms.json', 'clock_western_wpm435_5ms.json',
                     'clock_western_vs96_5ms.json'):
        reference = json.loads((ROOT/'tests/data'/filename).read_text(encoding='utf8'))
        size = reference['window_samples']
        for case in reference['cases']:
            engine = Engine(library, case['language'])
            try:
                assert engine.lib.eciSetParam(engine.h,5,1)>=0
                engine.tags(1 if case.get('wpm') else 0)
                for k, value in enumerate((case['gender'],50,65,30,0,0,case['speed'],100)):
                    engine.voice(k,value)
                text=(f"`vwpm{case['wpm']} " if case.get('wpm') else '')+case['text']
                samples=array('h');samples.frombytes(engine.speak(text))
            finally:
                engine.close()
            assert len(samples) == case['samples'], (filename,case['language'],case['speed'])
            actual = [math.sqrt(sum(v*v for v in samples[i:i+size])/size)
                      for i in range(0,len(samples)-size+1,size)]
            expected=case['rms_10ms'];assert len(actual)==len(expected)
            error=math.sqrt(sum((a-b)**2 for a,b in zip(actual,expected))/sum(v*v for v in expected))
            assert error<0.01,(filename,hex(case['language']),case['speed'],case['gender'],error)
            checks+=1
    print(f'Acoustic clocks: {checks} slow, fast and adjacent speech envelopes match')


if __name__ == '__main__':
    main()
