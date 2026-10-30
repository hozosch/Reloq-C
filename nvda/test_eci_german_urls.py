"""German prose with intact URLs/identifiers, native word boundaries and played indices.

PCM is hashed in RAM. Native reference accent/group comparisons are in the
C ECI tests; these checks exercise the public UTF-8/tag/fragment transport.
"""
from pathlib import Path
import hashlib
from _reloq_eci_direct import DirectEciEngine, EciIndex, SAMPLE_RATE, PROCESS_TAGS, NATIVE_SPEED

ROOT=Path(__file__).resolve().parents[1]
HEISE=('Eric Röder, "„Sagt Euren Abgeordneten, dass das Nachschlagen von Namen keine Straftat ist”, '
       'ruft Quad9 Franzosen auf, zu handeln. „Das Internet muss offen und für jedermann frei zugänglich bleiben" '
       'Zensur-Urteil: DNS-Dienst Quad9 verlässt Frankreich | heise online '
       'https://www.heise.de/news/Zensur-Urteil-DNS-Dienst-Quad9-verlaesst-Frankreich-11497622.html, 1 week ago')

def capture(engine,parts):
    digest=hashlib.sha256();size=[0];indices=[]
    def pcm(block):size[0]+=len(block)//2;digest.update(block)
    assert engine.speak([*parts,EciIndex(19)],pcm,lambda n:indices.append((n,size[0])))
    assert size[0]>0 and indices==[(19,size[0])]
    return size[0],digest.hexdigest(),indices

def main():
    cases=['reloq-c-android-arm64-0.2.197-preview9.apk',
           'Das ist reloq-c-android-arm64-0.2.197-preview9.apk und das ist gut.',
           'SAPI5 berücksichtige ich bei der Vorbereitung des ersten Releases.',
           'USB3-Gerät: Windows11 und OpenGL4.6.',HEISE,'Das ist https://example.org/Datei-42.html, bitte lesen.',
           'Bitte http://example.net/DNS-Dienst-Quad9-123abc.html lesen.',
           'Ein- und Ausgang: www.example.org/Text-503.htm, danach lesen.',
           'Unicode: https://example.org/α-42.html und https://example.org/ä-50.html.',
           'Eine lange Adresse https://example.org/'+('segment-12/'*40)+'Seite-504.html, bitte lesen.']
    engine=DirectEciEngine(ROOT/'eci/libreloq_eci.so',dialect=0x40000,encoding='utf8',experimental_generic=True)
    checks=0
    try:
        engine.set_parameter(NATIVE_SPEED,98)
        for rate in (1,6):
            engine.set_parameter(SAMPLE_RATE,rate)
            for tags in (0,1):
                engine.set_parameter(PROCESS_TAGS,tags)
                for text in cases:
                    expected=capture(engine,[text]);checks+=1
                    for cut in sorted({1,len(text)//2,text.index('://')+2 if '://' in text else 5}):
                        assert capture(engine,[text[:cut],text[cut:]])==expected,(rate,tags,cut,text)
                        checks+=1
    finally:engine.close()
    print(f'German URLs/identifiers: {checks} full-text/tag/rate/fragment/index checks passed; no audio files')

if __name__=='__main__':main()
