"""Check the reported metallic R onset without changing accepted contexts."""
from __future__ import annotations

import argparse
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'nvda'))
from _reloq_eci_direct import GENDER, HEAD_SIZE, NATIVE_SPEED, ORIGINAL_ACOUSTIC_CLOCK, SAMPLE_RATE
from test_persian_r_after_i_contexts import at_fraction, capture, middle
from test_us_articulation import voice


CASES = (
    ('باری', '`[.1ba.0ri]', 2, 1, 3),
    ('بردباری', '`[.0bord.0ba.1ri]', 6, 5, 7),
    ('بختیاری‌زاده', "`[.0bAG.0ti.0j'a:'.0ri.0za.1d'e:']", 7, 6, 8),
    ('زیرخط', '`[.1zir.0GAt]', 2, 1, 4),
    ('تیرخط', '`[.1tir.0GAt]', 2, 1, 4),
    ('Anlaut ri', '`[.1ri]', 0, None, 1),
)
CONTROLS = ("`[.0ti.1r'e:']", '`[.1ir]', '`[.1ra]',
            '`[.1ir] `[.1GAt]', '`[.1i] `[.1rGAt]')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',required=True,type=Path)
    parser.add_argument('--previous',type=Path)
    args=parser.parse_args()
    current,scope=voice(args.library,0x90000)
    old,old_scope=voice(args.previous,0x90000) if args.previous else (None,None)
    checks=0
    try:
        for rate in (1,3,6):
            for gender,head in ((0,50),(1,50),(1,20)):
                for speed,clock in ((40,0),(65,0),(82,0),(82,1),(100,0)):
                    for engine in (current,old):
                        if engine:
                            for key,value in ((SAMPLE_RATE,rate),(GENDER,gender),(HEAD_SIZE,head),
                                              (NATIVE_SPEED,speed),(ORIGINAL_ACOUSTIC_CLOCK,clock)):
                                engine.set_parameter(key,value)
                    for name,tag,r,before_vowel,after_vowel in CASES:
                        now=capture(current,scope,tag)
                        f3=at_fraction(now,r,0,15);f4=at_fraction(now,r,0,17)
                        # Gender/head-size mapping can compress this gap;
                        # require ordered separation after that mapping too.
                        assert f4-f3>=100,(name,rate,gender,head,speed,clock,f3,f4)
                        if old:
                            prior=capture(old,old_scope,tag)
                            assert now.syllables==prior.syllables,(name,'duration changed')
                            if now.pcm == prior.pcm:
                                # Some speaker/head-size combinations already
                                # have enough separation in the native targets.
                                assert f4-f3>=200,(name,rate,gender,head,speed,clock,'unprotected overlap')
                                continue
                            for index in (before_vowel,after_vowel):
                                if index is None:
                                    continue
                                start,end=now.syllables[index]
                                # A 3 ms vowel may have no captured 5 ms
                                # frame strictly inside its centre.
                                if end-start<50000:
                                    continue
                                assert middle(now,index,(9,13,15,17))==middle(prior,index,(9,13,15,17)), (
                                    name,index,'vowel centre changed')
                            assert at_fraction(now,r,0,17)>at_fraction(prior,r,0,17), (name,'R onset F4')
                        checks+=1
                    if old:
                        for tag in CONTROLS:
                            a,b=capture(current,scope,tag),capture(old,old_scope,tag)
                            assert a.pcm==b.pcm and a.frame_bytes==b.frame_bytes,(tag,rate,gender,head,speed,clock)
                        a,b=capture(current,scope,CASES[0][1],False),capture(old,old_scope,CASES[0][1],False)
                        assert a.pcm==b.pcm and a.frame_bytes==b.frame_bytes
    finally:
        current.close()
        if old:old.close()
    print(f'Passed {checks} R resonance separation and context checks.')


if __name__=='__main__':
    main()
