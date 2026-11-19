"""Exercise closed Persian phone groups across reuse, controls and Finnish scope."""
from __future__ import annotations
import argparse
from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'nvda'))
from _reloq_eci_direct import GENDER, NATIVE_PITCH, SAMPLE_RATE
from _reloq_persian_frames import FrameCapture
from test_us_articulation import voice


def render(engine,scope,tag,selected):
    old=scope(int(selected))
    try:
        item=FrameCapture(engine._dll).capture(engine,(tag,),phone_segments=True)
        assert item and item.pcm
        return item.pcm,item.frame_bytes,item.syllables
    finally:scope(old)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',required=True,type=Path)
    args=parser.parse_args()
    warmed,scope=voice(args.library,0x90000)
    try:
        tags=('`[.0sA.1lam]', '`[.0ke.1tab]', '`[.1men]', '`[.1sA] `[.1do]')
        checks=0
        for rate in (1,3,6):
            for gender in (0,1):
                for engine in (warmed,):
                    engine.set_parameter(SAMPLE_RATE,rate)
                    engine.set_parameter(GENDER,gender)
                    engine.set_parameter(NATIVE_PITCH,65)
                for tag in tags:
                    # A separately constructed voice gives every sample a
                    # freshly reset provider, independent of its predecessor.
                    fresh,fresh_scope=voice(args.library,0x90000)
                    try:
                        fresh.set_parameter(SAMPLE_RATE,rate);fresh.set_parameter(GENDER,gender)
                        fresh.set_parameter(NATIVE_PITCH,65)
                        expected=render(fresh,fresh_scope,tag,True)
                    finally:fresh.close()
                    for _ in range(2):
                        assert render(warmed,scope,tag,True)==expected,(rate,gender,tag,'reuse')
                    # A persistent voice tag takes the conservative reset
                    # path and cannot leak into a later closed phone group.
                    render(warmed,scope,'`vb80 `[.1mAn]',True)
                    assert render(warmed,scope,tag,True)==expected,(rate,gender,tag,'tag isolation')
                    # Switching out of the Persian scope and back must not
                    # preserve a prior phone or acoustic setting either.
                    render(warmed,scope,tag,False)
                    assert render(warmed,scope,tag,True)==expected,(rate,gender,tag,'scope isolation')
                    checks+=1
    finally:warmed.close()
    print(f'Passed {checks} repeated Persian provider, control and scope isolation cases.')


if __name__=='__main__':main()
