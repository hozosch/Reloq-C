"""Both acoustic clocks remain selectable without changing speech duration."""
import argparse
from array import array
import json
import math
from pathlib import Path
import sys
import tempfile
import wave

from test_eci_inline_queue import Engine, TEXTS

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
from observe_high_speed_reference import capture

CLOCK = 0x101b


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library', type=Path, required=True)
    library = parser.parse_args().library
    checks = 0
    reference=json.loads((ROOT/'tests/data/legacy_acoustic_clock.json').read_text())
    with tempfile.TemporaryDirectory(prefix='reloq-clock-option-') as folder:
        path=Path(folder)/'speech.wav'
        for case in reference['cases']:
            capture(library,False,case['text'],case['speed'],path,want_phones=False,
                    dialect=case['dialect'],gender=case['gender'],acoustic_clock=0)
            with wave.open(str(path),'rb') as speech:
                samples=array('h',speech.readframes(speech.getnframes()))
                if sys.byteorder!='little':samples.byteswap()
            assert len(samples)==case['samples'],(case['text'],len(samples),case['samples'])
            size=reference['window_samples']
            actual=[math.sqrt(sum(x*x for x in samples[i:i+size])/size)
                    for i in range(0,len(samples)-size+1,size)]
            expected=case['rms_10ms'];assert len(actual)==len(expected)
            error=math.sqrt(sum((a-b)**2 for a,b in zip(actual,expected))/sum(x*x for x in expected))
            assert error<.01,(case['text'],case['speed'],case['gender'],error)
            checks+=1
    changed=0
    for language,parts in TEXTS.items():
        engine=Engine(library,language)
        try:
            assert engine.lib.eciGetParam(engine.h,CLOCK)==0
            assert engine.lib.eciSetParam(engine.h,CLOCK,-1)==-1
            assert engine.lib.eciSetParam(engine.h,CLOCK,3)==-1
            for rate in (0,1,3,6):
                assert engine.lib.eciSetParam(engine.h,5,rate)>=0
                engine.tags(0)
                for speed in (62,99,106):
                    engine.voice(6,speed)
                    assert engine.lib.eciSetParam(engine.h,CLOCK,0)>=0
                    fine=engine.speak(''.join(parts))
                    assert engine.lib.eciSetParam(engine.h,CLOCK,1)>=0
                    original=engine.speak(''.join(parts))
                    assert len(fine)==len(original) and len(fine)>0,(hex(language),rate,speed)
                    changed+=fine!=original
                    assert engine.lib.eciSetParam(engine.h,CLOCK,2)>=0
                    automatic=engine.speak(''.join(parts))
                    expected=original if language in (0x20000,0x120000) else fine
                    assert automatic==expected,(hex(language),rate,speed,'automatic')
                    assert engine.lib.eciSetParam(engine.h,CLOCK,0)>=0
                    assert engine.speak(''.join(parts))==fine,(hex(language),rate,speed,'restore')
                    checks+=1
        finally:
            engine.close()
    assert changed>=len(TEXTS),changed
    print(f'Acoustic timing option: {checks} legacy, automatic, duration and switch-back checks; {changed} differing PCM pairs')


if __name__=='__main__':
    main()
