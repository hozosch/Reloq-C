"""Native clocks, all presets, source continuity, indices and rate switch-back."""
from pathlib import Path
from array import array
import hashlib,json,math,sys,tempfile
from _reloq_eci_direct import (DirectEciEngine,EciIndex,SAMPLE_RATE,SAMPLE_RATES,
    NATIVE_SPEED,GENDER,HEAD_SIZE,PROCESS_TAGS)
from _reloq_languages import GERMAN,LanguageEngine
ROOT=Path(__file__).resolve().parents[1]

def main():
 reports=[]
 with tempfile.TemporaryDirectory() as folder:
  folder=Path(folder);(folder/'eci.dll').symlink_to(ROOT/'eci/libreloq_eci.so')
  e=DirectEciEngine(folder/'eci.dll',experimental_generic=True,lexicon_path=None)
  e.set_parameter(NATIVE_SPEED,98)
  text='Sechs schnelle Fische. Wortanalyse.'
  def capture(parts):
   chunks=[];marks=[];samples=0
   def pcm(data):
    nonlocal samples
    samples+=len(data)//2;chunks.append(data)
   assert e.speak(parts,pcm,lambda n:marks.append((n,samples)))
   data=b''.join(chunks);assert data and len(data)%2==0
   assert marks==[(7,len(data)//2)],marks
   return data
  for code,hz in SAMPLE_RATES.items():
   e.set_parameter(SAMPLE_RATE,code)
   for v in GERMAN.voices:
    for k,n in v.parameters:e.set_parameter(k,n)
    assert e._dll.eciGetParam(e._handle,SAMPLE_RATE)==code
    data=capture([text,EciIndex(7)])
    assert data==capture([text[:11],text[11:],EciIndex(7)])
    a=array('h',data);ticks=len(a)//(hz//1000)
    assert ticks*(hz//1000)==len(a)
    peak=max(abs(x)for x in a);rms=math.sqrt(sum(x*x for x in a)/len(a))
    assert 100<rms<10000 and peak<32767,(hz,v.id,rms,peak)
    reports.append(dict(hz=hz,voice=v.id,samples=len(a),ticks=ticks,rms=round(rms,3),peak=peak,sha256=hashlib.sha256(data).hexdigest()))
  assert len({r['ticks']for r in reports})==1
  # No hidden reversion to 11.025 kHz when gender or small-head source changes.
  for code,hz in SAMPLE_RATES.items():
   for k,n in GERMAN.voices[0].parameters:e.set_parameter(k,n)
   e.set_parameter(SAMPLE_RATE,1);baseline=capture([text,EciIndex(7)])
   e.set_parameter(SAMPLE_RATE,code)
   for g,h in [(1,22),(1,50),(0,50)]:
    e.set_parameter(GENDER,g);e.set_parameter(HEAD_SIZE,h)
    data=capture([text,EciIndex(7)]);assert len(data)//2==reports[0]['ticks']*(hz//1000)
   e.set_parameter(SAMPLE_RATE,1);assert capture([text,EciIndex(7)])==baseline
  for value in [-1,7,100]:
   assert e._dll.eciSetParam(e._handle,SAMPLE_RATE,value)==-1
   assert e._dll.eciGetParam(e._handle,SAMPLE_RATE)==1
  e.close()
  routed=LanguageEngine(folder/'eci.dll')
  for code,hz in SAMPLE_RATES.items():
   routed.set_parameter(SAMPLE_RATE,code)
   for voice in ['1','2','3','1']:
    routed.select_voice(GERMAN.id,voice)
    assert all(x._dll.eciGetParam(x._handle,SAMPLE_RATE)==code for x in routed._engines.values())
  routed.close()
 if len(sys.argv)>1:Path(sys.argv[1]).write_text(json.dumps(reports,indent=2))
 print('Sample rates: 7 clocks × 8 presets; fragments, played-index positions, source/preset switching, rate rejection and bitwise switch-back passed')
if __name__=='__main__':main()
