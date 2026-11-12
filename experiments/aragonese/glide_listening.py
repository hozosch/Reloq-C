"""Compare private AU formant-target experiments by ear.

The supplied library is a throwaway Castilian-based probe, not a language or
an installable voice. Manual scope is for isolated words only and is explicitly
labelled in the script: it does not implement context handling in normal text.
"""
from __future__ import annotations

import argparse
from array import array
import os
from pathlib import Path
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT / 'nvda'))
from _reloq_eci_direct import DirectEciEngine, PROCESS_TAGS, SAMPLE_RATE
from castilian_sh_listening import millis

PREFIX = 'RELOQ_ARAGON_PROBE_'


def configure(**values):
    for suffix,value in values.items(): os.environ[PREFIX+suffix] = str(value)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline',required=True,type=Path)
    parser.add_argument('--glide',required=True,type=Path)
    parser.add_argument('--scope',choices=('auto','manual'),default='auto')
    parser.add_argument('--wav',required=True,type=Path)
    args=parser.parse_args()
    german=DirectEciEngine(args.baseline.resolve(),dialect=0x40000,encoding='utf8',native_voice=True)
    experiment=DirectEciEngine(args.glide.resolve(),dialect=0x20000,encoding='utf8',native_voice=True)
    engines=(german,experiment)
    saved={key:value for key,value in os.environ.items() if key.startswith(PREFIX)}
    samples=array('h')
    try:
        for key in saved: del os.environ[key]
        for engine in engines:
            engine.set_parameter(SAMPLE_RATE,3)
            engine.set_parameter(PROCESS_TAGS,1)
        configure(SH_PERCENT=65,AU_MEDIAL_F1=0,AU_MEDIAL_F2=40,
                  AU_FINAL_F1=0,AU_FINAL_F2=65)

        def render(engine,text):
            data=bytearray()
            if not engine.speak([text],data.extend,lambda _index:None) or not data:
                raise RuntimeError('No audio for '+text)
            out=array('h');out.frombytes(data)
            if sys.byteorder!='little':out.byteswap()
            return out

        def add(audio):
            samples.extend(audio);samples.extend([0]*millis(300))

        def say(text): add(render(german,text))

        say('Aragonesisch. Neuer Vergleich für das Au. Zuerst ursprünglich und die bisherige Probe eins. Danach zwei neue Versuche.')
        for title,word in (('Erstes Wort.', 'auca'),('Zweites Wort.', 'pauto'),
                           ('Drittes Wort.', 'cudiau'),('Viertes Wort.', 'costau')):
            say(title+' Ursprünglich.')
            configure(GLIDE_SCOPE=0,AU_PERCENT=0)
            add(render(experiment,word))
            say('Bisherige Probe eins.')
            configure(GLIDE_SCOPE=0,AU_PERCENT=40)
            old=render(experiment,word);add(old)
            configure(GLIDE_SCOPE=1 if args.scope=='auto' else 2,
                      MANUAL_FINAL=int(word.endswith('au')),
                      AU_MEDIAL_DURATION=100,AU_FINAL_DURATION=100,
                      AU_MEDIAL_F1=0,AU_FINAL_F1=0,
                      AU_MEDIAL_F2=40,AU_FINAL_F2=65)
            say('Neue Probe eins.')
            first=render(experiment,word);add(first)
            configure(AU_MEDIAL_F1=20,AU_FINAL_F1=20,AU_FINAL_F2=80)
            say('Neue Probe zwei.')
            second=render(experiment,word);add(second)
            if old==first or len(second)!=len(first) or second==first:
                raise RuntimeError('Expected independent formant changes were not applied to '+word)
    finally:
        for engine in engines: engine.close()
        for key in list(os.environ):
            if key.startswith(PREFIX): del os.environ[key]
        os.environ.update(saved)
    if sys.byteorder!='little':samples.byteswap()
    with wave.open(str(args.wav),'wb') as wav:
        wav.setnchannels(1);wav.setsampwidth(2);wav.setframerate(16000)
        wav.writeframes(samples.tobytes())
    print(f'Created {args.wav}: {len(samples)/16000:.1f} seconds, scope {args.scope}')


if __name__=='__main__':main()
