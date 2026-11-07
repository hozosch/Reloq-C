/* Synthesize on SAPI's caller thread and deliver each native PCM block
 * directly to its site. Each caller owns its initialized ECI contexts. */
#include "settings.h"
#define RELOQ_ECI_STATIC
#include "reloq_eci.h"
#include "reloq_voice_controls.h"
#include "speed.h"
#include "pitch.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#ifdef RELOQ_SAPI_TEST_HOST
#include <pthread.h>
#endif

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
struct output;
struct sapi_backend {
    volatile LONG cancel,busy;
    struct job job;int warming,cached;
    int dialect;ULONG fragment_volume;ECIHand handle;short pcm[512];
    struct output *output;
    SPEVENT *events;size_t event_count,event_capacity;
    struct bookmark *bookmarks;
    int dictionary_ready;WCHAR dictionary_path[2048];
    WIN32_FILE_ATTRIBUTE_DATA dictionary_files[6];
    HANDLE dictionary_watch;FILETIME dictionary_directory_created;
};
static int output_pcm(struct output *,const short *,size_t,ULONG);
static int output_event(struct output *,SPEVENT);
static void cancel_backend(struct sapi_backend *b)
{
    if(InterlockedExchange(&b->cancel,1))return;
    /* The owner stops inside its next callback; a completed render needs no stop. */
}
static int put(struct sapi_backend *b,const struct packet *packet)
{
    if(InterlockedCompareExchange(&b->cancel,0,0)||!b->output)return 0;
    if(packet->type==1)return output_pcm(b->output,packet->pcm,packet->bytes/2,packet->volume);
    if(packet->type==2)return output_event(b->output,packet->event);
    return 0;
}
static int ECICALL callback(ECIHand handle,int message,int value,void *context)
{
    struct sapi_backend *b=context;
    /* Startup preflight deliberately has no SAPI site or audible output. */
    if(b->warming)return InterlockedCompareExchange(&b->cancel,0,0)?eciDataAbort:eciDataProcessed;
    if(InterlockedCompareExchange(&b->cancel,0,0)){core.stop(handle);return 0;}
    int ok=1;
    if(message==RELOQ_ECI_WAVEFORM_BUFFER){
        if(value<0||value>512)return 0;
        ok=output_pcm(b->output,b->pcm,(size_t)value,b->fragment_volume);
    }else if(message==RELOQ_ECI_INDEX_REPLY){
        if(value<1||(size_t)value>b->event_count)return 0;
        ok=output_event(b->output,b->events[value-1]);
    }else return 1;
    if(!ok && InterlockedCompareExchange(&b->cancel,0,0))core.stop(handle);
    return ok;
}
struct output {
    struct sapi_backend *backend;ISpTTSEngineSite *site;
    ULONGLONG offset;USHORT volume;ULONG fragment_volume;DWORD rate;
    short pending[4096];size_t count;HRESULT result;
};
static int output_actions(struct output *o)
{
    struct sapi_backend *b=o->backend;
    if(InterlockedCompareExchange(&b->cancel,0,0))return 0;
    DWORD flags=o->site->lpVtbl->GetActions(o->site);
    if(flags&SPVES_SKIP){SPVSKIPTYPE kind;LONG count=0;
        if(SUCCEEDED(o->site->lpVtbl->GetSkipInfo(o->site,&kind,&count)))o->site->lpVtbl->CompleteSkip(o->site,0);
        cancel_backend(b);}
    if(flags&SPVES_ABORT)cancel_backend(b);
    if(flags&SPVES_VOLUME){USHORT volume=o->volume;
        if(SUCCEEDED(o->site->lpVtbl->GetVolume(o->site,&volume)))o->volume=volume>100?100:volume;}
    return !InterlockedCompareExchange(&b->cancel,0,0);
}
static int output_flush(struct output *o)
{
    size_t samples=o->count;o->count=0;if(!samples)return 1;
    if(!output_actions(o))return 0;
    ULONG bytes=(ULONG)samples*2,at=0;
    while(at<bytes){ULONG written=0;
        HRESULT hr=o->site->lpVtbl->Write(o->site,(BYTE *)o->pending+at,bytes-at,&written);
        if(!output_actions(o))return 0;
        if(hr!=S_OK||!written||written>bytes-at||(written&1)){
            o->result=FAILED(hr)?hr:hr==S_OK?E_FAIL:S_OK;cancel_backend(o->backend);return 0;}
        at+=written;o->offset+=written;
    }
    return 1;
}
static int output_pcm(struct output *o,const short *pcm,size_t samples,ULONG fragment_volume)
{
    if(!output_actions(o))return 0;
    o->fragment_volume=fragment_volume;
    while(samples){
        /* SAPI consumers such as NVDA wait for an initial 50 ms before
         * feeding the device. Deliver that reserve in one synchronous
         * Write instead of crossing COM once per short native frame. The
         * subsequent 20-ms batches bound cancellation and keep streaming. */
        size_t initial=((size_t)o->rate+19)/20;
        size_t already=(size_t)(o->offset/2);
        size_t target=already<initial?initial-already:((size_t)o->rate+49)/50;
        if(!target)target=1;if(target>4096)target=4096;
        size_t room=target>o->count?target-o->count:0;
        if(!room){if(!output_flush(o))return 0;continue;}
        size_t n=samples<room?samples:room;
        if(o->volume==100&&fragment_volume==100)memcpy(o->pending+o->count,pcm,n*sizeof *pcm);
        else for(size_t i=0;i<n;++i)o->pending[o->count+i]=(short)((int64_t)pcm[i]*o->volume*fragment_volume/10000);
        o->count+=n;pcm+=n;samples-=n;
        if(o->count==target&&!output_flush(o))return 0;
    }
    return 1;
}
static int output_event(struct output *o,SPEVENT event)
{
    if(!output_flush(o)||!output_actions(o))return 0;
    event.ullAudioStreamOffset=o->offset;
    HRESULT hr=o->site->lpVtbl->AddEvents(o->site,&event,1);
    if(FAILED(hr)){o->result=hr;cancel_backend(o->backend);return 0;}return 1;
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
/* eciSetDictionaryPath explicitly reloads the language as well as its .dic
 * files. Preserve initialized linguistic state while file metadata stays
 * unchanged. This retains no text, pronunciation or audio result. */
static HRESULT dictionary(struct sapi_backend *b,const struct sapi_settings *s)
{
    WCHAR path[2048]=L"";WIN32_FILE_ATTRIBUTE_DATA files[6]={0};
    if(s->dictionaries){
        if(s->directory[0])wcscpy(path,s->directory);
#ifdef RELOQ_SAPI_TEST_HOST
        else if(!sapi_module_path(path,2048,L"dictionaries"))return E_FAIL;
#else
        else if(!sapi_default_dictionary_path(path,2048))return E_FAIL;
#endif
        /* Retain initialized dictionaries without testing every file on
         * every announcement. Recheck the directory identity so deleting
         * and recreating it cannot strand the watch on the old directory. */
        WIN32_FILE_ATTRIBUTE_DATA dir={0};
        int exists=GetFileAttributesExW(path,GetFileExInfoStandard,&dir)!=0;
        int same=b->dictionary_ready&&!wcscmp(path,b->dictionary_path);
        if(b->dictionary_watch&&(!same||!exists||
           memcmp(&dir.ftCreationTime,&b->dictionary_directory_created,sizeof(FILETIME)))) {
            FindCloseChangeNotification(b->dictionary_watch);b->dictionary_watch=NULL;
        }
        int already_watching=b->dictionary_watch!=NULL;
        if(exists&&!b->dictionary_watch&&
           (dir.ftCreationTime.dwLowDateTime||dir.ftCreationTime.dwHighDateTime)) {
            HANDLE watch=FindFirstChangeNotificationW(path,FALSE,
                FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_CREATION);
            if(watch!=INVALID_HANDLE_VALUE) {
                b->dictionary_watch=watch;b->dictionary_directory_created=dir.ftCreationTime;
            }
        }
        if(b->dictionary_watch) {
            DWORD changed=WaitForSingleObject(b->dictionary_watch,0);
            if(changed==WAIT_TIMEOUT&&same&&already_watching)return S_OK;
            if(changed!=WAIT_TIMEOUT&&(changed!=WAIT_OBJECT_0||!FindNextChangeNotification(b->dictionary_watch))) {
                FindCloseChangeNotification(b->dictionary_watch);b->dictionary_watch=NULL;
            }
        }
        const WCHAR *prefix=NULL;
        switch(b->dialect){
        case 0x10000:prefix=L"ENU";break;case 0x10001:prefix=L"ENG";break;
        case 0x20000:prefix=L"ESP";break;case 0x20001:prefix=L"ESM";break;
        case 0x30000:prefix=L"FRA";break;case 0x30001:prefix=L"FRC";break;
        case 0x40000:prefix=L"DEU";break;case 0x50000:prefix=L"ITA";break;
        case 0x60000:prefix=L"CHS";break;case 0x70000:prefix=L"PTB";break;
        case 0x80000:prefix=L"JPN";break;case 0x90000:prefix=L"FIN";break;
        case 0xa0000:prefix=L"KOR";break;case 0x120000:prefix=L"ELL";break;
        default:return E_INVALIDARG;}
        static const WCHAR *suffix[]={L"main.dic",L"Root.dic",L"abbr.dic"};
        for(unsigned i=0;i<6;++i){WCHAR file[2112];
            if(i>=3&&b->dialect!=0x60000&&b->dialect!=0xa0000&&b->dialect!=0x120000)break;
            swprintf(file,2112,L"%ls\\%ls%ls",path,i<3?prefix:L"ENU",suffix[i%3]);
            if(!GetFileAttributesExW(file,GetFileExInfoStandard,files+i)){
                DWORD error=GetLastError();
                if(error!=ERROR_FILE_NOT_FOUND&&error!=ERROR_PATH_NOT_FOUND)return HRESULT_FROM_WIN32(error);
                memset(files+i,0,sizeof files[i]);
            }
            files[i].ftLastAccessTime=(FILETIME){0,0};
        }
    }
    if(!path[0]&&b->dictionary_watch) {
        FindCloseChangeNotification(b->dictionary_watch);b->dictionary_watch=NULL;
    }
    if(b->dictionary_ready&&!wcscmp(path,b->dictionary_path)&&
       !memcmp(files,b->dictionary_files,sizeof files))return S_OK;
    char utf8[8192];const char *directory=NULL;
    if(path[0]){
        if(!WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,path,-1,utf8,sizeof utf8,NULL,NULL))return E_INVALIDARG;
        directory=utf8;
    }
    /* A new handle already has an empty native dictionary. */
    if((b->dictionary_ready||directory)&&!core.dictionary(b->handle,directory))return E_FAIL;
    wcscpy(b->dictionary_path,path);memcpy(b->dictionary_files,files,sizeof files);
    b->dictionary_ready=1;return S_OK;
}
static HRESULT configure(struct sapi_backend *b,const struct job *j)
{
    const struct sapi_settings *s=&j->settings;
    static const int rates[]={8000,11025,22050,16000,32000,44100,48000};
    int rate=-1;for(int i=0;i<7;++i)if((DWORD)rates[i]==j->rate)rate=i;
    if(rate<0)return E_INVALIDARG;
    if(!b->handle){b->handle=core.new_ex(b->dialect);if(!b->handle)return E_OUTOFMEMORY;
        core.callback(b->handle,callback,b);if(!core.buffer(b->handle,512,b->pcm))return E_FAIL;}
    if(!core.clear(b->handle))return E_FAIL;
    struct {int key,value;} parameters[]={
        {RELOQ_ECI_VOICE_MODE,1},{RELOQ_ECI_SYNTH_MODE,1},{RELOQ_ECI_SAMPLE_RATE,rate},
        {RELOQ_ECI_NATIVE_SPEED,sapi_native_speed(j->speed,0,s->boost)},
        {RELOQ_ECI_NATIVE_PITCH,(int)s->pitch},{RELOQ_ECI_NATIVE_VOLUME,100},
        {RELOQ_ECI_NATIVE_INFLECTION,(int)s->inflection},{RELOQ_ECI_HEAD_SIZE,(int)s->head},
        {RELOQ_ECI_ROUGHNESS,(int)s->roughness},{RELOQ_ECI_BREATHINESS,(int)s->breathiness},
        {RELOQ_ECI_GENDER,(int)s->gender},{RELOQ_ECI_PROCESS_TAGS,(int)s->tags},
        {RELOQ_ECI_PAUSE_MODE,(int)s->pauses},{RELOQ_ECI_DICTIONARY,!s->abbreviations},
        {RELOQ_ECI_PHRASE_PREDICTION,(int)s->prediction},
        {RELOQ_ECI_ORIGINAL_QUESTIONS,(int)s->original_questions}};
    for(size_t i=0;i<sizeof parameters/sizeof *parameters;++i)
        if(core.set(b->handle,parameters[i].key,parameters[i].value)==-1)return E_FAIL;
    return dictionary(b,s);
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
    HRESULT hr=configure(b,&b->job);if(FAILED(hr))return hr;
    int sentence=1,has_text=0,last_pitch=(int)b->job.settings.pitch;
    int last_rate=sapi_native_speed(b->job.speed,0,b->job.settings.boost),last_range=0,last_emphasis=0;
    size_t delivered_events=0;b->fragment_volume=100;
    b->event_count=0;
    for(const SPVTEXTFRAG *f=b->job.fragments;f;f=f->pNext){
        if(!output_actions(b->output))return S_OK;
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
        int speed=sapi_native_speed(b->job.speed,f->State.RateAdj,b->job.settings.boost);
        ULONG fragment_volume=f->State.Volume>100?100:f->State.Volume;
        if(speed!=last_rate||f->State.PitchAdj.RangeAdj!=last_range||f->State.EmphAdj!=last_emphasis||
           fragment_volume!=b->fragment_volume){
            if(has_text){if(!core.synthesize(b->handle)||!core.clear(b->handle))return E_FAIL;
                has_text=0;delivered_events=b->event_count;}
            int inflection=(int)lround(b->job.settings.inflection*pow(2.0,f->State.PitchAdj.RangeAdj/10.0))+f->State.EmphAdj*3;
            if(inflection<0)inflection=0;if(inflection>100)inflection=100;
            if(core.set(b->handle,RELOQ_ECI_NATIVE_SPEED,speed)==-1||
               core.set(b->handle,RELOQ_ECI_NATIVE_INFLECTION,inflection)==-1)return E_FAIL;
            last_rate=speed;last_range=f->State.PitchAdj.RangeAdj;last_emphasis=f->State.EmphAdj;
            b->fragment_volume=fragment_volume;
            if(core.set(b->handle,RELOQ_ECI_NATIVE_PITCH,last_pitch)==-1)return E_FAIL;
        }
        /* SAPI already parsed XML. Absolute pitch marks preserve the native
         * analyzer's single connected utterance across ordinary fragments. */
        int pitch=sapi_native_pitch((int)b->job.settings.pitch,f->State.PitchAdj.MiddleAdj);
        if(pitch!=last_pitch){if(!core.pitch(b->handle,pitch))return E_FAIL;last_pitch=pitch;}
        if(!(b->job.interest&(((ULONGLONG)1<<SPEI_WORD_BOUNDARY)|((ULONGLONG)1<<SPEI_SENTENCE_BOUNDARY)))){
            if(!add_wide(b,f->pTextStart,f->ulTextLen,f->State.eAction==SPVA_SpellOut))return E_FAIL;
            has_text|=f->ulTextLen!=0;continue;
        }
        size_t at=0,next_poll=0;
        while(at<f->ulTextLen){size_t start=at;
            if(at>=next_poll){if(!output_actions(b->output))return S_OK;next_poll=at+256;}
            WCHAR c=f->pTextStart[at];
            if(iswspace(c)||iswpunct(c)){++at;
                if(sentence_end(c))sentence=1;
                if(!add_wide(b,f->pTextStart+start,at-start,f->State.eAction==SPVA_SpellOut))return E_FAIL;
            }else{
                do{++at;if((at&1023)==0&&!output_actions(b->output))return S_OK;
                    if(c>=0xd800&&c<=0xdbff&&at<f->ulTextLen)++at;
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
    if(InterlockedCompareExchange(&b->cancel,0,0))return S_OK;
    if(has_text&&!core.synthesize(b->handle))return InterlockedCompareExchange(&b->cancel,0,0)?S_OK:E_FAIL;
    /* Bookmark-only streams still need their zero-position index callbacks. */
    if(!has_text&&b->event_count>delivered_events){for(size_t i=delivered_events;i<b->event_count;++i){
        struct packet p={.type=2,.event=b->events[i]};if(!put(b,&p))return S_OK;}}
    return S_OK;
}
/* ECI contexts and their provider data belong to their creating thread.
 * Reuse a few per caller without sharing them between threads or retaining
 * speech. The slot destructor runs on that same thread at thread exit. */
enum {CONTEXT_CACHE_SIZE=4};
struct context_cache {struct sapi_backend *slot[CONTEXT_CACHE_SIZE];};
static void context_destroy(struct sapi_backend *b)
{
    if(!b)return;
    if(b->handle)core.destroy(b->handle);
    if(b->dictionary_watch)FindCloseChangeNotification(b->dictionary_watch);
    free(b->events);
    while(b->bookmarks){struct bookmark *next=b->bookmarks->next;free(b->bookmarks);b->bookmarks=next;}
    free(b);
}
static void cache_destroy(void *value)
{
    struct context_cache *cache=value;if(!cache)return;
    for(unsigned i=0;i<CONTEXT_CACHE_SIZE;++i)context_destroy(cache->slot[i]);
    free(cache);
}
#ifdef RELOQ_SAPI_TEST_HOST
static pthread_once_t cache_once=PTHREAD_ONCE_INIT;
static pthread_key_t cache_key;
static int cache_key_ready;
static void cache_open(void){cache_key_ready=pthread_key_create(&cache_key,cache_destroy)==0;}
static struct context_cache *thread_cache(void)
{
    pthread_once(&cache_once,cache_open);if(!cache_key_ready)return NULL;
    struct context_cache *cache=pthread_getspecific(cache_key);
    if(!cache){cache=calloc(1,sizeof *cache);if(cache&&pthread_setspecific(cache_key,cache)){free(cache);cache=NULL;}}
    return cache;
}
#else
static INIT_ONCE cache_once=INIT_ONCE_STATIC_INIT;
static DWORD cache_key=FLS_OUT_OF_INDEXES;
typedef BOOLEAN (WINAPI *shutdown_query)(void);
static shutdown_query query_shutdown;
static void WINAPI cache_gone(void *value)
{
    /* ExitProcess has already stopped the provider's other threads. Joining
     * them from an FLS callback would hang the host; the OS reclaims their
     * resources. Ordinary thread exits still free their own contexts. */
    if(query_shutdown&&!query_shutdown())cache_destroy(value);
}
static BOOL CALLBACK cache_open(PINIT_ONCE once,PVOID unused,PVOID *context)
{
    (void)once;(void)unused;(void)context;
    HMODULE ntdll=GetModuleHandleW(L"ntdll.dll");
    FARPROC p=ntdll?GetProcAddress(ntdll,"RtlDllShutdownInProgress"):NULL;
    if(!p)return FALSE;
    memcpy(&query_shutdown,&p,sizeof p);
    cache_key=FlsAlloc(cache_gone);return cache_key!=FLS_OUT_OF_INDEXES;
}
static struct context_cache *thread_cache(void)
{
    if(!InitOnceExecuteOnce(&cache_once,cache_open,NULL,NULL))return NULL;
    struct context_cache *cache=FlsGetValue(cache_key);
    if(!cache){cache=calloc(1,sizeof *cache);if(cache&&!FlsSetValue(cache_key,cache)){free(cache);cache=NULL;}}
    return cache;
}
#endif
static struct sapi_backend *context_acquire(int dialect)
{
    struct context_cache *cache=thread_cache();int slot=-1;
    if(cache){
        for(unsigned i=0;i<CONTEXT_CACHE_SIZE;++i){struct sapi_backend *b=cache->slot[i];
            if(b&&!b->busy&&b->dialect==dialect){b->busy=1;return b;}}
        for(unsigned i=0;i<CONTEXT_CACHE_SIZE;++i)if(!cache->slot[i]){slot=(int)i;break;}
        if(slot<0)for(unsigned i=0;i<CONTEXT_CACHE_SIZE;++i)if(!cache->slot[i]->busy){
            slot=(int)i;context_destroy(cache->slot[i]);cache->slot[i]=NULL;break;}
    }
    struct sapi_backend *b=calloc(1,sizeof *b);if(!b)return NULL;
    b->dialect=dialect;b->busy=1;b->cached=slot>=0;
    if(slot>=0)cache->slot[slot]=b;return b;
}
static void context_release(struct sapi_backend *b)
{b->busy=0;if(!b->cached)context_destroy(b);}
static struct sapi_backend *backend_new(int dialect,const struct sapi_settings *initial)
{
    if(!InitOnceExecuteOnce(&core_once,load_core,NULL,NULL))return NULL;
    if(initial&&initial->rate>=7)return NULL;
    struct sapi_backend *public=calloc(1,sizeof *public);if(!public)return NULL;public->dialect=dialect;
    if(initial){
        static const DWORD rates[]={8000,11025,22050,16000,32000,44100,48000};
        struct sapi_backend *b=context_acquire(dialect);
        if(b){struct job job={.settings=*initial,.rate=rates[initial->rate]};b->warming=1;
            InterlockedExchange(&b->cancel,0);
            if(SUCCEEDED(configure(b,&job))&&core.add(b->handle,"a",1,0))core.synthesize(b->handle);
            if(b->handle)core.clear(b->handle);b->warming=0;context_release(b);}
    }
    return public;
}
struct sapi_backend *sapi_backend_new(int dialect){return backend_new(dialect,NULL);}
struct sapi_backend *sapi_backend_new_prepared(int dialect,const struct sapi_settings *settings)
{return settings?backend_new(dialect,settings):NULL;}
void sapi_backend_delete(struct sapi_backend *b){free(b);}
HRESULT sapi_backend_speak(struct sapi_backend *b,const struct sapi_settings *settings,DWORD flags,
    const WAVEFORMATEX *format,const SPVTEXTFRAG *fragments,ISpTTSEngineSite *site)
{
    if(InterlockedCompareExchange(&b->busy,1,0))return HRESULT_FROM_WIN32(ERROR_BUSY);
    struct sapi_backend *context=context_acquire(b->dialect);
    if(!context){InterlockedExchange(&b->busy,0);return E_OUTOFMEMORY;}
    LONG speed=0;USHORT volume=100;site->lpVtbl->GetRate(site,&speed);site->lpVtbl->GetVolume(site,&volume);
    if(speed< -10)speed=-10;if(speed>10)speed=10;if(volume>100)volume=100;
    ULONGLONG interest=0;site->lpVtbl->GetEventInterest(site,&interest);
    context->job=(struct job){.settings=*settings,.fragments=fragments,.flags=flags,.rate=format->nSamplesPerSec,.speed=speed,.volume=volume,.interest=interest};
    struct output output={.backend=context,.site=site,.volume=volume,.fragment_volume=100,.rate=format->nSamplesPerSec,.result=S_OK};
    context->output=&output;InterlockedExchange(&context->cancel,0);
    HRESULT result=output_actions(&output)?synthesize(context):S_OK;
    if(SUCCEEDED(result)&&!InterlockedCompareExchange(&context->cancel,0,0))output_flush(&output);
    if(FAILED(output.result))result=output.result;
    else if(InterlockedCompareExchange(&context->cancel,0,0))result=S_OK;
    if(context->handle)core.clear(context->handle);
    free(context->events);context->events=NULL;context->event_count=context->event_capacity=0;
    while(context->bookmarks){struct bookmark *next=context->bookmarks->next;free(context->bookmarks);context->bookmarks=next;}
    context->output=NULL;context_release(context);
    InterlockedExchange(&b->busy,0);return result;
}
