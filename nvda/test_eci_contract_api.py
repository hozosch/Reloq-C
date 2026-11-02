"""Client-independent ECI callbacks, errors, reset and live dictionary editing."""
import argparse
import ctypes as c
from pathlib import Path
import tempfile
import threading
import time


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    args=parser.parse_args();checks=0
    lib=(c.WinDLL if hasattr(c,'WinDLL') else c.CDLL)(str(args.library.resolve()))
    H=c.c_void_p;I=c.c_int;P=c.POINTER
    spec={
        'eciNewEx':(H,(I,)),'eciDelete':(H,(H,)),
        'eciSetParam':(I,(H,I,I)),'eciGetParam':(I,(H,I)),
        'eciGetDefaultParam':(I,(I,)), 'eciReset':(I,(H,)),
        'eciGetVoiceParam':(I,(H,I,I)),'eciSetVoiceParam':(I,(H,I,I,I)),
        'eciAddText':(I,(H,H)),'eciInsertIndex':(I,(H,I)),
        'eciSynthesize':(I,(H,)),'eciSynchronize':(I,(H,)),
        'eciStop':(I,(H,)),'eciClearInput':(I,(H,)),
        'eciGetIndex':(I,(H,)),'eciProgStatus':(I,(H,)),
        'eciClearErrors':(None,(H,)),'eciErrorMessage':(None,(H,H)),
        'eciIsBeingReentered':(I,(H,)),
        'eciSetOutputBuffer':(I,(H,I,P(c.c_short))),
        'eciNewDict':(H,(H,)),'eciSetDict':(I,(H,H)),
        'eciGetDict':(H,(H,)),'eciDeleteDict':(H,(H,H)),
        'eciLoadDict':(I,(H,H,I,H)),'eciSaveDict':(I,(H,H,I,H)),
        'eciDictLookup':(H,(H,H,I,H)),
        'eciDictLookupA':(I,(H,H,I,H,P(H),P(I))),
        'eciUpdateDict':(I,(H,H,I,H,H)),
        'eciUpdateDictA':(I,(H,H,I,H,H,I)),
        'eciDictFindFirst':(I,(H,H,I,P(H),P(H))),
        'eciDictFindNext':(I,(H,H,I,P(H),P(H))),
        'eciDictFindFirstA':(I,(H,H,I,P(H),P(H),P(I))),
        'eciDictFindNextA':(I,(H,H,I,P(H),P(H),P(I))),
    }
    for name,(result,params) in spec.items():
        f=getattr(lib,name);f.restype=result;f.argtypes=params
    CB=(c.WINFUNCTYPE if hasattr(c,'WINFUNCTYPE') else c.CFUNCTYPE)(I,H,I,I,H)
    lib.eciRegisterCallback.argtypes=(H,CB,H);lib.eciRegisterCallback.restype=None
    h=lib.eciNewEx(0x40000);other=None
    assert h and lib.eciProgStatus(h)==0 and not lib.eciIsBeingReentered(h)
    assert lib.eciProgStatus(None)==0x80 and not lib.eciIsBeingReentered(None);checks+=3
    buffer=(c.c_short*4096)();assert lib.eciSetOutputBuffer(h,len(buffer),buffer)
    pcm=bytearray();indices=[];attempts=[];retries={};mode='normal';errors=[]
    waiting=threading.Event()
    @CB
    def callback(handle,message,value,data):
        try:
            assert lib.eciIsBeingReentered(handle)==1
            payload=c.string_at(buffer,2*value) if message==0 else None
            attempts.append((message,value,lib.eciGetIndex(handle)))
            if mode=='reenter' and message==0:
                assert lib.eciSetParam(handle,0,0)==-1
                assert not lib.eciSynchronize(handle)
                assert not lib.eciDelete(handle)
                assert lib.eciProgStatus(handle)&0x800
                lib.eciClearErrors(handle)
                assert lib.eciProgStatus(handle)&0x800
            if mode=='wait' and message==0:waiting.set();return 0
            if mode=='abort' and message==0:return 2
            if mode=='retry' and message not in retries:
                retries[message]=(value,payload,time.perf_counter());return 0
            if mode=='retry' and message in retries and retries[message] is not None:
                old,repeated,when=retries[message]
                assert old==value and repeated==payload
                assert time.perf_counter()-when>=.015
                retries[message]=None
            if message==0:pcm.extend(payload)
            elif message==2:indices.append((value,len(pcm)))
            return -3 if mode=='other' else 1
        except BaseException as error:
            errors.append(error);return 2
    lib.eciRegisterCallback(h,callback,None)
    def capture(text=b'Stimme bei Gro\xdfbuchstaben anheben.',kind='normal'):
        nonlocal mode
        mode=kind;pcm.clear();indices.clear();attempts.clear();retries.clear();errors.clear()
        assert lib.eciClearInput(h) and lib.eciInsertIndex(h,11)
        assert lib.eciAddText(h,text) and lib.eciInsertIndex(h,23)
        ok=lib.eciSynthesize(h)
        if errors:raise errors[0]
        return ok,bytes(pcm),tuple(indices)
    def wide(text):return c.create_string_buffer(text.encode('utf-16le')+b'\0\0')
    def read_wide(pointer):
        units=c.cast(pointer,P(c.c_uint16));data=bytearray()
        for i in range(8192):
            if not units[i]:return data.decode('utf-16le')
            data.extend(int(units[i]).to_bytes(2,'little'))
        raise AssertionError('unterminated dictionary result')
    def find_all(d,extended=False,unicode=False):
        key=H();value=H();part=I(99);out={};first=True
        while True:
            fn=getattr(lib,'eciDictFind'+('First' if first else 'Next')+('A' if extended else ''))
            result=fn(h,d,0,c.byref(key),c.byref(value),c.byref(part)) if extended else fn(h,d,0,c.byref(key),c.byref(value))
            if result==4:break
            assert result==0
            if extended:assert part.value==0
            decode=read_wide if unicode else c.string_at
            out[decode(key)]=decode(value);first=False
        return out
    try:
        normal=capture();assert normal[0] and normal[1] and normal[2][-1]==(23,len(normal[1]));checks+=1
        assert capture(kind='retry')==normal
        # On the first index attempt GetIndex still reports the old value;
        # its retry sees the value published after that first callback.
        reports=[index for message,value,index in attempts if message==2 and value==11]
        assert reports==[23,11] and retries=={0:None,2:None};checks+=3
        waiting.clear();stopped=[]
        def stop():
            assert waiting.wait(10)
            time.sleep(.005);stopped.append(time.perf_counter());lib.eciStop(h)
        thread=threading.Thread(target=stop);thread.start()
        cancelled=capture(kind='wait');returned=time.perf_counter();thread.join(10)
        assert not thread.is_alive() and stopped and not cancelled[0] and not cancelled[1]
        assert returned-stopped[0]<.5
        assert capture()==normal;checks+=3
        assert not capture(kind='abort')[0] and capture()==normal;checks+=2
        assert capture(kind='other')==normal;checks+=1
        # All 32 index bits are payload. A failed AddText must keep input
        # that was already accepted instead of clearing the whole queue.
        mode='normal';pcm.clear();indices.clear()
        assert lib.eciClearInput(h) and lib.eciAddText(h,b'Hallo.')
        assert lib.eciInsertIndex(h,-1) and lib.eciInsertIndex(h,-2147483648)
        assert lib.eciInsertIndex(h,2147483647)
        assert not lib.eciAddText(h,None)
        assert lib.eciSynthesize(h) and pcm
        assert [i for i,size in indices]==[-1,-2147483648,2147483647]
        assert len({size for i,size in indices})==1
        assert lib.eciGetIndex(h)==2147483647;checks+=6
        # A successful call must not erase accumulated standard ECI errors.
        lib.eciClearErrors(h);assert not lib.eciProgStatus(h)
        assert lib.eciSetParam(h,5,999)==-1
        assert lib.eciSetParam(h,5,1)==1 and lib.eciProgStatus(h)==0x80
        message=c.create_string_buffer(100);lib.eciErrorMessage(h,message)
        assert message.value==b'Invalid or out of range parameter.';checks+=4
        assert capture(kind='reenter')==normal and lib.eciProgStatus(h)==0x880
        lib.eciErrorMessage(h,message);assert message.value==b'Cannot reenter ECI on the same thread.'
        lib.eciClearErrors(h);lib.eciErrorMessage(h,message)
        assert not lib.eciProgStatus(h) and not message.value;checks+=3
        # Reset works the same after standard controls and native aliases.
        assert lib.eciSetParam(h,0x1007,110)>=0
        assert lib.eciSetParam(h,0x1011,1)==0
        assert lib.eciReset(h)
        assert lib.eciGetParam(h,9)==lib.eciGetDefaultParam(9)
        assert lib.eciGetVoiceParam(h,0,6)==50 and lib.eciGetVoiceParam(h,0,2)==65
        assert lib.eciGetParam(h,0x1011)==1
        assert lib.eciSetParam(h,9,0x40000)>=0;checks+=6
        original=capture(b'TokenX.')
        d=lib.eciNewDict(h);assert d
        assert find_all(d)=={} and lib.eciDictLookup(h,d,0,b'missing') is None;checks+=2
        for volume in (0,1,2):
            key=(b'TokenX',b'RootToken',b'AbbvToken')[volume]
            assert lib.eciUpdateDict(h,d,volume,key,b'bei')==0
            assert c.string_at(lib.eciDictLookup(h,d,volume,key))==b'bei';checks+=2
        assert lib.eciSetDict(h,d)==0 and lib.eciGetDict(h)==d
        assert capture(b'TokenX.')==capture(b'bei.');checks+=2
        assert lib.eciUpdateDictA(h,d,0,b'TokenX',b'Haus',77)==0
        assert capture(b'TokenX.')==capture(b'Haus.');checks+=2
        # Dictionary activation belongs to its language, including when a
        # different language currently occupies the same engine handle.
        german=capture(b'TokenX.')
        assert lib.eciSetParam(h,9,0x10000)==0x40000 and not lib.eciGetDict(h)
        english=lib.eciNewDict(h);assert english
        assert lib.eciUpdateDict(h,english,0,b'TokenX',b'house')==0
        assert lib.eciSetDict(h,english)==0
        assert capture(b'TokenX.')==capture(b'house.');checks+=4
        assert lib.eciSetParam(h,9,0x40000)==0x10000 and lib.eciGetDict(h)==d
        assert capture(b'TokenX.')==german
        assert lib.eciSetDict(h,english)==0 and lib.eciGetDict(h)==d
        assert capture(b'TokenX.')==german;checks+=4
        assert lib.eciReset(h) and lib.eciGetParam(h,9)==0x10000
        assert lib.eciGetDict(h)==english
        assert capture(b'TokenX.')==capture(b'house.')
        assert lib.eciSetParam(h,9,0x40000)==0x10000
        assert not lib.eciDeleteDict(h,english) and lib.eciGetDict(h)==d;checks+=5
        assert lib.eciUpdateDict(h,d,0,b'bad\tkey',b'bad')==5
        assert capture(b'TokenX.')==capture(b'Haus.');checks+=2
        assert lib.eciUpdateDict(h,d,0,b'Gro\xdfe',b'gro\xdf')==0
        assert find_all(d)=={b'TokenX':b'Haus',b'Gro\xdfe':b'gro\xdf'}
        assert find_all(d,True)==find_all(d);checks+=3
        other=lib.eciNewEx(0x40000)
        assert other and not lib.eciGetDict(other)
        assert lib.eciDictLookup(other,d,0,b'TokenX') is None;checks+=2
        with tempfile.TemporaryDirectory(prefix='reloq-ä-') as temp:
            path=Path(temp)/'wörterbuch.dic'
            assert lib.eciSaveDict(h,d,0,str(path).encode('cp1252'))==0
            assert set(path.read_bytes().splitlines())=={b'TokenX\tHaus',b'Gro\xdfe\tgro\xdf'};checks+=2
            assert lib.eciSetParam(h,9,0x40800)==0x40000
            translated=H();part=I(9)
            assert not lib.eciDictLookup(h,d,0,wide('TokenX'))
            assert lib.eciDictLookupA(h,d,0,wide('Große'),c.byref(translated),c.byref(part))==0
            assert read_wide(translated)=='groß' and part.value==0
            assert find_all(d,True,True)=={'TokenX':'Haus','Große':'groß'};checks+=4
            assert lib.eciUpdateDictA(h,d,0,wide('Große'),wide('klein'),0)==0
            assert lib.eciDictLookupA(h,d,0,wide('Große'),c.byref(translated),None)==0
            assert read_wide(translated)=='klein';checks+=3
            assert lib.eciSaveDict(h,d,0,wide(str(path)))==0
            replacement=lib.eciNewDict(h);assert replacement
            assert lib.eciLoadDict(h,replacement,0,wide(str(path)))==0
            path.unlink()
            assert lib.eciSetDict(h,replacement)==0
            assert lib.eciDictLookupA(h,replacement,0,wide('Große'),c.byref(translated),None)==0
            assert read_wide(translated)=='klein';checks+=5
            assert lib.eciDictLookupA(h,replacement,3,wide('Große'),c.byref(translated),None)==7
            assert not translated.value;checks+=2
            assert lib.eciSetParam(h,5,99)==-1
            msg=c.create_string_buffer(200);lib.eciErrorMessage(h,msg)
            assert msg.raw[:68].decode('utf-16le').rstrip('\0')=='Invalid or out of range parameter.';checks+=1
            assert lib.eciSetParam(h,9,0x40000)==0x40800
            assert not lib.eciDeleteDict(h,replacement) and not lib.eciGetDict(h);checks+=1
        assert lib.eciUpdateDict(h,d,0,b'TokenX',None)==0
        assert lib.eciDictLookup(h,d,0,b'TokenX') is None
        assert lib.eciSetDict(h,d)==0 and capture(b'TokenX.')==original;checks+=3
        assert not lib.eciDeleteDict(h,d);checks+=1
    finally:
        if other:lib.eciDelete(other)
        lib.eciDelete(h)
    print(f'ECI contract: {checks} retry/abort/stop/error/reentry/reset/dictionary/read-write/Unicode/PCM/index checks; no audio files')


if __name__=='__main__':main()
