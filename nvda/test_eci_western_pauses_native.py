"""Native pause durations and preserved explicit annotations in every language."""
from pathlib import Path
from _reloq_eci_direct import (DirectEciEngine,EciText,EciIndex,PAUSE_MODE,
    SAMPLE_RATE,NATIVE_SPEED,PROCESS_TAGS)
from _reloq_languages import LANGUAGES
ROOT=Path(__file__).resolve().parents[1]
WORDS={0x10000:'One, two. Three.',0x10001:'One, two. Three.',
    0x20000:'Uno, dos. Tres.',0x20001:'Uno, dos. Tres.',
    0x30000:'Un, deux. Trois.',0x30001:'Un, deux. Trois.',
    0x50000:'Uno, due. Tre.',0x70000:'Um, dois. Três.',
    0x90000:'Yksi, kaksi. Kolme.'}

def main():
    count=0
    for language,text in WORDS.items():
        e=DirectEciEngine(ROOT/'eci/libreloq_eci.so',dialect=language,
            encoding='utf-8',experimental_generic=True)
        def render(text,mode):
            e.set_parameter(PAUSE_MODE,mode);blocks=[];marks=[]
            assert e.speak([EciText(text,False),EciIndex(7)],blocks.append,marks.append)
            assert marks==[7]
            return b''.join(blocks)
        try:
            for rate in [0,1,2,3,4,5,6]:
                e.set_parameter(SAMPLE_RATE,rate)
                for speed in [40,98,250]:
                    e.set_parameter(NATIVE_SPEED,speed)
                    normal,final,always=[render(text,m)for m in range(3)]
                    assert 0<len(always)<=len(final)<len(normal),(hex(language),rate,speed,[len(x)for x in [normal,final,always]])
                    assert normal.startswith(final),(hex(language),rate,speed,'final mode changed speech')
                    count+=1
                e.set_parameter(PROCESS_TAGS,1)
                explicit=text.split(',')[0]+'`p250'
                normal=render(explicit,0)
                assert render(explicit,1)==normal and render(explicit,2)==normal,(hex(language),rate,'explicit pause changed')
                e.set_parameter(PROCESS_TAGS,0)
            print('Native pauses',LANGUAGES[language].locale,'passed',flush=True)
        finally:e.close()
    print(count,'rate/speed pause comparisons and explicit durations passed; no audio files')

if __name__=='__main__':main()
