"""Persian and Pinglish retain independent, effective acoustic clock choices."""
from __future__ import annotations
import argparse
from pathlib import Path
from unittest.mock import patch
from nvda_driver_probe import make_language_engine,PERSIAN_PROBE,PINGLISH_PROBE
from _reloq_languages import ACOUSTIC_CLOCK_BITS,FINNISH,US_ENGLISH
from _reloq_eci_direct import GENDER,NATIVE_SPEED,ORIGINAL_ACOUSTIC_CLOCK,SAMPLE_RATE
from _reloq_persian_frames import FrameReplay
import continuous_driver


def actual(engine,text):
    samples=bytearray();frames=[];fitted=[]
    original=FrameReplay.replay;fit=continuous_driver.fit_us_word_pitch
    def replay(self,voice,rows,marks,*args,**kwargs):
        frames.extend((int(r.tick),tuple(r.controls)) for r in rows)
        return original(self,voice,rows,marks,*args,**kwargs)
    def fitted_call(*args,**kwargs):
        result=fit(*args,**kwargs);fitted.append(result);return result
    with patch.object(FrameReplay,'replay',new=replay),patch.object(continuous_driver,'fit_us_word_pitch',new=fitted_call):
        assert engine.speak((text,),samples.extend,lambda _:None)
    assert samples and frames and any(fitted),'Persian US pitch planning missing'
    return bytes(samples),frames


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--library',required=True,type=Path);a=p.parse_args()
    dll=a.library.resolve();count=0
    fa=ACOUSTIC_CLOCK_BITS[PERSIAN_PROBE.id];ping=ACOUSTIC_CLOCK_BITS[PINGLISH_PROBE.id]
    assert fa and ping and fa!=ping and fa!=ACOUSTIC_CLOCK_BITS[FINNISH.id]
    for profile,text in ((PERSIAN_PROBE,'تغییر در پیکربندیِ زبان.'),(PINGLISH_PROBE,'salam donya.')):
        owner=make_language_engine(dll,dll,None,Path('/unavailable-espeak'),Path('/unavailable'),language=profile.id)
        try:
            for key,value in ((SAMPLE_RATE,3),(NATIVE_SPEED,65),(GENDER,0)):owner.set_parameter(key,value)
            owner.set_acoustic_clocks(0)
            fine,rows=actual(owner,text)
            native=owner._activate(owner._key(profile.id,'1'))._voice
            assert native._dll.eciGetParam(native._handle,ORIGINAL_ACOUSTIC_CLOCK)==0
            owner.set_acoustic_clocks(ACOUSTIC_CLOCK_BITS[profile.id])
            coarse,held=actual(owner,text)
            assert native._dll.eciGetParam(native._handle,ORIGINAL_ACOUSTIC_CLOCK)==1
            assert len(fine)==len(coarse) and fine!=coarse
            assert len(rows)==len(held)
            changed=[held[i][0] for i in range(1,len(held))
                     if any(abs(held[i][1][col]-held[i-1][1][col])>.001 for col in (1,9,13,15))]
            assert changed and all(t%5==0 for t in changed),(profile.locale,'not held on 5 ms grid')
            owner.set_acoustic_clocks(0)
            restored,_=actual(owner,text)
            assert restored==fine,(profile.locale,'clock did not restore')
            other=PINGLISH_PROBE if profile==PERSIAN_PROBE else PERSIAN_PROBE
            owner.set_acoustic_clocks(ACOUSTIC_CLOCK_BITS[other.id])
            assert native._dll.eciGetParam(native._handle,ORIGINAL_ACOUSTIC_CLOCK)==0
            count+=1
        finally:owner.close()
    owner=make_language_engine(dll,dll,None,Path('/unavailable-espeak'),Path('/unavailable'),language=PERSIAN_PROBE.id)
    try:
        for key,value in ((SAMPLE_RATE,3),(NATIVE_SPEED,65),(GENDER,0)):owner.set_parameter(key,value)
        samples=[]
        for mask in (0,fa,0):
            owner.set_acoustic_clocks(mask)
            pcm=bytearray()
            assert owner.speak(('سلام دنیا NVDA تغییر در پیکربندیِ زبان.',),pcm.extend,lambda _:None)
            samples.append(bytes(pcm))
            persian=owner._activate(owner._key(PERSIAN_PROBE.id,'1'))._voice
            english=owner._activate(owner._key(US_ENGLISH.id,'1'))
            assert persian._dll.eciGetParam(persian._handle,ORIGINAL_ACOUSTIC_CLOCK)==bool(mask)
            assert english._dll.eciGetParam(english._handle,ORIGINAL_ACOUSTIC_CLOCK)==0
        # A mixed-language transition may move by one 1-ms frame when the
        # native controls change. It must not change the speaking rate.
        assert abs(len(samples[0])-len(samples[1]))<=64 and len(samples[0])==len(samples[2])
        assert samples[0]!=samples[1] and samples[0]==samples[2],'mixed clock or restoration failed'
        count+=1
    finally:owner.close()
    print(f'Passed {count} Persian/Pinglish real-US-contour clock, restoration and isolation cases.')

if __name__=='__main__':main()
