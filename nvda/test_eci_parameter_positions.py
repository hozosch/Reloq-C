"""Public ECI word-positioned controls and uninterrupted inline parameters.

PCM is compared in memory. There is no audio device or saved speech cache.
"""
from pathlib import Path
import argparse
import array
import ctypes
import hashlib
import math
from _reloq_eci_direct import (DirectEciEngine,PROCESS_TAGS,NATIVE_SPEED,NATIVE_VOLUME,SAMPLE_RATE)
from _reloq_languages import LANGUAGES


def capture(engine,parts):
    dll,handle=engine._dll,engine._handle
    for name in ('eciInsertRate','eciInsertVolume'):
        function=getattr(dll,name)
        function.argtypes=(ctypes.c_void_p,ctypes.c_int);function.restype=ctypes.c_int
    output=[];marks=[]
    engine._cancelled.clear();engine._callback_error=None
    engine._on_pcm=output.append
    engine._on_index=lambda n:marks.append((n,sum(map(len,output))))
    try:
        assert dll.eciClearInput(handle)==1
        for part in parts:
            if isinstance(part,str):
                text=part.encode('utf-8')
                assert dll.eciAddTextUtf8(handle,text,len(text),0)==1
            else:
                assert getattr(dll,part[0])(handle,part[1])==1,part
        assert dll.eciSynthesize(handle)==1,(dll.eciGetParam(handle,0x1000),dll.eciGetParam(handle,0x1002))
        assert dll.eciSynchronize(handle)==1
        if engine._callback_error:raise engine._callback_error
        return b''.join(output),marks
    finally:
        dll.eciClearInput(handle);engine._on_pcm=engine._on_index=None


def rms(data):
    samples=array.array('h',data)
    return math.sqrt(sum(s*s for s in samples)/max(1,len(samples)))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    args=parser.parse_args();checks=0
    index=lambda n:('eciInsertIndex',n)
    for language in LANGUAGES.values():
        engine=DirectEciEngine(args.library.resolve(),dialect=language.id,encoding='utf8',experimental_generic=True)
        first='Die erste Hälfte des Satzes. ' if language.id==0x40000 else 'The first half of the sentence. '
        try:
            engine.set_parameter(NATIVE_SPEED,50);engine.set_parameter(NATIVE_VOLUME,100)
            for rate in (1,6):
                engine.set_parameter(SAMPLE_RATE,rate)
                for tags in (0,1):
                    engine.set_parameter(PROCESS_TAGS,tags)
                    base=capture(engine,[first,index(1),first,index(2)])
                    assert len(base[1])==2 and base[1][-1]==(2,len(base[0]));checks+=1
                    for command in ('eciInsertRate','eciInsertVolume'):
                        setting=50 if command=='eciInsertRate' else 100
                        assert capture(engine,[first,index(1),(command,setting),first,index(2)])==base,(language.name,rate,tags,command,'unchanged value');checks+=1
                    if tags:
                        for tag in ('`vs50 ','`vv100 '):
                            assert capture(engine,[first,index(1),tag+first,index(2)])==base,(language.name,rate,tag,'added pause');checks+=1
                    for volume in (30,0):
                        pcm,marks=capture(engine,[first,index(1),('eciInsertVolume',volume),first,index(2)])
                        cut=base[1][0][1]
                        assert marks==base[1] and pcm[:cut]==base[0][:cut],(language.name,'earlier text changed');checks+=1
                        expected=(181.907 if volume==30 else 57.363)/2768.533
                        ratio=rms(pcm[cut:])/rms(base[0][cut:])
                        assert abs(ratio-expected)<.004,(language.name,volume,ratio,expected);checks+=1
                    pcm,marks=capture(engine,[first,index(1),('eciInsertRate',150),first,index(2)])
                    cut=base[1][0][1]
                    assert len(pcm)<len(base[0])*.8 and marks[0]==base[1][0],(language.name,'speed has no positional effect');checks+=1
                    assert pcm[:cut]==base[0][:cut] and marks[-1]==(2,len(pcm)),(language.name,'speed changed earlier text');checks+=1
                    assert engine._dll.eciGetParam(engine._handle,NATIVE_SPEED)==50
                    assert engine._dll.eciGetParam(engine._handle,NATIVE_VOLUME)==100;checks+=2
                    engine.set_parameter(NATIVE_VOLUME,30)
                    quiet=capture(engine,[first,index(1),first,index(2)])
                    engine.set_parameter(NATIVE_VOLUME,100)
                    assert quiet[1]==base[1] and abs(rms(quiet[0])/rms(base[0])-181.907/2768.533)<.004,(language.name,'configured volume ignored');checks+=1
                    # Explicit controls still work when user ECI tags are disabled.
                    # Literal caller tags retain their original text treatment.
                    if not tags:
                        text='Test `vs120 text.'
                        assert capture(engine,[text,index(9)])==capture(engine,[('eciInsertRate',50),('eciInsertVolume',100),text,index(9)]),(language.name,'tag enablement leaked');checks+=1
            # Invalid input is rejected without destroying already queued text.
            engine._dll.eciClearInput(engine._handle)
            assert engine._dll.eciAddText(engine._handle,b'test')==1
            assert engine._dll.eciInsertRate(engine._handle,251)==0
            assert engine._dll.eciInsertVolume(engine._handle,-1)==0
            engine._dll.eciClearInput(engine._handle);checks+=2
        finally:engine.close()
    # Fragmented words, explicit pitch, Unicode admission and language changes.
    engine=DirectEciEngine(args.library.resolve(),dialect=0x40000,encoding='utf8',experimental_generic=True)
    try:
        engine.set_parameter(PROCESS_TAGS,1);engine.set_parameter(NATIVE_SPEED,50)
        cases=[['Die Syn',('eciInsertRate',80),'these ist gut.'],
               ['Die Stimme liest ü β ',('eciInsertVolume',30),'und weiter.',index(1)],
               ['Die Stimme spricht. ',('eciInsertRate',80),'`l1.0 English speech. ',('eciInsertVolume',30),'`l4.0 Deutsch.',index(1)]]
        for parts in cases:
            expected=capture(engine,parts)
            assert expected[0] and capture(engine,parts)==expected;checks+=1
        unchanged=capture(engine,['Die Synthese ist gut.'])
        assert capture(engine,['Die Syn',('eciInsertVolume',100),'these ist gut.'])==unchanged;checks+=1
        assert capture(engine,['Die Syn',('eciInsertRate',50),'these ist gut.'])==unchanged;checks+=1
    finally:engine.close()
    print(f'ECI parameters: {checks} public ABI/PCM/index/inline/fragment/control checks across {len(LANGUAGES)} languages; no audio files')


if __name__=='__main__':main()
