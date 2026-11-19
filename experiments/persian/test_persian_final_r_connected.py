"""Preserve the selected word-final R before a following Persian word."""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
from unittest.mock import patch

from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from _reloq_persian_frames import FrameReplay, FrameStream, NativeFrame
from nvda_driver_probe import make_language_engine, PERSIAN_PROBE
from direct_fa_sequence import project
import continuous_driver


def r_frames(rows):
    return [row for row in rows if row.controls[8] >= 40 and row.controls[35] < 5
            and row.controls[36] < 5 and row.controls[37] >= 60
            and 35 <= row.controls[38] < 55 and 15 <= row.controls[2] <= 30]


def actual_sentence(library, text):
    owner = make_language_engine(library, library, None, Path('/unavailable-espeak'), Path('/unavailable'))
    for key, value in ((SAMPLE_RATE,3),(NATIVE_SPEED,65),(NATIVE_PITCH,65),(GENDER,0)):
        owner.set_parameter(key,value)
    engine = owner._activate(owner._key(PERSIAN_PROBE.id,'1'))
    project((text,), engine._frontend, allow_fallback=False)
    traces, fits, audio = [], [], bytearray()
    original = FrameReplay.replay
    original_append = FrameStream.append
    fit = continuous_driver.fit_us_word_pitch
    def replay(self, voice, rows, marks, *args, **kwargs):
        traces.append(bytes(rows))
        return original(self, voice, rows, marks, *args, **kwargs)
    def fitted(*args, **kwargs):
        result=fit(*args, **kwargs); fits.append(result); return result
    def streamed(self, rows, first, last, marks=()):
        if first < last:
            traces.append(ctypes.string_at(ctypes.byref(rows, first * ctypes.sizeof(NativeFrame)),
                                           (last - first) * ctypes.sizeof(NativeFrame)))
        return original_append(self, rows, first, last, marks)
    try:
        with patch.object(FrameReplay,'replay',new=replay), patch.object(FrameStream,'append',new=streamed), \
             patch.object(continuous_driver,'fit_us_word_pitch',new=fitted):
            assert owner.speak((text,),audio.extend,lambda _:None)
    finally:
        owner.close()
    assert audio and traces and any(fits), (text,'missing US-contour replay')
    arrays=[(NativeFrame*(len(data)//ctypes.sizeof(NativeFrame))).from_buffer_copy(data) for data in traces]
    return [row for rows in arrays for row in rows]


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate',required=True,type=Path)
    a=ap.parse_args();v,scope=voice(a.candidate,0x90000); checks=0
    try:
        for gender in (0,1):
            for rate in (1,3,6):
                for speed in (40,98,170):
                    for k,x in ((GENDER,gender),(SAMPLE_RATE,rate),(NATIVE_SPEED,speed)):v.set_parameter(k,x)
                    for vowel in 'aeoiu':
                        tag='`[.1'+vowel+'r]'
                        alone=r_frames(capture(v,scope,tag,True)[1]); assert len(alone)>=5,(gender,rate,speed,tag)
                        base=alone[len(alone)//2].controls
                        for following in ('`[.1da]','`[.1pa]'):
                            connected=r_frames(capture(v,scope,tag+' '+following,True)[1])
                            assert len(connected)>=5,(gender,rate,speed,tag,following,'no final R')
                            actual=connected[len(connected)//2].controls
                            for col in (2,7,8,9,13,15,17,35,36,37,38):
                                assert abs(actual[col]-base[col])<=3,(gender,rate,speed,tag,following,col,actual[col],base[col])
                            checks+=1
        # User's original sentence, through the complete Persian text
        # projection and US-contour planner, not a raw Finnish preview.
        isolated=actual_sentence(a.candidate,'تغییر')
        connected=actual_sentence(a.candidate,'تغییر در پیکربندیِ زبان')
        solo=r_frames(isolated); phrase=r_frames(connected)
        assert len(solo)>=5 and len(phrase)>=5, 'user example: missing final R'
        assert phrase[0].tick<500, ('user example: first word R is missing',phrase[0].tick)
        print(f'Passed {checks} connected final-R contexts and the exact US-contour sentence.')
    finally:
        v.close()

if __name__=='__main__':main()
