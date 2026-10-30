"""Actual PCM/index checks for Greek accents and Chinese source identities.

The optional baseline covers text that should stay unchanged. PCM is hashed
in memory; no audio files or speech-result caches are used.
"""
from pathlib import Path
import argparse
import hashlib
import unicodedata
from _reloq_eci_direct import DirectEciEngine,EciText,EciIndex,NATIVE_SPEED,SAMPLE_RATE

ROOT=Path(__file__).resolve().parents[1]


def capture(engine,parts):
    size=[0];digest=hashlib.sha256();indices=[]
    def pcm(data):size[0]+=len(data);digest.update(data)
    assert engine.speak([*parts,EciIndex(7)],pcm,lambda i:indices.append((i,size[0])))
    assert size[0] and indices==[(7,size[0])]
    return size[0],digest.hexdigest(),indices


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--baseline",type=Path)
    a=p.parse_args();checks=0
    for dialect in (0x120000,0x60000):
        current=DirectEciEngine(ROOT/"eci/libreloq_eci.so",dialect=dialect,encoding="utf8",experimental_generic=True)
        old=DirectEciEngine(a.baseline,dialect=dialect,encoding="utf8",experimental_generic=True) if a.baseline else None
        try:
            for rate in (1,6):
                for engine in (current,old):
                    if engine:engine.set_parameter(SAMPLE_RATE,rate);engine.set_parameter(NATIVE_SPEED,98)
                ordinary=("Hello world.","Καλημέρα σας, voice clock shop.","503 και 4004.","(λόγος): τέλος.") if dialect==0x120000 else (
                    "Hello world.","ni3 hao3 ma0?", "5g 12kg 50 g.","503和4004，还有10004。", "中文abc123 test.")
                if old:
                    for text in ordinary:
                        assert capture(current,[text])==capture(old,[text]),(hex(dialect),rate,text);checks+=1
                if dialect==0x120000:
                    for text in ("Müller","ÄÖÜäöü","café","naïve","élève","façade","Straße","Œuvre",
                        "A für Müller café naïve.","Γεια Müller και café."):
                        expected=capture(current,[text])
                        assert expected==capture(current,[unicodedata.normalize("NFD",text)]),(text,"NFD");checks+=1
                        for cut in range(1,len(text)):
                            assert expected==capture(current,[text[:cut],text[cut:]]),(text,cut);checks+=1
                    if old:assert capture(current,["Müller"])!=capture(old,["Müller"]);checks+=1
                else:
                    for speed in (55,98):
                        current.set_parameter(NATIVE_SPEED,speed)
                        expected=capture(current,[EciText("g",True)])
                        for text in ("g","G","ｇ"):
                            assert capture(current,[text])==expected,(text,rate,speed);checks+=1
                    current.set_parameter(NATIVE_SPEED,98)
                    for first,second in (("發","髮"),("隻","只"),("臟","髒"),("癥","症")):
                        assert capture(current,[first])!=capture(current,[second]),(first,second);checks+=1
                    for first,second in (("頭髮","头发"),("理髮","理发"),("發展","发展"),("發現","发现"),
                        ("幹部","干部"),("甲殼","甲壳"),("乾坤","乾坤")):
                        expected=capture(current,[first]);assert expected==capture(current,[second]),(first,second)
                        for cut in range(1,len(first)):
                            assert expected==capture(current,[first[:cut],first[cut:]]),(first,cut);checks+=1
                        checks+=1
                    if old:assert capture(old,["g"])!=capture(current,["g"]);checks+=1
        finally:
            current.close()
            if old:old.close()
    print(f"Native text input: {checks} Greek Unicode/word/fragment and Chinese letter/tone/context PCM-index checks passed; no audio files")


if __name__=="__main__":main()
