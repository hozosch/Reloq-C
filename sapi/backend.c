/* The owner thread keeps one initialized language. Only eight small live PCM
 * packets cross to SAPI's caller thread; no speech result is retained. */
#include "settings.h"
#define RELOQ_ECI_STATIC
#include "reloq_eci.h"
#include "reloq_voice_controls.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

struct core_api {
    HMODULE module;
    ECIHand (ECICALL *new_ex)(int);
    ECIHand (ECICALL *destroy)(ECIHand);
    void (ECICALL *callback)(ECIHand,ECICallback,void *);
    int (ECICALL *buffer)(ECIHand,int,short *);
    int (ECICALL *set)(ECIHand,int,int);
    int (ECICALL *add)(ECIHand,const void *,size_t,int);
    int (ECICALL *index)(ECIHand,int);
    int (ECICALL *pitch)(ECIHand,int);
    int (ECICALL *synthesize)(ECIHand);
    int (ECICALL *stop)(ECIHand);
    int (ECICALL *clear)(ECIHand);
    int (ECICALL *dictionary)(ECIHand,const char *);
};
static struct core_api core;
static INIT_ONCE core_once=INIT_ONCE_STATIC_INIT;
static BOOL CALLBACK load_core(PINIT_ONCE once,PVOID unused,PVOID *context)
{
    (void)once;(void)unused;(void)context;
    WCHAR path[2048];if(!sapi_module_path(path,2048,L"eci.dll"))return FALSE;
    core.module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!core.module)return FALSE;
#define LOAD(field,name) do {FARPROC p=GetProcAddress(core.module,name); \
    if(!p)goto fail;memcpy(&core.field,&p,sizeof p);}while(0)
    LOAD(new_ex,"eciNewEx");LOAD(destroy,"eciDelete");LOAD(callback,"eciRegisterCallback");
    LOAD(buffer,"eciSetOutputBuffer");LOAD(set,"eciSetParam");LOAD(add,"eciAddTextUtf8");
    LOAD(index,"eciInsertIndex");LOAD(pitch,"eciInsertPitch");LOAD(synthesize,"eciSynthesize");
    LOAD(stop,"eciStop");LOAD(clear,"eciClearInput");LOAD(dictionary,"eciSetDictionaryPath");
#undef LOAD
    /* The initialized provider owns process/thread-local lookup state. Keep
     * its adapter mapped until process exit; per-voice workers still close. */
    HMODULE pinned=NULL;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN|GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCWSTR)&core_once,&pinned))goto fail;
    InterlockedIncrement(&sapi_locks);
    return TRUE;
