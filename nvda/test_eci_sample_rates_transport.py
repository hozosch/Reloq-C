"""A queued clock change cannot feed PCM to a mismatched or closed player."""
import threading
from _reloq_eci_direct import SAMPLE_RATE,SAMPLE_RATES
from _reloq_eci_transport import EciTextTransport
from test_eci_transport import until

def main():
 state={'code':1};players=[];errors=[];done=[]
 class Player:
  def __init__(self,hz):self.hz=hz;self.closed=False;self.chunks=0;players.append(self)
  def feed(self,pcm,onDone=None):
   assert not self.closed and self.hz==SAMPLE_RATES[state['code']]
   self.chunks+=1
   if onDone:onDone()
  def idle(self):assert not self.closed
  def pause(self,value):assert not self.closed
  def stop(self):assert not self.closed
  def close(self):assert not self.closed;self.closed=True
 class Engine:
  def __init__(self):self.owner=threading.get_ident()
  def set_parameter(self,k,v):assert threading.get_ident()==self.owner and k==SAMPLE_RATE;state['code']=v
  def speak(self,parts,pcm,index):pcm(b'\0\0'*1000);return True
  def cancel(self):pass
  def close(self):pass
 p=Player(11025);t=EciTextTransport(Engine,p,lambda n:None,done.append,errors.append)
 for code in [3,5,6,0,2,4,1]:
  t.set_sample_rate(code,Player);number=t.submit(['test']);until(lambda:number in done)
  assert state['code']==code
 # Cancel while the next format is being created, before publication.
 started=threading.Event();release=threading.Event()
 def factory(hz):started.set();assert release.wait(2);return Player(hz)
 t.set_sample_rate(6,factory);assert started.wait(2);t.cancel();release.set()
 until(lambda:state['code']==6);number=t.submit(['after cancel']);until(lambda:number in done)
 # A device failure leaves native clock, player and subsequent speech usable.
 def fail(hz):raise OSError('simulated format rejection')
 accepted=[]
 t.set_sample_rate(3,fail,accepted.append);number=t.submit(['after failure']);until(lambda:number in done)
 assert accepted==[6]
 assert state['code']==6 and len(errors)==1 and isinstance(errors[0],OSError)
 assert t.shutdown() and all(p.closed for p in players)
 print('Sample-rate transport: format/PCM ordering, cancellation during switch, device-failure recovery and shutdown passed')
if __name__=='__main__':main()
