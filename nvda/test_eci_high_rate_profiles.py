"""Public ECI hearing profiles: isolation, rollback, timing and stable levels."""
from pathlib import Path
import argparse
import array
import hashlib
import math
from _reloq_eci_direct import (DirectEciEngine,EciIndex,HIGH_RATE_PROFILE,SAMPLE_RATE,
    SAMPLE_RATES,NATIVE_SPEED,NATIVE_VOLUME,GENDER,HEAD_SIZE,BREATHINESS)
from _reloq_languages import LANGUAGES


def capture(engine,text):
    output=[];indices=[];size=0
    def pcm(data):
        nonlocal size
        output.append(data);size+=len(data)
    assert engine.speak([text,EciIndex(19)],pcm,lambda n:indices.append((n,size)))
    data=b''.join(output)
    assert data and indices==[(19,len(data))]
    return data,indices


def rms(data):
    samples=array.array('h',data)
    return math.sqrt(sum(n*n for n in samples)/len(samples))


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path,required=True)
    args=p.parse_args();checks=0
    for language,text in ((0x40000,'Susi sieht sieben Segel. Sie sitzen zusammen am See.'),
                          (0x10000,'Sue sees six ships. Susan visits the zoo on Thursday.')):
        engine=DirectEciEngine(args.library.resolve(),dialect=language,encoding='utf8',experimental_generic=True)
        other=DirectEciEngine(args.library.resolve(),dialect=language,encoding='utf8',experimental_generic=True)
        try:
            assert engine._dll.eciGetParam(engine._handle,HIGH_RATE_PROFILE)==3;checks+=1
            other.set_parameter(HIGH_RATE_PROFILE,0)
            for rate in SAMPLE_RATES:
                for e in (engine,other):
                    e.set_parameter(SAMPLE_RATE,rate);e.set_parameter(NATIVE_SPEED,75)
                for female in (0,1):
                    for e in (engine,other):
                        for parameter,value in ((GENDER,female),(HEAD_SIZE,29 if female else 50),(BREATHINESS,60 if female else 0)):
                            e.set_parameter(parameter,value)
                    engine.set_parameter(HIGH_RATE_PROFILE,0);baseline=capture(engine,text)
                    variants=[]
                    for profile in (1,2,3):
                        engine.set_parameter(HIGH_RATE_PROFILE,profile);actual=capture(engine,text)
                        assert len(actual[0])==len(baseline[0]) and actual[1]==baseline[1];checks+=1
                        if SAMPLE_RATES[rate]<16000:
                            assert actual==baseline,(language,rate,profile,'narrow-band changed');checks+=1
                        else:
                            assert actual[0]!=baseline[0],(language,rate,profile,'profile has no audible data effect');checks+=1
                        # A second handle's voice and filter state stay independent.
                        assert capture(other,text)==baseline,(language,rate,profile,'cross-engine leak');checks+=1
                        variants.append(actual)
                    assert variants[0]==variants[1],(language,rate,'legacy C must alias the accepted combination');checks+=1
                    if SAMPLE_RATES[rate]<=16000:
                        assert variants[0]==variants[2],(language,rate,'richness changed the foundation');checks+=1
                    else:
                        assert variants[0][1]==variants[2][1] and len(variants[0][0])==len(variants[2][0]);checks+=1
                        assert variants[0][0]!=variants[2][0],(language,rate,'richness did not reach speech');checks+=1
                    engine.set_parameter(HIGH_RATE_PROFILE,0)
                    assert capture(engine,text)==baseline,(language,rate,'rollback mismatch');checks+=1
                if SAMPLE_RATES[rate]>=16000 and language==0x40000:
                    for e in (engine,other):
                        for parameter,value in ((GENDER,0),(HEAD_SIZE,50),(BREATHINESS,0)):
                            e.set_parameter(parameter,value)
                    for word in ('Papa.','Fifi.','Fee.'):
                        engine.set_parameter(HIGH_RATE_PROFILE,0);before=capture(engine,word)
                        engine.set_parameter(HIGH_RATE_PROFILE,1)
                        after=capture(engine,word)
                        # All consonants, including P/F, now use the complete
                        # Classic target. Their source timing stays fixed.
                        onset=int(SAMPLE_RATES[rate]*.008)*2
                        assert after[1]==before[1] and len(after[0])==len(before[0])
                        assert any(before[0][:onset]) and after[0][:onset]!=before[0][:onset],(rate,word,'p/f bypassed Classic target');checks+=1
            for bad in (-1,4):
                assert engine._dll.eciSetParam(engine._handle,HIGH_RATE_PROFILE,bad)==-1;checks+=1
        finally:engine.close();other.close()
    # Configured volume must work in every module, including CJK, without
    # changing speech duration or source-phase progression.
    for language in LANGUAGES.values():
        engine=DirectEciEngine(args.library.resolve(),dialect=language.id,encoding='utf8',experimental_generic=True)
        try:
            engine.set_parameter(HIGH_RATE_PROFILE,1)
            for rate in (1,6):
                engine.set_parameter(SAMPLE_RATE,rate)
                engine.set_parameter(NATIVE_VOLUME,100);full=capture(engine,'Hello world. 504.')
                engine.set_parameter(NATIVE_VOLUME,30);quiet=capture(engine,'Hello world. 504.')
                assert len(full[0])==len(quiet[0]) and full[1]==quiet[1]
                assert abs(rms(quiet[0])/rms(full[0])-181.907/2768.533)<.004,(language.name,rate,'volume ignored');checks+=2
        finally:engine.close()
    print(f'Hearing profiles: {checks} PCM/index/p-f/low-rate/rollback/two-engine/volume checks; audio in RAM only')


if __name__=='__main__':main()
