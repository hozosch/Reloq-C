"""Measure speech after cancellation with a fully loaded native US dictionary.

Every recovery must reproduce a complete uninterrupted PCM/index result.
PCM stays in RAM; output contains timings, sizes and hashes only.
"""
from pathlib import Path
import argparse
import hashlib
import json
import sys
import time

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"nvda"))
from _reloq_eci_direct import DirectEciEngine,EciIndex,NATIVE_SPEED,SAMPLE_RATE


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--library",type=Path,default=ROOT/("eci/eci.dll" if sys.platform=="win32" else "eci/libreloq_eci.so"))
    p.add_argument("--dictionaries",type=Path,default=ROOT/"data/community")
    p.add_argument("--cycles",type=int,default=6)
    p.add_argument("--rate",type=int,choices=range(7),default=1)
    p.add_argument("--output",type=Path)
    a=p.parse_args()
    if a.cycles<1:p.error("--cycles must be positive")
    engine=DirectEciEngine(a.library,dialect=0x10000,encoding="utf8",experimental_generic=True)
    engine.set_parameter(NATIVE_SPEED,98);engine.set_parameter(SAMPLE_RATE,a.rate)
    rows=[]
    def complete():
        begin=time.perf_counter();first=[None];size=[0];marks=[];digest=hashlib.sha256()
        def take(data):
            if first[0] is None:first[0]=time.perf_counter()-begin
            size[0]+=len(data);digest.update(data)
        assert engine.speak(["Speech after cancellation: Eloquence, NVDA, 504 and 4004.",EciIndex(7)],
                            take,lambda i:marks.append((i,size[0])))
        assert size[0] and marks==[(7,size[0])]
        return (size[0],digest.hexdigest(),marks),time.perf_counter()-begin,first[0]
    try:
        begin=time.perf_counter();engine.set_dictionary_path(a.dictionaries)
        load=time.perf_counter()-begin
        expected,_,_=complete()
        for _ in range(a.cycles):
            begin=time.perf_counter();received=[]
            def interrupted(data):
                if not received:received.append(len(data));engine.cancel()
            assert not engine.speak(["This is an interrupted sentence, followed by another sentence with ordinary English words."],
                                    interrupted,lambda i:None)
            assert received
            cancel=time.perf_counter()-begin
            actual,total,first=complete()
            assert actual==expected,"Recovery changed PCM or index positions"
            rows.append(dict(cancel_seconds=cancel,recovery_seconds=total,recovery_first_pcm_seconds=first))
    finally:engine.close()
    result=dict(native_version=engine.native_version,rate=a.rate,load_seconds=load,
        pcm_bytes=expected[0],sha256=expected[1],indices=expected[2],rows=rows,
        pcm_and_indices_identical=True,no_audio_files=True)
    if a.output:a.output.write_text(json.dumps(result,indent=2)+"\n")
    print(json.dumps(result))


if __name__=="__main__":main()
