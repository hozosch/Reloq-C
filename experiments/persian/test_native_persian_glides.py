"""Compare the shared-C Persian U-to-A rule with the established planner."""
from __future__ import annotations
import argparse
import ctypes as C
from pathlib import Path
import random
from test_us_articulation import voice
from test_glide_planner import native_frames
from _reloq_persian_frames import FrameCapture, NativeFrame
from _reloq_persian_glide import apply_u_a_glides, plan_u_a_glides
from _reloq_eci_direct import EciIndex, SAMPLE_RATE, SAMPLE_RATES

class Candidate(C.Structure):
    _fields_=(('word',C.c_uint32),('left',C.c_int64),('boundary',C.c_int64),('right',C.c_int64))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',required=True,type=Path)
    p.add_argument('--shim',type=Path)
    args=p.parse_args()
    lib=C.CDLL(str((args.shim or args.library).resolve()))
    fn=lib.reloq_fa_apply_glides if args.shim else lib.eciPersianApplyGlides
    fn.argtypes=(C.POINTER(NativeFrame),C.c_size_t,C.POINTER(Candidate),C.c_size_t,C.c_int,C.POINTER(C.c_size_t))
    fn.restype=C.c_int
    tested=applied=0
    def compare(rows,ticks,tags,separators,boundaries,candidates):
        nonlocal tested,applied
        n=len(rows); originals=(NativeFrame*n)(*rows); native=(NativeFrame*n).from_buffer_copy(originals)
        old=[[float(c) for c in row.controls] for row in originals]
        glides=plan_u_a_glides(old,ticks,tags,separators,boundaries)
        apply_u_a_glides(old,ticks,glides)
        cc=(Candidate*len(candidates))(*(Candidate(*v) for v in candidates)) if candidates else None
        result=C.c_size_t(999)
        assert fn(native,n,cc,len(candidates),1,C.byref(result))==0
        assert result.value==len(glides),(result.value,len(glides))
        for i in range(n):
            for j in range(62):
                assert native[i].controls[j]==old[i][j],(i,j,native[i].controls[j],old[i][j])
        tested+=1;applied+=result.value
    tags=("`[.1'u:']", "`[.1'a:'.0mAd]")
    original,ticks=native_frames()
    rng=random.Random(176)
    for i in range(180):
        rows=(NativeFrame*len(ticks))()
        for j,tick in enumerate(ticks):
            rows[j].tick=tick
            for column,value in enumerate(original[j]): rows[j].controls[column]=value
            for column in (9,13):rows[j].controls[column]+=rng.randrange(-2,3)*.1
            if i%7==0 and 25<=j<50:rows[j].controls[2]=0
            if i%11==0 and 50<=j<70:rows[j].controls[8]=1
        compare(rows,ticks,tags,(' ',),{1:50},[(1,0,50,140)])
    eng,scope=voice(args.library,0x90000)
    try:
        for rate in (1,3,6):
            eng.set_parameter(SAMPLE_RATE,rate)
            for second in ("`[.1'a:'.0mAd]",'`[.0a.1mAd]','`[.1a]'):
                prior=scope(1)
                try: result=FrameCapture(eng._dll).capture(eng,(tags[0]+' ',EciIndex(9),second))
                finally: scope(prior)
                assert result and result.frames() and result.indexes
                rows=result.frames(); ticks=[r.tick for r in rows]
                boundary=round(dict(result.indexes)[9]*1000/SAMPLE_RATES[rate])
                if 0<boundary<=ticks[-1]:
                    compare(rows,ticks,(tags[0],second),(' ',),{1:boundary},
                            [(1,0,boundary,ticks[-1]+1)])
    finally:eng.close()
    assert applied>0
    print(f'Passed {tested} exact native/Python frame comparisons, including {applied} applied Persian glides.')

if __name__=='__main__':main()
