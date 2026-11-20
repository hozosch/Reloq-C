"""Check uppercase currency magnitude in both English native voices."""
import argparse
from pathlib import Path
from _reloq_eci_direct import DirectEciEngine, EciIndex, NATIVE_SPEED, PROCESS_TAGS, SAMPLE_RATE


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',required=True,type=Path)
    args=parser.parse_args()
    checks=0
    for dialect in (0x10000,0x10001):
        engine=DirectEciEngine(args.library.resolve(),dialect=dialect,encoding='utf8',native_voice=True)
        try:
            engine.set_parameter(PROCESS_TAGS,1)
            for rate in (1,3,6):
                engine.set_parameter(SAMPLE_RATE,rate)
                for speed in (50,98):
                    engine.set_parameter(NATIVE_SPEED,speed)
                    def render(parts):
                        pcm=bytearray();marks=[]
                        assert engine.speak(parts,pcm.extend,lambda index:marks.append((index,len(pcm))))
                        return bytes(pcm),marks
                    for upper,lower in (('$2B','$2b'),('$2.5B','$2.5b'),
                                        ('$1,234B','$1,234b'),('£2B','£2b'),('€2B','€2b')):
                        a=render([upper,EciIndex(42)])
                        b=render([lower,EciIndex(42)])
                        assert a==b and a[0] and a[1]==[(42,len(a[0]))],(dialect,rate,speed,upper)
                        checks+=1
            engine.set_parameter(SAMPLE_RATE,1)
            assert engine._phonemes_for_probe('2B') != engine._phonemes_for_probe('$2B')
            full=render(['Cost $2B today.',EciIndex(9)])
            fragmented=render(['Cost $2',EciIndex(7),'B today.',EciIndex(9)])
            assert fragmented[0]==full[0] and fragmented[1][-1]==full[1][-1]
            assert fragmented[1][0][0]==7 and fragmented[1][0][1]<=len(full[0])
        finally:
            engine.close()
    print(f'Passed {checks} currency PCM and index comparisons in US and UK English.')


if __name__=='__main__':
    main()
