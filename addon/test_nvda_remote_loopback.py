"""NVDA 2026.3 speech manager and Remote handlers, with loopback transport.

Uses the packaged ReLoq driver and real ECI core. Network, device playback
and NVDA GUI are simulated; this is not an end-to-end Remote connection.
"""
import argparse
import ast
import ctypes
from enum import IntEnum
import inspect as py_inspect
import json
from pathlib import Path
import queue
import sys
import threading
import time
import types
import typing
from unittest import mock

from nvda_test_api import SOURCE
from test_synth import main as contract_test

p=argparse.ArgumentParser()
p.add_argument('--library',type=Path,required=True)
p.add_argument('--addon',type=Path,required=True)
args=p.parse_args()
loader=ctypes.WinDLL if sys.platform=='win32' else ctypes.CDLL

def inspect(module):
    commands=sys.modules['speech.commands']
    native=sys.modules[module.__name__+'._reloq_eci_direct']
    class Signal:
        def __init__(self):self.handlers=[]
        def register(self,h):self.handlers.append(h)
        def unregister(self,h):self.handlers.remove(h)
        def notify(self,**kw):
            for h in self.handlers[:]:h(**kw)
        def decide(self,**kw):
            return all(h(**kw) is not False for h in self.handlers[:])
    index=Signal();done=Signal();pre=Signal();synth_pre=Signal()
    events=queue.Queue()
    qh=types.SimpleNamespace(eventQueue=events,
        queueFunction=lambda q,f,*a,**k:q.put((f,a,k)))
    class Spri(IntEnum):NORMAL=0;NOW=1
    class Log:
        DEBUG=0
        def isEnabledFor(self,*a):return False
        def __getattr__(self,n):
            if n=='exception':
                def error(*a,**k):raise RuntimeError('NVDA manager swallowed callback exception: '+repr(a))
                return error
            return lambda *a,**k:None
    play_gate=threading.Event();play_gate.set()
    feed_started=threading.Event()
    class Player:
        def feed(self,pcm,onDone=None):
            feed_started.set()
            assert play_gate.wait(10)
            if onDone:onDone()
        def idle(self):pass
        def stop(self):pass
        def pause(self,s):pass
        def close(self):pass
    class Native(native.DirectEciEngine):
        def __init__(self,path,**kw):super().__init__(args.library.resolve(),_loader=loader,**kw)
    class Driver(module.SynthDriver):
        @staticmethod
        def _make_player(hz):return Player()
    original=module.DirectEciEngine;module.DirectEciEngine=Native
    driver=Driver()
    tone_events=[];wave_events=[];sent_speech=[];wire=[]
    tone_decider=Signal();wave_decider=Signal()
    leader_tones=types.SimpleNamespace(beep=lambda hz,length,left,right:tone_events.append((hz,length,left,right)))
    leader_wave=types.SimpleNamespace(playWaveFile=lambda **kw:wave_events.append(kw['fileName']))
    def extract(path,cls,methods,namespace):
        tree=ast.parse(Path(path).read_text(encoding='utf8'))
        old=next(n for n in tree.body if isinstance(n,ast.ClassDef) and n.name==cls)
        functions=[n for n in old.body if isinstance(n,(ast.FunctionDef,ast.AsyncFunctionDef)) and n.name in methods]
        assert len(functions)==len(methods),(cls,methods)
        new=ast.ClassDef(name=cls,bases=[],keywords=[],body=functions,decorator_list=[])
        tree=ast.Module(body=[ast.ImportFrom(module='__future__',names=[ast.alias(name='annotations')],level=0),new],type_ignores=[])
        exec(compile(ast.fix_missing_locations(tree),str(path),'exec'),namespace)
        return namespace[cls]
    source=SOURCE / '_remoteClient'
    leader_ns={'os':types.SimpleNamespace(path=types.SimpleNamespace(exists=lambda f:f!='missing.wav')),
               'tones':leader_tones,'nvwave':leader_wave}
    Leader=extract(source/'localMachine.py','LocalMachine',('beep','playWave'),leader_ns)
    leader=Leader();leader.isMuted=False
    RemoteExtension=extract(source/'transport.py','RemoteExtensionPoint',('remoteBridge','register','unregister'),{})
    types_=types.SimpleNamespace(TONE='tone',WAVE='wave',CANCEL='cancel',PAUSE_SPEECH='pause',SPEAK='speak')
    class Transport:
        def __init__(self):self.outboundHandlers={}
        def registerOutbound(self,extensionPoint,messageType,filter=None):
            instance=RemoteExtension();instance.extensionPoint=extensionPoint;instance.messageType=messageType
            instance.filter=filter;instance.transport=None;instance.register(self);self.outboundHandlers[messageType]=instance
        def unregisterOutbound(self,messageType):self.outboundHandlers.pop(messageType).unregister()
        def send(self,type,**kw):
            if type=='speak':sent_speech.append(kw['sequence']);return
            payload=json.loads(json.dumps(kw));wire.append((type,payload))
            func={'tone':leader.beep,'wave':leader.playWave}.get(type)
            if func:
                accepted=py_inspect.signature(func).parameters
                func(**{k:v for k,v in payload.items() if k in accepted})
    follower_ns={'tones':types.SimpleNamespace(decide_beep=tone_decider),
      'decide_playWaveFile':wave_decider,'speechCanceled':Signal(),'post_speechPaused':Signal(),
      'pre_speechQueued':pre,'braille':types.SimpleNamespace(extensions=types.SimpleNamespace(pre_writeCells=Signal())),
      'RemoteMessageType':types_,'EXCLUDED_SPEECH_COMMANDS':(commands.BaseCallbackCommand,commands._CancellableSpeechCommand)}
    Follower=extract(source/'session.py','FollowerSession',('registerCallbacks','unregisterCallbacks','_filterUnsupportedSpeechCommands','sendSpeech'),follower_ns)
    follower=Follower();follower.callbacksAdded=False;follower.transport=Transport();follower.display=lambda **kw:None
    follower.registerCallbacks()
    def beep(hz,length,left=50,right=50,**kwargs):
        tone_decider.decide(hz=hz,length=length,left=left,right=right,**kwargs)
    def wave(path,**kwargs):
        wave_decider.decide(fileName=path,**kwargs)
    ns=dict(vars(commands),typing=typing,Any=typing.Any,cast=typing.cast,
        _IndexT=int,SpeechSequence=list,Spri=Spri,SPEECH_PRIORITIES=(Spri.NOW,Spri.NORMAL),
        _shouldCancelExpiredFocusEvents=lambda:False,shouldSwitchVoice=lambda:True,
        synthDriverHandler=types.SimpleNamespace(synthIndexReached=index,synthDoneSpeaking=done),
        pre_speechQueued=pre,pre_synthSpeak=synth_pre,queueHandler=qh,log=Log(),getSynth=lambda:driver,config=types.SimpleNamespace())
    tree=ast.parse((SOURCE/'speech/manager.py').read_text(encoding='utf8'))
    classes=[n for n in tree.body if isinstance(n,ast.ClassDef) and n.name in ('ParamChangeTracker','_ManagerPriorityQueue','SpeechManager')]
    source=ast.Module(body=[ast.ImportFrom(module='__future__',names=[ast.alias(name='annotations')],level=0),*classes],type_ignores=[])
    exec(compile(ast.fix_missing_locations(source),'NVDA speech/manager.py','exec'),ns)
    manager=ns['SpeechManager']()
    old_index,old_done=module.synthIndexReached,module.synthDoneSpeaking
    module.synthIndexReached=index;module.synthDoneSpeaking=done
    try:
      with mock.patch.dict(sys.modules,{'tones':types.SimpleNamespace(beep=beep)}),mock.patch.object(sys.modules['nvwave'],'playWaveFile',wave,create=True):
       for mode in ('0','1','2'):
        driver.pauseMode=mode
        for name,seq,expected in (
         ('tone-only',[commands.BeepCommand(660,50)],'tone'),
         ('wave-only',[commands.WaveFileCommand('start.wav')],'wave'),
         ('speech-tone-speech',['Hallo.',commands.BeepCommand(660,50),'Welt.'],'tone'),
         ('speech-wave-speech',['Hallo.',commands.WaveFileCommand('start.wav'),'Welt.'],'wave')):
          before=(len(tone_events),len(wave_events))
          manager.speak(seq,Spri.NORMAL)
          deadline=time.monotonic()+10
          while time.monotonic()<deadline and (not manager._hasNoMoreSpeech() or not events.empty()):
           try:f,a,k=events.get(timeout=.1)
           except queue.Empty:continue
           f(*a,**k)
          assert manager._hasNoMoreSpeech(),(mode,name,'manager stuck')
          after=(len(tone_events),len(wave_events))
          assert after==((before[0]+1,before[1]) if expected=='tone' else (before[0],before[1]+1)),(mode,name,before,after)
          assert all(not isinstance(x,commands.BaseCallbackCommand) for x in sent_speech[-1])
          assert wire[-1][0]==expected,(mode,name,wire)
          print('PASS',mode,name,after,flush=True)
       before=len(tone_events);beep(440,40)
       assert wire[-1][0]=='tone' and len(tone_events)==before+1
       print('PASS direct progress beep crosses the simulated Remote link',flush=True)
       play_gate.clear();feed_started.clear()
       manager.speak(['Ein längerer Test läuft gerade.'],Spri.NORMAL)
       assert feed_started.wait(10),'speech did not reach playback'
       before=len(tone_events);beep(440,40)
       assert wire[-1][0]=='tone' and len(tone_events)==before+1
       play_gate.set()
       deadline=time.monotonic()+10
       while time.monotonic()<deadline and (not manager._hasNoMoreSpeech() or not events.empty()):
        try:f,a,k=events.get(timeout=.1)
        except queue.Empty:continue
        f(*a,**k)
       assert manager._hasNoMoreSpeech(),'speech remained queued after concurrent beep'
       print('PASS direct progress beep during queued speech',flush=True)
       before=len(wave_events);wave('missing.wav',asynchronous=True)
       assert wire[-1][0]=='wave' and len(wave_events)==before
       print('PASS unavailable WAV path is not played by the receiving NVDA',flush=True)
    finally:
        play_gate.set()
        follower.unregisterCallbacks()
        module.synthIndexReached=old_index;module.synthDoneSpeaking=old_done
        driver.terminate();module.DirectEciEngine=original

contract_test(inspect_adapter=inspect,inspect_only=True,package_path=args.addon.resolve())
