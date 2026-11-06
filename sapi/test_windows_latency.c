/* Measure the real Windows SAPI path up to the application stream.  This is
 * not a measurement of the sound device or of NVDA's playback queue. */
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef struct timed_stream {
    IStream iface;
    IStream *memory;
    LONG refs;
    LARGE_INTEGER start,first,ready;
    ULONGLONG bytes,required;
    unsigned writes;
} timed_stream;
static HRESULT STDMETHODCALLTYPE query(IStream *p,REFIID iid,void **out)
{
    if(!out)return E_POINTER;*out=NULL;
    if(IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_ISequentialStream)||IsEqualIID(iid,&IID_IStream)){
        *out=p;InterlockedIncrement(&((timed_stream *)p)->refs);return S_OK;
    }
    return E_NOINTERFACE;
}
static ULONG STDMETHODCALLTYPE addref(IStream *p){return (ULONG)InterlockedIncrement(&((timed_stream *)p)->refs);}
static ULONG STDMETHODCALLTYPE release(IStream *p){return (ULONG)InterlockedDecrement(&((timed_stream *)p)->refs);}
static HRESULT STDMETHODCALLTYPE read_stream(IStream *p,void *data,ULONG size,ULONG *read)
{return IStream_Read(((timed_stream *)p)->memory,data,size,read);}
static HRESULT STDMETHODCALLTYPE write_stream(IStream *p,const void *data,ULONG size,ULONG *written)
{
    timed_stream *s=(timed_stream *)p;LARGE_INTEGER when;QueryPerformanceCounter(&when);
    if(size&&!s->first.QuadPart)s->first=when;
    HRESULT hr=IStream_Write(s->memory,data,size,written);
    if(SUCCEEDED(hr)){s->bytes+=written?*written:size;++s->writes;
        if(!s->ready.QuadPart&&s->bytes>=s->required)s->ready=when;}
    return hr;
}
static HRESULT STDMETHODCALLTYPE seek_stream(IStream *p,LARGE_INTEGER offset,DWORD origin,ULARGE_INTEGER *pos)
{return IStream_Seek(((timed_stream *)p)->memory,offset,origin,pos);}
static HRESULT STDMETHODCALLTYPE resize_stream(IStream *p,ULARGE_INTEGER size)
{return IStream_SetSize(((timed_stream *)p)->memory,size);}
static HRESULT STDMETHODCALLTYPE copy_stream(IStream *p,IStream *to,ULARGE_INTEGER count,ULARGE_INTEGER *read,ULARGE_INTEGER *written)
{return IStream_CopyTo(((timed_stream *)p)->memory,to,count,read,written);}
static HRESULT STDMETHODCALLTYPE commit_stream(IStream *p,DWORD flags)
{return IStream_Commit(((timed_stream *)p)->memory,flags);}
static HRESULT STDMETHODCALLTYPE revert_stream(IStream *p){return IStream_Revert(((timed_stream *)p)->memory);}
static HRESULT STDMETHODCALLTYPE lock_stream(IStream *p,ULARGE_INTEGER at,ULARGE_INTEGER size,DWORD type)
{return IStream_LockRegion(((timed_stream *)p)->memory,at,size,type);}
static HRESULT STDMETHODCALLTYPE unlock_stream(IStream *p,ULARGE_INTEGER at,ULARGE_INTEGER size,DWORD type)
{return IStream_UnlockRegion(((timed_stream *)p)->memory,at,size,type);}
static HRESULT STDMETHODCALLTYPE stat_stream(IStream *p,STATSTG *stat,DWORD flags)
{return IStream_Stat(((timed_stream *)p)->memory,stat,flags);}
static HRESULT STDMETHODCALLTYPE clone_stream(IStream *p,IStream **other){(void)p;(void)other;return E_NOTIMPL;}
static IStreamVtbl stream_vtable={query,addref,release,read_stream,write_stream,seek_stream,
    resize_stream,copy_stream,commit_stream,revert_stream,lock_stream,unlock_stream,stat_stream,clone_stream};
