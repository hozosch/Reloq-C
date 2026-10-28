"""First English selection: real native text, PCM, controls and transport.

These are integration/continuity checks, not claims of original PCM equality
for the connected preview. The separate 850 exact development cases remain
under reference-apple's release verifier.
"""
from pathlib import Path
from array import array
import hashlib
import json
import sys
import tempfile
import threading
from _reloq_eci_direct import (DirectEciEngine, EciIndex, EciPitch, EciLanguage, EciText,
    NATIVE_SPEED, NATIVE_PITCH, NATIVE_INFLECTION, NATIVE_VOLUME, SAMPLE_RATE, SAMPLE_RATES,
    PROCESS_TAGS, PAUSE_MODE, GENDER)
from _reloq_languages import GERMAN, US_ENGLISH, LANGUAGES, LanguageEngine, language_for
from _reloq_english_unicode import normalize_generic_text, normalize_spelling_text
ROOT=Path(__file__).resolve().parents[1]


def check_zero_groups():
    """A numeric final group must sound like its complete written words."""
    e=DirectEciEngine(ROOT/'eci/libreloq_eci.so',dialect=US_ENGLISH.id,
        experimental_generic=True,encoding='utf-8')
    e.set_parameter(NATIVE_SPEED,98)
    units=['','one','two','three','four','five','six','seven','eight','nine']
    tens=['','ten','twenty','thirty','forty','fifty','sixty','seventy','eighty','ninety']
    def pcm(text):
        chunks=[]
        assert e.speak([EciText(text,False)],chunks.append,lambda _:None)
        return b''.join(chunks)
    cases=0
    try:
        for scale in [100,1000,10000]:
            for leading in range(1,10):
                for trailing in range(1,10):
                    number=str(scale*leading+trailing)
                    prefix=(units[leading]+' hundred'if scale==100 else
                        units[leading]+' thousand'if scale==1000 else
                        tens[leading]+' thousand')
                    words=prefix+' '+units[trailing]
                    for before,after in [('',''),('There are ',' files left.')]:
                        actual=pcm(before+number+after)
                        assert actual==pcm(before+words+after),(number,before,after)
                        assert actual!=pcm(before+prefix+after),(number,'missing final group')
                        cases+=1
        for rate in SAMPLE_RATES:
            e.set_parameter(SAMPLE_RATE,rate)
            for mode in [0,1,2]:
                e.set_parameter(PAUSE_MODE,mode)
                for number,words in [('503','five hundred three'),('4004','four thousand four'),
                                     ('10008','ten thousand eight')]:
                    assert pcm(number)==pcm(words),(rate,mode,number)
                    cases+=1
    finally:
        e.close()
    print('English zero groups:',cases,'complete numeric outputs passed')


