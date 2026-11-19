"""Check Persian R after I before a vowel and before an Ach consonant."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
from statistics import median

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
from test_us_articulation import voice


VOWEL_CASES = ("`[.0ti.1r'e:']", '`[.0pi.1ra]', '`[.0si.1ri]')
X_CASES = ('`[.1zir.0GAt]', '`[.1tir.0GAt]')
UNCHANGED = ('`[.1ir]', '`[.1ri]', '`[.1i.0ro]',
             '`[.1pir] `[.1a]', '`[.1pir.0dAr]', '`[.1zar.0GAt]',
             '`[.1ir] `[.1GAt]', '`[.1i] `[.1rGAt]')


def capture(engine, scope, text, selected=True):
    previous=scope(int(selected))
    try:
        result=FrameCapture(engine._dll).capture(engine,(text,),phone_segments=True)
        assert result and result.pcm and len(result.syllables)>=2,text
        return result
    finally:
        assert scope(previous)==int(selected)


def middle(result,index,columns):
    start,end=(x/10000.0 for x in result.syllables[index])
    rows=[r for r in result.frames() if start+(end-start)*.35<=r.tick<=start+(end-start)*.65]
    assert rows,(index,start,end)
    return tuple(median(r.controls[col] for r in rows) for col in columns)


def at_fraction(result,index,fraction,column):
    start,end=(x/10000.0 for x in result.syllables[index])
    point=start+(end-start)*fraction
    return min(result.frames(),key=lambda r:abs(r.tick-point)).controls[column]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',required=True,type=Path)
    parser.add_argument('--previous',type=Path,help='optional immediate pre-change build')
    args=parser.parse_args()
    current,scope=voice(args.library,0x90000)
    old,old_scope=voice(args.previous,0x90000) if args.previous else (None,None)
    checks=0
    try:
        for rate in (1,3,6):
            for gender,head in ((0,50),(1,50),(1,20)):
                # 82 is NVDA's unboosted speed 38 (40 + 110 * .38). Test
                # both available acoustic clocks at this reported speed.
                for speed,clock in ((40,0),(65,0),(82,0),(82,1),(100,0)):
                    for v in (current,old):
                        if v:
                            for key,value in ((SAMPLE_RATE,rate),(GENDER,gender),(HEAD_SIZE,head),
                                              (NATIVE_SPEED,speed),(ORIGINAL_ACOUSTIC_CLOCK,clock)):
                                v.set_parameter(key,value)
                    for tag in VOWEL_CASES:
                        changed=capture(current,scope,tag)
                        native=capture(current,scope,tag,False)
                        assert len(changed.syllables)>=4 and len(native.syllables)>=4,tag
                        # Last phone segment is a pause, the preceding one is
                        # the vowel, and the phone before that is the medial r.
                        r=middle(changed,-3,(15,17));native_r=middle(native,-3,(15,17))
                        back_a = tag == '`[.0pi.1ra]'
                        if back_a:
                            # The newer R-to-back-A transition intentionally
                            # moves both upper R resonances towards the vowel.
                            # Check that improvement instead of imposing the
                            # old requirement that its F4 stay unchanged.
                            for index,column in enumerate((15,17)):
                                target=at_fraction(changed,-2,.05,column)
                                assert abs(r[index]-target)+50 <= abs(native_r[index]-target), (
                                    tag,rate,gender,head,speed,column,r,native_r,target)
                        else:
                            assert 60 <= r[0]-native_r[0] <= 350, (tag,rate,gender,head,speed,r,native_r)
                            # Disabling the full Persian scope also changes
                            # timing, so allow an adjacent-frame difference.
                            assert abs(r[1]-native_r[1])<=50, (tag,rate,gender,head,speed,'R F4 outside reference',r,native_r)
                        if old:
                            before=capture(old,old_scope,tag)
                            prior=middle(before,-3,(15,17))
                            assert changed.syllables==before.syllables, (tag,rate,gender,head,speed)
                            if back_a:
                                for index,column in enumerate((15,17)):
                                    target=at_fraction(changed,-2,.05,column)
                                    assert abs(r[index]-target)<=abs(prior[index]-target)+1, (
                                        tag,rate,gender,head,speed,column,r,prior,target)
                            else:
                                assert r[0]<prior[0] and abs(r[1]-prior[1])<=1, (
                                    tag,rate,gender,head,speed,r,prior)
                            assert middle(changed,-2,(9,13,15,17)) == middle(before,-2,(9,13,15,17)), (
                                tag,rate,gender,head,speed,'vowel nucleus changed')
                        checks+=1
                    for tag in X_CASES:
                        changed=capture(current,scope,tag)
                        assert len(changed.syllables)>=7,tag
                        # The trailing part of /r/ should no longer be pulled
                        # down toward /x/'s old low F3 onset before the noise.
                        assert at_fraction(changed,2,.9,15)>=2000,(tag,rate,gender,head,speed,'r F3 falls too low')
                        if old:
                            before=capture(old,old_scope,tag)
                            assert changed.syllables==before.syllables and changed.pcm!=before.pcm, (
                                tag,rate,gender,head,speed,'missing transition adjustment')
                            assert at_fraction(changed,2,.9,15)-at_fraction(before,2,.9,15)>=250, (
                                tag,rate,gender,head,speed,'r transition was not raised')
                            a,b=changed.frames(),before.frames()
                            assert len(a)==len(b)
                            for new,prior in zip(a,b):
                                assert new.tick==prior.tick
                                for col in (1,2,7,8,9,13,17,35,36,37,38,39,40):
                                    assert abs(new.controls[col]-prior.controls[col])<.01, (
                                        tag,rate,gender,head,speed,'unrelated source or formant changed',col)
                            assert middle(changed,4,(9,13,15,17))==middle(before,4,(9,13,15,17)), (
                                tag,rate,gender,head,speed,'following vowel nucleus changed')
                        checks+=1
                    if old:
                        for tag in UNCHANGED:
                            a=capture(current,scope,tag);b=capture(old,old_scope,tag)
                            assert a.pcm==b.pcm and a.frame_bytes==b.frame_bytes, (
                                tag,rate,gender,head,speed,'outside affected i-r contexts changed')
    finally:
        current.close()
        if old:old.close()
    print(f'Passed {checks} i-r-vowel and i-r-Ach resonance and context-isolation cases.')


if __name__=='__main__':main()
