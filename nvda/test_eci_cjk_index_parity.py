"""CJK source indices must not change spoken audio or introduce extra pauses."""
from pathlib import Path
import argparse

from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciPitch,
    NATIVE_SPEED, PAUSE_MODE, SAMPLE_RATE)
from test_eci_german_clock_native import render

CASES={
    0x80000:('Windows 11 を使います。','東京 日本 の テストです。',
             'これは音声のテストです。Windowsとcomputer。','あ い う'),
    0x60000:('我使用 Windows 11。','你好 世界 我们 去北京。','This is a test。'),
    0x60001:('我使用 Windows 11。','你好 世界 我們 去北京。','This is a test。'),
    0xa0000:('Windows 11 을 사용합니다.','안녕하세요 여러분 이건 시험입니다.',
             'This is a test.','하나 둘 셋.'),
}


def marked(text):
    parts=[EciIndex(100)]
    for i,character in enumerate(text):
        parts.append(character)
        parts.append(EciIndex(101+i))
    return parts


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    parser.add_argument('--reference',type=Path,help='Optional core before the index fix')
    args=parser.parse_args();checks=0
    for language,texts in CASES.items():
        e=DirectEciEngine(args.library.resolve(),dialect=language,encoding='utf8',native_voice=True)
        previous=(DirectEciEngine(args.reference.resolve(),dialect=language,encoding='utf8',native_voice=True)
                  if args.reference else None)
        try:
            for engine in (e,previous):
                if engine:engine.set_parameter(NATIVE_SPEED,60)
            for rate,pause in ((1,0),(3,2),(6,1)):
                for engine in (e,previous):
                    if engine:engine.set_parameter(SAMPLE_RATE,rate);engine.set_parameter(PAUSE_MODE,pause)
                for text in texts:
                    plain=render(e,[text])[0];audio,events=render(e,marked(text))
                    assert plain and audio==plain,(hex(language),rate,pause,text,'index changed speech')
                    assert [value for value,_ in events]==list(range(100,101+len(text)))
                    assert events[0][1]==0 and all(0<=where<=len(audio) for _,where in events)
                    assert all(events[i][1]<=events[i+1][1] for i in range(len(events)-1))
                    if previous:
                        assert plain==render(previous,[text])[0],(hex(language),rate,pause,text,'plain speech changed')
                    checks+=1
                if previous:
                    # Explicit changes in pitch still use their existing acoustic boundary.
                    parts=['Windows ',EciPitch(80),'11 ',EciPitch(65),'test.']
                    assert render(e,parts)==render(previous,parts),(hex(language),rate,pause,'pitch')
                    checks+=1
        finally:
            e.close()
            if previous:previous.close()
    print(f'CJK index parity: {checks} real PCM/index/pitch comparisons, including Japanese SAPI text')


if __name__=='__main__':main()
