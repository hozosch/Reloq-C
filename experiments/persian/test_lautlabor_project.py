"""Verify the saved Persian sound project against real native speech."""
from __future__ import annotations
import argparse
from array import array
from concurrent.futures import ThreadPoolExecutor
import ctypes
import math
from pathlib import Path
import tempfile

from test_phoneme_editor import package
from test_us_articulation import capture, voice
from test_persian_acoustic_clock import actual
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE, PINGLISH_PROBE
from _reloq_languages import ACOUSTIC_CLOCK_BITS
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES
from _reloq_persian_frames import FrameCapture

# The listener's saved project. Frequency deltas are Hz and the band gains
# are the actual additional peaking filters used by the sound editor.
EDITS = {
    'A': ([25,-50,25,75,0], [0,0,0,-9,0]),
    'a': ([0,-25,50,25,0], [1,-1,-3,-4,0]),
    'e': ([0,0,-25,75,0], [0,0,0,0,0]),
    'o': ([0,0,50,425,0], [0,0,0,0,0]),
    'r_i': ([0,0,0,0,0], [0,0,-4,-2,0]),
    'u': ([0,0,25,50,25], [1,3,6,12,9]),
}
# The newer project was saved while the earlier project was active; these
# are therefore additional edits relative to that earlier voice.
BACK_A_REVISION = ([100,150,-75,-50,0], [1,1,-1,2,0])


def tuning(library):
    loader = ctypes.WinDLL if __import__('sys').platform == 'win32' else ctypes.CDLL
    dll = loader(str(library.resolve()))
    fn = dll.eciPersianTuningScope
    fn.argtypes, fn.restype = (ctypes.c_int,), ctypes.c_int
    return dll, fn


