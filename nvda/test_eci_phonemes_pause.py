"""Standard phoneme output and pause/resume/Stop, for arbitrary DLL clients."""
import argparse
import ctypes as c
from pathlib import Path
import threading
import time


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--library',type=Path,required=True)
    args=p.parse_args();checks=0
    lib=(c.WinDLL if hasattr(c,'WinDLL') else c.CDLL)(str(args.library.resolve()))
    H=c.c_void_p;I=c.c_int
    spec={'eciNewEx':(H,(I,)),'eciDelete':(H,(H,)),
        'eciSetParam':(I,(H,I,I)),'eciGetParam':(I,(H,I)),
        'eciAddText':(I,(H,H)),'eciInsertIndex':(I,(H,I)),
        'eciGeneratePhonemes':(I,(H,I,H)),'eciSynthesize':(I,(H,)),
        'eciClearInput':(I,(H,)),'eciPause':(I,(H,I)),
        'eciStop':(I,(H,)),'eciSpeaking':(I,(H,)),
        'eciGetIndex':(I,(H,)),'eciSetOutputBuffer':(I,(H,I,H)),
        'eciNewDict':(H,(H,)),'eciSetDict':(I,(H,H)),
        'eciUpdateDict':(I,(H,H,I,H,H)),
        'eciProgStatus':(I,(H,))}
    for name,(result,params) in spec.items():
        f=getattr(lib,name);f.restype=result;f.argtypes=params
    CB=(c.WINFUNCTYPE if hasattr(c,'WINFUNCTYPE') else c.CFUNCTYPE)(I,H,I,I,H)
    lib.eciRegisterCallback.argtypes=(H,CB,H);lib.eciRegisterCallback.restype=None
    h=lib.eciNewEx(0x40000);assert h
    pcm=(c.c_short*4096)();guard=c.create_string_buffer(4098)
    assert lib.eciSetOutputBuffer(h,4096,pcm)
    audio=bytearray();phones=bytearray();indices=[];errors=[];seen=[]
    room=31;mode='normal';retried=False;paused=False;waiting=threading.Event()
    @CB
    def callback(handle,message,value,data):
        nonlocal retried,paused
        try:
            assert message in (0,1,2)
            if message==1:
                assert 0<value<=room
                payload=c.string_at(c.addressof(guard)+1,value)
                if mode=='abort':return 2
                if mode=='retry' and not retried:
                    retried=True;seen.append(payload);return 0
                if mode=='retry' and len(seen)==1:
                    assert seen[0]==payload;seen.append(payload)
                phones.extend(payload)
            elif message==0:audio.extend(c.string_at(pcm,2*value))
            else:indices.append((value,len(phones),lib.eciGetIndex(handle)))
            if mode=='pause' and message in (0,1) and not paused:
                paused=True;assert lib.eciPause(handle,1);waiting.set()
            return 1
        except BaseException as error:errors.append(error);return 2
    lib.eciRegisterCallback(h,callback,None)
    def queue(text=b'Hallo Welt.',indexed=True):
        assert lib.eciClearInput(h)
        if indexed:assert lib.eciInsertIndex(h,-1)
        if isinstance(text,list):
            for part in text:assert lib.eciAddText(h,part)
        else:assert lib.eciAddText(h,text)
        if indexed:assert lib.eciInsertIndex(h,23)
    def capture(text=b'Hallo Welt.',size=31,kind='normal',phonetic=True,indexed=True):
        nonlocal room,mode,retried,paused
        room=size;mode=kind;retried=paused=False;waiting.clear()
        audio.clear();phones.clear();indices.clear();errors.clear();seen.clear()
        c.memset(guard,0xa5,len(guard));queue(text,indexed)
        operation=lambda:lib.eciGeneratePhonemes(h,room,c.addressof(guard)+1) if phonetic else lib.eciSynthesize(h)
        if kind in ('pause','stop'):
            mode='pause';result=[]
            thread=threading.Thread(target=lambda:result.append(operation()));thread.start()
            assert waiting.wait(10),'no output before pause'
            progress=(len(audio),len(phones),len(indices));time.sleep(.03)
            assert thread.is_alive() and lib.eciSpeaking(h)
            assert progress==(len(audio),len(phones),len(indices))
            assert lib.eciStop(h) if kind=='stop' else lib.eciPause(h,0)
            thread.join(10);assert not thread.is_alive();ok=result[0]
        else:ok=operation()
        if errors:raise errors[0]
        assert guard.raw[0]==0xa5 and guard.raw[room+1]==0xa5
        return ok,bytes(phones),bytes(audio),tuple((i,n) for i,n,old in indices)
    try:
        # Mode and buffer errors must retain already accepted input.
        assert lib.eciSetParam(h,0,0)>=0
        queue();assert lib.eciGetParam(h,0)==0
        assert not lib.eciGeneratePhonemes(h,31,c.addressof(guard)+1)
        assert not lib.eciProgStatus(h);checks+=1
        assert lib.eciSetParam(h,0,1)==0
        assert not lib.eciGeneratePhonemes(h,0,c.addressof(guard)+1)
        assert not lib.eciGeneratePhonemes(h,31,None)
        assert lib.eciGeneratePhonemes(h,31,c.addressof(guard)+1) and phones;checks+=5
        normal=capture();assert normal[0] and normal[1] and not normal[2]
        assert [i for i,n in normal[3]]==[-1,23]
        assert normal[3][-1][1]==len(normal[1]) and lib.eciGetIndex(h)==23;checks+=4
        for size in (1,7,4096):assert capture(size=size)==normal;checks+=1
        assert capture([b'Hal',b'lo ',b'Welt.'])==normal;checks+=1
        assert capture(kind='retry')==normal and retried;checks+=2
        assert not capture(kind='abort')[0] and capture()==normal;checks+=2
        assert capture(kind='pause')==normal;checks+=4
        assert not capture(kind='stop')[0] and capture()==normal;checks+=4
        spoken=capture(phonetic=False);assert spoken[0] and spoken[2] and not spoken[1];checks+=2
        assert capture(phonetic=False,kind='pause')==spoken;checks+=4
        assert not capture(phonetic=False,kind='stop')[0] and capture(phonetic=False)==spoken;checks+=4
        for language,encoding,text in ((0x10000,'cp1252','The time is 13:28:25.'),
            (0x40000,'cp1252','Größe und zwölf Züge.'),
            (0x60000,'gbk','你好。'),(0x60001,'cp950','你好。'),
            (0x80000,'cp932','こんにちは。'),(0xa0000,'cp949','안녕하세요.'),
            (0x120000,'cp1253','Καλημέρα.')):
            assert lib.eciSetParam(h,9,language)>=0
            narrow=capture(text.encode(encoding),indexed=False)
            assert narrow[0] and narrow[1]
            assert lib.eciSetParam(h,9,language|0x800)==language
            wide=c.create_string_buffer(text.encode('utf-16le')+b'\0\0')
            assert capture(wide,indexed=False)==narrow;checks+=4
        assert lib.eciSetParam(h,9,0x40000)>=0
        d=lib.eciNewDict(h);assert d
        assert lib.eciUpdateDict(h,d,0,b'TokenX',b'Haus')==0 and lib.eciSetDict(h,d)==0
        assert capture(b'TokenX.')==capture(b'Haus.');checks+=3
        assert not lib.eciSpeaking(h) and not lib.eciPause(None,1);checks+=2
    finally:lib.eciStop(h);lib.eciDelete(h)
    print(f'ECI phonemes/pause: {checks} buffer/fragment/index/Unicode/dictionary/retry/abort/pause/stop/PCM-restoration checks; RAM only')


if __name__=='__main__':main()
