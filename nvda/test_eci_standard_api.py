"""Exercise the public ECI API without ReLoq's control/dictionary extensions."""
import argparse,ctypes,hashlib,tempfile
from pathlib import Path

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--library',type=Path,required=True);a=p.parse_args()
 lib=(ctypes.WinDLL if hasattr(ctypes,'WinDLL') else ctypes.CDLL)(str(a.library.resolve()))
 H=ctypes.c_void_p;I=ctypes.c_int;P=ctypes.POINTER
 signatures={
 'eciNew':(H,()),'eciNewEx':(H,(I,)),'eciDelete':(H,(H,)),
 'eciGetAvailableLanguages':(I,(P(ctypes.c_uint),P(I))),
 'eciSetParam':(I,(H,I,I)),'eciGetParam':(I,(H,I)),
 'eciSetVoiceParam':(I,(H,I,I,I)),'eciGetVoiceParam':(I,(H,I,I)),
 'eciCopyVoice':(I,(H,I,I)),'eciGetVoiceName':(I,(H,I,H)),
 'eciSetVoiceName':(I,(H,I,H)), 'eciNewDict':(H,(H,)),
 'eciSetDict':(I,(H,H)),'eciGetDict':(H,(H,)),
 'eciDeleteDict':(H,(H,H)),'eciLoadDict':(I,(H,H,I,H)),
 'eciAddText':(I,(H,H)),'eciInsertIndex':(I,(H,I)),
 'eciSetOutputBuffer':(I,(H,I,P(ctypes.c_short))),
 'eciSynthesize':(I,(H,)),'eciSynchronize':(I,(H,)),
 'eciClearInput':(I,(H,))}
 for name,(result,args) in signatures.items():f=getattr(lib,name);f.restype=result;f.argtypes=args
 CB=(ctypes.WINFUNCTYPE if hasattr(ctypes,'WINFUNCTYPE') else ctypes.CFUNCTYPE)(I,H,I,I,H)
 lib.eciRegisterCallback.argtypes=(H,CB,H);lib.eciRegisterCallback.restype=None
 checks=0;count=I(0)
 assert lib.eciGetAvailableLanguages(None,ctypes.byref(count))==0 and count.value==15
 languages=(ctypes.c_uint*17)(*([0xdeadbeef]*17));capacity=I(15)
 assert not lib.eciGetAvailableLanguages(languages,ctypes.byref(capacity));assert capacity.value==15
 assert languages[15]==languages[16]==0xdeadbeef;checks+=3
 small=(ctypes.c_uint*2)(0,0xdeadbeef);n=I(1)
 assert lib.eciGetAvailableLanguages(small,ctypes.byref(n))==0 and n.value==1 and small[1]==0xdeadbeef
 bad=I(-1);assert lib.eciGetAvailableLanguages(None,ctypes.byref(bad))==0x80;checks+=2
 h=lib.eciNew();assert h and lib.eciGetParam(h,9)==0x10000;lib.eciDelete(h);checks+=1
 for language in languages[:15]:
  h=lib.eciNewEx(language);assert h and lib.eciGetParam(h,9)==language;lib.eciDelete(h);checks+=1
 h=lib.eciNewEx(0x10000);buffer=(ctypes.c_short*1100)();audio=bytearray();indices=[]
 @CB
 def callback(handle,message,param,data):
  if message==0:audio.extend(ctypes.string_at(buffer,param*2))
  elif message==2:indices.append((param,len(audio)))
  return 1
 lib.eciRegisterCallback(h,callback,None);assert lib.eciSetOutputBuffer(h,1100,buffer)
 def speak(data):
  audio.clear();indices.clear();assert lib.eciClearInput(h)
  storage=ctypes.create_string_buffer(data+b'\0\0');assert lib.eciAddText(h,storage)
  assert lib.eciInsertIndex(h,23) and lib.eciSynthesize(h) and lib.eciSynchronize(h)
  assert audio and indices==[(23,len(audio))]
  return bytes(audio),tuple(indices)
 try:
  default=speak(b'Hello world.')
  voice_audio=set()
  for voice in range(1,9):
   values=[lib.eciGetVoiceParam(h,voice,i) for i in range(8)]
   assert lib.eciCopyVoice(h,voice,0)
   assert values==[lib.eciGetVoiceParam(h,0,i) for i in range(8)]
   voice_audio.add(hashlib.sha256(speak(b'Hello world.')[0]).digest())
   assert lib.eciSetVoiceParam(h,voice,2,50)==-1
   assert lib.eciCopyVoice(h,voice,voice+8)
   assert lib.eciSetVoiceParam(h,voice+8,2,42)==values[2]
   assert lib.eciGetVoiceParam(h,voice,2)==values[2];checks+=6
  assert len(voice_audio)==8;checks+=8
  assert lib.eciCopyVoice(h,1,0)
  assert speak(b'Hello world.')==default;checks+=1
  assert lib.eciSetVoiceParam(h,0,6,75)==50
  assert lib.eciSetVoiceParam(h,0,6,251)==-1
  assert lib.eciSetVoiceParam(h,0,0,2)==-1
  assert lib.eciSetVoiceParam(h,0,2,-1)==-1;checks+=4
  for voice in (-1,17):assert lib.eciGetVoiceParam(h,voice,0)==-1;checks+=1
  assert lib.eciSetVoiceName(h,9,b'My voice');name=ctypes.create_string_buffer(30)
  assert lib.eciGetVoiceName(h,9,name) and name.value==b'My voice';checks+=1
  corpus=[(0x10000,'cp1252','The time is 13:28:25.'),(0x40000,'cp1252','Größe und zehn.'),
   (0x120000,'cp1253','Καλημέρα.'),(0x60000,'gbk','你好。'),(0x60001,'cp950','你好。'),
   (0x80000,'cp932','こんにちは。'),(0xa0000,'cp949','안녕하세요.')]
  for language,encoding,text in corpus:
   assert lib.eciSetParam(h,9,language)>=0
   legacy=speak(text.encode(encoding))
   assert lib.eciSetParam(h,9,language|0x800)==language
   unicode=speak(text.encode('utf-16le'))
   assert legacy==unicode,(hex(language),'byte code set / UTF-16 mismatch')
   assert lib.eciGetParam(h,9)==language|0x800;checks+=4
  assert lib.eciSetParam(h,9,0x40800)>=0
  assert lib.eciSetVoiceName(h,9,ctypes.create_string_buffer('Größe'.encode('utf-16le')+b'\0\0'))
  name=ctypes.create_string_buffer(60);assert lib.eciGetVoiceName(h,9,name)
  assert name.raw[:12].decode('utf-16le').rstrip('\0')=='Größe';checks+=1
  assert lib.eciSetParam(h,9,0xb0000)==-1;checks+=1
  assert lib.eciSetParam(h,9,0x40000)>=0
  plain=speak(b'bei.');unmodified=speak(b'TokenX.');assert plain!=unmodified
  with tempfile.TemporaryDirectory() as temp:
   path=Path(temp)/'anything.dic';path.write_bytes(b'TokenX\tbei\n')
   d=lib.eciNewDict(h);assert d and not lib.eciGetDict(h)
   assert lib.eciSetParam(h,9,0x40800)==0x40000
   filename=ctypes.create_string_buffer(str(path).encode('utf-16le')+b'\0\0')
   assert lib.eciLoadDict(h,d,0,filename)==0
   assert lib.eciSetParam(h,9,0x40000)==0x40800;checks+=2
   path.unlink() # Load owns the volume; attachment must not reopen it.
   assert lib.eciSetDict(h,d)==0 and lib.eciGetDict(h)==d
   assert speak(b'TokenX.')==plain
   assert lib.eciLoadDict(h,d,0,b'/does/not/exist')==6
   assert speak(b'TokenX.')==plain
   assert lib.eciLoadDict(h,d,3,str(path).encode())==7
   assert not lib.eciDeleteDict(h,d) and not lib.eciGetDict(h)
   assert speak(b'TokenX.')==unmodified;checks+=9
  normal=speak(b'Abc.')
  assert lib.eciSetParam(h,2,1)==0
  assert speak(b'Abc.')!=normal
  assert lib.eciSetParam(h,2,0)==1;checks+=3
  assert lib.eciSetParam(h,1,1)==0
  tagged=speak(b'`vs75 Hallo.')
  assert lib.eciSetParam(h,1,0)==1;checks+=2
 finally:lib.eciDelete(h)
 print(f'Standard ECI: {checks} export/voice/slot/name/language/code-set/dictionary/PCM/index checks; no speech files')
if __name__=='__main__':main()
