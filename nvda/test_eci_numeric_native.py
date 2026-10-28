"""Numeric normalization, index delivery and currency-equivalent PCM."""
from pathlib import Path
from _reloq_eci_direct import DirectEciEngine,EciIndex,GENERIC_RATE,GENERIC_VOLUME
ROOT=Path(__file__).resolve().parents[1]
def main():
    engine=DirectEciEngine(ROOT/'eci/libreloq_eci.so',experimental_generic=True,
        lexicon_path=ROOT/'data/german_apple_observed.rlex')
    checks=0
    try:
        for rate,volume in [(50,100),(80,100),(50,0)]:
            engine.set_parameter(GENERIC_RATE,rate);engine.set_parameter(GENERIC_VOLUME,volume)
            money=[]
            for text in ('21','101','21 Fragen und 101 Antworten.',
                         '3,14 EUR','3,14 Euro','3,14 €','3,14€','0,50 EUR',
                         '08.10.2026','8:05 Uhr','@','-5'):
                plain,marked,indices=[],[],[]
                assert engine.speak([text],plain.append,lambda _:None)
                assert engine.speak([text,EciIndex(91)],marked.append,indices.append)
                pcm=b''.join(marked)
                assert indices==[91] and b''.join(plain)==pcm
                if volume==0:assert not any(pcm)
                if text.startswith('3,14'):money.append(pcm)
                checks+=1
            assert all(pcm==money[0] for pcm in money)
    finally:engine.close()
    print(f'{checks} numeric ECI cases: identical indexed PCM; Euro spellings equal; speed and mute passed')
if __name__=='__main__':main()
