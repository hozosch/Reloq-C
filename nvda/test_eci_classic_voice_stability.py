"""Real speech regression for Classic body peaks; all PCM stays in memory."""
import argparse
import array
import ctypes
from pathlib import Path

from _reloq_eci_direct import (DirectEciEngine, EciIndex, SAMPLE_RATE,
    SAMPLE_RATES, NATIVE_SPEED, NATIVE_PITCH, HIGH_RATE_PROFILE, GENDER, HEAD_SIZE, BREATHINESS)


def capture(engine, text):
    chunks=[];indices=[];size=0
    def receive(data):
        nonlocal size
        chunks.append(data);size+=len(data)
    assert engine.speak([text,EciIndex(17)],receive,lambda n:indices.append((n,size)))
    data=b''.join(chunks)
    assert data and indices==[(17,len(data))]
    return data,indices


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path,required=True)
    p.add_argument('--baseline',type=Path)
    args=p.parse_args();checked=identities=0;maximum=0
    texts={
        0x10000:'The warm morning voice should remain smooth. We are hearing more body in every vowel. Susan visits the zoo on Thursday.',
        0x40000:'Ohne obere Bässe wirkt die Stimme dünner. Am Morgen hören wir ruhige Musik. Karl und Werner wohnen neben dem Bahnhof. Zwölf Züge fahren zum Zentrum.',
        0x20000:'La voz de Susana suena suave por la mañana. Todos los días trae un vaso de agua. Este texto tiene consonantes y vocales.',
    }
    for language,text in texts.items():
        for gender,head,breath in ((0,50,0),(1,29,60)):
            engines=[DirectEciEngine(path.resolve(),dialect=language,
                encoding='utf8',experimental_generic=True)
                for path in ([args.library,args.baseline] if args.baseline else [args.library])]
            try:
                for e in engines:
                    for param,value in ((NATIVE_SPEED,75),(GENDER,gender),
                        (HEAD_SIZE,head),(BREATHINESS,breath)):
                        e.set_parameter(param,value)
                for rate,hz in SAMPLE_RATES.items():
                    for e in engines:e.set_parameter(SAMPLE_RATE,rate)
                    if len(engines)>1:
                        for e in engines:e.set_parameter(HIGH_RATE_PROFILE,0)
                        assert capture(engines[0],text)==capture(engines[1],text)
                        identities+=1
                    for e in engines:e.set_parameter(HIGH_RATE_PROFILE,1)
                    actual=capture(engines[0],text)
                    if hz<16000:
                        if len(engines)>1:
                            assert actual==capture(engines[1],text);identities+=1
                        continue
                    samples=array.array('h',actual[0]);peak=max(abs(x) for x in samples)
                    assert peak<32767,(hex(language),gender,hz,peak,'PCM limit reached')
                    maximum=max(maximum,peak);checked+=1
            finally:
                for e in engines:e.close()
    # The upper shelf must not clip female vowels at the five-formant
    # head size, or unusual but supported pitch/head combinations.
    stress='Am Morgen hören wir ruhige Musik. Oma und Werner wohnen neben dem Bahnhof.'
    for gender in (0,1):
        e=DirectEciEngine(args.library.resolve(),encoding='utf8',experimental_generic=True)
        try:
            for param,value in ((NATIVE_SPEED,75),(HIGH_RATE_PROFILE,1),(GENDER,gender)):
                e.set_parameter(param,value)
            for head in (29,50,80):
                e.set_parameter(HEAD_SIZE,head)
                for pitch in (30,65,90):
                    e.set_parameter(NATIVE_PITCH,pitch)
                    for rate in (3,6):
                        e.set_parameter(SAMPLE_RATE,rate)
                        samples=array.array('h',capture(e,stress)[0])
                        peak=max(abs(x) for x in samples)
                        assert peak<32767,(gender,head,pitch,SAMPLE_RATES[rate],peak)
                        maximum=max(maximum,peak);checked+=1
        finally:e.close()
    # Exercise every exposed preset with its actual anatomy and source.
    for language,text in texts.items():
        e=DirectEciEngine(args.library.resolve(),dialect=language,encoding='utf8',experimental_generic=True)
        try:
            copy=e._dll.eciCopyVoice
            copy.argtypes=(ctypes.c_void_p,ctypes.c_int,ctypes.c_int);copy.restype=ctypes.c_int
            e.set_parameter(HIGH_RATE_PROFILE,1)
            for voice in range(1,9):
                assert copy(e._handle,voice,0);e.set_parameter(NATIVE_SPEED,75)
                for rate in (3,2,4,5,6):
                    e.set_parameter(SAMPLE_RATE,rate)
                    samples=array.array('h',capture(e,text)[0]);peak=max(abs(x) for x in samples)
                    assert peak<32767,(hex(language),voice,SAMPLE_RATES[rate],peak)
                    maximum=max(maximum,peak);checked+=1
        finally:e.close()
    print(f'Classic voice stability: {checked} real speech/model/rate peak checks; '
        f'{identities} A/low-rate baseline PCM identities; maximum peak {maximum}; audio in RAM only')


if __name__=='__main__':main()
