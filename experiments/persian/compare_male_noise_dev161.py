"""A listening-only experiment: broaden a valid male sibilant resonance.

This deliberately changes a normal finite-width male band; it is not the
zero-width whistle fix and is never enabled in production.
"""
from __future__ import annotations
import argparse
import ctypes
from pathlib import Path
import sys
import wave

ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'experiments/persian'),str(ROOT/'nvda')]
from _reloq_eci_direct import DirectEciEngine,GENDER,HEAD_SIZE,NATIVE_PITCH,NATIVE_SPEED,SAMPLE_RATE
from _reloq_persian_frames import FrameCapture,FrameReplay,NativeFrame
from compare_fa_clarity import matched_levels


def render(library,dialect,text,head):
    voice=DirectEciEngine(library.resolve(),dialect=dialect,encoding='utf8',native_voice=True)
    for p,v in ((SAMPLE_RATE,6),(GENDER,0),(HEAD_SIZE,head),(NATIVE_SPEED,65),(NATIVE_PITCH,65)):
        voice.set_parameter(p,v)
    try:
        cap=FrameCapture(voice._dll).capture(voice,(text,),audio=False)
        if cap is None: raise RuntimeError('capture failed')
        variants=[]
        for variant in (0,1):
            rows=cap.frames(); changed=0
            for row in rows:
                c=row.controls
                if variant and c[8]>=30 and c[39]>0 and 0<c[48]<1500:
                    c[48]=1500;changed+=1
            if variant and not changed: raise RuntimeError('No valid male noise band found')
            data=bytearray()
            assert FrameReplay(voice._dll).replay(voice,rows,(),data.extend,lambda _:None) and data
            variants.append(bytes(data))
        if variants[0]==variants[1]: raise RuntimeError('Experiment did not change the sound')
        return matched_levels(variants)[0]
    finally:voice.close()


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--library',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    narrator=DirectEciEngine(a.library.resolve(),dialect=0x40000,encoding='utf8',native_voice=True)
    for k,v in ((SAMPLE_RATE,6),(GENDER,0),(HEAD_SIZE,50),(NATIVE_SPEED,65),(NATIVE_PITCH,65)):narrator.set_parameter(k,v)
    audio=bytearray();pause=bytes(48000)
    def say(text):
        d=bytearray();assert narrator.speak((text,),d.extend,lambda _:None) and d
        audio.extend(d);audio.extend(pause)
    try:
        say('Männlicher Hörversuch bei 48 Kilohertz. Das Pfeifen war hier nicht vorhanden. Der Versuch verändert eine vorhandene Rauschresonanz. Jeweils original, dann Versuch.')
        for label,dialect,head,text in (
            ('Deutsch, normale Kopfgröße',0x40000,50,'Sechs süße Sachen. Sie sieht die Straße.'),
            ('Deutsch, kleine Kopfgröße',0x40000,15,'Sechs süße Sachen. Sie sieht die Straße.'),
            ('Japanisch, normale Kopfgröße',0x80000,50,'すし。さしすせそ。'),
            ('Japanisch, kleine Kopfgröße',0x80000,15,'すし。さしすせそ。')):
            before,after=render(a.library,dialect,text,head);say(label+'.')
            for title,sound in (('Original.',before),('Versuch.',after)):
                say(title);audio.extend(sound);audio.extend(pause)
        say('Ende des Hörversuchs.')
    finally:narrator.close()
    a.output.parent.mkdir(parents=True,exist_ok=True)
    with wave.open(str(a.output),'wb') as out:
        out.setnchannels(1);out.setsampwidth(2);out.setframerate(48000);out.writeframes(audio)
    print('Wrote male-only listening experiment:',a.output)

if __name__=='__main__':main()
