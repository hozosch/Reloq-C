"""Native analysis controls, source character names and scope isolation.

PCM and indices are compared in memory. No audio files are written.
"""
from pathlib import Path
import argparse,hashlib,sys,tempfile
from _reloq_eci_direct import DirectEciEngine,EciText,EciIndex,DICTIONARY,PHRASE_PREDICTION,PROCESS_TAGS,NATIVE_SPEED,SAMPLE_RATE
from _reloq_languages import LANGUAGES
ROOT=Path(__file__).resolve().parents[1]

GERMAN=("Das ist ein Test der eine längere Wortgruppe ohne Satzzeichen enthält und danach "
        "noch viel mehr Wörter hinzufügt damit die automatische Satzvorhersage Grenzen finden kann.")
ENGLISH="The man who went to the station yesterday took a train to the city and then went home again."


def capture(engine,parts):
    digest=hashlib.sha256();size=[0];marks=[]
    def pcm(data):digest.update(data);size[0]+=len(data)
    assert engine.speak([*parts,EciIndex(31)],pcm,lambda i:marks.append((i,size[0])))
    assert size[0] and marks==[(31,size[0])]
    return size[0],digest.hexdigest(),marks


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path,default=ROOT/'eci/libreloq_eci.so')
    p.add_argument('--baseline',type=Path)
    a=p.parse_args();checks=0
    with tempfile.TemporaryDirectory() as folder:
        def engine(language,library=None):
            return DirectEciEngine((library or a.library).resolve(),dialect=language,encoding='utf8',
                experimental_generic=True,lexicon_path=None)
        for language in LANGUAGES.values():
            e=engine(language.id)
            try:
                for parameter in (DICTIONARY,PHRASE_PREDICTION):
                    assert e._dll.eciGetParam(e._handle,parameter)==0;checks+=1
                    for value in (-1,2):
                        assert e._dll.eciSetParam(e._handle,parameter,value)==-1;checks+=1
                e.set_parameter(PROCESS_TAGS,1)
                text='Dr. Müller kommt heute.' if language.id==0x40000 else 'Dr. Smith works at St. James with Mrs. Jones.'
                original=capture(e,[text])
                e.set_parameter(DICTIONARY,1);disabled=capture(e,[text]);e.set_parameter(DICTIONARY,0)
                if language.id not in (0x60000,0x80000,0xa0000,0x120000):
                    assert capture(e,['`da0 '+text])==disabled,(language.name,'dictionary flag/tag');checks+=1
                assert capture(e,[text])==original,(language.name,'dictionary restore');checks+=1
                sentence=GERMAN if language.id==0x40000 else ENGLISH
                e.set_parameter(PHRASE_PREDICTION,1);predicted=capture(e,[sentence]);e.set_parameter(PHRASE_PREDICTION,0)
                if language.id not in (0x60000,0x80000,0xa0000,0x120000):
                    assert capture(e,['`pp1 '+sentence])==predicted,(language.name,'prediction flag/tag');checks+=1
                assert e._dll.eciGetParam(e._handle,PHRASE_PREDICTION)==0;checks+=1
                if language.id in (0x40000,0x10000):
                    assert original!=disabled,(language.name,'abbreviation effect');checks+=1
                    assert predicted!=capture(e,[sentence]),(language.name,'prediction effect');checks+=1
                    expected=capture(e,['`pp1 '+sentence])
                    for cut in (1,3,len(sentence)//2):
                        text='`pp1 '+sentence
                        assert capture(e,[text[:cut],text[cut:]])==expected,(language.name,cut);checks+=1
                if a.baseline and language.id!=0x40000:
                    old=engine(language.id,a.baseline)
                    try:
                        old.set_parameter(PROCESS_TAGS,1)
                        for text in (language.locale.replace('_',' ')+' 503 4004.',sentence):
                            assert capture(e,[text])==capture(old,[text]),(language.name,'unchanged default',text);checks+=1
                    finally:old.close()
            finally:e.close()
        # Exercise the packaged path: no supplemental German RLEX file.
        e=engine(0x40000)
        try:
            for tags in (0,1):
                e.set_parameter(PROCESS_TAGS,tags)
                for abbreviation in ('Dr.','z. B.','d. h.','u. s. w.','z. Z.','usw.','km','kg','min','cm','bzw.'):
                    text='Das ist '+abbreviation+' gut.'
                    e.set_parameter(DICTIONARY,0);enabled=capture(e,[text])
                    e.set_parameter(DICTIONARY,1);disabled=capture(e,[text])
                    assert enabled!=disabled,(tags,abbreviation,'dictionary must affect actual speech');checks+=1
                    e.set_parameter(DICTIONARY,0)
                    assert capture(e,[text])==enabled;checks+=1
            e.set_parameter(PROCESS_TAGS,1)
            for rate in (1,6):
                e.set_parameter(SAMPLE_RATE,rate)
                if a.baseline:
                    old=engine(0x40000,a.baseline)
                    try:
                        old.set_parameter(SAMPLE_RATE,rate);old.set_parameter(PROCESS_TAGS,1)
                        for letter in ('Å','å'):
                            assert capture(e,[EciText(letter,True)])==capture(old,[EciText(letter,True)]);checks+=1
                    finally:old.close()
                assert capture(e,['°.'])==capture(e,['Grad.']);checks+=1
                for symbol in ('°','_'):
                    assert capture(e,[EciText(symbol,True),'.'])==capture(e,['`ts2 '+symbol+'`ts0.']);checks+=1
                    if a.baseline:
                        old=engine(0x40000,a.baseline)
                        try:
                            old.set_parameter(SAMPLE_RATE,rate)
                            if old.native_version.startswith('0.2.192.'):
                                assert capture(e,[symbol+'.'])!=capture(old,[symbol+'.']);checks+=1
                        finally:old.close()
        finally:e.close()
        # Main/Root volumes are independent of abbreviation expansion.
        dictionaries=Path(folder)/'dictionaries';dictionaries.mkdir()
        (dictionaries/'ENUmain.dic').write_bytes(b'cancellationtoken\tbanana\n')
        (dictionaries/'DEUmain.dic').write_bytes(b'TokenX\tbei\n')
        for language,text in [(0x10000,'This is cancellationtoken.'),(0x40000,'Ich bin TokenX dir.')]:
            e=engine(language)
            try:
                e.set_dictionary_path(dictionaries);expected=capture(e,[text])
                e.set_parameter(DICTIONARY,1)
                assert capture(e,[text])==expected,(hex(language),'Main dictionary retained');checks+=1
            finally:e.close()
        # A second engine must keep its own options on the same owner thread.
        e=engine(0x40000);other=engine(0x40000)
        try:
            expected=capture(other,[GERMAN]);e.set_parameter(PHRASE_PREDICTION,1)
            assert capture(e,[GERMAN])!=expected;checks+=1
            assert capture(other,[GERMAN])==expected;checks+=1
            e.set_parameter(PHRASE_PREDICTION,0)
            assert capture(e,[GERMAN])==expected;checks+=1
        finally:e.close();other.close()
    print(f'Analysis options: {checks} native setting/tag/scope/fragment/index/character comparisons passed; PCM in memory')


if __name__=='__main__':main()
