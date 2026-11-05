"""Mixed Mandarin/English, long NVDA example, native controls and indices."""
from pathlib import Path
from _reloq_eci_direct import (DirectEciEngine,EciIndex,NATIVE_INFLECTION,
    NATIVE_SPEED,GENDER,SAMPLE_RATE,PAUSE_MODE)
from test_eci_german_clock_native import render

ROOT=Path(__file__).resolve().parents[1]


def main():
    text=(ROOT/'tests/fixtures/chinese_nvda_about.txt').read_text(encoding='utf8')
    assert 'Version: 2026.3 (2026.3.0.58174)' in text
    checks=0
    for dialect in (0x60000,0x60001):
        engine=DirectEciEngine(ROOT/'eci/libreloq_eci.so',dialect=dialect,encoding='utf8',native_voice=True)
        try:
            engine.set_parameter(SAMPLE_RATE,1)
            for gender in (0,1):
                engine.set_parameter(GENDER,gender)
                for speed in (75,150):
                    engine.set_parameter(NATIVE_SPEED,speed)
                    for pause in (0,2):
                        engine.set_parameter(PAUSE_MODE,pause)
                        sounds=[]
                        for inflection in (0,30,100):
                            engine.set_parameter(NATIVE_INFLECTION,inflection)
                            result=render(engine,[text+' 你好世界。',EciIndex(9)])
                            assert result[0] and result[1][-1]==(9,len(result[0]))
                            sounds.append(result[0]);checks+=1
                        # The native Chinese words and numbers remain sensitive
                        # to the user's setting after many English fragments.
                        assert sounds[0]!=sounds[1] and sounds[1]!=sounds[2]
                        engine.set_parameter(NATIVE_INFLECTION,30)
                        split=text.index('Copyright')
                        plain=render(engine,[text,EciIndex(9)])
                        chunked=render(engine,[text[:split],EciIndex(1),text[split:],EciIndex(9)])
                        assert plain[0]==chunked[0] and [i for i,_ in chunked[1]]==[1,9]
                        assert 0<=chunked[1][0][1]<=chunked[1][1][1]==len(chunked[0]);checks+=1
            # A wholly English bank retains its neutral local inflection.
            engine.set_parameter(PAUSE_MODE,0)
            engine.set_parameter(NATIVE_INFLECTION,0);a=render(engine,['Hello world.',EciIndex(9)])[0]
            engine.set_parameter(NATIVE_INFLECTION,100);b=render(engine,['Hello world.',EciIndex(9)])[0]
            assert a==b,(hex(dialect),'English bank inflection');checks+=1
        finally:engine.close()
    print(f'Chinese mixed text: {checks} long-text, inflection, gender, speed, pause and index checks passed')


if __name__=='__main__':main()
