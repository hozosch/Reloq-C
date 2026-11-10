"""Exercise the optional equalizer through the real multilingual ECI core."""
import argparse
import ctypes
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine, EciIndex, EQ_LOW, EQ_MID, EQ_HIGH, SAMPLE_RATE
from test_eci_german_clock_native import render


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    library=parser.parse_args().library.resolve()
    params=(EQ_LOW, EQ_MID, EQ_HIGH)
    checks=0
    for language,text in ((0x40000,'Das ist ein Klangtest.'),
                          (0x10000,'The voice stays clear.'),
                          (0x70000,'12 e 19')):
        engine=DirectEciEngine(library,dialect=language,encoding='utf8',native_voice=True)
        try:
            engine._dll.eciReset.argtypes=(ctypes.c_void_p,)
            engine._dll.eciReset.restype=ctypes.c_int
            for rate in (0,1,3,2,4,5,6):
                engine.set_parameter(SAMPLE_RATE,rate)
                for param in params: assert engine._dll.eciGetParam(engine._handle,param)==12
                flat=render(engine,[text,EciIndex(3)])
                engine.set_parameter(EQ_LOW,18)
                engine.set_parameter(EQ_MID,9)
                engine.set_parameter(EQ_HIGH,15)
                changed=render(engine,[text,EciIndex(3)])
                assert changed[0]!=flat[0] and changed[1]==flat[1],(language,rate)
                assert changed==render(engine,[text,EciIndex(3)]),(language,rate,'repeat')
                for param in params: engine.set_parameter(param,12)
                assert render(engine,[text,EciIndex(3)])==flat,(language,rate,'bypass')
                checks+=1
            assert engine._dll.eciSetParam(engine._handle,EQ_LOW,-1)==-1
            assert engine._dll.eciSetParam(engine._handle,EQ_HIGH,25)==-1
            engine.set_parameter(EQ_MID,0)
            assert engine._dll.eciReset(engine._handle)
            assert all(engine._dll.eciGetParam(engine._handle,p)==12 for p in params)
        finally:
            engine.close()
    print(f'Equalizer ECI: {checks} multilingual/rate checks, bypass, indices, repeat and reset passed')


if __name__=='__main__': main()
