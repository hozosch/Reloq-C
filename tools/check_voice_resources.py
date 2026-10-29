"""Measure eight US presets with full dictionaries; PCM stays in RAM.

Run in a fresh process for meaningful peak RSS. --adapter can load a previous
LanguageEngine module against the very same library to isolate adapter costs.
"""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import sys
import time

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/"nvda"))
from _reloq_eci_direct import DirectEciEngine,EciIndex,NATIVE_SPEED
import _reloq_languages as languages


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--library",type=Path,default=ROOT/("eci/eci.dll" if sys.platform=="win32" else "eci/libreloq_eci.so"))
    parser.add_argument("--adapter",type=Path)
    parser.add_argument("--dictionaries",type=Path,default=ROOT/"data/community")
    args=parser.parse_args()
    module=languages
    if args.adapter:
        spec=importlib.util.spec_from_file_location("comparison_adapter",args.adapter)
        module=importlib.util.module_from_spec(spec)
        sys.modules[spec.name]=module
        spec.loader.exec_module(module)
    handles=[]
    def factory(*a,**kw):
        result=DirectEciEngine(*a,**kw);handles.append(result);return result
    start=time.perf_counter()
    engine=module.LanguageEngine(args.library,language=0x10000,engine_factory=factory)
    rows=[]
    try:
        engine.set_parameter(NATIVE_SPEED,98)
        engine.set_dictionary_path(args.dictionaries)
        for voice in list("12345678")+list("87654321"):
            begin=time.perf_counter();engine.select_voice(0x10000,voice)
            digest=hashlib.sha256();length=[0];indices=[]
            def receive(data): digest.update(data);length[0]+=len(data)
            assert engine.speak(["This is Eloquence and NVDA, with 504 and 4004.",EciIndex(7)],
                                receive,lambda i:indices.append((i,length[0])))
            assert indices==[(7,length[0])] and length[0]>0
            rows.append(dict(voice=voice,seconds=time.perf_counter()-begin,
                             pcm_bytes=length[0],sha256=digest.hexdigest()))
        result=dict(handles=len(handles),before_close_seconds=time.perf_counter()-start,
                    no_audio_files=True,rows=rows)
        if sys.platform!="win32":
            import resource
            result["peak_rss_kib"]=resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    finally:
        begin=time.perf_counter();engine.close()
    result["close_seconds"]=time.perf_counter()-begin
    print(json.dumps(result))


if __name__=="__main__": main()
