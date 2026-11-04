"""German grammar must survive the control layout used by external clients."""
import argparse
from pathlib import Path
import re

from _reloq_eci_direct import (DirectEciEngine, EciIndex, PROCESS_TAGS,
    NATIVE_SPEED, SAMPLE_RATE, SAMPLE_RATES, HIGH_RATE_PROFILE)


TEXTS=(
    'Er kommt heute, sagte sie.',
    'Stimme bei Großbuchstaben anheben.',
    'Diese Fehler treten nur in einem anderen Add-on auf.',
    'Am 1. November sind wir wieder zu Hause.',
    'Am 12. November um 12:30 Uhr.',
    'Wenn ich nach Hause komme, rufe ich dich an.',
    'Ich habe ihr gesagt, dass sie morgen kommen kann.',
    'Der Betrag ist 1.234,56 Euro.',
    'Die Datei E-Mail/Chat wurde nach der Vorbereitung geöffnet.',
    'Die Ein- / Ausgabe wurde nach der Vorbereitung geprüft.',
)


def punctuation_commands(text, milliseconds):
    # A client puts a presentation space before its pause command.
    return re.sub(r'(?<=[A-Za-z0-9\s])([,.:;?!)])(?=\s|[\\/]|$)',
        lambda match:f' `p{milliseconds}'+match[1],text)


def capture(engine, parts, tags):
    engine.set_parameter(PROCESS_TAGS,tags)
    chunks=[];indices=[];size=0
    def receive(data):
        nonlocal size
        chunks.append(data);size+=len(data)
    assert engine.speak([*parts,EciIndex(9001)],receive,
        lambda index:indices.append((index,size)))
    assert indices[-1]==(9001,size)
    return b''.join(chunks),indices


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    args=parser.parse_args();checks=0
    e=DirectEciEngine(args.library.resolve(),encoding='utf8',native_voice=True)
    try:
        e.set_parameter(NATIVE_SPEED,75);e.set_parameter(HIGH_RATE_PROFILE,1)
        for rate,hz in SAMPLE_RATES.items():
            e.set_parameter(SAMPLE_RATE,rate)
            for text in TEXTS:
                expected=capture(e,[text],0)
                variants=(text,'`pp0 `da1 '+text,
                    '`pp0 `da1 '+punctuation_commands(text,0),
                    '`pp0 `da1 '+text.rstrip('.')+' `p0 ')
                for variant in variants:
                    assert capture(e,[variant],1)==expected,(hz,text,variant)
                    checks+=1
                plain=[];tagged=[]
                for index,word in enumerate(text.split()):
                    plain.extend([word+' ',EciIndex(index+1)])
                    tagged.extend(['`pp0 `da1 '+punctuation_commands(word+' ',0),EciIndex(index+1)])
                assert capture(e,tagged,1)==capture(e,plain,0),(hz,text,'fragment/index context')
                checks+=1
            # A client prefix must not alter nonzero pause output or indices.
            short=capture(e,['Stimme `p50 bei Großbuchstaben anheben.'],1)
            long=capture(e,['Stimme `p250 bei Großbuchstaben anheben.'],1)
            assert len(long[0])>len(short[0]),(hz,'pause duration')
            for pause,expected in ((50,short),(250,long)):
                assert capture(e,[f'`pp0 `da1 Stimme `p{pause} bei Großbuchstaben anheben.'],1)==expected
                checks+=1
            # Multiple pauses share positions with client indices. Keep the
            # source order when moving both through sentence annotations.
            plain=[];prefixed=[]
            for index,word in enumerate(('Stimme','bei','Großbuchstaben','anheben.')):
                plain.extend([word+' ',EciIndex(index+100), '`p1 '])
                prefixed.extend(['`pp0 `da1 '+word+' ',EciIndex(index+100), '`p1 '])
            assert capture(e,plain,1)==capture(e,prefixed,1);checks+=1
    finally:e.close()
    print(f'German client context: {checks} zero-pause/analysis-prefix/date/fragment/index/duration checks; audio in RAM only')


if __name__=='__main__':main()