static double ms(LARGE_INTEGER end,LARGE_INTEGER start,LARGE_INTEGER frequency)
{return 1000.0*(double)(end.QuadPart-start.QuadPart)/(double)frequency.QuadPart;}
int wmain(void)
{
    setvbuf(stdout,NULL,_IONBF,0);LARGE_INTEGER frequency;QueryPerformanceFrequency(&frequency);
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 1;
    ISpVoice *voice=NULL;ISpObjectToken *token=NULL;ISpStream *stream=NULL;timed_stream output={0};
    output.iface.lpVtbl=&stream_vtable;output.refs=1;output.required=1104;
    hr=CoCreateInstance(&CLSID_SpVoice,NULL,CLSCTX_INPROC_SERVER,&IID_ISpVoice,(void **)&voice);
    if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpObjectToken,NULL,CLSCTX_INPROC_SERVER,&IID_ISpObjectToken,(void **)&token);
    if(SUCCEEDED(hr))hr=ISpObjectToken_SetId(token,NULL,L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\ReLoqC_01_00040000",FALSE);
    if(SUCCEEDED(hr))hr=ISpVoice_SetVoice(voice,token);
    if(SUCCEEDED(hr))hr=CreateStreamOnHGlobal(NULL,TRUE,&output.memory);
    if(SUCCEEDED(hr))hr=CoCreateInstance(&CLSID_SpStream,NULL,CLSCTX_INPROC_SERVER,&IID_ISpStream,(void **)&stream);
    WAVEFORMATEX fmt={WAVE_FORMAT_PCM,1,11025,22050,2,16,0};
    if(SUCCEEDED(hr))hr=ISpStream_SetBaseStream(stream,&output.iface,&SPDFID_WaveFormatEx,&fmt);
    if(SUCCEEDED(hr))hr=ISpVoice_SetOutput(voice,(IUnknown *)stream,FALSE);
    /* NVDA asks for bookmarks rather than word or sentence events. */
    ULONGLONG interests=(1ULL<<SPEI_START_INPUT_STREAM)|(1ULL<<SPEI_TTS_BOOKMARK)|(1ULL<<SPEI_END_INPUT_STREAM);
    if(SUCCEEDED(hr))hr=ISpVoice_SetInterest(voice,interests,interests);
    for(int words=1;SUCCEEDED(hr)&&words<=100;words*=100){
        WCHAR message[1800]=L"<pitch absmiddle=\"0\">";
        for(int i=0;i<words;i++)wcscat(message,L"Beispiel ");
        wcscat(message,L"<bookmark mark=\"7\"/></pitch>");
        for(int iteration=0;SUCCEEDED(hr)&&iteration<12;++iteration){
            LARGE_INTEGER zero={0};ULARGE_INTEGER empty={0};
            hr=IStream_SetSize(output.memory,empty);if(SUCCEEDED(hr))hr=IStream_Seek(output.memory,zero,STREAM_SEEK_SET,NULL);
            output.first.QuadPart=output.ready.QuadPart=0;output.bytes=output.writes=0;
            QueryPerformanceCounter(&output.start);
            if(SUCCEEDED(hr))hr=ISpVoice_Speak(voice,message,SPF_IS_XML,NULL);
            if(SUCCEEDED(hr)&&(!output.first.QuadPart||!output.ready.QuadPart))hr=E_FAIL;
            if(SUCCEEDED(hr))printf("words=%d iteration=%d first_ms=%.3f ready_ms=%.3f bytes=%llu writes=%u\n",words,iteration,
                ms(output.first,output.start,frequency),ms(output.ready,output.start,frequency),(unsigned long long)output.bytes,output.writes);
        }
    }
    if(stream)ISpStream_Release(stream);if(output.memory)IStream_Release(output.memory);
    if(token)ISpObjectToken_Release(token);if(voice)ISpVoice_Release(voice);CoUninitialize();
    if(FAILED(hr))fprintf(stderr,"SAPI latency test failed: %08lx\n",(unsigned long)hr);
    return FAILED(hr)?1:0;
}
