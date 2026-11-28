"""Normal English /s,z/ before /r/ and connected post-/s/ t/p release."""
import argparse
from pathlib import Path

from _reloq_eci_direct import DirectEciEngine,EciIndex,NATIVE_SPEED,PROCESS_TAGS,SAMPLE_RATE
from test_eci_german_clock_native import phonemes,render

def engine(path,dialect):
    e=DirectEciEngine(path.resolve(),dialect=dialect,encoding='utf8',native_voice=True)
    e.set_parameter(PROCESS_TAGS,1)
    return e

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--library',type=Path,required=True)
    ap.add_argument('--reference',type=Path,required=True)
    ap.add_argument('--dictionary',type=Path)
    args=ap.parse_args()
    sr=('misrate','misread','Israeli','transracial','transrational','transreceiver',
        'transreal','transrhenane','transriverine','transregionate','Lebensraum',
        'disray','disrating','disranking','`[.1sri]','`[.1zri]')
    tp=('still','spill','speak')
    controls=('cat','cry','scat','scry','scout','school','ski','see','say','zero','zoo',
              'ask','this car','strain','spring','split','shrink','shrug','strange',
              '`[.1skhWt]','`[.1trAnsh.2krIpt]')
    checks=0
    for dialect in (0x10000,0x10001):
        current,before=engine(args.library,dialect),engine(args.reference,dialect)
        try:
            for text in sr+tp+controls:
                assert phonemes(current,text)==phonemes(before,text),(hex(dialect),text,'phonemes')
                checks+=1
            for rate in (1,3,6):
                for speed in (50,105,220,250):
                    for e in (current,before):
                        e.set_parameter(SAMPLE_RATE,rate);e.set_parameter(NATIVE_SPEED,speed)
                    for text in sr+tp+controls:
                        actual=render(current,[text,EciIndex(7)])
                        old=render(before,[text,EciIndex(7)])
                        assert len(actual[0])==len(old[0]) and actual[1]==old[1], \
                            (hex(dialect),rate,speed,text,'timing')
                        if text in controls:assert actual==old,(hex(dialect),rate,speed,text,'unrelated')
                        elif speed<=105:assert actual[0]!=old[0],(hex(dialect),rate,speed,text,'missing change')
                        checks+=1
            if args.dictionary and dialect==0x10000:
                for e in (current,before):e.set_dictionary_path(args.dictionary.resolve())
                for text in ('transrational','transreceiver','transrhenane','transriverine','Lebensraum'):
                    actual=render(current,[text,EciIndex(7)]);old=render(before,[text,EciIndex(7)])
                    assert actual[0]!=old[0] and len(actual[0])==len(old[0]) and actual[1]==old[1],text
                    checks+=1
                for text in ('transcript','transcranial','discretization'):
                    assert render(current,[text])==render(before,[text]),text
                    checks+=1
        finally:current.close();before.close()
    for dialect,text in ((0x40000,'Du wachst, wenn ihr wachst.'),
                         (0x120000,'Καλημέρα σας!')):
        current,before=engine(args.library,dialect),engine(args.reference,dialect)
        try:
            assert render(current,[text])==render(before,[text]),hex(dialect)
            checks+=1
        finally:current.close();before.close()
    print('English normal s/z and connected post-s t/p:',checks,'speech, phoneme, timing and dictionary checks')

if __name__=='__main__':main()
