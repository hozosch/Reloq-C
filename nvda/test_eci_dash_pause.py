"""Verify dash pauses and German underscore stress in the real ECI engine."""
from pathlib import Path
import argparse
import ctypes as c

from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciText, NATIVE_SPEED,
    SAMPLE_RATE, PAUSE_MODE, PROCESS_TAGS)
from test_eci_german_clock_native import phonemes, render


def character_phonemes(engine, text):
    dll=engine._dll;handle=engine._handle;chunks=[];buffer=c.create_string_buffer(8192)
    H,I=c.c_void_p,c.c_int
    dll.eciGeneratePhonemes.argtypes=(H,I,H);dll.eciGeneratePhonemes.restype=I
    dll.eciAddTextUtf8.argtypes=(H,c.c_char_p,c.c_size_t,I);dll.eciAddTextUtf8.restype=I
    callback_type=(c.WINFUNCTYPE if hasattr(c,'WINFUNCTYPE') else c.CFUNCTYPE)(I,H,I,I,H)
    @callback_type
    def callback(_,message,count,data):
        if message==1:chunks.append(buffer.raw[:count])
        return 1
    dll.eciRegisterCallback(handle,callback,None)
    try:
        assert dll.eciSetParam(handle,0,1)>=0
        payload=text.encode('utf8')
        assert dll.eciAddTextUtf8(handle,payload,len(payload),1)
        assert dll.eciGeneratePhonemes(handle,len(buffer),buffer)
    finally:
        dll.eciSetParam(handle,0,0);dll.eciRegisterCallback(handle,engine._callback,None)
    return b''.join(chunks).decode('cp1252')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    parser.add_argument('--reference',type=Path,help='Optional pre-change core for unaffected behavior')
    args=parser.parse_args();checks=0
    for dialect in (0x10000,0x10001):
        for encoding in ('utf8','cp1252'):
            engine=DirectEciEngine(args.library.resolve(),dialect=dialect,encoding=encoding,native_voice=True)
            try:
                engine.set_parameter(NATIVE_SPEED,85)
                for rate,pause in ((1,0),(3,0),(1,2)):
                    engine.set_parameter(SAMPLE_RATE,rate);engine.set_parameter(PAUSE_MODE,pause)
                    for left,right in (('',''),(' ',' '),(' ','') ,('',' ')):
                        def capture(dash):
                            return render(engine,['The weather was pleasant',EciIndex(1),left+dash+right,
                                                  EciIndex(2),'until the storm arrived.',EciIndex(3)])
                        long_dash=capture('—');short_dash=capture('–')
                        assert len(long_dash[0]) and long_dash==short_dash,(dialect,encoding,rate,pause,left,right)
                        assert [m[0] for m in long_dash[1]]==[1,2,3]
                        checks+=1
                # A hyphen is a word connector, not the sentence dash in #13.
                engine.set_parameter(SAMPLE_RATE,3);engine.set_parameter(PAUSE_MODE,0)
                hyphen=render(engine,['The weather was pleasant-until the storm arrived.'])[0]
                dash=render(engine,['The weather was pleasant—until the storm arrived.'])[0]
                comma=render(engine,['The weather was pleasant, until the storm arrived.'])[0]
                assert len(dash)>len(hyphen) and dash==comma,(dialect,encoding,'dash pause')
                checks+=1
            finally:engine.close()
    for dialect,text in ((0x40000,'Das Wetter war schön–bis der Sturm kam.'),
                         (0x20000,'Hola–mundo.'),(0x20001,'Hola–mundo.'),
                         (0x30000,'Bonjour–monde.'),(0x30001,'Bonjour–monde.'),
                         (0x50000,'Ciao–mondo.'),(0x70000,'Olá–mundo.'),
                         (0x90000,'Hyvää–päivää.'),(0x120000,'Καλός καιρός–ήρθε η καταιγίδα.')):
        engine=DirectEciEngine(args.library.resolve(),dialect=dialect,encoding='utf8',native_voice=True)
        try:
            engine.set_parameter(NATIVE_SPEED,85);engine.set_parameter(SAMPLE_RATE,3)
            hyphen=render(engine,[text.replace('–','-')])[0]
            short=render(engine,[text])[0]
            long=render(engine,[text.replace('–','—')])[0]
            assert short==long and len(short)>len(hyphen),(dialect,'native dash pause')
            checks+=1
        finally:engine.close()
    # Issue #15: an isolated dash before a number in running text is a
    # separator, including after sentence punctuation and across ECI chunks.
    examples=('Post - 0 of 3000 characters','Post- 0 of 3000 characters','This is - 1 test.',
              'It was related to dictionaries, not sound. - 14 minutes ago.')
    for dialect in (0x10000,0x10001,0x40000,0x60000,0x80000):
        engine=DirectEciEngine(args.library.resolve(),dialect=dialect,encoding='utf8',native_voice=True)
        try:
            engine.set_parameter(NATIVE_SPEED,85)
            for rate,pause in ((1,0),(3,0),(1,2)):
                engine.set_parameter(SAMPLE_RATE,rate);engine.set_parameter(PAUSE_MODE,pause)
                for text in examples:
                    left,right=text.split('-',1)
                    def capture(separator):
                        return render(engine,[left,EciIndex(1),separator,EciIndex(2),right,EciIndex(3)])
                    assert capture('-')==capture(','),(hex(dialect),rate,pause,text)
                    assert phonemes(engine,text)==phonemes(engine,text.replace('-',',')),(hex(dialect),text)
                    checks+=2
            assert character_phonemes(engine,'-')!=character_phonemes(engine,',')
            checks+=1
        finally:engine.close()
    german=DirectEciEngine(args.library.resolve(),dialect=0x40000,encoding='utf8',native_voice=True)
    try:
        german.set_parameter(PROCESS_TAGS,1)
        noun=phonemes(german,'Der Unterstrich ist lang.')
        verb=phonemes(german,'Er unterstrich den Satz.')
        assert '[.1Un.0tR.2StrIX]' in noun,noun
        assert '[.0Un.0tR.1StrIX]' in verb,verb
        spelling=character_phonemes(german,'_')
        assert '[.1Un.0tR.2StrIX]' in spelling,spelling
        character=render(german,[EciText('_',True)])[0]
        assert character==render(german,[' `[.1Un.0tR.2StrIX] '])[0]
        # Both modes now use the same native character-name table.
        assert character==render(german,['_'])[0]
        # U+2014 used to share the underscore action. Its own entry in the
        # native character table says Gedankenstrich, at all speeds tested.
        for speed in (40,85,150):
            german.set_parameter(NATIVE_SPEED,speed)
            assert character_phonemes(german,'—')==phonemes(german,'Gedankenstrich')
            assert render(german,[EciText('—',True),EciIndex(7)])==render(german,['Gedankenstrich',EciIndex(7)])
            checks+=2
        german.set_parameter(NATIVE_SPEED,85)
        character=render(german,[EciText('_',True)])[0]
        german.set_parameter(PROCESS_TAGS,0)
        assert character==render(german,[EciText('_',True)])[0]
        checks+=6
    finally:german.close()
    if args.reference:
        for dialect in (0x10000,0x10001,0x40000):
            for encoding in ('utf8','cp1252'):
                old=DirectEciEngine(args.reference.resolve(),dialect=dialect,encoding=encoding,native_voice=True)
                new=DirectEciEngine(args.library.resolve(),dialect=dialect,encoding=encoding,native_voice=True)
                try:
                    for engine in (old,new):
                        engine.set_parameter(NATIVE_SPEED,85);engine.set_parameter(SAMPLE_RATE,3)
                    texts=('2–4','2020 – 2024','2 – -4','2 – +4','und 24–32 Punkte',
                           '2 - 1','2- 1','2020 - 2024','- 1','-1','- 42','Room 2 - 1',
                           '(- 1)','( - 1)','[ - 1]',
                           'Das ist ein einfacher Satz.','Der Unterstrich ist lang.','Er unterstrich den Satz.')
                    if encoding=='utf8':texts+=('2 – +4',)
                    for text in texts:
                        before=render(old,[text,EciIndex(3)]);after=render(new,[text,EciIndex(3)])
                        assert before==after,(dialect,encoding,text,len(before[0]),len(after[0]))
                        checks+=1
                finally:old.close();new.close()
    print(f'Dash pauses and German character stress: {checks} real ECI audio, index and phoneme checks')


if __name__=='__main__':main()