def audio_error(a, b):
    assert len(a) == len(b) and len(a) > 100
    x=array('h');x.frombytes(a);y=array('h');y.frombytes(b)
    energy=sum(v*v for v in x); other=sum(v*v for v in y)
    assert energy and other
    error=math.sqrt(sum((p-q)**2 for p,q in zip(x,y))/energy)
    corr=sum(p*q for p,q in zip(x,y))/math.sqrt(energy*other)
    return error,corr


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--library',required=True,type=Path)
    args=ap.parse_args();library=args.library.resolve();dll,switch=tuning(library);checks=0
    original=switch(1);assert original in (0,1)
    try:
        # A temporary bypass is request-thread scoped and restores correctly.
        assert switch(0)==1 and switch(0)==0 and switch(-1)==-1 and switch(3)==-1
        def another_thread():
            _, local=tuning(library)
            first=local(0);second=local(1)
            return first,second
        with ThreadPoolExecutor(max_workers=1) as pool:
            assert pool.submit(another_thread).result()==(1,0)
        assert switch(1)==0

        # Compare each chosen edited sound against the existing laboratory's
        # independently rendered reference, using the very same native DLL.
        # Both male and female models, several heads, rates and both clocks
        # are covered. The 16-kHz default reproduces the listener's project.
        with tempfile.TemporaryDirectory() as tmp:
            eng,_,_,_=package(Path(tmp))
            for gender in (0,1):
                for head in (15,50,85):
                    for rate in (1,3,6):
                        for clock in (0,1):
                            for phone,(frequencies,gains) in EDITS.items():
                                p=eng.defaults();p.update(language='Persisch',phone=phone,gender=gender,
                                    head=head,rate=rate,clock=clock,context=True,
                                    frequencies=list(frequencies),gains_db=list(gains))
                                previous_project=phone in ('a','u')
                                if phone=='a':
                                    p['frequencies'],p['gains_db']=map(list,BACK_A_REVISION)
                                assert switch(2 if previous_project else 0)==1
                                try:
                                    old=eng.load(library,p)
                                    try: expected, normalized=eng.render(library,p,old,modified=True)
                                    except ValueError as error:
                                        # The lab itself cannot filter a band beyond Nyquist.
                                        if 'außerhalb des hörbaren Bereichs' not in str(error): raise
                                        continue
                                finally:
                                    assert switch(1)==(2 if previous_project else 0)
                                new=eng.load(library,p);obtained,_=eng.render(library,p,new,modified=False)
                                error,corr=audio_error(expected,obtained)
                                if normalized:
                                    assert error<.15 and corr>.99,(phone,gender,head,rate,clock,error,corr)
                                elif phone=='r_i':
                                    # The accepted saved R filter must still
                                    # match in the R body. Its following I
                                    # now has an intentionally gentler F2
                                    # transition, so the full contextual
                                    # audio is allowed to differ there.
                                    start=old.start_ms+(old.end_ms-old.start_ms)*.1
                                    end=old.start_ms+(old.end_ms-old.start_ms)*.5
                                    first=round(start*SAMPLE_RATES[rate]/1000)*2
                                    last=round(end*SAMPLE_RATES[rate]/1000)*2
                                    body_error,body_corr=audio_error(expected[first:last],obtained[first:last])
                                    assert body_error<.03 and body_corr>.998,(phone,gender,head,rate,clock,body_error,body_corr)
                                    assert error<.09 and corr>.995,(phone,gender,head,rate,clock,error,corr)
                                else:
                                    assert error<.03 and corr>.998,(phone,gender,head,rate,clock,error,corr)
                                checks+=1

            # Reproduce both exact saved editor settings as well as the
            # connected and cross-voice cases above.
            for phone,edit in (('a',BACK_A_REVISION),('u',EDITS['u'])):
                p=eng.defaults();p.update(language='Persisch',phone=phone,gender=0,
                    head=50,rate=3,speed=65,pitch=65,clock=0,context=False,
                    frequencies=list(edit[0]),gains_db=list(edit[1]))
                assert switch(2)==1
                try:
                    old=eng.load(library,p);expected,normalized=eng.render(library,p,old,modified=True)
                finally:assert switch(1)==2
                new=eng.load(library,p);obtained,_=eng.render(library,p,new,modified=False)
                error,corr=audio_error(expected,obtained)
                assert error<(.15 if normalized else .03) and corr>(.99 if normalized else .998),(phone,error,corr)
                checks+=1

        # R is edited only for the actual R before I, never a final R or R
        # before another vowel. The neighbouring vowel remains its own phone.
        fa,scope=voice(library,0x90000)
        try:
            for tag,expected in (('`[.1ri]',True),('`[.1ra]',False),('`[.1ro]',False),('`[.1ir]',False)):
                previous=scope(1)
                try: item=FrameCapture(fa._dll).capture(fa,(tag,),phone_segments=True)
                finally:scope(previous)
                first=item.syllables[0][0]/10000
                end=item.syllables[1][0]/10000
                marker=any(first<=row.tick<end and round(row.controls[11])==16405 for row in item.frames())
                assert marker==expected,(tag,marker,expected)
                checks+=1
            for tag in ('`[.1i]',):
                assert switch(0)==1;before,_=capture(fa,scope,tag,True)
                assert switch(1)==0;after,_=capture(fa,scope,tag,True)
                assert before.pcm==after.pcm,(tag,'unedited sound changed')
                checks+=1
        finally:fa.close()

        # Original 5-ms timing and the US pitch contour survive the new
        # per-phone metadata, including both Persian and Pinglish profiles.
        for profile,text in ((PERSIAN_PROBE,'سلام دنیا.'),(PINGLISH_PROBE,'salam donya.')):
            owner=make_language_engine(library,library,None,Path('/unavailable-espeak'),
                                       Path('/unavailable'),language=profile.id)
            try:
                for key,value in ((SAMPLE_RATE,3),(NATIVE_SPEED,65),(GENDER,0),(HEAD_SIZE,50)):
                    owner.set_parameter(key,value)
                for clock in (0,1):
                    owner.set_acoustic_clocks(ACOUSTIC_CLOCK_BITS[profile.id] if clock else 0)
                    assert switch(0)==1;before,rows_before=actual(owner,text)
                    assert switch(1)==0;after,rows_after=actual(owner,text)
                    assert len(before)==len(after) and before!=after,(profile.locale,clock)
                    assert len(rows_before)==len(rows_after)
                    for (t0,c0),(t1,c1) in zip(rows_before,rows_after):
                        assert t0==t1 and abs(c0[1]-c1[1])<.01,(profile.locale,clock,t0,t1,c0[1],c1[1])
                    assert any(16401<=c[11]<=16405 and 0<c[12]<=1 for _,c in rows_after)
                    checks+=1
            finally:owner.close()
        print(f'Passed {checks} project-to-speech, R-context, voice, clock and US-contour checks.')
    finally:
        assert switch(original)>=0


if __name__=='__main__':main()
