"""Standard ECI units, defaults, reset, long Unicode names and live indices."""
import argparse
import ctypes as c
from pathlib import Path


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--library',type=Path,required=True)
    args=parser.parse_args();checks=0
    lib=(c.WinDLL if hasattr(c,'WinDLL') else c.CDLL)(str(args.library.resolve()))
    H=c.c_void_p;I=c.c_int
    signatures={
        'eciNew':(H,()),'eciNewEx':(H,(I,)),'eciDelete':(H,(H,)),
        'eciSetParam':(I,(H,I,I)),'eciGetParam':(I,(H,I)),
        'eciGetDefaultParam':(I,(I,)),'eciSetDefaultParam':(I,(I,I)),
        'eciSetVoiceParam':(I,(H,I,I,I)),'eciGetVoiceParam':(I,(H,I,I)),
        'eciCopyVoice':(I,(H,I,I)), 'eciReset':(I,(H,)),
        'eciGetVoiceName':(I,(H,I,H)),'eciSetVoiceName':(I,(H,I,H)),
        'eciAddText':(I,(H,H)),'eciInsertIndex':(I,(H,I)),
        'eciGetIndex':(I,(H,)),'eciSynthesize':(I,(H,)),
        'eciSetOutputBuffer':(I,(H,I,c.POINTER(c.c_short))),
    }
    for name,(result,params) in signatures.items():
        f=getattr(lib,name);f.restype=result;f.argtypes=params
    CB=(c.WINFUNCTYPE if hasattr(c,'WINFUNCTYPE') else c.CFUNCTYPE)(I,H,I,I,H)
    lib.eciRegisterCallback.argtypes=(H,CB,H);lib.eciRegisterCallback.restype=None
    defaults={p:lib.eciGetDefaultParam(p) for p in (0,1,2,3,5,8,9)}
    h=None;other=None
    try:
        h=lib.eciNewEx(0x40000);assert h
        raw=[lib.eciGetVoiceParam(h,0,p) for p in range(8)]
        assert lib.eciSetParam(h,8,1)==0
        # Independent expected anchors on the original SDK calibration.
        assert lib.eciGetVoiceParam(h,0,2)==118
        assert lib.eciGetVoiceParam(h,0,6)==176
        assert lib.eciGetVoiceParam(h,0,7)==65535;checks+=3
        for p,value in ((2,123),(6,301),(7,20000)):
            before=lib.eciGetVoiceParam(h,0,p)
            assert lib.eciSetVoiceParam(h,0,p,value)==before
            assert lib.eciGetVoiceParam(h,0,p)==value
            assert lib.eciCopyVoice(h,0,9)
            assert lib.eciGetVoiceParam(h,9,p)==value
            assert lib.eciCopyVoice(h,9,0)
            assert lib.eciGetVoiceParam(h,0,p)==value;checks+=6
        for p in range(1,8):
            assert lib.eciSetVoiceParam(h,0,p,0)==-1;checks+=1
        for p,bad in ((2,24001),(6,69),(6,1298),(7,65536)):
            assert lib.eciSetVoiceParam(h,0,p,bad)==-1;checks+=1
        for hz in (1,39,423,1000,24000):
            assert lib.eciSetVoiceParam(h,0,2,hz)>=0
            assert lib.eciGetVoiceParam(h,0,2)==hz;checks+=2
        assert lib.eciSetVoiceParam(h,0,2,123)>=0
        assert lib.eciSetParam(h,8,0)==1
        raw_pitch=lib.eciGetVoiceParam(h,0,2)
        assert 0<=raw_pitch<=100 and raw_pitch!=123;checks+=2
        assert lib.eciSetParam(h,8,1)==0
        assert lib.eciGetVoiceParam(h,0,2)==123;checks+=1
        # A name is bounded by input code units, not UTF-8 storage bytes.
        assert lib.eciSetParam(h,9,0x40800)==0x40000
        name='Äöü日本語'*4+'名'*5
        assert len(name)==29
        storage=c.create_string_buffer(name.encode('utf-16le')+b'\0\0')
        assert lib.eciSetVoiceName(h,9,storage)
        result=c.create_string_buffer(60)
        assert lib.eciGetVoiceName(h,9,result)
        assert result.raw[:58].decode('utf-16le')==name
        assert lib.eciCopyVoice(h,9,10)
        assert lib.eciGetVoiceName(h,10,result)
        assert result.raw[:58].decode('utf-16le')==name;checks+=6
        oversized=c.create_string_buffer(('名'*30).encode('utf-16le')+b'\0\0')
        assert not lib.eciSetVoiceName(h,9,oversized);checks+=1
        # Defaults affect new engines; an existing one changes only on Reset.
        original_rate=lib.eciGetParam(h,5)
        changed={0:1,1:1,2:0,3:1,5:3,8:0,9:0x40000}
        for p,value in changed.items():
            assert lib.eciSetDefaultParam(p,value)==defaults[p];checks+=1
        assert lib.eciGetParam(h,5)==original_rate;checks+=1
        other=lib.eciNew();assert other
        for p,value in changed.items():
            assert lib.eciGetParam(other,p)==value;checks+=1
        assert lib.eciGetParam(h,9)==0x40800;checks+=1
        for p,value in ((5,7),(8,2),(9,0xb0000),(-1,0)):
            assert lib.eciSetDefaultParam(p,value)==-1;checks+=1
        assert lib.eciReset(h)
        for p,value in changed.items():
            assert lib.eciGetParam(h,p)==value;checks+=1
        assert [lib.eciGetVoiceParam(h,0,p) for p in range(8)]==raw;checks+=1
        namebuf=c.create_string_buffer(30)
        assert lib.eciGetVoiceName(h,9,namebuf) and namebuf.value==b'User-Defined';checks+=1
        # Callback/buffer remain valid, and GetIndex is safe in the callback.
        buffer=(c.c_short*1100)();seen=[];pcm=[]
        @CB
        def callback(handle,message,value,data):
            if message==0:pcm.append(c.string_at(buffer,2*value))
            elif message==2:seen.append((value,lib.eciGetIndex(handle)))
            return 1
        assert lib.eciSetOutputBuffer(h,1100,buffer)
        lib.eciRegisterCallback(h,callback,None)
        assert lib.eciInsertIndex(h,12) and lib.eciAddText(h,b'Hallo Welt.')
        assert lib.eciInsertIndex(h,23) and lib.eciSynthesize(h)
        assert pcm and seen==[(12,0),(23,12)] and lib.eciGetIndex(h)==23;checks+=4
        assert lib.eciReset(h) and lib.eciGetIndex(h)==23;checks+=1
        assert lib.eciAddText(h,b'Hallo Welt.') and lib.eciSynthesize(h);checks+=1
    finally:
        if h:lib.eciDelete(h)
        if other:lib.eciDelete(other)
        for p,value in defaults.items():lib.eciSetDefaultParam(p,value)
    print(f'Extended standard ECI: {checks} unit/default/reset/Unicode-name/live-index checks; audio in RAM only')


if __name__=='__main__':main()
