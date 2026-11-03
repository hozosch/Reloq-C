"""Discretionary layout hyphens must not split German lexical words.

Exercise actual PCM and source indices through the public ECI fragment path,
including Main replacements and explicit spelling. Audio stays in RAM.
"""
from pathlib import Path
import argparse
import tempfile

from _reloq_eci_direct import (DirectEciEngine,EciIndex,PROCESS_TAGS,NATIVE_SPEED,
    SAMPLE_RATE,SAMPLE_RATES,HIGH_RATE_PROFILE)

WORDS=(("Verschlusslaut",3),("Wortanalyse",7),("Sprachausgabe",6),
    ("Schnellaktionen",7),("Vorbereitung",3),("Konsonantenspektrum",11),
    ("Deutschunterricht",7),("Hörbücher",3))

def capture(engine,word,split,soft="",prefix=""):
    blocks=[];indices=[];size=0
    def take(data):
        nonlocal size
        blocks.append(data);size+=len(data)
    assert engine.speak([prefix+"Die ",EciIndex(1),word[:split]+soft,EciIndex(2),
        word[split:],EciIndex(3)," ist hörbar.",EciIndex(4)],take,
        lambda n:indices.append((n,size)))
    assert size and indices[-1]==(4,size)
    return b"".join(blocks),indices

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path,required=True)
    p.add_argument('--baseline',type=Path)
    args=p.parse_args();checks=0
    engine=DirectEciEngine(args.library.resolve(),dialect=0x40000,
        encoding='utf8',experimental_generic=True)
    old=DirectEciEngine(args.baseline.resolve(),dialect=0x40000,
        encoding='utf8',experimental_generic=True) if args.baseline else None
    try:
        engine.set_parameter(NATIVE_SPEED,120)
        if old:old.set_parameter(NATIVE_SPEED,120)
        for profile in (0,1):
            engine.set_parameter(HIGH_RATE_PROFILE,profile)
            for rate in SAMPLE_RATES:
                engine.set_parameter(SAMPLE_RATE,rate)
                for tags in (0,1):
                    engine.set_parameter(PROCESS_TAGS,tags)
                    for word,split in WORDS:
                        base=capture(engine,word,split)
                        for soft in ('\u00ad','\u00ad\u00ad'):
                            assert capture(engine,word,split,soft)==base,(profile,rate,tags,word)
                            checks+=1
        # Normalized lexical input enters Main exactly once, before grammar.
        with tempfile.TemporaryDirectory() as temp:
            path=Path(temp)/'DEUmain.dic'
            path.write_bytes(b'Wortanalyse\tBanane\nBanane\tApfel\n')
            engine.set_dictionary_path(temp)
            base=capture(engine,'Wortanalyse',7)
            assert capture(engine,'Wortanalyse',7,'\u00ad')==base;checks+=1
            path.unlink()
            assert capture(engine,'Wortanalyse',7,'\u00ad')==base;checks+=1
        engine.set_dictionary_path(None)
        # Explicit spelling retains the original character stream. An older
        # build is optional, so CI does not depend on an archived binary.
        engine.set_parameter(SAMPLE_RATE,2);engine.set_parameter(HIGH_RATE_PROFILE,0)
        engine.set_parameter(PROCESS_TAGS,1)
        if old:
            for parameter,value in ((SAMPLE_RATE,2),(HIGH_RATE_PROFILE,0),(PROCESS_TAGS,1)):
                old.set_parameter(parameter,value)
            for mode in ('`ts1 ','`ts2 '):
                assert capture(engine,'Wortanalyse',7,'\u00ad',mode)==capture(old,'Wortanalyse',7,'\u00ad',mode)
                checks+=1
    finally:
        engine.close()
        if old:old.close()
    print(f'German discretionary hyphens: {checks} actual PCM/index/dictionary/spelling checks; no audio files')

if __name__=='__main__':main()
