"""Exercise actual phoneme boundaries, replay and formant-local level filters."""
from __future__ import annotations
import argparse
from array import array
import importlib
import math
from pathlib import Path
import sys
import tempfile
import types

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'experiments/persian'))
from build_soundlab_nvda import entries


def package(folder):
    for name,data in entries().items():
        target=folder/name;target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes(data)
    module=types.ModuleType('soundlab_test');module.__path__=[str(folder/'globalPlugins/reloqSoundLab')]
    sys.modules[module.__name__]=module
    return (importlib.import_module('soundlab_test.phoneme_engine'),
            importlib.import_module('soundlab_test.phonemes'),
            importlib.import_module('soundlab_test.resonance'),
            importlib.import_module('soundlab_test._reloq_persian_frames'))


def spectral_amplitude(samples,freq,rate):
    samples=samples[rate//10:]
    real=sum(x*math.cos(2*math.pi*freq*i/rate) for i,x in enumerate(samples))
    imag=sum(x*math.sin(2*math.pi*freq*i/rate) for i,x in enumerate(samples))
    return math.hypot(real,imag)


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--dll',required=True,type=Path);a=ap.parse_args()
    with tempfile.TemporaryDirectory() as tmp:
        engine,phones,eq,frames=package(Path(tmp));checks=0
        # The level control is a genuine frequency-local peaking filter. It
        # must be bit-identical at zero and change the selected band without
        # multiplying the entire signal or clipping it.
        for rate in (8000,16000,48000):
            original=array('h',(round(3500*math.sin(2*math.pi*1000*i/rate)+
                                      3500*math.sin(2*math.pi*2500*i/rate)) for i in range(rate//2)))
            rows=(frames.NativeFrame*500)()
            for i,row in enumerate(rows):
                row.tick=i;row.controls[0]=1;row.controls[9]=500;row.controls[10]=100
                row.controls[13]=1000;row.controls[14]=200;row.controls[15]=2500;row.controls[16]=300
            pcm=original.tobytes();same,active,normalized=eq.apply(pcm,rows,[0]*5,rate)
            assert same==pcm and active==0 and not normalized
            for gain in (-6,6):
                result,active,normalized=eq.apply(pcm,rows,[0,gain,0,0,0],rate)
                samples=array('h');samples.frombytes(result)
                low=spectral_amplitude(samples,1000,rate)/spectral_amplitude(original,1000,rate)
                high=spectral_amplitude(samples,2500,rate)/spectral_amplitude(original,2500,rate)
                assert (.45<low<.55 if gain<0 else 1.9<low<2.1),(rate,gain,low)
                assert .8<high<1.2 and active==1 and not normalized,(rate,gain,high)
                checks+=1
        # Tracing is opt-in: it changes neither PCM, frames, nor the normal
        # syllable capture. The selected mode resets with each request.
        direct=importlib.import_module('soundlab_test._reloq_eci_direct')
        for language in ('Persisch','Deutsch','Englisch (USA)','Englisch (Großbritannien)','Japanisch'):
            p=engine.defaults();p['language']=language
            voice,scope=engine._voice(a.dll,p)
            previous=scope(int(language=='Persisch'))
            try:
                api=frames.FrameCapture(voice._dll)
                assert api._phone_trace(1)==-1,'phone trace allowed without capture'
                plain=api.capture(voice,('`[.1sa]',),audio=True)
                traced=api.capture(voice,('`[.1sa]',),audio=True,phone_segments=True)
                again=api.capture(voice,('`[.1sa]',),audio=True)
                assert plain.pcm==traced.pcm==again.pcm,(language,'trace changed PCM')
                assert plain.frame_bytes==traced.frame_bytes==again.frame_bytes,(language,'trace changed frames')
                assert plain.syllables==again.syllables and len(traced.syllables)>=3,(language,'trace leaked')
                checks+=1
            finally:scope(previous);voice.close()
        # Every offered entry must be a real isolated phone, not a spelled
        # name for an invalid phonetic tag. Includes H and affricate spans.
        for language,catalog in phones.PHONES.items():
            assert catalog and len({phone.key for phone in catalog})==len(catalog)
            for phone in catalog:
                p=engine.defaults();p.update(language=language,phone=phone.key)
                loaded=engine.load(a.dll,p);pcm,_=engine.render(a.dll,p,loaded)
                samples=array('h');samples.frombytes(pcm)
                assert 0<=loaded.start_ms<loaded.end_ms and len(pcm)>1000,(language,phone.key)
                assert max(map(abs,samples),default=0)>100,(language,phone.key,'silent')
                checks+=1
        for language,key in (('Persisch','i'),('Persisch','s'),('Persisch','p'),('Persisch','h'),
                             ('Persisch','C'),('Persisch','r_i'),('Persisch','i_r'),
                             ('Deutsch','u'),('Englisch (USA)','y'),('Japanisch','s')):
            p=engine.defaults();p.update(language=language,phone=key)
            loaded=engine.load(a.dll,p);plain,_=engine.render(a.dll,p,loaded)
            p['frequencies'][1]=200;shift,_=engine.render(a.dll,p,loaded)
            p['frequencies'][1]=0;p['gains_db'][1]=6;gain,_=engine.render(a.dll,p,loaded)
            assert len(plain)==len(shift)==len(gain) and plain!=shift and plain!=gain,(language,key)
            samples=array('h');samples.frombytes(gain)
            assert max(map(abs,samples),default=0)<=32760,(language,key,'clipped')
            p['context']=True;context,_=engine.render(a.dll,p,loaded)
            assert len(context)>len(plain),(language,key,'carrier context missing')
            checks+=1
        for gender in (0,1):
            for rate in (1,3,6):
                for clock in (0,1):
                    p=engine.defaults();p.update(gender=gender,rate=rate,clock=clock,phone='i')
                    loaded=engine.load(a.dll,p)
                    original,_=engine.render(a.dll,p,loaded,modified=False)
                    unchanged,_=engine.render(a.dll,p,loaded,modified=True)
                    assert original==unchanged,(gender,rate,clock,'zero edits changed sound')
                    checks+=1
        print(f'Passed {checks} actual phoneme, keyboard-editor sound and band-filter checks.')

if __name__=='__main__':main()
