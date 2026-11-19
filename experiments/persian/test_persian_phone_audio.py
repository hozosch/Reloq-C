"""Compare structured-phone output with the established Persian acoustics."""
from __future__ import annotations
import argparse
from pathlib import Path
from test_us_articulation import capture, voice
from _reloq_eci_direct import GENDER, NATIVE_SPEED, SAMPLE_RATE
from test_persian_phone_inventory import decode, encode, install


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--candidate',required=True,type=Path)
    parser.add_argument('--reference',type=Path,help='optional previous complete ECI library')
    args=parser.parse_args()
    candidate, scope=voice(args.candidate,0x90000)
    reference, old_scope=voice(args.reference,0x90000) if args.reference else (candidate,scope)
    install(candidate._dll)
    tags=('`[.0sA.1lam]', "`[.1'i:'.0r'a:'n]", '`[.1solhw]', '`[.1hAft]',
          '`[.1Ri]', '`[.1Ga]', '`[.1Ca]', '`[.1Ja]', '`[.1Si]', '`[.1zi]',
          '`[.1pa]', '`[.1ja]')
    checks=0
    try:
        # The parser accepts and produces audio for each current input symbol.
        for phone in 'AaeioupbtdkgRwfvszSZGhCJmnlrj':
            tag='`[.1'+phone+('i' if phone not in 'Aaeiou' else '')+']'
            assert tag in candidate._phonemes_for_probe(tag),tag
            capture(candidate,scope,tag,True)
        for rate in (1,3,6):
            for gender in (0,1):
                for engine in (candidate,reference):
                    engine.set_parameter(SAMPLE_RATE,rate)
                    engine.set_parameter(GENDER,gender)
                    engine.set_parameter(NATIVE_SPEED,98)
                for tag in tags:
                    rebuilt=encode(candidate._dll,decode(candidate._dll,tag))
                    assert rebuilt==tag
                    before,_=capture(reference,old_scope,tag,True)
                    after,_=capture(candidate,scope,rebuilt,True)
                    assert before.pcm==after.pcm,(rate,gender,tag)
                    checks+=1
    finally:
        if reference is not candidate:reference.close()
        candidate.close()
    print(f'Passed 29 parser symbols and {checks} exact PCM comparisons '
          f'{"against the previous library" if args.reference else "within the candidate"}.')

if __name__=='__main__':main()
