"""Release integration: plain text, mid-token indices and NVDA C transport."""
from pathlib import Path
import sys,tempfile
from _reloq_eci_direct import DirectEciEngine,EciIndex,GENERIC_RATE,GENERIC_VOLUME
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'addon'))
from build_synth import merged_german_lexicon

def main():
    texts=["Gesagt.","Ich habe es gesagt.","Sie hat es mir gestern gesagt.",
        "Gefragt, gelebt und geregt.","Veränderung und Verarbeitung.",
        "Das ist unauffällig.","Zwei Eier und ein Apfel.","Die ist gut.",
        "Ein alter und ein neuer Anfang.","Auch ich gehe heute nach Hause.",
        "Sechs Menschen sprechen leise.","Ich möchte das richtig aussprechen.",
        "Die Garage ist offen.","Eine Liste mit gesicherten Informationen.",
        "Tempo und Konsonantenkluster.","Hören und höher.","Hans' Haus.",
        "Wie geht’s?", "Ge'sagt.","' Gesagt '","Gesagt\u00ad und getan.",
        "Gesagt\u00a0und getan.","8:05 Uhr, 8:00 Uhr und 08:05:30 Uhr.",
        "1 234 und 1.234 Euro.","3,14 EUR und 3,14 €.","A, B und C."]
    checks=0
    with tempfile.TemporaryDirectory() as tmp:
        lex=Path(tmp)/'german.rlex';lex.write_bytes(merged_german_lexicon())
        engine=DirectEciEngine(ROOT/'eci/libreloq_eci.so',experimental_generic=True,lexicon_path=lex)
        try:
            for rate,volume in [(50,100),(80,100),(50,0)]:
                engine.set_parameter(GENERIC_RATE,rate);engine.set_parameter(GENERIC_VOLUME,volume)
                for text in texts:
                    a,b,indices=[],[],[]
                    assert engine.speak([text],a.append,lambda _:None),text
                    cut=min(2,len(text))
                    assert engine.speak([text[:cut],EciIndex(31),text[cut:],EciIndex(32)],b.append,indices.append),text
                    pcm=b''.join(a)
                    assert pcm and pcm==b''.join(b),(text,'indexed PCM')
                    assert indices==[31,32],(text,indices)
                    assert (not any(pcm)) if volume==0 else any(pcm),text
                    checks+=1
        finally:engine.close()
    print(f'{checks} release ECI cases: plain/indexed PCM, mid-token indices, fast speech and mute passed')
if __name__=='__main__':main()
