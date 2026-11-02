"""Time the first feed through NVDA's transport, with dictionaries and tags.

The player is simulated. These host timings include the add-on's queue and
PCM batching; they do not measure NVDA, Windows or the audio device.
"""
import argparse
import json
from pathlib import Path
import statistics
import sys
import threading
import time
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'nvda'))
from _reloq_eci_direct import EciIndex,EciPitch,NATIVE_SPEED,PROCESS_TAGS,HIGH_RATE_PROFILE
from _reloq_eci_transport import EciTextTransport
from _reloq_languages import LanguageEngine
TEXTS={
 'plain':'Die Stimme erklärt das Backend. Die zusätzlichen Obertöne sollen den Stimmkörper erweitern. ',
 'technical':'Die Stimme erklärt das Backend. Die Funktion `eciSetVoice` gibt es dort nicht. '
     'Wir verwenden `eciSetVoiceParam` und `eciCopyVoice`. Das Wörterbuch heißt `DEUmain`. ',
 'volume':'Die Stimme erklärt das Backend. Die Lautstärke folgt dem nächsten `vv50 Wort. '
     'Die Stimme spricht weiter. `vv100 Jetzt ist sie wieder lauter. ',
}
class Player:
    def __init__(self):self.first=threading.Event();self.at=None
    def feed(self,pcm,onDone=None):
        assert pcm
        if self.at is None:self.at=time.perf_counter();self.first.set()
        if onDone:onDone()
    def idle(self):pass
    def stop(self):pass
    def pause(self,switch):pass
    def close(self):pass

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--library',type=Path,required=True)
    p.add_argument('--dictionary',type=Path,default=ROOT/'data/community')
    p.add_argument('--repeat',type=int,default=3)
    p.add_argument('--copies',type=int,default=64)
    p.add_argument('--output',type=Path)
    a=p.parse_args()
    if a.repeat<1 or a.copies<1:p.error('counts must be positive')
    errors=[];player=Player()
    transport=EciTextTransport(lambda:LanguageEngine(a.library.resolve()),player,
        lambda n:None,lambda n:None,errors.append)
    def measure(text):
        player.at=None;player.first.clear();start=time.perf_counter()
        transport.submit([EciIndex(1),EciPitch(65),text,EciIndex(2)])
        assert player.first.wait(15),'No first PCM feed'
        elapsed=1000*(player.at-start)
        transport.cancel();assert not errors,errors
        return elapsed
    rows=[]
    try:
        transport.set_dictionary_path(a.dictionary.resolve())
        for param,value in ((NATIVE_SPEED,75),(PROCESS_TAGS,1),(HIGH_RATE_PROFILE,1)):
            transport.set_parameter(param,value)
        measure('Die Stimme ist bereit.')
        for label,unit in TEXTS.items():
            text=unit*a.copies
            values=[measure(text) for _ in range(a.repeat)]
            rows.append(dict(case=label,characters=len(text),first_feed_ms=values,
                median_ms=statistics.median(values)))
    finally:
        assert transport.shutdown(timeout=5),'Transport did not close'
    assert not errors,errors
    result=dict(library=str(a.library.resolve()),simulated_player=True,
        dictionaries=str(a.dictionary.resolve()),rows=rows,no_audio_files=True)
    if a.output:a.output.write_text(json.dumps(result,ensure_ascii=False,indent=2)+'\n')
    print(json.dumps(result,ensure_ascii=False))
if __name__=='__main__':main()
