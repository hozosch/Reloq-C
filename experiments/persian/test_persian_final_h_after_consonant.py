"""Check coda quality and the native breath after a final H carrier."""
from __future__ import annotations
import argparse
from pathlib import Path
import statistics
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
from test_us_articulation import voice


def capture(engine, scope, tag, selected=True):
    previous = scope(int(selected))
    try:
        result = FrameCapture(engine._dll).capture(engine, (tag,), phone_segments=True)
        assert result and result.pcm and len(result.syllables) >= 4, tag
        return result
    finally:
        assert scope(previous) == int(selected)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', required=True, type=Path)
    args=parser.parse_args()
    engine,scope=voice(args.library,0x90000)
    checks=0
    try:
        for rate in (1,3,6):
            for gender,head in ((0,50),(1,50),(1,20)):
                for speed in (40,65,100):
                    for key,value in ((SAMPLE_RATE,rate),(GENDER,gender),(HEAD_SIZE,head),(NATIVE_SPEED,speed)):
                        engine.set_parameter(key,value)
                    for tag in ('`[.1solhw]', '`[.1sAlhw]', '`[.1somhw]', '`[.1sAthw]', '`[.1tArhw]',
                                '`[.1solhw] `[.1mAn]'):
                        result=capture(engine,scope,tag)
                        # In this finite set the first final-H carrier is the
                        # fourth phone, just after /l/, /t/ or /r/.
                        lo=result.syllables[2][1]/10000
                        hi=result.syllables[3][0]/10000
                        assert 10<=hi-lo<=130,(tag,rate,gender,speed,lo,hi)
                        rows=[row for row in result.frames() if lo+(hi-lo)*.35<=row.tick<=lo+(hi-lo)*.7]
                        assert rows,(tag,rate,gender,speed)
                        ah=statistics.median(row.controls[7] for row in rows)
                        assert ah>=20,(tag,rate,gender,head,speed,'H aspiration',ah)
                        if tag.startswith(('`[.1solh', '`[.1sAlh', '`[.1somh')):
                            early=[row.controls[2] for row in result.frames()
                                   if lo+(hi-lo)*.1<=row.tick<=lo+(hi-lo)*.3]
                            late=[row.controls[2] for row in result.frames()
                                  if lo+(hi-lo)*.8<=row.tick<=lo+(hi-lo)*.95]
                            assert early and late and statistics.median(early)>5 and statistics.median(late)<=5, (
                                tag,rate,gender,head,speed,'voiced coda release',early,late)
                        if tag in ('`[.1solhw]', '`[.1sAlhw]'):
                            reference=capture(engine,scope,tag.replace('hw]',']'))
                            # The H carrier must not make the core choose its
                            # medial L instead of the established final L.
                            def middle(sample):
                                begin,end=(n/10000 for n in sample.syllables[2])
                                return [row for row in sample.frames()
                                        if begin+(end-begin)*.3<=row.tick<=begin+(end-begin)*.7]
                            actual,expected=middle(result),middle(reference)
                            assert actual and expected
                            for control,tolerance in ((2,8),(9,60),(13,160),(15,100)):
                                a=statistics.median(row.controls[control] for row in actual)
                                b=statistics.median(row.controls[control] for row in expected)
                                assert abs(a-b)<=tolerance,(tag,rate,gender,head,speed,'final L',control,a,b)
                        checks+=1
    finally:engine.close()
    print(f'Passed {checks} final-H impulse checks after consonants across voices and rates.')


if __name__=='__main__':main()
