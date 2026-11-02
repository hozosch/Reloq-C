"""Public queue snapshots, word-scoped volume and fluid inline controls.

All speech remains in RAM; tests exercise the DLL directly, with no NVDA shim.
"""
import argparse
import ctypes as C
from pathlib import Path

TEXTS = {
 0x10000: ('Please ', 'remember', ' the message.'),
 0x10001: ('Please ', 'remember', ' the message.'),
 0x20000: ('La ', 'semana', ' termina.'),
 0x20001: ('La ', 'semana', ' termina.'),
 0x30000: ('La ', 'maison', ' est ouverte.'),
 0x30001: ('La ', 'maison', ' est ouverte.'),
 0x40000: ('Die ', 'Stimme', ' klingt ruhig.'),
 0x50000: ('La ', 'settimana', ' finisce.'),
 0x60000: ('这是', '一个', '测试。'),
 0x60001: ('這是', '一個', '測試。'),
 0x70000: ('A ', 'semana', ' termina.'),
 0x80000: ('これは', 'テスト', 'です。'),
 0x90000: ('Tämä ', 'viesti', ' kuuluu.'),
 0xa0000: ('이것은 ', '테스트', '입니다.'),
 0x120000: ('Η ', 'φωνή', ' ακούγεται.'),
}

class Engine:
 def __init__(self, path, language):
  self.lib=(C.WinDLL if hasattr(C,'WinDLL') else C.CDLL)(str(path.resolve()))
  H=C.c_void_p;I=C.c_int
  for name,result,args in (
   ('eciNewEx',H,(I,)),('eciDelete',H,(H,)),
   ('eciSetVoiceParam',I,(H,I,I,I)),('eciGetVoiceParam',I,(H,I,I)),
   ('eciSetParam',I,(H,I,I)),('eciGetParam',I,(H,I)),
   ('eciAddText',I,(H,C.c_char_p)),('eciAddTextUtf8',I,(H,C.c_char_p,C.c_size_t,I)),
   ('eciSetOutputBuffer',I,(H,I,C.POINTER(C.c_short))),
   ('eciSynthesize',I,(H,)),('eciClearInput',I,(H,))):
   f=getattr(self.lib,name);f.restype=result;f.argtypes=args
  self.h=self.lib.eciNewEx(language);assert self.h
  self.buffer=(C.c_short*1100)();self.audio=bytearray()
  CB=(C.WINFUNCTYPE if hasattr(C,'WINFUNCTYPE') else C.CFUNCTYPE)(I,H,I,I,H)
  @CB
  def take(h,msg,n,data):
   if msg==0:self.audio.extend(C.string_at(self.buffer,n*2))
   return 1
  self.callback=take
  self.lib.eciRegisterCallback.argtypes=(H,CB,H)
  self.lib.eciRegisterCallback(self.h,take,None)
  assert self.lib.eciSetOutputBuffer(self.h,1100,self.buffer)
  self.direct=False;self.tags(1)
 def close(self):self.lib.eciDelete(self.h)
 def tags(self,on):assert self.lib.eciSetParam(self.h,1,on)>=0
 def voice(self,p,v):
  if self.direct:
   parameters=(0x1010,0x100d,0x1008,0x1009,0x100e,0x100f,0x1007,0x100a)
   assert self.lib.eciSetParam(self.h,parameters[p],v)>=0
  else:assert self.lib.eciSetVoiceParam(self.h,0,p,v)>=0
 def add(self,s,legacy=False):
  data=s.encode('cp1252' if legacy else 'utf8')
  assert (self.lib.eciAddText(self.h,data) if legacy else self.lib.eciAddTextUtf8(self.h,data,len(data),0))
 def finish(self):
  self.audio.clear();assert self.lib.eciSynthesize(self.h),self.lib.eciGetParam(self.h,0x1002)
  return bytes(self.audio)
 def speak(self,s):self.add(s);return self.finish()

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--library',type=Path,required=True);a=p.parse_args()
 checks=0
 for language,(prefix,word,suffix) in TEXTS.items():
  e=Engine(a.library,language)
  try:
   # The identical lexical input must receive one volume for the whole
   # containing word, even when the command occurs between its letters.
   cut=max(1,len(word)//2)
   plain=e.speak(prefix+word+suffix)
   assert e.speak(prefix+word[:cut]+'`vv100'+word[cut:]+suffix)==plain,hex(language)
   expected=e.speak(prefix+'`vv30'+word+suffix)
   actual=e.speak(prefix+word[:cut]+'`vv30'+word[cut:]+suffix)
   # CJK source characters can be separate native lexical words.
   if language>>16 not in (6,8,10):
    assert actual==expected,(hex(language),'mid-word vv',len(actual),len(expected))
   assert expected!=plain;checks+=3
  finally:e.close()
 for language in (0x10000,0x40000):
  e=Engine(a.library,language)
  try:
   prefix='Apply button Alt plus ' if language==0x10000 else 'Bitte lesen Sie '
   suffix='another message.' if language==0x10000 else 'diesen Text.'
   text=prefix+suffix
   plain=e.speak(text)
   for tag in ('vv100','vs50','vb65','vf30','vh50','vr0','vy0','vg0'):
    actual=e.speak(prefix+'`'+tag+' '+suffix)
    # A no-op parameter must never add the reported ~0.4-second silence.
    assert abs(len(actual)-len(plain))<=220,(hex(language),tag,len(actual),len(plain))
    checks+=1
   for direct in (False,True):
    e.direct=direct
    for legacy in (False,True):
     for tags in (0,1):
      for parameter,name,value in ((6,'vs',150),(7,'vv',30),(2,'vb',80),(3,'vf',0),
                                   (1,'vh',80),(0,'vg',1),(4,'vr',30),(5,'vy',40)):
       e.tags(1)
       # Establish the native defaults, then create a real queued change.
       default=(0,50,65,30,0,0,50,100)[parameter]
       e.voice(parameter,default)
       expected=e.speak(prefix+'`'+name+str(value)+suffix)
       e.tags(tags);e.add(prefix,legacy);e.voice(parameter,value);e.add(suffix,legacy)
       actual=e.finish()
       assert actual==expected,(hex(language),legacy,tags,parameter,len(actual),len(expected))
       assert e.lib.eciGetVoiceParam(e.h,0,parameter)==value
       assert e.lib.eciGetParam(e.h,1)==tags
       e.voice(parameter,default);checks+=3
   # A setter after the final AddText affects the next utterance only.
   e.tags(0);e.voice(7,100);expected=e.speak(text)
   e.add(text);e.voice(7,30);assert e.finish()==expected;checks+=1
  finally:e.close()
 print(f'Inline/queue: {checks} word-volume, fluid-control and public snapshot checks passed; audio in RAM')

if __name__=='__main__':main()
