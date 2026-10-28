"""Virtual playback clocks exercise the real transport without Windows audio."""
from pathlib import Path
import json,sys,threading
from _reloq_eci_transport import EciTextTransport
from _reloq_eci_direct import SAMPLE_RATES,SAMPLE_RATE
rows=[]
for code,hz in SAMPLE_RATES.items():
 for cost in [.05,.1,.2,.3,.5,.8,.95]:
  row=dict(rate=hz,render_time_per_audio_second=cost)
  for label,settings in [('custom_buffers',dict(first_pcm_bytes=220,pcm_batch_bytes=4096,final_pcm_min_bytes=1764)),('default_buffers',{})]:
   state=dict(t=0.,end=0.,seen=False,gaps=[],bytes=0,feeds=0,first=None)
   done=threading.Event();errors=[]
   class Player:
    def feed(self,pcm,onDone=None):
     if state['seen'] and state['t']>state['end']+1e-9:state['gaps'].append(state['t']-state['end'])
     if not state['seen']:state['first']=state['t']
     state['end']=max(state['t'],state['end'])+len(pcm)/(2*hz)
     state['seen']=True;state['bytes']+=len(pcm);state['feeds']+=1
     if onDone:onDone()
    def idle(self):pass
    def stop(self):pass
    def close(self):pass
    def pause(self,v):pass
   class Engine:
    def set_parameter(self,k,v):assert k==SAMPLE_RATE and v==code
    def speak(self,parts,pcm,index):
     prev=0
     for ms in range(1,1001):
      count=round(ms*hz/1000);state['t']=ms*.001*cost
      pcm(b'\1\0'*(count-prev));prev=count
      if ms in (5,10,35,85,165,450,999):index(ms)
     return True
    def cancel(self):pass
    def close(self):pass
   t=EciTextTransport(Engine,Player(),lambda n:None,lambda n:done.set(),errors.append,**settings)
   t.set_sample_rate(code,lambda rate:Player());t.submit(['one second'])
   assert done.wait(5) and not errors,errors
   assert t.shutdown() and state['bytes']==2*hz
   row[label]=dict(first_ms=state['first']*1000,underruns=len(state['gaps']),gap_ms=sum(state['gaps'])*1000,feeds=state['feeds'])
  assert row['default_buffers']['underruns']==0,row
  rows.append(row)
if len(sys.argv)>1:Path(sys.argv[1]).write_text(json.dumps(rows,indent=2))
print(len(rows),'clock/cost cases; custom-buffer gaps',sum(r['custom_buffers']['underruns']for r in rows),'default-buffer gaps',sum(r['default_buffers']['underruns']for r in rows))
