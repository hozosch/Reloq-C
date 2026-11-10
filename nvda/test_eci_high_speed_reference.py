"""Compare independent ECI timing observations at higher native speeds."""
import argparse
import json
import math
from pathlib import Path
import sys
import tempfile
import wave
from array import array

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from observe_high_speed_reference import capture


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--library',type=Path,required=True);args=parser.parse_args()
    data=json.loads((ROOT/'tests/data/original_high_speed_enu.json').read_text())
    assert data['sample_rate']==11025
    with tempfile.TemporaryDirectory(prefix='reloq-speed-test-') as folder:
        output=Path(folder)/'sample.wav'
        for case in data['cases']:
            actual=capture(args.library,False,case['text'],case['eci_rate'],output)
            tolerance=max(55,round(case['samples']*.015))
            assert abs(actual['samples']-case['samples'])<=tolerance,(case,actual['samples'],tolerance)
        clock=json.loads((ROOT/'tests/data/original_acoustic_clock_enu.json').read_text())
        assert clock['sample_rate']==11025
        fast_cases=[case for case in clock['cases'] if case['eci_rate']>=106]
        for case in fast_cases:
            capture(args.library,False,case['text'],case['eci_rate'],output)
            with wave.open(str(output),'rb') as speech:
                assert speech.getframerate()==clock['sample_rate']
                samples=array('h',speech.readframes(speech.getnframes()))
                if sys.byteorder!='little':samples.byteswap()
            size=clock['window_samples']
            actual=[math.sqrt(sum(v*v for v in samples[i:i+size])/size)
                    for i in range(0,len(samples)-size+1,size)]
            reference=case['rms_10ms']
            assert abs(len(samples)-case['samples'])<=55,(case['text'],case['eci_rate'],len(samples))
            assert abs(len(actual)-len(reference))<=1
            n=min(len(actual),len(reference))
            difference=sum((actual[i]-reference[i])**2 for i in range(n))
            energy=sum(reference[i]**2 for i in range(n))
            error=math.sqrt(difference/energy)
            assert error<0.05,(case['text'],case['eci_rate'],error)
    print(f"English speed: {len(data['cases'])} held-out original ECI sample counts match at 11.025 kHz")
    print(f"Fast acoustic timing: {len(fast_cases)} original ECI energy envelopes match")

if __name__=='__main__':main()
