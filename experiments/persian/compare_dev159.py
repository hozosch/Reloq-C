"""Matched listening comparisons for the Persian vowel and whistle fix."""
from __future__ import annotations
import argparse
from pathlib import Path
import sys
import wave
import numpy as np
from scipy.signal import resample_poly

ROOT = Path(__file__).resolve().parents[2]
sys.path[:0] = [str(ROOT / 'experiments/persian'), str(ROOT / 'nvda')]
from _reloq_eci_direct import DirectEciEngine, GENDER, HEAD_SIZE, NATIVE_PITCH, NATIVE_SPEED, SAMPLE_RATE
from compare_fa_clarity import matched_levels, write_wav
from compare_persian_r_us_contour import actual_us_render


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--previous', required=True, type=Path)
    ap.add_argument('--candidate', required=True, type=Path)
    ap.add_argument('--output-dir', required=True, type=Path)
    a=ap.parse_args(); a.output_dir.mkdir(parents=True, exist_ok=True)
    def narrator(rate):
        voice=DirectEciEngine(a.candidate.resolve(),dialect=0x40000,encoding='utf8',native_voice=True)
        for k,v in ((SAMPLE_RATE,rate),(NATIVE_SPEED,65),(NATIVE_PITCH,65),(GENDER,0)):
            voice.set_parameter(k,v)
        return voice
    def speech(voice,text):
        data=bytearray(); assert voice.speak((text,),data.extend,lambda _:None) and data
        return data
    voice=narrator(3); audio=bytearray(); silence=bytes(16000)
    try:
        audio.extend(speech(voice,'Persische Vokale. Die Proben verwenden die amerikanische Melodie. Jeweils bisher und neu.'));audio.extend(silence)
        for label,text in (('Iran','ایران.'),('Ein persischer Satz','سلام دنیا. این یک آزمایش است.'),('Die Zahlen drei, zwei, eins','سه، دو، یک.')):
            before,after=actual_us_render(a.previous,text),actual_us_render(a.candidate,text)
            if before==after: raise ValueError('sample unchanged: '+text)
            (before,after),_=matched_levels((before,after))
            audio.extend(speech(voice,label+'.'));audio.extend(silence)
            for name,data in (('Bisher.',before),('Neu.',after)):
                audio.extend(speech(voice,name));audio.extend(silence);audio.extend(data);audio.extend(silence)
        audio.extend(speech(voice,'Ende des Vergleichs.'))
    finally: voice.close()
    write_wav(a.output_dir/'Persisch_Vokale_dev159.wav',audio)
    voice=narrator(6); audio=bytearray(); silence=bytes(48000)
    def render(lib,dialect,code,text):
        v=DirectEciEngine(lib.resolve(),dialect=dialect,encoding='utf8',native_voice=True)
        try:
            for k,value in ((SAMPLE_RATE,code),(GENDER,1),(HEAD_SIZE,15),(NATIVE_SPEED,65),(NATIVE_PITCH,65)):
                v.set_parameter(k,value)
            return bytes(speech(v,text))
        finally: v.close()
    try:
        audio.extend(speech(voice,'Pfeifvergleich, weibliche Stimme mit kleiner Kopfgröße. Jeweils bisher und neu.'));audio.extend(silence)
        for label,dialect,code,text in (('Deutsch, 22 Kilohertz',0x40000,2,'Sechs süße Sachen. Sie sieht die Straße.'),
                ('Deutsch, 48 Kilohertz',0x40000,6,'Sechs süße Sachen. Sie sieht die Straße.'),
                ('Japanisch, 22 Kilohertz',0x80000,2,'すし。さしすせそ。'),
                ('Japanisch, 48 Kilohertz',0x80000,6,'すし。さしすせそ。')):
            (before,after),_=matched_levels((render(a.previous,dialect,code,text),render(a.candidate,dialect,code,text)))
            audio.extend(speech(voice,label+'.'));audio.extend(silence)
            for name,data in (('Bisher.',before),('Neu.',after)):
                audio.extend(speech(voice,name));audio.extend(silence)
                if code==2:
                    data=np.clip(np.rint(resample_poly(np.frombuffer(data,dtype='<i2').astype(float),320,147)),-32768,32767).astype('<i2').tobytes()
                audio.extend(data);audio.extend(silence)
        audio.extend(speech(voice,'Ende des Vergleichs.'))
    finally: voice.close()
    with wave.open(str(a.output_dir/'Deutsch_Japanisch_Pfeifen_dev159.wav'),'wb') as out:
        out.setnchannels(1);out.setsampwidth(2);out.setframerate(48000);out.writeframes(audio)
    print('Wrote two narrated, level-matched comparisons.')

if __name__=='__main__':main()
