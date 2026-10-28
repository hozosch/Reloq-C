"""Unicode-native Asian/Greek models, word reading and transport; PCM in RAM."""
from pathlib import Path
import hashlib,json,sys,unicodedata
from _reloq_eci_direct import (DirectEciEngine,EciText,EciIndex,EciLanguage,
    SAMPLE_RATE,SAMPLE_RATES,PAUSE_MODE,NATIVE_SPEED,PROCESS_TAGS,GENDER,HEAD_SIZE,BREATHINESS)
from _reloq_languages import LANGUAGES,LANGUAGE_ALIASES,LanguageEngine,language_for
R=Path(__file__).resolve().parents[1];H=R/'reference-apple/behavior-0.2.150'
CASES={0x80000:['こんにちは。Windowsとcomputerを使います。','503と4004です。'],
0xa0000:['한국어를 사용합니다. 한자 漢字.','503과 4004입니다.'],
0x60000:['我是中国人，今天很好。重庆银行欢迎你。','503和4004，还有10004。'],
0x60001:['我是中國人，今天很好。重慶銀行歡迎你。','503和4004，還有10004。'],
0x120000:['Καλημέρα σας! clock shop Nevada why computer.','503 και 4004, μετά 10004.']}
def capture(engine,parts):
    blocks=[];indices=[];samples=0
    def take(b):
        nonlocal samples
        samples+=len(b)//2;blocks.append(b)
    assert engine.speak(parts,take,lambda i:indices.append((i,samples)))
    data=b''.join(blocks);assert data and any(data)
    assert indices[-1]==(7,len(data)//2),indices
    return data,indices

def main():
    rows=[];pause_rows=[]
    for language,texts in CASES.items():
        e=DirectEciEngine(R/'eci/libreloq_eci.so',dialect=language,encoding='utf-8',experimental_generic=True)
        try:
            profile = LANGUAGES[LANGUAGE_ALIASES.get(language,language)]
            assert language_for("zh_TW" if language==0x60001 else profile.locale,0x40000)==profile.id
            for rate in SAMPLE_RATES:
                e.set_parameter(SAMPLE_RATE,rate)
                for voice in profile.voices:
                    for key,value in [*voice.parameters,(NATIVE_SPEED,98)]:e.set_parameter(key,value)
                    for mode in [0,1,2]:
                        e.set_parameter(PAUSE_MODE,mode)
                        for text in texts:
                            try:data,ix=capture(e,[EciText(text),EciIndex(7)])
                            except Exception as error:raise AssertionError((hex(language),rate,voice.id,mode,text)) from error
                            rows.append(dict(language=language,rate=rate,voice=voice.id,pause_mode=mode,
                                text=text,samples=len(data)//2,sha256=hashlib.sha256(data).hexdigest()))
            for key,value in [*profile.voices[0].parameters,(NATIVE_SPEED,98),(SAMPLE_RATE,1),(PAUSE_MODE,0)]:e.set_parameter(key,value)
            text=texts[0];expected=capture(e,[text,EciIndex(7)])
            assert expected==capture(e,[unicodedata.normalize('NFD',text),EciIndex(7)]),(language,'canonical composition')
            middle=len(text)//2
            assert expected==capture(e,[text[:middle],text[middle:],EciIndex(7)]),(language,'fragmented input')
            assert expected==capture(e,[text,EciIndex(7)]),(language,'repeat without speech cache')
            capture(e,[EciText('α+@'if language==0x120000 else'あ+@'if language==0x80000 else'ㄱ+@'if language==0xa0000 else'中+@',True),EciIndex(7)])
            for rate in SAMPLE_RATES:
                e.set_parameter(SAMPLE_RATE,rate)
                for speed in [50,98,150]:
                    e.set_parameter(NATIVE_SPEED,speed);lengths=[];explicit=[]
                    for mode in [0,1,2]:
                        e.set_parameter(PAUSE_MODE,mode)
                        lengths.append(len(capture(e,[texts[0],EciIndex(7)])[0])//2)
                        e.set_parameter(PROCESS_TAGS,1)
                        explicit.append(len(capture(e,[texts[0]+' `p250 ',EciIndex(7)])[0])//2)
                        e.set_parameter(PROCESS_TAGS,0)
                    assert lengths[0]>lengths[2],(language,rate,speed,lengths)
                    assert lengths[0]>=lengths[1]>=lengths[2],(language,rate,speed,lengths)
                    # The explicitly requested tail cannot be removed by the
                    # final-only policy. Always mode may shorten earlier punctuation.
                    assert explicit[0]==explicit[1],(language,rate,speed,explicit)
                    pause_rows.append(dict(language=language,rate=rate,speed=speed,lengths=lengths,explicit=explicit))
        finally:e.close()
        print('Asian/Greek models, Unicode and pauses passed',hex(language),flush=True)
    assert language_for('en-US',0x120000)==0x120000
    engine=LanguageEngine(R/'eci/libreloq_eci.so',language=0x40000)
    try:
        parts=['Deutsch. ']
        for lang,texts in CASES.items():parts.extend([EciLanguage(lang,'1'),texts[0]])
        parts.extend([EciLanguage(0x40000,'1'),'Wieder deutsch.',EciIndex(7)])
        capture(engine,parts)
    finally:engine.close()
    report=dict(cases=len(rows),pause_cases=len(pause_rows),no_audio_files=True,
        eci_so_sha256=hashlib.sha256((R/'eci/libreloq_eci.so').read_bytes()).hexdigest(),rows=rows,pauses=pause_rows)
    (H/'asian-greek-native150.json').write_text(json.dumps(report,ensure_ascii=False,separators=(',',':'))+'\n')
    print(len(rows),'Asian/Greek renders and',len(pause_rows),'pause comparisons passed',flush=True)
if __name__=='__main__':main()
