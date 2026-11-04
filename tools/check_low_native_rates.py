"""Check low native speeds needed by SAPI5IBMTTS, without audio files."""
import argparse
import hashlib
import json
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'nvda'))
from _reloq_eci_direct import DirectEciEngine,NATIVE_SPEED,SAMPLE_RATE,PROCESS_TAGS
from _reloq_languages import LANGUAGES
from compare_eci_builds import TEXTS


def capture(engine,text):
    digest=hashlib.sha256();count=0
    def pcm(data):
        nonlocal count
        count+=len(data)//2;digest.update(data)
    assert engine.speak([text],pcm,None)
    assert count>20
    return count,digest.hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path,required=True)
    p.add_argument('--report',type=Path,required=True)
    a=p.parse_args();records=[];inline=0
    for language in LANGUAGES.values():
        e=DirectEciEngine(a.library.resolve(),dialect=language.id,encoding='utf8',native_voice=True)
        try:
            for rate in (1,6):
                e.set_parameter(SAMPLE_RATE,rate);lengths=[]
                for speed in (0,1,10,20,30,39,40,60,160,250):
                    e.set_parameter(NATIVE_SPEED,speed);samples,digest=capture(e,TEXTS[language.id]);lengths.append(samples)
                    records.append(dict(language=language.locale,rate=rate,native_speed=speed,samples=samples,sha256=digest))
                assert all(x>=y for x,y in zip(lengths,lengths[1:])),(language.locale,rate,lengths)
                assert lengths[0]>lengths[6]>lengths[7]>lengths[-1],(language.locale,lengths)
            if language.id in (0x10000,0x40000):
                e.set_parameter(SAMPLE_RATE,1);e.set_parameter(PROCESS_TAGS,1)
                for speed in (0,10,20,30):
                    text='A short test.' if language.id==0x10000 else 'Ein kurzer Test.'
                    e.set_parameter(NATIVE_SPEED,speed);plain=capture(e,text)
                    e.set_parameter(NATIVE_SPEED,60);tagged=capture(e,f'`vs{speed} '+text)
                    assert plain==tagged,(language.locale,speed,plain,tagged);inline+=1
        finally:
            e.close()
    report=dict(cases=len(records),inline_pcm_matches=inline,all_duration_curves_monotonic=True,no_audio_files=True,records=records)
    a.report.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    print(f'Low native rates: {len(records)} language/rate/speed cases, {inline} inline/API PCM matches')


if __name__=='__main__':
    main()
