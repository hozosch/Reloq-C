"""Keep German text and its explicit ECI phonemes aligned in the native path."""
import argparse
from pathlib import Path
from _reloq_eci_direct import DirectEciEngine,EciIndex,PROCESS_TAGS,SAMPLE_RATE
from test_eci_german_clock_native import phonemes,render


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True);args=parser.parse_args()
    engine=DirectEciEngine(args.library.resolve(),dialect=0x40000,native_voice=True)
    words=('See','Sah','Tag','Miete','Höhle','früher','Chance','Restaurant',
           'lower','Auto','Kino','Radio','Foto','Eloquence','Thermometer',
           'Sonnenblume','Fahrrad','Hauptbahnhof','Landstraße','Wortbetonung',
           'Sprachaufnahme','Januar','Sonnabend','wohlauf','Gabriel','Israel')
    count=0
    try:
        engine.set_parameter(PROCESS_TAGS,1)
        for rate in (1,3,2,4,5,6):
            engine.set_parameter(SAMPLE_RATE,rate)
            for word in words:
                spr=phonemes(engine,word)
                original=render(engine,[word,EciIndex(7)])
                explicit=render(engine,[spr,EciIndex(7)])
                assert original==explicit,(rate,word,spr,len(original[0]),len(explicit[0]))
                count+=1
    finally:engine.close()
    print(f'German native phonetics: {count} text/SPR PCM, length and index round trips; RAM only')

if __name__=='__main__':main()