fail:FreeLibrary(core.module);memset(&core,0,sizeof core);return FALSE;
}
struct packet {int type;ULONG bytes,volume;SPEVENT event;HRESULT result;short pcm[512];};
struct job {
    struct sapi_settings settings;const SPVTEXTFRAG *fragments;
    DWORD flags,rate;LONG speed;USHORT volume;ULONGLONG interest;
};
struct bookmark {struct bookmark *next;WCHAR text[];};
struct sapi_backend {
    HANDLE thread,start,room,ready;volatile LONG cancel,exit,busy;
    void *volatile cancel_handle;
    unsigned put,get;struct packet packets[8];struct job job;
    int dialect;ULONG fragment_volume;ECIHand handle;short pcm[512];
    SPEVENT *events;size_t event_count,event_capacity;
    struct bookmark *bookmarks;
};
static void cancel_backend(struct sapi_backend *b)
{
    if(InterlockedExchange(&b->cancel,1))return;
    /* Stop is the ECI API's explicit cross-thread exception. Signal the
     * native planner as well as the small output queue. */
    ECIHand handle=InterlockedCompareExchangePointer(&b->cancel_handle,NULL,NULL);
    if(handle)core.stop(handle);
}
static int put(struct sapi_backend *b,const struct packet *packet)
{
    for(;;){if(InterlockedCompareExchange(&b->cancel,0,0))return 0;
        DWORD rc=WaitForSingleObject(b->room,10);if(rc==WAIT_TIMEOUT)continue;
        if(rc!=WAIT_OBJECT_0)return 0;break;}
    b->packets[b->put]=*packet;b->put=(b->put+1)%8;
    return ReleaseSemaphore(b->ready,1,NULL)!=0;
}
static int ECICALL callback(ECIHand handle,int message,int value,void *context)
{
    struct sapi_backend *b=context;(void)handle;
    if(InterlockedCompareExchange(&b->cancel,0,0))return 0;
    struct packet packet={0};
    if(message==RELOQ_ECI_WAVEFORM_BUFFER){
        if(value<0||value>512)return 0;
        packet.type=1;packet.bytes=(ULONG)value*2;packet.volume=b->fragment_volume;
        memcpy(packet.pcm,b->pcm,packet.bytes);
    }else if(message==RELOQ_ECI_INDEX_REPLY){
        if(value<1||(size_t)value>b->event_count)return 0;
        packet.type=2;packet.event=b->events[value-1];
    }else return 1;
    return put(b,&packet);
}
static struct bookmark *bookmark_copy(LPCWSTR text,size_t length)
{
    if(length>(SIZE_MAX-sizeof(struct bookmark))/sizeof(WCHAR)-1)return NULL;
    struct bookmark *mark=malloc(sizeof *mark+(length+1)*sizeof(WCHAR));if(!mark)return NULL;
    memcpy(mark->text,text,length*sizeof(WCHAR));mark->text[length]=0;return mark;
}
static int event_mark(struct sapi_backend *b,WORD kind,ULONG offset,ULONG length,LPCWSTR name)
{
    if(!(b->job.interest&((ULONGLONG)1<<kind)))return 1;
    if(b->event_count>=INT_MAX)return 0;
    if(b->event_count==b->event_capacity){size_t n=b->event_capacity?b->event_capacity*2:64;
        if(n>SIZE_MAX/sizeof *b->events)return 0;
        void *p=realloc(b->events,n*sizeof *b->events);if(!p)return 0;
        b->events=p;b->event_capacity=n;}
    SPEVENT event={0};event.eEventId=kind;
    if(kind==SPEI_TTS_BOOKMARK){event.elParamType=SPET_LPARAM_IS_STRING;
        struct bookmark *mark=bookmark_copy(name,length);if(!mark)return 0;
        mark->next=b->bookmarks;b->bookmarks=mark;
        event.wParam=(WPARAM)wcstoul(mark->text,NULL,10);event.lParam=(LPARAM)mark->text;
    }else{event.elParamType=SPET_LPARAM_IS_UNDEFINED;event.wParam=length;event.lParam=offset;}
    b->events[b->event_count++]=event;
    return core.index(b->handle,(int)b->event_count);
}
static int add_wide(struct sapi_backend *b,const WCHAR *text,size_t length,int spell)
{
    if(!length)return 1;
    if(length>INT_MAX)return 0;
    int n=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text,(int)length,NULL,0,NULL,NULL);
    if(n<=0)return 0;
    char local[512];char *utf8=(size_t)n<=sizeof local?local:malloc((size_t)n);if(!utf8)return 0;
    int ok=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,text,(int)length,utf8,n,NULL,NULL)==n&&
        core.add(b->handle,utf8,(size_t)n,spell);
    if(utf8!=local)free(utf8);return ok;
}
static int native_pitch(int base,LONG adjustment)
{
    double target=reloq_voice_pitch_f0_scale(base)*pow(2.0,adjustment/10.0);
    int best=0;double error=HUGE_VAL;
    for(int i=0;i<=100;++i){double d=fabs(log(reloq_voice_pitch_f0_scale(i)/target));
        if(d<error){error=d;best=i;}}
    return best;
}
static HRESULT configure(struct sapi_backend *b)
{
    struct job *j=&b->job;const struct sapi_settings *s=&j->settings;
    static const int rates[]={8000,11025,22050,16000,32000,44100,48000};
    int rate=-1;for(int i=0;i<7;++i)if((DWORD)rates[i]==j->rate)rate=i;
    if(rate<0)return E_INVALIDARG;
    if(!b->handle){b->handle=core.new_ex(b->dialect);if(!b->handle)return E_OUTOFMEMORY;
        InterlockedExchangePointer(&b->cancel_handle,b->handle);
        core.callback(b->handle,callback,b);if(!core.buffer(b->handle,512,b->pcm))return E_FAIL;}
    if(!core.clear(b->handle))return E_FAIL;
    struct {int key,value;} parameters[]={
        {RELOQ_ECI_VOICE_MODE,1},{RELOQ_ECI_SYNTH_MODE,1},{RELOQ_ECI_SAMPLE_RATE,rate},
        {RELOQ_ECI_NATIVE_SPEED,reloq_voice_speed_from_nvda((int)(j->speed+10)*5,s->boost)},
        {RELOQ_ECI_NATIVE_PITCH,(int)s->pitch},{RELOQ_ECI_NATIVE_VOLUME,100},
        {RELOQ_ECI_NATIVE_INFLECTION,(int)s->inflection},{RELOQ_ECI_HEAD_SIZE,(int)s->head},
        {RELOQ_ECI_ROUGHNESS,(int)s->roughness},{RELOQ_ECI_BREATHINESS,(int)s->breathiness},
        {RELOQ_ECI_GENDER,(int)s->gender},{RELOQ_ECI_PROCESS_TAGS,(int)s->tags},
        {RELOQ_ECI_PAUSE_MODE,(int)s->pauses},{RELOQ_ECI_DICTIONARY,!s->abbreviations},
        {RELOQ_ECI_PHRASE_PREDICTION,(int)s->prediction}};
    for(size_t i=0;i<sizeof parameters/sizeof *parameters;++i)
        if(core.set(b->handle,parameters[i].key,parameters[i].value)==-1)return E_FAIL;
    WCHAR path[2048];char utf8[8192];const char *directory=NULL;
    if(s->dictionaries){
        if(s->directory[0])wcscpy(path,s->directory);
        else if(!sapi_module_path(path,2048,L"dictionaries"))return E_FAIL;
        if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,path,-1,utf8,sizeof utf8,NULL,NULL))return E_INVALIDARG;
        directory=utf8;
    }
    return core.dictionary(b->handle,directory)?S_OK:E_FAIL;
}
static int sentence_end(WCHAR c)
{return c==L'.'||c==L'!'||c==L'?'||c==0x3002||c==0xff01||c==0xff1f;}
static ULONG sentence_length(const SPVTEXTFRAG *fragment,size_t at)
{
    uint64_t begin=(uint64_t)fragment->ulTextSrcOffset+at,end=begin;
    for(const SPVTEXTFRAG *f=fragment;f;f=f->pNext,at=0){
        if(f->State.eAction!=SPVA_Speak&&f->State.eAction!=SPVA_SpellOut&&f->State.eAction!=SPVA_Pronounce)continue;
        for(;at<f->ulTextLen;++at){end=(uint64_t)f->ulTextSrcOffset+at+1;
            if(sentence_end(f->pTextStart[at]))return end>begin&&end-begin<=ULONG_MAX?(ULONG)(end-begin):0;}
    }
    return end>begin&&end-begin<=ULONG_MAX?(ULONG)(end-begin):0;
}
static HRESULT synthesize(struct sapi_backend *b)
{
    HRESULT hr=configure(b);if(FAILED(hr))return hr;
    int sentence=1,has_text=0,last_pitch=(int)b->job.settings.pitch;
    int last_rate=(int)b->job.speed,last_range=0,last_emphasis=0;
    size_t delivered_events=0;b->fragment_volume=100;
    b->event_count=0;
    for(const SPVTEXTFRAG *f=b->job.fragments;f;f=f->pNext){
        if(InterlockedCompareExchange(&b->cancel,0,0))return S_OK;
        if(f->State.eAction==SPVA_Bookmark){
            if(!f->pTextStart||!event_mark(b,SPEI_TTS_BOOKMARK,0,f->ulTextLen,f->pTextStart))return E_FAIL;
            continue;
        }
        if(f->State.eAction==SPVA_Silence){
            /* Silence is represented by a packet, outside the pronunciation
             * frontend. Flush only where the host explicitly requested it. */
            if(has_text){if(!core.synthesize(b->handle)||!core.clear(b->handle))return E_FAIL;has_text=0;delivered_events=b->event_count;}
            uint64_t samples=(uint64_t)b->job.rate*f->State.SilenceMSecs/1000;
            struct packet p={.type=1,.volume=100};
            while(samples){size_t n=samples>512?512:(size_t)samples;p.bytes=(ULONG)n*2;
                if(!put(b,&p))return S_OK;samples-=n;}
            continue;
        }
        if(f->State.eAction==SPVA_ParseUnknownTag||f->State.eAction==SPVA_Section)continue;
        if(f->State.eAction!=SPVA_Speak&&f->State.eAction!=SPVA_SpellOut&&f->State.eAction!=SPVA_Pronounce)return E_NOTIMPL;
        if(f->State.eAction==SPVA_Pronounce&&f->State.pPhoneIds)return E_NOTIMPL;
        if(!f->pTextStart&&f->ulTextLen)return E_INVALIDARG;
        int speed=(int)b->job.speed+(int)f->State.RateAdj;
        if(speed< -10)speed=-10;if(speed>10)speed=10;
        ULONG fragment_volume=f->State.Volume>100?100:f->State.Volume;
        if(speed!=last_rate||f->State.PitchAdj.RangeAdj!=last_range||f->State.EmphAdj!=last_emphasis||
           fragment_volume!=b->fragment_volume){
            if(has_text){if(!core.synthesize(b->handle)||!core.clear(b->handle))return E_FAIL;
                has_text=0;delivered_events=b->event_count;}
            int inflection=(int)lround(b->job.settings.inflection*pow(2.0,f->State.PitchAdj.RangeAdj/10.0))+f->State.EmphAdj*3;
            if(inflection<0)inflection=0;if(inflection>100)inflection=100;
            if(core.set(b->handle,RELOQ_ECI_NATIVE_SPEED,reloq_voice_speed_from_nvda((speed+10)*5,b->job.settings.boost))==-1||
               core.set(b->handle,RELOQ_ECI_NATIVE_INFLECTION,inflection)==-1)return E_FAIL;
            last_rate=speed;last_range=f->State.PitchAdj.RangeAdj;last_emphasis=f->State.EmphAdj;
            b->fragment_volume=fragment_volume;
            if(core.set(b->handle,RELOQ_ECI_NATIVE_PITCH,last_pitch)==-1)return E_FAIL;
        }
        /* SAPI already parsed XML. Absolute pitch marks preserve the native
         * analyzer's single connected utterance across ordinary fragments. */
        int pitch=native_pitch((int)b->job.settings.pitch,f->State.PitchAdj.MiddleAdj);
        if(pitch!=last_pitch){if(!core.pitch(b->handle,pitch))return E_FAIL;last_pitch=pitch;}
        if(!(b->job.interest&(((ULONGLONG)1<<SPEI_WORD_BOUNDARY)|((ULONGLONG)1<<SPEI_SENTENCE_BOUNDARY)))){
            if(!add_wide(b,f->pTextStart,f->ulTextLen,f->State.eAction==SPVA_SpellOut))return E_FAIL;
            has_text|=f->ulTextLen!=0;continue;
        }
        size_t at=0;
        while(at<f->ulTextLen){size_t start=at;
            if(InterlockedCompareExchange(&b->cancel,0,0))return S_OK;
            WCHAR c=f->pTextStart[at];
            if(iswspace(c)||iswpunct(c)){++at;
                if(sentence_end(c))sentence=1;
                if(!add_wide(b,f->pTextStart+start,at-start,f->State.eAction==SPVA_SpellOut))return E_FAIL;
            }else{
                do{++at;if(c>=0xd800&&c<=0xdbff&&at<f->ulTextLen)++at;
                    if(at>=f->ulTextLen)break;c=f->pTextStart[at];
                }while(!iswspace(c)&&!iswpunct(c));
                if(start>ULONG_MAX-f->ulTextSrcOffset||at-start>ULONG_MAX)return E_INVALIDARG;
                if(sentence){if(!event_mark(b,SPEI_SENTENCE_BOUNDARY,f->ulTextSrcOffset+(ULONG)start,sentence_length(f,start),NULL))return E_FAIL;sentence=0;}
                if(!event_mark(b,SPEI_WORD_BOUNDARY,f->ulTextSrcOffset+(ULONG)start,(ULONG)(at-start),NULL)||
                   !add_wide(b,f->pTextStart+start,at-start,f->State.eAction==SPVA_SpellOut))return E_FAIL;
            }
            has_text=1;
        }
    }
    if(has_text&&!core.synthesize(b->handle))return InterlockedCompareExchange(&b->cancel,0,0)?S_OK:E_FAIL;
    /* Bookmark-only streams still need their zero-position index callbacks. */
    if(!has_text&&b->event_count>delivered_events){for(size_t i=delivered_events;i<b->event_count;++i){
        struct packet p={.type=2,.event=b->events[i]};if(!put(b,&p))return S_OK;}}
    return S_OK;
}
static DWORD WINAPI worker(void *context)
{
    struct sapi_backend *b=context;
    for(;;){if(WaitForSingleObject(b->start,INFINITE)!=WAIT_OBJECT_0)break;
        if(InterlockedCompareExchange(&b->exit,0,0))break;
        HRESULT hr=synthesize(b);
        /* Cancellation discards queued packets on the caller, but always
         * acknowledges the job before its borrowed fragment list expires. */
        if(b->handle)core.clear(b->handle);
        free(b->events);b->events=NULL;b->event_count=0;b->event_capacity=0;
        struct packet p={.type=3,.result=hr};
        while(WaitForSingleObject(b->room,10)==WAIT_TIMEOUT){}
        b->packets[b->put]=p;b->put=(b->put+1)%8;ReleaseSemaphore(b->ready,1,NULL);
    }
    InterlockedExchangePointer(&b->cancel_handle,NULL);
    if(b->handle)core.destroy(b->handle);return 0;
}
struct sapi_backend *sapi_backend_new(int dialect)
{
    if(!InitOnceExecuteOnce(&core_once,load_core,NULL,NULL))return NULL;
    struct sapi_backend *b=calloc(1,sizeof *b);if(!b)return NULL;b->dialect=dialect;
    b->start=CreateEventW(NULL,FALSE,FALSE,NULL);b->room=CreateSemaphoreW(NULL,8,8,NULL);
    b->ready=CreateSemaphoreW(NULL,0,8,NULL);
    if(b->start&&b->room&&b->ready)b->thread=CreateThread(NULL,0,worker,b,0,NULL);
    if(!b->thread){sapi_backend_delete(b);return NULL;}return b;
}
void sapi_backend_delete(struct sapi_backend *b)
{
    if(!b)return;
    if(b->thread){InterlockedExchange(&b->exit,1);SetEvent(b->start);
        WaitForSingleObject(b->thread,INFINITE);CloseHandle(b->thread);}
    if(b->start)CloseHandle(b->start);if(b->room)CloseHandle(b->room);if(b->ready)CloseHandle(b->ready);
    free(b->events);free(b);
}
HRESULT sapi_backend_speak(struct sapi_backend *b,const struct sapi_settings *settings,DWORD flags,
    const WAVEFORMATEX *format,const SPVTEXTFRAG *fragments,ISpTTSEngineSite *site)
{
    if(InterlockedCompareExchange(&b->busy,1,0))return HRESULT_FROM_WIN32(ERROR_BUSY);
    LONG speed=0;USHORT volume=100;site->lpVtbl->GetRate(site,&speed);site->lpVtbl->GetVolume(site,&volume);
    if(speed< -10)speed=-10;if(speed>10)speed=10;if(volume>100)volume=100;
    ULONGLONG interest=0;site->lpVtbl->GetEventInterest(site,&interest);
    b->job=(struct job){.settings=*settings,.fragments=fragments,.flags=flags,.rate=format->nSamplesPerSec,.speed=speed,.volume=volume,.interest=interest};
    InterlockedExchange(&b->cancel,0);SetEvent(b->start);ULONGLONG offset=0;HRESULT result=S_OK;
    for(;;){DWORD action=site->lpVtbl->GetActions(site);
        if(action&SPVES_ABORT)cancel_backend(b);
        if(action&SPVES_SKIP){SPVSKIPTYPE kind;LONG count=0;
            if(SUCCEEDED(site->lpVtbl->GetSkipInfo(site,&kind,&count)))site->lpVtbl->CompleteSkip(site,0);
            /* No fabricated sentence boundary: stop this job if the caller
             * requests an unsupported skip, as the SAPI DDI specifies. */
            cancel_backend(b);
        }
        if(action&SPVES_VOLUME){USHORT v=volume;if(SUCCEEDED(site->lpVtbl->GetVolume(site,&v)))volume=v>100?100:v;}
        if(action&SPVES_RATE){LONG next=0;site->lpVtbl->GetRate(site,&next);}
        DWORD wait=WaitForSingleObject(b->ready,10);if(wait==WAIT_TIMEOUT)continue;
        if(wait!=WAIT_OBJECT_0){cancel_backend(b);result=E_FAIL;continue;}
        struct packet p=b->packets[b->get];b->get=(b->get+1)%8;ReleaseSemaphore(b->room,1,NULL);
        if(p.type==3){if(SUCCEEDED(result))result=InterlockedCompareExchange(&b->cancel,0,0)?S_OK:p.result;break;}
        if(InterlockedCompareExchange(&b->cancel,0,0))continue;
        if(p.type==2){p.event.ullAudioStreamOffset=offset;
            HRESULT hr=site->lpVtbl->AddEvents(site,&p.event,1);
            if(FAILED(hr)){result=hr;cancel_backend(b);}
        }else if(p.type==1){
            if(volume!=100||p.volume!=100)for(ULONG i=0;i<p.bytes/2;++i)p.pcm[i]=(short)((int64_t)p.pcm[i]*volume*p.volume/10000);
            ULONG at=0;while(at<p.bytes){ULONG written=0;
                HRESULT hr=site->lpVtbl->Write(site,(BYTE *)p.pcm+at,p.bytes-at,&written);
                if(site->lpVtbl->GetActions(site)&SPVES_ABORT){cancel_backend(b);break;}
                if(FAILED(hr)||!written||written>p.bytes-at||(written&1)){
                    result=FAILED(hr)?hr:E_FAIL;cancel_backend(b);break;}
                at+=written;offset+=written;
            }
        }
    }
    while(b->bookmarks){struct bookmark *next=b->bookmarks->next;free(b->bookmarks);b->bookmarks=next;}
    InterlockedExchange(&b->busy,0);return result;
}