def main():
    check_zero_groups()
    assert language_for('en-US', GERMAN.id)==US_ENGLISH.id
    assert LANGUAGES[US_ENGLISH.id].locale=='en_US'
    assert normalize_generic_text('café ΩЖ')=='cafe  omega  '
    assert 'k' in normalize_spelling_text('Ж')
    e=DirectEciEngine(ROOT/'eci/libreloq_eci.so', dialect=US_ENGLISH.id, experimental_generic=True)
    for k,v in US_ENGLISH.voices[0].parameters:e.set_parameter(k,v)
    e.set_parameter(NATIVE_SPEED,98)
    def run(parts):
        chunks=[];marks=[];samples=0
        def pcm(b):
            nonlocal samples
            samples+=len(b)//2;chunks.append(b)
        assert e.speak(parts,pcm,lambda i:marks.append((i,samples)))
        return b''.join(chunks),marks
    text='Hello. The quick brown fox jumps over the lazy dog. Human speech settings.'
    report=[]
    for code,hz in SAMPLE_RATES.items():
        e.set_parameter(SAMPLE_RATE,code)
        data,marks=run([text,EciIndex(1)])
        assert marks==[(1,len(data)//2)] and any(data)
        values=array('h',data)
        assert max(abs(v)for v in values)<32767
        assert run([text[:8],text[8:31],text[31:],EciIndex(1)])==(data,marks)
        report.append(dict(hz=hz,samples=len(values),peak=max(abs(v)for v in values),sha256=hashlib.sha256(data).hexdigest()))
    e.set_parameter(SAMPLE_RATE,1)
    # New productive forms and multiword numeric expansion must retain
    # input-token index ownership across arbitrary AddText boundaries.
    for text in ['The quickly changing files.', '123456789012 items.',
                 'At 10:05, pay $12.50 for 3-5 items.',
                 'Open a_b@test.example / version 1.2.3.',
                 'Read (this); then: continue... Done!']:
        full,_=run([text])
        for split in range(len(text)+1):
            data,marks=run([text[:split],EciIndex(77),text[split:]])
            assert data==full and len(marks)==1 and 0<=marks[0][1]<=len(data)//2
    # Long phrases used to extrapolate declination to negative F0. Maximum
    # melody settings must also keep valid pulse frequencies at all pitches.
    data,marks=run([' '.join(['quickly','123456789012','safely']*80),EciIndex(78)])
    assert data and marks==[(78,len(data)//2)]
    for melody in [0,30,100]:
        e.set_parameter(NATIVE_INFLECTION,melody)
        for pitch in [0,65,100]:
            e.set_parameter(NATIVE_PITCH,pitch)
            data,marks=run(['The quickly changing files.',EciIndex(79)])
            assert data and marks==[(79,len(data)//2)]
    e.set_parameter(NATIVE_INFLECTION,30);e.set_parameter(NATIVE_PITCH,65)
    assert run(['... ',EciIndex(6)])==(b'',[(6,0)])
    assert run([': Hello.'])[0]==run(['Hello.'])[0]
    full,_=run(['Hello world.'])
    for split in range(len('Hello world.')+1):
        data,marks=run(['Hello world.'[:split],EciIndex(7),'Hello world.'[split:]])
        assert data==full and len(marks)==1
        assert 0<=marks[0][1]<=len(data)//2
        if split==0:assert marks==[(7,0)]
        if split==12:assert marks==[(7,len(data)//2)]
    baseline,marks=run(['Hello',EciIndex(1),' ',EciIndex(2),'world.',EciIndex(3)])
    assert baseline==full and marks[0][1]==marks[1][1] and 0<marks[0][1]<marks[2][1]
    assert marks[-1][1]==len(full)//2
    for text in ['The than again.','The them again.','a human','hmm','whale',
                 'The file is open.','Do you want to save the changes?',
                 'One hundred twenty three.','A file.', 'qzxvjk',
                 normalize_spelling_text('Abz 9.éЖΩ'),
                 'Text '+normalize_spelling_text('A')+' continues.']:
        data,_=run([text]);assert data and any(data),text
    # Productive words use the complete native frontend; explicit character
    # navigation retains letter names. The original may itself spell an
    # unpronounceable consonant string such as qzxvjk.
    assert run(['uncharacteristically'])[0]!=run([normalize_spelling_text('uncharacteristically')])[0]
    e.set_parameter(PROCESS_TAGS,1)
    for parameter,tag,value,default in [(NATIVE_SPEED,'vs',69,98),(NATIVE_PITCH,'vb',72,65),(NATIVE_VOLUME,'vv',50,100)]:
        e.set_parameter(parameter,value);control=run(['Hello world.'])[0]
        e.set_parameter(parameter,default);tagged=run([f'`{tag}{value}Hello world.'])[0]
        assert control==tagged,(tag,len(control),len(tagged))
    e.set_parameter(PROCESS_TAGS,0)
    data,_=run(['Hello world.']);pitched,_=run([EciPitch(82),'Hello ',EciPitch(65),'world.'])
    assert len(data)==len(pitched) and data!=pitched
    e.set_parameter(PAUSE_MODE,0);normal=run(['Hello. World.'])[0]
    e.set_parameter(PAUSE_MODE,1);short_final=run(['Hello. World.'])[0]
    e.set_parameter(PAUSE_MODE,2);short_all=run(['Hello. World.'])[0]
    assert len(normal)>len(short_final)>len(short_all)
    seen=[]
    def stop(b):
        seen.append(b)
        thread=threading.Thread(target=e.cancel);thread.start();thread.join(2)
        assert not thread.is_alive()
    assert not e.speak(['Hello world.',EciIndex(7)],stop,lambda i:(_ for _ in ()).throw(AssertionError('index after cancellation')))
    assert seen
    assert run(['Hello again.',EciIndex(8)])[1][-1][0]==8
    e.close()
    # Use the actual language catalog with real independent ECI handles.
    sys.path.insert(0,str(ROOT/'addon'))
    from build_synth import merged_german_lexicon
    with tempfile.TemporaryDirectory() as folder:
        folder=Path(folder);(folder/'eci.dll').symlink_to(ROOT/'eci/libreloq_eci.so')
        (folder/GERMAN.lexicon).write_bytes(merged_german_lexicon())
        engine=LanguageEngine(folder/'eci.dll');engine.set_parameter(NATIVE_SPEED,98)
        chunks=[];indices=[]
        assert engine.speak(['Hallo.',EciLanguage(US_ENGLISH.id,'1'),'Hello.',EciIndex(4),
                            EciLanguage(GERMAN.id,'1'),'Wieder deutsch.',EciIndex(5)],chunks.append,indices.append)
        assert indices==[4,5] and chunks
        engine.select_voice(US_ENGLISH.id,'1')
        assert engine.speak(['English is selected.'],chunks.append,indices.append)
        engine.select_voice(GERMAN.id,'2')
        assert engine.speak([EciLanguage(US_ENGLISH.id,'1'),'Hello.',
                            EciLanguage(GERMAN.id,'2'),'Hallo.'],chunks.append,indices.append)
        english = engine._engines[(US_ENGLISH.id,'1')]
        german = engine._engines[(GERMAN.id,'2')]
        assert english._dll.eciGetParam(english._handle,GENDER)==1
        assert german._dll.eciGetParam(german._handle,GENDER)==1
        engine.close()
    if len(sys.argv)>1:Path(sys.argv[1]).write_text(json.dumps(report,indent=2)+'\n')
    print('English preview: seven rates, non-clipping Male 1, fragments/indices, letter names, tags, pauses, pitch, cancellation and real language switching passed')
if __name__=='__main__':main()
