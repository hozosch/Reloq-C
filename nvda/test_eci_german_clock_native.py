"""German clocks through the ECI API: wording, controls and source indices."""
import argparse
import ctypes as c
from pathlib import Path
from _reloq_eci_direct import DirectEciEngine, EciIndex, PROCESS_TAGS, SAMPLE_RATE


def render(engine, parts):
    audio=bytearray();indices=[]
    assert engine.speak(parts,audio.extend,lambda i:indices.append((i,len(audio))))
    return bytes(audio),indices


def phonemes(engine,text):
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
        assert dll.eciAddTextUtf8(handle,payload,len(payload),0)
        assert dll.eciGeneratePhonemes(handle,len(buffer),buffer)
    finally:
        dll.eciSetParam(handle,0,0);dll.eciRegisterCallback(handle,engine._callback,None)
    return b''.join(chunks).decode('cp1252')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True);args=parser.parse_args()
    engine=DirectEciEngine(args.library.resolve(),dialect=0x40000,native_voice=True)
    checks=0
    try:
        engine.set_parameter(PROCESS_TAGS,1)
        cases=(('8:05','8 `00 Uhr 5'),('08:5 Uhr','8 `00 Uhr 5'),
               ('08.05 Uhr','8 `00 Uhr 5'),('01:00','1 Uhr'),
               ('01:05',"`[.1'aj'n] `00 Uhr 5"),
               ('08:05:03','8 `00 Uhr 5 und 3 Sekunden'),
               ('08:00:00 Uhr','8 Uhr und 0 Sekunden'),
               ('08:01:01','8 `00 Uhr 1 und eine Sekunde'))
        for rate in (1,3,2,4,5,6):
            engine.set_parameter(SAMPLE_RATE,rate)
            for clock,expanded in cases:
                assert render(engine,[clock,EciIndex(7)])==render(engine,[expanded,EciIndex(7)]),(rate,clock)
                checks+=1
            for number,expanded in (('1\u00a0234','1.234'),('1  234 567','1.234.567'),
                                    ('1\u00a0234,56 EUR','1.234,56 EUR')):
                assert render(engine,[number,EciIndex(7)])==render(engine,[expanded,EciIndex(7)]),(rate,number)
                checks+=1
        engine.set_parameter(SAMPLE_RATE,1)
        for clock,expanded in cases:
            assert phonemes(engine,clock)==phonemes(engine,expanded),clock
            checks+=1
        # Input positions on either side of the seconds colon map to the
        # equivalent native word boundaries in the expanded utterance.
        clock=render(engine,['08:',EciIndex(1),'05:',EciIndex(2),'03',EciIndex(3)])
        expanded=render(engine,['8',EciIndex(1),' `00 Uhr 5',EciIndex(2),' und 3 Sekunden',EciIndex(3)])
        assert clock==expanded,'clock fragment/index mapping';checks+=1
    finally:engine.close()
    print(f'German clocks and grouped numbers: {checks} speech, phoneme, rate and fragment/index comparisons; RAM only')

if __name__=='__main__':main()
