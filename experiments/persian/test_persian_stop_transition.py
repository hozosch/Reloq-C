"""Check stop-to-voiced language joins using captured, live native frames."""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'nvda'))
from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import NativeFrame
import continuous_driver
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',required=True,type=Path)
    p.add_argument('--transition-shim',type=Path)
    args=p.parse_args()
    native=ctypes.CDLL(str((args.transition_shim or args.library).resolve()))
    fn=(getattr(native,'eciPersianBridgeStopOnsets',None) or
        getattr(native,'reloq_persian_bridge_stop_onsets'))
    fn.argtypes=(ctypes.POINTER(NativeFrame),ctypes.POINTER(NativeFrame),
        ctypes.POINTER(ctypes.c_double),ctypes.POINTER(ctypes.c_size_t),
        ctypes.POINTER(ctypes.c_ubyte),ctypes.c_size_t,ctypes.c_size_t,
        ctypes.c_double,ctypes.POINTER(ctypes.c_ubyte))
    fn.restype=ctypes.c_int
    observed=[];original=continuous_driver.selected_sentence
    def observe(plan,parts,en,german,rate,**kwargs):
        plan._native_bridge_stop_onsets=fn
        edge=plan.counts[0]
        left=ctypes.string_at(plan.frames,edge*ctypes.sizeof(NativeFrame))
        onset=tuple(tuple(plan.frames[edge+j].controls[i] for i in (2,7,8)) for j in range(8))
        result=original(plan,parts,en,german,rate,**kwargs)
        after=tuple(tuple(plan.frames[edge+j].controls[i] for i in (2,7,8)) for j in range(8))
        assert ctypes.string_at(plan.frames,edge*ctypes.sizeof(NativeFrame))==left, 'English stop changed'
        observed.append((onset,after,parts))
        return result
    continuous_driver.selected_sentence=observe
    engine=make_language_engine(args.library,args.library,None,Path('/unavailable'),
                                Path('/unavailable'),language=PERSIAN_PROBE.id)
    checks=0
    try:
        for rate in (1,3,6):
            for gender in (0,1):
                for speed in (50,65,95):
                    for parameter,value in ((SAMPLE_RATE,rate),(GENDER,gender),(NATIVE_SPEED,speed)):
                        engine.set_parameter(parameter,value)
                    for text,changed in (('Persian text نقطه txt',True),
                                         ('Persian test نقطه txt',True),
                                         ('text نقطه txt',True),
                                         ('رفت now',True),
                                         ('Persian dog نقطه txt',False),
                                         ('Persian text هوا txt',False),
                                         ('رفت hello',False)):
                        observed.clear();pcm=bytearray();marks=[]
                        assert engine.speak((EciIndex(1),text,EciIndex(2)),pcm.extend,marks.append)
                        assert observed and pcm and marks==[1,2],(text,rate,gender,speed)
                        before,after,_=observed[0]
                        if changed:
                            first=next((i for i,frame in enumerate(before) if frame[0]>=20 and frame[2]==0),None)
                            assert first is not None and before[first][0]>after[first][0]>0 and \
                                   after[first][2]==before[first][2],(text,before,after)
                        else:
                            assert before[0]==after[0],(text,before,after)
                        checks+=1
    finally:
        continuous_driver.selected_sentence=original
        engine.close()
    print(f'Passed {checks} Persian stop-to-voiced joins, H onsets, controls and indexes.')

if __name__=='__main__':main()
