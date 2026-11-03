"""Check German native acoustic changes against the previous native build.

These measurements confirm signal and timing changes; they are not a
substitute for listening to the voice. All PCM remains in memory.
"""
import argparse
import array
import math
from pathlib import Path
from _reloq_eci_direct import DirectEciEngine,EciIndex,GENDER,PROCESS_TAGS,SAMPLE_RATE,SAMPLE_RATES


def render(engine,text):
    pcm=bytearray();indices=[]
    assert engine.speak([text,EciIndex(7)],pcm.extend,lambda n:indices.append((n,len(pcm))))
    assert pcm and indices==[(7,len(pcm))]
    return bytes(pcm),indices


def rms(pcm,rate,start=.025,end=.075):
    samples=array.array('h',pcm)
    window=samples[round(start*rate):round(end*rate)]
    return math.sqrt(sum(x*x for x in window)/len(window))


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    parser.add_argument('--baseline',type=Path,required=True)
    args=parser.parse_args()
    old,new=[DirectEciEngine(p.resolve(),dialect=0x40000,experimental_generic=True)
             for p in (args.baseline,args.library)]
    checks=0
    try:
        for engine in (old,new):engine.set_parameter(PROCESS_TAGS,1)
        for rate,hz in SAMPLE_RATES.items():
            for gender in (0,1):
                for engine in (old,new):
                    engine.set_parameter(SAMPLE_RATE,rate);engine.set_parameter(GENDER,gender)
                for text in ('Fisch.','Fee.','Wasser.','Vase.'):
                    before,after=render(old,text),render(new,text)
                    assert len(before[0])==len(after[0]) and before[1]==after[1],(rate,gender,text,'timing')
                    if hz<22050:assert before==after,(rate,gender,text,'unexpected low-rate change')
                    else:assert before!=after,(rate,gender,text,'missing gain')
                    checks+=1
                for text in ('Susi.','Katze.','Mann.'):
                    assert render(old,text)==render(new,text),(rate,gender,text,'unrelated')
                    checks+=1
                for text in ('Chance.','Restaurant.',"`[.1S'a~'.0s@]."):
                    before,after=render(old,text),render(new,text)
                    assert before!=after and len(before[0])==len(after[0]) and before[1]==after[1],(rate,gender,text)
                    checks+=1
                assert render(new,'Chance.')==render(new,"`[.1S'a~'.0s@].")
                checks+=1
                for text in ('Hör.', "`[.1h'oe'r]."):
                    before,after=render(old,text),render(new,text)
                    assert before!=after and len(before[0])==len(after[0]) and before[1]==after[1],(rate,gender,text)
                    checks+=1
                for text in ('Höhle.','Der Zahn.'):
                    assert render(old,text)==render(new,text),(rate,gender,text,'unrelated r')
                    checks+=1
                for text in ('Rot.', 'frisch.', '`[.1rot].'):
                    before,after=render(old,text),render(new,text)
                    assert before!=after and len(before[0])==len(after[0]) and before[1]==after[1],(rate,gender,text,'consonantal r')
                    checks+=1
                for text in ('lower.', '`[.1lo.0R].'):
                    before,after=render(old,text),render(new,text)
                    assert before!=after and len(before[0])==len(after[0]) and before[1]==after[1],(rate,gender,text,'reduced er')
                    checks+=1
                if hz>=22050 and not gender:
                    before,after=render(old,'Fee.')[0],render(new,'Fee.')[0]
                    ratio=rms(after,hz)/rms(before,hz)
                    assert ratio>1.05,(hz,ratio,'frication energy')
                    checks+=1
    finally:
        old.close();new.close()
    print(f'German native voice: {checks} source, nasal, unaffected controls, model, rate, timing and index checks; hearing comparison remains open')

if __name__=='__main__':main()
