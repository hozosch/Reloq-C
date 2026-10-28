"""Voice/model switching and control extremes through the shipped ECI API."""
from pathlib import Path
import hashlib
import itertools
import sys
import tempfile
from _reloq_eci_direct import (DirectEciEngine, EciIndex, GENDER, HEAD_SIZE,
    ROUGHNESS, BREATHINESS, NATIVE_SPEED, NATIVE_PITCH, NATIVE_INFLECTION, NATIVE_VOLUME,
    SAMPLE_RATE)
from _reloq_languages import GERMAN, LanguageEngine
ROOT=Path(__file__).resolve().parents[1]

def main():
    sys.path.insert(0,str(ROOT/'addon'))
    from build_synth import merged_german_lexicon
    with tempfile.TemporaryDirectory() as directory:
        folder=Path(directory)
        (folder/'eci.dll').symlink_to(ROOT/'eci/libreloq_eci.so')
        lex=folder/GERMAN.lexicon;lex.write_bytes(merged_german_lexicon())
        direct=DirectEciEngine(folder/'eci.dll',experimental_generic=True,lexicon_path=lex)
        routed=LanguageEngine(folder/'eci.dll')
        def render(engine,parts):
            chunks=[];indices=[]
            assert engine.speak(parts,chunks.append,indices.append),parts
            result=b''.join(chunks)
            assert result and len(result)%2==0
            assert indices==[7]
            return result
        direct.set_parameter(NATIVE_SPEED,98)
        routed.set_parameter(NATIVE_SPEED,98)
        hashes=set();checks=0
        texts=['Wortanalyse. Ordners. Programms.', 'Quelle. Chance. Hören Sie mich?', 'Fünfundzwanzig und sechsundvierzig.']
        for ident in ['1','2','3','4','5','6','7','8','2','1']:
            routed.select_voice(GERMAN.id,ident)
            for param,value in GERMAN.voice(ident).parameters:direct.set_parameter(param,value)
            for text in texts:
                expected=render(direct,[text,EciIndex(7)])
                actual=render(routed,[text,EciIndex(7)])
                assert actual==expected,(ident,text)
                hashes.add(hashlib.sha256(actual).digest());checks+=1
        assert len(hashes)>=8
        for g,h,b,r in itertools.product((0,1),(0,22,30,40,100),(0,50,100),(0,100)):
            for param,value in ((GENDER,g),(HEAD_SIZE,h),(BREATHINESS,b),(ROUGHNESS,r)):
                direct.set_parameter(param,value)
            text='Mann. Wortanalyse, Ordners und Programms. Neu starten.'
            pcm=render(direct,[text,EciIndex(7)])
            split=render(direct,[text[:17],text[17:],EciIndex(7)])
            assert pcm==split,(g,h,b,r)
            checks+=1
        # A used session must converge to a fresh one regardless of which
        # head size was active when its rate/model defaults were created.
        for code,g,h in itertools.product((0,1,3,6),(0,1),(22,29,30,40,50)):
            fresh=DirectEciEngine(folder/'eci.dll',experimental_generic=True,lexicon_path=lex)
            try:
                fresh.set_parameter(HEAD_SIZE,h)
                for engine in (fresh,direct):
                    for param,value in ((SAMPLE_RATE,code),(GENDER,g),
                            (BREATHINESS,60),(ROUGHNESS,0),(NATIVE_SPEED,98),
                            (NATIVE_PITCH,65),(NATIVE_INFLECTION,50),(NATIVE_VOLUME,50)):
                        engine.set_parameter(param,value)
                direct.set_parameter(HEAD_SIZE,100 if h<=40 else 22)
                render(direct,['Sie sehen eine Meldung.',EciIndex(7)])
                direct.set_parameter(HEAD_SIZE,h)
                parts=['Sie sehen eine graue Meldung.',EciIndex(7)]
                assert render(direct,parts)==render(fresh,parts),(code,g,h)
                checks+=1
            finally:
                fresh.close()
        # The ABI rejects bad values without changing the current model.
        for param,valid,invalid in ((GENDER,0,2),(HEAD_SIZE,50,101),
                                    (ROUGHNESS,0,-1),(BREATHINESS,0,101)):
            direct.set_parameter(param,valid)
            assert direct._dll.eciSetParam(direct._handle,param,invalid)==-1
            assert direct._dll.eciGetParam(direct._handle,param)==valid
        direct.close();routed.close()
    print(f'Voice models: {checks} native preset/extreme/fragment checks; switch-back and parameter rejection passed')

if __name__=='__main__':main()
