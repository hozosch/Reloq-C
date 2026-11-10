"""Compare independent ECI timing observations at higher native speeds."""
import argparse
import json
from pathlib import Path
import sys
import tempfile

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
    print(f"English speed: {len(data['cases'])} held-out original ECI sample counts match at 11.025 kHz")

if __name__=='__main__':main()
