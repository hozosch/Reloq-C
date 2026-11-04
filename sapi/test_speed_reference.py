"""Compare SAPI rate mapping with direct ECI; audio remains in RAM."""
import argparse
import array
import json
from pathlib import Path
import sys
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, NATIVE_SPEED, NATIVE_PITCH, NATIVE_INFLECTION, NATIVE_VOLUME


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library', type=Path, required=True)
    p.add_argument('--report', type=Path, required=True)
    a = p.parse_args()
    report = json.loads(a.report.read_text())
    for language in (0x10000, 0x40000):
        e = DirectEciEngine(a.library.resolve(), dialect=language, encoding='utf8', native_voice=True)
        try:
            for key, value in ((NATIVE_PITCH,65),(NATIVE_INFLECTION,30),(NATIVE_VOLUME,100)):
                e.set_parameter(key,value)
            for row in report['cases']:
                if row['language'] != language:
                    continue
                expected = max(0,min(250,60+10*(row['sapi_rate']+row['xml_adjustment'])))
                if row['boost']:
                    expected = min(250,(8*expected+2)//5)
                assert row['native_speed'] == expected
                e.set_parameter(NATIVE_SPEED,expected)
                pcm=bytearray();assert e.speak(['Ein kurzer Test.' if language==0x40000 else 'A short test.'],pcm.extend,None)
                values=array.array('h',pcm);digest=1469598103934665603
                for value in values:
                    digest=((digest^(value&0xffff))*1099511628211)&0xffffffffffffffff
                assert (len(values),f'{digest:016x}')==(row['samples'],row['hash']),row
        finally:
            e.close()
    report['direct_native_pcm_matches']=len(report['cases'])
    a.report.write_text(json.dumps(report,indent=2)+'\n')
    print(f'SAPI reference speed: {len(report["cases"])} direct ECI PCM matches')


if __name__ == '__main__':
    main()
