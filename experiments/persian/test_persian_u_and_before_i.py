"""Check the German-shaped Persian U and contextual R before I."""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
from test_us_articulation import capture, voice
from test_persian_r_after_i import nucleus
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--candidate',required=True,type=Path)
    p.add_argument('--previous',type=Path)
    a=p.parse_args()
    fa,fs=voice(a.candidate,0x90000); de,ds=voice(a.candidate,0x40000)
    old,os=voice(a.previous,0x90000) if a.previous else (None,None)
    tuning=fa._dll.eciPersianTuningScope
    tuning.argtypes=(ctypes.c_int,);tuning.restype=ctypes.c_int

    def base_u(tag):
        # The saved U project is an additional editor step. Check the German
        # base with the prior project, then restore the actual voice for all
        # other tests. The new U project is tested independently against the
        # real sound editor in test_lautlabor_project.py.
        previous=tuning(2)
        assert previous>=0
        try:
            return capture(fa,fs,tag,True)
        finally:
            assert tuning(previous)==2
    count=0
    try:
        for gender in (0,1):
            for head in (0,15,50,85,100):
                for rate in (1,3,6):
                    for engine in (fa,de,old):
                        if engine:
                            for key,val in ((GENDER,gender),(HEAD_SIZE,head),(SAMPLE_RATE,rate),(NATIVE_SPEED,65)):
                                engine.set_parameter(key,val)
                    _,actual=base_u('`[.1u]')
                    _,reference=capture(de,ds,'`[.1u]',False)
                    left,right=nucleus(actual),nucleus(reference)
                    for col in (9,10,13,14,15,16,17,18,19,20):
                        assert abs(left[col]-right[col])<=3,(gender,head,rate,col,left[col],right[col])
                    for tag in ('`[.1pu]','`[.1tu]','`[.1mu]','`[.1nu]',
                                '`[.1ru]','`[.1su]','`[.1bu]'):
                        _,context=base_u(tag)
                        stable=[row for row in context if row.controls[2]>20 and
                                all(abs(row.controls[c]-right[c])<=3 for c in (9,13,15))]
                        assert len(stable)>=3,(gender,head,rate,tag,'no German U target')
                    if old:
                        for tag in ('`[.1ri]','`[.1ari]','`[.1iri]'):
                            _,new_rows=capture(fa,fs,tag,True)
                            _,old_rows=capture(old,os,tag,True)
                            assert len(new_rows)==len(old_rows),(gender,head,rate,tag,'duration changed')
                            diff=[n.controls[15]-o.controls[15] for n,o in zip(new_rows,old_rows)
                                  if n.controls[2]>10 and o.controls[2]>10]
                            if tag == '`[.1iri]':
                                # The accepted after-I correction already covers this R.
                                assert min(diff,default=0)>=-1,(gender,head,rate,tag,'existing R correction weakened')
                            else:
                                assert max(diff,default=0)>=100,(gender,head,rate,tag,'R F3 not raised')
                        for tag in ('`[.1ra]','`[.1ro]','`[.1i]'):
                            now=capture(fa,fs,tag,True)[0].pcm
                            before=capture(old,os,tag,True)[0].pcm
                            assert now==before,(gender,head,rate,tag,'unrelated phone changed')
                    count+=1
        print(f'Passed {count} German-U and R-before-I comparisons.')
    finally:
        fa.close();de.close()
        if old:old.close()

if __name__=='__main__':main()
