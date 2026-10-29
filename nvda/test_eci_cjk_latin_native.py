"""CJK accented Latin: complete native words, canonical Unicode and indices.

Optional --baseline compares ordinary text/Pinyin and Japanese PCM with the
previous release. Accented Korean/Chinese words intentionally change.
"""
from pathlib import Path
import argparse
import hashlib
import unicodedata
from _reloq_eci_direct import DirectEciEngine,EciIndex,NATIVE_SPEED

ROOT=Path(__file__).resolve().parents[1]
ORDINARY=("Hello world.","chocolate jam watch.","Geschwindigkeit Windows Wi-Fi.",
    "OK Ok ok ABC A g 504 4004 10004.","test123abc version2.5 x86-64.",
    "ni3 hao3 ma0? nuu3 xue2 xi2.","你好，这是中文。","한국어로 읽습니다.",
    "こんにちは。Windows computer.")
ACCENTS=("Müller","café","naïve","élève","Göteborg","façade","Straße",
    "æ ø œ Ā Č ǜ","Müller café naïve.","café2-élève",
    "你好Müller café。","한국어 Müller café입니다.")


def capture(engine,parts):
    digest=hashlib.sha256();length=[0];indices=[]
    def receive(data):digest.update(data);length[0]+=len(data)
    assert engine.speak([*parts,EciIndex(7)],receive,lambda i:indices.append((i,length[0])))
    assert indices==[(7,length[0])]
    return length[0],digest.hexdigest(),indices


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline",type=Path)
    args=parser.parse_args();checks=0
    for dialect in (0x60000,0xa0000,0x80000):
        current=DirectEciEngine(ROOT/"eci/libreloq_eci.so",dialect=dialect,encoding="utf8",experimental_generic=True)
        previous=DirectEciEngine(args.baseline,dialect=dialect,encoding="utf8",experimental_generic=True) if args.baseline else None
        try:
            for engine in (current,previous):
                if engine:engine.set_parameter(NATIVE_SPEED,98)
            for text in ORDINARY:
                result=capture(current,[text])
                if any(c.isascii() and c.isalpha() for c in text):assert result[0]>0,(hex(dialect),text)
                if previous:assert result==capture(previous,[text]),(hex(dialect),text,"baseline")
                checks+=1
            for text in ACCENTS:
                result=capture(current,[text])
                if dialect!=0x80000:assert result[0]>0,(hex(dialect),text,"accent-only output omitted")
                assert result==capture(current,[unicodedata.normalize("NFD",text)]),(hex(dialect),text,"NFD")
                for cut in range(1,len(text)):
                    assert result==capture(current,[text[:cut],text[cut:]]),(hex(dialect),text,cut)
                    checks+=1
                if previous and dialect==0x80000:
                    assert result==capture(previous,[text]),("Japanese changed",text)
                checks+=1
        finally:
            current.close()
            if previous:previous.close()
    print(f"CJK Latin: {checks} native output/Unicode/fragment/index checks passed; no audio files")


if __name__=="__main__":main()
