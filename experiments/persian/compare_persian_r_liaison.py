"""Audible before/after comparisons for Persian final-R liaison."""
from __future__ import annotations
import argparse
from pathlib import Path
import sys
import numpy as np

ROOT=Path(__file__).resolve().parents[2]
sys.path[:0]=[str(ROOT/'experiments/persian'),str(ROOT/'nvda')]
from _reloq_eci_direct import DirectEciEngine,GENDER,NATIVE_PITCH,NATIVE_SPEED,SAMPLE_RATE
from compare_persian_r_us_contour import actual_us_render
from compare_fa_clarity import write_wav


def levels(samples):
    arrays=[np.frombuffer(x,dtype='<i2').astype(float) for x in samples]
    rms=[np.sqrt(np.mean(x*x)) for x in arrays]
    peaks=[np.max(np.abs(x)) for x in arrays]
    if any(r<=0 for r in rms) or any(p>=32767 for p in peaks):raise ValueError('silent or clipped sample')
    target=min(rms[0],*(r*.95*32767/p for r,p in zip(rms,peaks)))
    return [np.clip(np.rint(x*target/r),-32768,32767).astype('<i2').tobytes() for x,r in zip(arrays,rms)]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--previous',required=True,type=Path);p.add_argument('--candidate',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    narrator=DirectEciEngine(a.candidate.resolve(),dialect=0x40000,encoding='utf8',native_voice=True)
    for k,v in ((SAMPLE_RATE,3),(NATIVE_SPEED,65),(NATIVE_PITCH,65),(GENDER,0)):narrator.set_parameter(k,v)
    data=bytearray();silence=bytes(16000)
    def say(text):
        out=bytearray();assert narrator.speak((text,),out.extend,lambda _:None) and out
        data.extend(out);data.extend(silence)
    try:
        say('Persisches R im Satz. Alle Proben verwenden die amerikanische Melodie.')
        say('Änderung vor ist, das folgende Wort beginnt mit Vokal. Jeweils bisher, dann neu.')
        for source in ('تغییر است','امر است'):
            old=actual_us_render(a.previous,source);new=actual_us_render(a.candidate,source)
            if old==new:raise ValueError('No difference for '+source)
            old,new=levels((old,new))
            if source.startswith('امر'):say('Jetzt R nach m, gefolgt von einem Vokal.')
            for label,sound in (('Bisher.',old),('Neu.',new)):
                say(label);data.extend(sound);data.extend(silence)
        say('Finales R vor einem Konsonanten, im bereits bekannten Satz.')
        data.extend(actual_us_render(a.candidate,'تغییر در پیکربندیِ زبان'));data.extend(silence)
        say('R nach f vor einem Vokal, im Wort für Null.')
        data.extend(actual_us_render(a.candidate,'صفر است'));data.extend(silence)
        for explanation, source in (
                ('R vor I am Wortanfang, im Wort Rial.', 'ریال'),
                ('R vor I im Wortinneren, im Namen Aria.', 'آریا'),
                ('Persisches U, im Wort für Freund.', 'دوست')):
            say(explanation + ' Jeweils bisher, dann neu.')
            old=actual_us_render(a.previous,source);new=actual_us_render(a.candidate,source)
            if old==new:raise ValueError('No difference for '+source)
            old,new=levels((old,new))
            for label,sound in (('Bisher.',old),('Neu.',new)):
                say(label);data.extend(sound);data.extend(silence)
        say('Zum Vergleich das deutsche U in gut. Gut.')
        say('Ende des Vergleichs.')
    finally:narrator.close()
    a.output.parent.mkdir(parents=True,exist_ok=True);write_wav(a.output,data)
    print('Wrote verified Persian R liaison comparison:',a.output)

if __name__=='__main__':main()
