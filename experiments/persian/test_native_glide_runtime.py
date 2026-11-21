"""Compare real Persian playback with the shared-C and previous glide rules."""
from __future__ import annotations
import argparse
import ctypes as C
from pathlib import Path
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, PINGLISH_PROBE
from _reloq_eci_direct import EciIndex, GENDER, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture, NativeFrame, NativeGlideCandidate


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',required=True,type=Path)
    p.add_argument('--shim',type=Path)
    args=p.parse_args()
    shim=C.CDLL(str(args.shim.resolve())) if args.shim else None
    engine=make_language_engine(args.library,args.library,None,None,None)
    original_init=FrameCapture.__init__
    enabled=[False];calls=[]
    def init(instance,dll):
        original_init(instance,dll)
        native=(shim.reloq_fa_apply_glides if shim else instance.apply_glides)
        if native is not None:
            native.argtypes=(C.POINTER(NativeFrame),C.c_size_t,C.POINTER(NativeGlideCandidate),
                             C.c_size_t,C.c_int,C.POINTER(C.c_size_t))
            native.restype=C.c_int
        if enabled[0] and native is not None:
            def observe(*args):
                result=native(*args);calls.append(result);return result
            instance.apply_glides=observe
        else:
            instance.apply_glides=None
    FrameCapture.__init__=init
    def render(text):
        pcm,indices=bytearray(),[]
        assert engine.speak((EciIndex(1),text,EciIndex(2)),pcm.extend,lambda x:indices.append((x,len(pcm))))
        assert pcm
        return bytes(pcm),tuple(indices)
    checks=0
    try:
        for lang,text in ((PERSIAN_PROBE.id,'او آمد.'),(PINGLISH_PROBE.id,'u aamad.')):
            engine.select_voice(lang,'1')
            for rate in (1,3,6):
                engine.set_parameter(SAMPLE_RATE,rate)
                for gender in (0,1):
                    engine.set_parameter(GENDER,gender);engine.set_parameter(NATIVE_SPEED,65)
                    enabled[0]=False;before=render(text)
                    enabled[0]=True;calls.clear();after=render(text)
                    assert calls and all(result==0 for result in calls),(lang,rate,gender,calls)
                    assert before==after,(lang,rate,gender)
                    checks+=1
    finally:
        FrameCapture.__init__=original_init
        engine.close()
    print(f'Passed {checks} native-glide runtime comparisons across languages, voices and rates.')

if __name__=='__main__':main()
