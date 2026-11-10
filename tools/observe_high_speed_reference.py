"""Capture matched ECI speech samples for private timing and spectral analysis.

The original DLL and voice data remain external; output consists only of short
speech examples and public ECI settings. Run in a process of matching bitness.
"""
import argparse
import ctypes as C
import json
import os
from pathlib import Path
import sys
import wave

TEXTS = ('adictive drums', 'addictive drums', 'six ships and fresh cups', 'hello friend')
SLIDERS = (40, 50, 60, 80, 100)


def capture(library, reference, text, speed, path):
    library=library.resolve();directory=os.add_dll_directory(str(library.parent)) if os.name=='nt' else None
    old=Path.cwd();os.chdir(library.parent)
    try:
        lib=(C.WinDLL if os.name=='nt' else C.CDLL)(str(library))
        H,I=C.c_void_p,C.c_int
        CB=(C.WINFUNCTYPE if os.name=='nt' else C.CFUNCTYPE)(I,H,I,I,H)
        for name,result,args in (
            ('eciNewEx',H,(I,)),('eciDelete',H,(H,)),('eciGetParam',I,(H,I)),
            ('eciSetParam',I,(H,I,I)),('eciCopyVoice',I,(H,I,I)),
            ('eciGetVoiceParam',I,(H,I,I)),('eciSetVoiceParam',I,(H,I,I,I)),
            ('eciRegisterCallback',None,(H,CB,H)),('eciSetOutputBuffer',I,(H,I,C.POINTER(C.c_short))),
            ('eciAddText',I,(H,C.c_char_p)),('eciSynthesize',I,(H,)),('eciSynchronize',I,(H,))):
            fn=getattr(lib,name);fn.restype,fn.argtypes=result,args
        engine=lib.eciNewEx(0x10000)
        if not engine:raise RuntimeError('Cannot create US English engine')
        chunks=[];events=[];samples=0;buffer=(C.c_short*110)()
        @CB
        def callback(handle,message,param,context):
            nonlocal samples
            if message==0:
                if not 0<=param<=len(buffer):return 2
                chunks.append(C.string_at(buffer,param*2));samples+=param
            elif message==3 and reference and C.sizeof(H)==4 and param:
                address=C.c_uint32(param).value
                name=C.string_at(address,5).split(b'\0',1)[0].decode('latin1',errors='replace')
                events.append({'sample':samples,'phoneme':name})
            return 1
        try:
            assert lib.eciCopyVoice(engine,1,0)
            for param,val in enumerate((0,50,65,30,0,0,speed,100)):
                if lib.eciSetVoiceParam(engine,0,param,val)<0:raise RuntimeError(f'voice parameter {param} refused')
            if not reference:
                assert lib.eciSetParam(engine,0x1003,1)>=0
            assert lib.eciSetParam(engine,5,1)>=0
            assert lib.eciSetParam(engine,1,1)>=0
            phones=lib.eciSetParam(engine,7,1) if reference else -1
            lib.eciRegisterCallback(engine,callback,None)
            assert lib.eciSetOutputBuffer(engine,len(buffer),buffer)>0
            assert lib.eciAddText(engine,text.encode('cp1252'))
            assert lib.eciSynthesize(engine)
            assert lib.eciSynchronize(engine)
            raw=b''.join(chunks)
            if not raw:raise RuntimeError('No speech samples captured')
            with wave.open(str(path),'wb') as out:
                out.setnchannels(1);out.setsampwidth(2);out.setframerate(11025);out.writeframes(raw)
            return dict(text=text,speed=speed,samples=samples,seconds=samples/11025,
                        phoneme_indices=phones,events=events,
                        voice=[lib.eciGetVoiceParam(engine,0,p) for p in range(8)])
        finally:lib.eciDelete(engine)
    finally:
        os.chdir(old)
        if directory:directory.close()


def main():
    p=argparse.ArgumentParser();p.add_argument('--library',type=Path,required=True)
    p.add_argument('--engine',choices=('original','reloq'),required=True);p.add_argument('--output',type=Path,required=True)
    args=p.parse_args();args.output.mkdir(parents=True,exist_ok=True);records=[]
    for text in TEXTS:
        slug=text.replace(' ','_')
        for slider in SLIDERS:
            original=40+(110*slider+50)//100
            ours=40+(116*slider+50)//100
            speeds=[('driver',original)] if args.engine=='original' else [('matched',original),('driver',ours)]
            for kind,speed in speeds:
                wav=args.output/f'{args.engine}-{kind}-{slider}-{slug}.wav'
                data=capture(args.library,args.engine=='original',text,speed,wav.resolve())
                data.update(slider=slider,kind=kind,file=wav.name);records.append(data)
                print(f"{args.engine} {kind} slider={slider} speed={speed} {text!r}: {data['seconds']:.3f}s, {len(data['events'])} phonemes",flush=True)
    (args.output/(args.engine+'.json')).write_text(json.dumps(records,indent=2)+'\n')

if __name__=='__main__':main()
