"""Compare incremental German with the whole-text path through public ECI.

A matching leading speed control selects the existing whole-text path in
one copy of the same library. Only this test inserts that control; runtime
scheduling uses admitted source boundaries and no added text or pause tags.
PCM is hashed in RAM; equality includes every delivered index position.
"""
from pathlib import Path
import argparse
import hashlib
import json
import tempfile

from _reloq_eci_direct import (DirectEciEngine,EciIndex,NATIVE_SPEED,SAMPLE_RATE,
    PAUSE_MODE,PROCESS_TAGS,GENDER,HEAD_SIZE,BREATHINESS,PHRASE_PREDICTION,HIGH_RATE_PROFILE)

ROOT=Path(__file__).resolve().parents[1]
TAIL='Die Stimme liest den nächsten Abschnitt. '
TEXTS=(
 'Die Stimme spricht einen längeren Text mit mehreren vollständigen Sätzen. '*24,
 'Das ist sehr gut, denke ich. '+TAIL*32,
 'Das passt, z. B. bei diesem Text. '+TAIL*32,
 'NVDA liest 504 und 4004. Die Datei heißt reloq-c-android-arm64-1.0.1.apk. '+TAIL*30,
 '„Vorbereitung“ wurde isoliert richtig gelesen. Das zuvor zerlegte „SAPI5“ veränderte seine Analyse. '+TAIL*30,
 'NVDA-Einstellungen: Sprachausgabe (Standard-Konfiguration). '+TAIL*30,
 'Dr. Meyer kommt um 8.30 Uhr. Danach gehen wir nach Frankfurt/Main. '+TAIL*30,
 'Die Stimme liest β, Ω, ü, U\u0308 und ein Telefon 📞. '+TAIL*30,
 'Das ist ein Beispiel, denke ich. '+'Das ist ein weiteres Beispiel, denke ich. '*32,
 'Es ist nass. Das Fass ist weiß. Klaus isst Reis. '+TAIL*30,
 'Die Stimme liest weiter. '+(' '.join(['Wort']*70)+'. ')*5,
 'Eine Frage: Wie geht es dir? Bist du schon fertig? '+TAIL*30,
 'Der Brief ist scharf. Viele Vögel fliegen. Die Wiese ist weit. '*24,
 'Wir sehen sieben Segel. Es ist nass. Fünf Vögel fliegen vorbei. '*24,
)


def capture(engine,text,speed,legacy):
    end=text.find('. ')+2
    points=[0,0,3,len(text)//3,len(text)//2,len(text)-3,len(text),len(text)]
    if end>1:
        points += list(range(max(0,end-3),min(len(text),end+5)))
    combining=text.find('\u0308')
    if combining>=0:points += [combining,combining+1]
    fragments=[f'`vs{speed} '] if legacy else []
    previous=0
    for i,at in enumerate(sorted(points)):
        fragments += [text[previous:at],EciIndex(i+1)]
        previous=at
    fragments.append(text[previous:])
    digest=hashlib.sha256();size=0;marks=[]
    def take(data):
        nonlocal size
        digest.update(data);size+=len(data)
    assert engine.speak(fragments,take,lambda n:marks.append((n,size))), 'Synthesis did not complete'
    assert size and len(marks)==len(points)
    return dict(sha256=digest.hexdigest(),bytes=size,indices=marks)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    parser.add_argument('--reference',type=Path,help='Optional build with RELOQ_GERMAN_STREAMING=0')
    parser.add_argument('--output',type=Path)
    parser.add_argument('--profile',type=int,choices=(0,1,2),default=0)
    args=parser.parse_args()
    paths=(args.reference or args.library,args.library)
    engines=[DirectEciEngine(p.resolve(),dialect=0x40000,encoding='utf8',experimental_generic=True) for p in paths]
    for e in engines:e.set_parameter(HIGH_RATE_PROFILE,args.profile)
    rows=[]
    def check(label,text,speed=120,rate=1,pause=0,gender=0,head=50,breath=0):
        for e in engines:
            for p,v in ((PROCESS_TAGS,1),(NATIVE_SPEED,speed),(SAMPLE_RATE,rate),(PAUSE_MODE,pause),
                        (GENDER,gender),(HEAD_SIZE,head),(BREATHINESS,breath),(PHRASE_PREDICTION,1)):
                e.set_parameter(p,v)
        before=capture(engines[0],text,speed,args.reference is None)
        after=capture(engines[1],text,speed,False)
        assert before==after,(label,speed,rate,pause,gender,head,before,after)
        rows.append(dict(case=label,speed=speed,rate=rate,pause=pause,gender=gender,head=head,breath=breath,**after))
    try:
        for rate,pause in ((1,0),(6,2)):
            for i,text in enumerate(TEXTS):check(str(i),text,rate=rate,pause=pause)
        for speed in (0,75,156):
            check('speed',TEXTS[0],speed=speed)
        for rate in (0,3,2,4,5,6):
            check('female',TEXTS[0],rate=rate,pause=1,gender=1,head=29,breath=60)
        # Keep loaded snapshots usable throughout later native sub-requests.
        # Nested plans must not apply Main replacement a second time.
        with tempfile.TemporaryDirectory() as temp:
            source=Path(temp)/'DEUmain.dic'
            source.write_bytes(b'StreamWord\tBanane\nBanane\tApfel\nUpperWord\tVorbereitung\n')
            for e in engines:e.set_dictionary_path(Path(temp))
            text=('StreamWord und UpperWord. ETI-StreamWord, Banane und StreamWord. '+TAIL)*16
            check('dictionary',text)
            source.unlink()
            for rate in (1,6):check('dictionary-source-absent',text,rate=rate,pause=2)
    finally:
        for e in engines:e.close()
    report=dict(profile=args.profile,cases=len(rows),all_pcm_and_indices_exact=True,no_audio_files=True,rows=rows)
    if args.output:args.output.write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
    print(f'German streaming: {len(rows)} exact whole-text PCM/index comparisons; all rates, small female model, speeds, Unicode, fragment boundaries and retained Main dictionaries; no audio files')


if __name__=='__main__':main()
