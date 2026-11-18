"""Compare the reported word and sentence on the real US-contour path."""
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
    arrays=[np.frombuffer(p,dtype='<i2').astype(float) for p in samples]
    rms=[np.sqrt(np.mean(x*x)) for x in arrays]
    peaks=[np.max(np.abs(x)) for x in arrays]
    target=min(rms[0],*(r*.95*32767/p for r,p in zip(rms,peaks)))
    return [np.clip(np.rint(x*(target/r)),-32768,32767).astype('<i2').tobytes() for x,r in zip(arrays,rms)]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--previous',required=True,type=Path);p.add_argument('--candidate',required=True,type=Path)
    p.add_argument('--output',required=True,type=Path);a=p.parse_args()
    n=DirectEciEngine(a.candidate.resolve(),dialect=0x40000,encoding='utf8',native_voice=True)
    for k,v in ((SAMPLE_RATE,3),(NATIVE_SPEED,65),(NATIVE_PITCH,65),(GENDER,0)):n.set_parameter(k,v)
    data=bytearray(); silence=bytes(16000)
    def say(s):
        out=bytearray(); assert n.speak((s,),out.extend,lambda _:None) and out
        data.extend(out);data.extend(silence)
    try:
        say('Finales R im gemeldeten Satz. Alle persischen Proben verwenden die amerikanische Melodie.')
        say('Das erste Wort allein als Referenz.')
        data.extend(actual_us_render(a.candidate,'تغییر'));data.extend(silence)
        before=actual_us_render(a.previous,'تغییر در پیکربندیِ زبان')
        after=actual_us_render(a.candidate,'تغییر در پیکربندیِ زبان')
        assert before!=after
        before,after=levels((before,after))
        for label,sound in (('Der Satz bisher.',before),('Der Satz mit korrigiertem R.',after)):
            say(label);data.extend(sound);data.extend(silence)
        say('Ende des Vergleichs.')
    finally:n.close()
    write_wav(a.output,data)
    print('Wrote exact connected-R comparison:',a.output)

if __name__=='__main__':main()
