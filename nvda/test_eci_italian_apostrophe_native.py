"""Compare Italian elisions against the original ECI and exercise recovery."""
import argparse
import json
from pathlib import Path
import re
import tempfile

from _reloq_eci_direct import DirectEciEngine, EciIndex
from test_eci_german_clock_native import render


def clean(phonemes):
    return re.sub(r'`vv\d+\s*', '', phonemes).replace('\0','').strip()


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    args=parser.parse_args()
    reference=json.loads((Path(__file__).resolve().parents[1]/
        'tests/data/original_italian_apostrophes.json').read_text(encoding='utf8'))
    engine=DirectEciEngine(args.library.resolve(),dialect=0x50000,
                           encoding='utf8',native_voice=True)
    checks=0
    try:
        for case in reference['cases']:
            actual=clean(engine._phonemes_for_probe(case['text']))
            assert actual==case['phonemes'],(case['text'],actual,case['phonemes'])
            pcm,_=render(engine,[case['text']])
            assert len(pcm)>5000,(case['text'],len(pcm))
            checks+=1
        for left,right in (("po'",'po\u2019'),("Aspetta un po'",'Aspetta un po\u2019'),
                           ("un po' di latte",'un po\u2019 di latte')):
            assert render(engine,[left])==render(engine,[right]),(left,right)
            checks+=1
        # Markers on each side of the original word remain ordered and the
        # following word is still synthesized.
        a=render(engine,['Aspetta un ',EciIndex(1),"po'",EciIndex(2),' di latte'])
        b=render(engine,['Aspetta un ',EciIndex(1),'po\u2019',EciIndex(2),' di latte'])
        assert a==b and [x[0] for x in a[1]]==[1,2] and a[1][0][1]<=a[1][1][1]<len(a[0])
        checks+=1
        with tempfile.TemporaryDirectory() as temporary:
            directory=Path(temporary)
            (directory/'ITAmain.dic').write_bytes(b"po'\tciao\n")
            engine.set_dictionary_path(directory)
            assert clean(engine._phonemes_for_probe("po'"))==clean(engine._phonemes_for_probe('ciao'))
            checks+=1
            engine.set_dictionary_path(None)
            assert clean(engine._phonemes_for_probe("po'"))=="`[.1po]"
            checks+=1
        assert render(engine,['Un esempio normale.'])[0]
        checks+=1
    finally:
        engine.close()
    print(f'Italian apostrophes: {checks} original pronunciation, audio, dictionary and index checks passed')


if __name__=='__main__':main()
