"""Measure native speech duration for a controlled six-syllable sequence.

This measures output duration, not synthesis CPU time or perceived intelligibility.
The native /m a/ spellings are shared by the tested Western phone inventories.
Every language keeps its own vowel, duration and stress model. PCM stays in RAM.
No automatic cross-language rate normalization is applied.
"""
from pathlib import Path
import argparse,json,sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'nvda'))
from _reloq_eci_direct import DirectEciEngine,NATIVE_SPEED,NATIVE_INFLECTION,PAUSE_MODE,PROCESS_TAGS
from _reloq_languages import LANGUAGES


def measure(library):
    rows=[]
    for language in LANGUAGES.values():
        if language.id in (0x60000,0x80000,0xa0000):continue
        engine=DirectEciEngine(library,dialect=language.id,encoding='utf8',experimental_generic=True)
        try:
            engine.set_parameter(PROCESS_TAGS,1)
            engine.set_parameter(NATIVE_INFLECTION,0)
            for speed in (55,98):
                engine.set_parameter(NATIVE_SPEED,speed)
                for pauses in (0,2):
                    engine.set_parameter(PAUSE_MODE,pauses)
                    samples=[0]
                    def count(data):samples[0]+=len(data)//2
                    syllable='`[.m1a]' if language.id in (0x30000,0x30001) else '`[.1ma]'
                    text=' '.join([syllable]*6)+'.'
                    assert engine.speak([text],count,lambda _:None),language.name
                    rows.append(dict(language=language.name,dialect=language.id,speed=speed,
                        pause_mode=pauses,sample_rate=11025,samples=samples[0],seconds=samples[0]/11025))
        finally:engine.close()
    return dict(method='Six separate native /ma/ syllables; inflection 0; 11025 Hz; PCM in memory',measurements=rows)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path,default=ROOT/'eci/libreloq_eci.so')
    p.add_argument('--output',type=Path)
    a=p.parse_args();result=measure(a.library.resolve())
    if a.output:a.output.write_text(json.dumps(result,indent=2)+'\n')
    for row in result['measurements']:
        if row['speed']==55 and row['pause_mode']==2:print(f"{row['language']}: {row['seconds']:.3f} s")


if __name__=='__main__':main()
