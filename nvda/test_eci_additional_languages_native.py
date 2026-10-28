"""Native new-language rendering, source marks, models and language routing.

PCM is inspected only in memory. No speech-result or listening files.
"""
from pathlib import Path
import hashlib,sys
from _reloq_eci_direct import (DirectEciEngine,EciText,EciIndex,EciLanguage,
    SAMPLE_RATE,SAMPLE_RATES,NATIVE_SPEED,PAUSE_MODE,HEAD_SIZE,GENDER,BREATHINESS)
from _reloq_languages import LANGUAGES,LanguageEngine,language_for

R=Path(__file__).resolve().parents[1]
DLL=R/'eci/libreloq_eci.so'
CASES={0x70000:['Olá, São Paulo!','503 e 4004, depois 10004.',
    'A informação (nova) - fácil/rápida.','Sa\u0303o Paulo: família, ação e avó.'],
0x90000:['Hyvää päivää!','503 ja 4004, sitten 10004.',
    'Avaa (tiedosto) - Helsinki/Turku.','A\u0308äni, yö ja käyttäjät.']}
CASES.update({
0x10001:['Good morning, London!','503 and 4004, then 10004.',
    'Open (the file) - continue.','The community uses a café.'],
0x20000:['¡Buenos días, España!','503 y 4004, después 10004.',
    'Abra (el archivo) - continúe.','El pingüino oye la canción.'],
0x20001:['¡Buenos días, México!','503 y 4004, después 10004.',
    'Abra (el archivo) - continúe.','La información está disponible.'],
0x30000:['Bonjour, ça va bien !','503 et 4004, puis 10004.',
    'Ouvrez (le fichier) - continuez.','Une communauté française au café.'],
0x30001:['Bonjour, Québec !','503 et 4004, puis 10004.',
    'Ouvrez (le fichier) - continuez.','La communauté utilise un téléphone.'],
0x50000:['Buongiorno, come stai?','503 e 4004, poi 10004.',
    'Apri (il file) - continua.','La comunità usa il telefono.']})

def capture(engine,parts):
    pcm=[];marks=[];position=0
    def take(block):
        nonlocal position
        position+=len(block)//2;pcm.append(block)
    assert engine.speak(parts,take,lambda n:marks.append((n,position)))
    data=b''.join(pcm)
    assert data and any(data)and marks[-1]==(7,len(data)//2)
    return data,marks

def main():
    count=0;different=set()
    for lang,texts in CASES.items():
        assert language_for(LANGUAGES[lang].locale,0x40000)==lang
        engine=DirectEciEngine(DLL,dialect=lang,encoding='utf-8',experimental_generic=True)
        try:
            for code in SAMPLE_RATES:
                engine.set_parameter(SAMPLE_RATE,code)
                for voice in LANGUAGES[lang].voices:
                    for key,value in [*voice.parameters,(NATIVE_SPEED,98)]:
                        engine.set_parameter(key,value)
                    for mode in [0,1,2]:
                        engine.set_parameter(PAUSE_MODE,mode)
                        for text in texts:
                            try:data,marks=capture(engine,[EciText(text,False),EciIndex(7)])
                            except Exception as error:
                                raise AssertionError((hex(lang),code,voice.id,mode,text))from error
                            different.add((lang,code,hashlib.sha256(data).hexdigest()))
                            count+=1
            engine.set_parameter(SAMPLE_RATE,1);engine.set_parameter(PAUSE_MODE,0)
            engine.set_parameter(GENDER,0);engine.set_parameter(HEAD_SIZE,50);engine.set_parameter(BREATHINESS,0)
            import unicodedata
            canonical=texts[0]
            decomposed=unicodedata.normalize('NFD',canonical)
            assert capture(engine,[canonical,EciIndex(7)])==capture(engine,[decomposed,EciIndex(7)])
            assert capture(engine,[EciText('a+@',True),EciIndex(7)])[0]
        finally:engine.close()
        print('New language',hex(lang),'models/rates/pauses passed',flush=True)
    assert len(different)>100
    # One owner switches both new modules and returns to its home module.
    engine=LanguageEngine(DLL,language=0x70000)
    try:
        data,marks=capture(engine,['Olá. ',EciLanguage(0x90000,'1'),'Hyvää päivää.',EciIndex(7)])
        assert data and len(marks)==1
        assert capture(engine,['Olá.',EciIndex(7)])[0]
    finally:engine.close()
    print(count,'native new-language cases, composed accents, native characters and switching passed',flush=True)

if __name__=='__main__':main()
