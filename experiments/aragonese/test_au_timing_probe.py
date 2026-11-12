"""Regression checks for a private, position-limited AU timing experiment."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'nvda'))
from _reloq_eci_direct import DirectEciEngine,GENDER,NATIVE_SPEED,PROCESS_TAGS,SAMPLE_RATE
from test_eci_german_clock_native import render

PREFIX='RELOQ_ARAGON_PROBE_'


def set_controls(**values):
    for key,value in values.items():os.environ[PREFIX+key]=str(value)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference',required=True,type=Path)
    parser.add_argument('--timing',required=True,type=Path)
    parser.add_argument('--mode',required=True,type=int,choices=(2,),help='validated whole-nucleus timing mode')
    args=parser.parse_args()
    engines=[DirectEciEngine(p.resolve(),dialect=0x20000,encoding='utf8',native_voice=True)
             for p in (args.reference,args.timing)]
    old,candidate=engines
    saved={k:v for k,v in os.environ.items() if k.startswith(PREFIX)}
    targeted=('auca','auto','la auca','mi auto')
    controls=('pauto','pauca','la pauto','cudiau','cuna','puent','hola','casa',
              'a hueso','a huevo','la huerta','la pausa',
              '`[.1Sa.0da]','`[.1Ca.0da]','`[.1Ta.0da]')
    checks=0
    try:
        for k in saved:del os.environ[k]
        set_controls(AU_PERCENT=40,SH_PERCENT=65,TIMING_MODE=0,TIMING_SCOPE=1)
        for e in engines:e.set_parameter(PROCESS_TAGS,1)
        for rate in (1,3,6):
            for speed in (40,80):
                for gender in (0,1):
                    for e in engines:
                        e.set_parameter(SAMPLE_RATE,rate)
                        e.set_parameter(NATIVE_SPEED,speed)
                        e.set_parameter(GENDER,gender)
                    for text in (*targeted,*controls):
                        set_controls(TIMING_MODE=0)
                        original=render(old,[text])[0]
                        assert render(candidate,[text])[0]==original,(rate,speed,gender,text,'reference')
                        set_controls(TIMING_MODE=args.mode,TIMING_PERCENT=100)
                        assert render(candidate,[text])[0]==original,(rate,speed,gender,text,'100 percent')
                        checks+=2
                    for text in targeted:
                        set_controls(TIMING_MODE=0)
                        original=render(candidate,[text])[0]
                        phones=candidate._phonemes_for_probe(text)
                        sizes=[]
                        for pct in (85,70):
                            set_controls(TIMING_MODE=args.mode,TIMING_PERCENT=pct)
                            audio=render(candidate,[text])[0]
                            assert audio and audio!=original,(rate,speed,gender,text,pct,'changed')
                            assert candidate._phonemes_for_probe(text)==phones,(rate,speed,gender,text,pct,'phonemes')
                            sizes.append(len(audio));checks+=1
                        assert sizes[1]<sizes[0]<len(original),(rate,speed,gender,text,sizes,len(original))
                    for text in controls:
                        for pct in (85,70):
                            set_controls(TIMING_MODE=args.mode,TIMING_PERCENT=pct)
                            assert render(candidate,[text])[0]==render(old,[text])[0],(rate,speed,gender,text,pct,'control')
                            checks+=1
    finally:
        for e in engines:e.close()
        for k in list(os.environ):
            if k.startswith(PREFIX):del os.environ[k]
        os.environ.update(saved)
    print(f'Passed {checks} native AU-timing and preserved-reference comparisons')


if __name__=='__main__':main()
