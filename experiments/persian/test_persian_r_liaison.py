"""Persian word-final R: following vowel, consonant and phrase boundaries."""
from __future__ import annotations
import argparse
from pathlib import Path
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from test_persian_final_r_connected import actual_sentence, r_frames


def unvoiced_r(rows):
    return [r for r in rows if r.controls[8]>=35 and r.controls[37]>=50 and
            30<=r.controls[38]<55 and r.controls[2]<=1 and r.controls[7]<=1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',required=True,type=Path)
    parser.add_argument('--previous',type=Path)
    args=parser.parse_args(); v,scope=voice(args.candidate,0x90000)
    old,old_scope=voice(args.previous,0x90000) if args.previous else (None,None)
    checks=0
    try:
        for gender in (0,1):
            for rate in (1,3,6):
                for speed in (40,98,170):
                    for engine in (v,old):
                        if engine:
                            for k,x in ((GENDER,gender),(SAMPLE_RATE,rate),(NATIVE_SPEED,speed)):engine.set_parameter(k,x)
                    for vowel in 'aAeiou':
                        tag='`[.1'+vowel+'r]'
                        for next_vowel in 'aAeiou':
                            linked=capture(v,scope,tag+' `[.1'+next_vowel+']',True)[1]
                            assert not r_frames(linked),(gender,rate,speed,tag,next_vowel,'final R before vowel')
                            checks+=1
                        for following in ('`[.1da]','`[.1pa]','`[.1ha]','`[.1hi]'):
                            connected=capture(v,scope,tag+' '+following,True)[1]
                            assert len(r_frames(connected))>=5,(gender,rate,speed,tag,following,'missing final R')
                            checks+=1
                        for boundary in ('.', ',', ':'):
                            after_boundary=capture(v,scope,tag+boundary+' `[.1a]',True)[1]
                            assert len(r_frames(after_boundary))>=5,(gender,rate,speed,tag,boundary,'lost R at boundary')
                            checks+=1
                    # F and voiced/unvoiced stops retain a purely unvoiced
                    # coda even before a vowel; its sound is not borrowed
                    # from the next word's initial vowel.
                    for consonant in 'fptkbdgvszSZhmnlGj':
                        tag='`[.1se'+consonant+'r]'
                        for next_word in ('`[.1da]','`[.1a]'):
                            rows=capture(v,scope,tag+' '+next_word,True)[1]
                            assert len(unvoiced_r(rows))>=3,(gender,rate,speed,consonant,next_word)
                            checks+=1
                    if old:
                        # Ordinary R and the accepted isolated/stop+R remain
                        # byte-identical to the preceding build.
                        for tag in ('`[.1ra]','`[.1i.0ra]','`[.1sefr]','`[.1sedr]','`[.1sehr]','`[.1ir]'):
                            assert capture(v,scope,tag,True)[0].pcm==capture(old,old_scope,tag,True)[0].pcm,(gender,rate,speed,tag)
                            checks+=1
        for text in ('تغییر است','تغییر آمد'):
            assert not r_frames(actual_sentence(args.candidate,text)),(text,'Brazilian R before vowel')
        for text in ('تغییر هوا','تغییر هست'):
            assert r_frames(actual_sentence(args.candidate,text)),(text,'lost final R before H')
        assert r_frames(actual_sentence(args.candidate,'تغییر در پیکربندیِ زبان'))
        assert r_frames(actual_sentence(args.candidate,'تغییر. است'))
        assert len(unvoiced_r(actual_sentence(args.candidate,'صفر است')))>=5
        assert len(unvoiced_r(actual_sentence(args.candidate,'قدر است')))>=5
        assert len(unvoiced_r(actual_sentence(args.candidate,'امر است')))>=5
        assert len(unvoiced_r(actual_sentence(args.candidate,'قصر است')))>=5
        for text in ('شهر است','قهر است'):
            assert len(unvoiced_r(actual_sentence(args.candidate,text)))>=5,(text,'missing unvoiced H-R')
        print(f'Passed {checks} Persian R liaison, cluster and sentence-boundary checks.')
    finally:
        v.close()
        if old:old.close()

if __name__=='__main__':main()
