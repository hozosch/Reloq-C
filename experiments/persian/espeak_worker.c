/* Private text-analysis worker for the Persian NVDA integration experiment.
 * eSpeak is loaded in a separate process so its global voice and decoder
 * state cannot overwrite another speech engine in the host process. The
 * protocol is private and length-prefixed; this is not an installable voice.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#else
#include <dlfcn.h>
#endif

enum { MAX_REQUEST = 1024 * 1024, MAX_RESPONSE = 8 * 1024 * 1024,
       MAX_CLAUSES = 65536, MAX_EVENTS = 262144, EVENT_REQUEST = 0x80 };

typedef int (*initialize_fn)(int, int, const char *, int);
typedef int (*set_voice_fn)(const char *);
typedef const char *(*phonemes_fn)(const void **, int, int, int *);
typedef int (*terminate_fn)(void);
/* Public speak_lib.h event layout. The helper and the loaded eSpeak DLL use
 * the same architecture. This remains entirely inside the helper process. */
struct speech_event {
    int type; unsigned int identifier; int text_position, length, audio_position, sample;
    void *user_data;
    union { int number; const char *name; char string[8]; } id;
};
typedef int (*speech_callback)(short *,int,struct speech_event *);
typedef void (*set_callback_fn)(speech_callback);
typedef int (*synth_fn)(const void *,size_t,unsigned int,int,unsigned int,
                        unsigned int,unsigned int *,void *);

struct buffer { unsigned char *data; size_t size, capacity; };
static struct buffer *event_output;
static uint32_t event_count;
static int event_failed;

static int append(struct buffer *b,const void *p,size_t count)
{
    if(count>MAX_RESPONSE-b->size)return 0;
    size_t need=b->size+count;
    if(need>b->capacity) {
        size_t cap=b->capacity?b->capacity:256;
        while(cap<need)cap=cap>MAX_RESPONSE/2?MAX_RESPONSE:cap*2;
        void *q=realloc(b->data,cap);
        if(!q)return 0;
        b->data=q;b->capacity=cap;
    }
    memcpy(b->data+b->size,p,count);b->size+=count;return 1;
}

static int append_u32(struct buffer *b,uint32_t value)
{
    unsigned char v[4]={(unsigned char)value,(unsigned char)(value>>8),
                        (unsigned char)(value>>16),(unsigned char)(value>>24)};
    return append(b,v,sizeof v);
}

/* eSpeak supplies character-based word starts and corresponding phoneme
 * events. Audio is discarded: the selected Eloquence core speaks the text.
 * Events remain evidence to validate, not an unquestioned source map. */
static int collect_events(short *audio,int samples,struct speech_event *events)
{
    (void)audio;(void)samples;
    if(!events || !event_output)return 0;
    for(;events->type;events++) {
        if(events->type!=1 && events->type!=7)continue;
        if(events->text_position<0 || events->length<0 || event_count>=MAX_EVENTS) {
            event_failed=1;return 1;
        }
        size_t n=0;
        if(events->type==7)while(n<sizeof events->id.string && events->id.string[n])++n;
        if(!append_u32(event_output,(uint32_t)events->type) ||
           !append_u32(event_output,(uint32_t)events->text_position) ||
           !append_u32(event_output,(uint32_t)events->length) ||
           !append_u32(event_output,(uint32_t)n) ||
           !append(event_output,events->id.string,n)) {
            event_failed=1;return 1;
        }
        ++event_count;
    }
    return 0;
}

static int read_u32(uint32_t *value)
{
    unsigned char b[4];
    if(fread(b,1,sizeof b,stdin)!=sizeof b)return 0;
    *value=(uint32_t)b[0]|((uint32_t)b[1]<<8)|((uint32_t)b[2]<<16)|((uint32_t)b[3]<<24);
    return 1;
}

static int write_u32(uint32_t value)
{
    unsigned char b[4]={(unsigned char)value,(unsigned char)(value>>8),
                        (unsigned char)(value>>16),(unsigned char)(value>>24)};
    return fwrite(b,1,sizeof b,stdout)==sizeof b;
}

static int reply(uint32_t status,struct buffer *b)
{
    return write_u32(status)&&write_u32((uint32_t)b->size)&&
           (!b->size||fwrite(b->data,1,b->size,stdout)==b->size)&&fflush(stdout)==0;
}

static int serve(set_voice_fn set_voice,phonemes_fn phonemes,synth_fn synth)
{
    static const char *voices[]={NULL,"fa","fa-latn","en-us"};
    for(;;) {
        uint32_t length;
        if(!read_u32(&length))return feof(stdin)?0:1;
        if(!length)return 0;
        if(length>MAX_REQUEST)return 1;
        unsigned char header[2];
        if(fread(header,1,sizeof header,stdin)!=sizeof header)return 1;
        unsigned voice=header[0], separator=header[1], want_events=voice&EVENT_REQUEST;
        voice&=~EVENT_REQUEST;
        if(voice<1||voice>3||(separator&&(separator<0x21||separator>0x7e)) ||
           (want_events && separator))return 1;
        char *input=malloc((size_t)length+1);
        if(!input)return 1;
        if(fread(input,1,length,stdin)!=length||memchr(input,0,length)) {free(input);return 1;}
        input[length]=0;
        struct buffer result={0};
        int ok=set_voice(voices[voice])==0&&append_u32(&result,0);
        const void *cursor=input;
        uint32_t clauses=0;
        if(ok && want_events) {
            event_output=&result;event_count=0;event_failed=0;
            /* Synchronous, UTF-8, no audio device. A cancelled request can
             * always be interrupted by terminating this isolated helper. */
            int status=synth(input,(size_t)length+1,0,1,0,1,NULL,NULL);
            event_output=NULL;clauses=event_count;
            if(status || event_failed)ok=0;
        }
        while(ok&&!want_events&&cursor) {
            const char *start=cursor;
            int terminator=0;
            const char *row=phonemes(&cursor,1,(int)(separator<<8),&terminator);
            const char *stop=cursor?cursor:input+length;
            if(!row||start<input||start>=stop||stop>input+length||clauses>=MAX_CLAUSES) {ok=0;break;}
            size_t n=strlen(row);
            if(n>MAX_RESPONSE||!append_u32(&result,(uint32_t)terminator)||
               !append_u32(&result,(uint32_t)n)||!append(&result,row,n)) {ok=0;break;}
            ++clauses;
        }
        free(input);
        if(ok) {
            result.data[0]=(unsigned char)clauses;result.data[1]=(unsigned char)(clauses>>8);
            result.data[2]=(unsigned char)(clauses>>16);result.data[3]=(unsigned char)(clauses>>24);
        } else result.size=0;
        int sent=reply(ok?0:1,&result);
        free(result.data);
        if(!sent)return 1;
    }
}

#ifdef _WIN32
int wmain(int argc,wchar_t **argv)
{
    if(argc!=3)return 2;
    _setmode(_fileno(stdin),_O_BINARY);_setmode(_fileno(stdout),_O_BINARY);
    /* NVDA's eSpeak DLL can have dependencies alongside it, not alongside
     * this helper. Search the DLL's own directory just as Python's modern
     * ctypes loader does, without modifying the process-wide PATH. */
    HMODULE library=LoadLibraryExW(argv[1],NULL,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if(!library){fwprintf(stderr,L"Cannot load eSpeak DLL %ls (Windows error %lu).\n",argv[1],GetLastError());return 2;}
#define LOAD(name,type) (type)(void *)GetProcAddress(library,name)
    int n=WideCharToMultiByte(CP_UTF8,0,argv[2],-1,NULL,0,NULL,NULL);
    char *data=n>0?malloc((size_t)n):NULL;
    if(!data||!WideCharToMultiByte(CP_UTF8,0,argv[2],-1,data,n,NULL,NULL))return 2;
#else
int main(int argc,char **argv)
{
    if(argc!=3)return 2;
    void *library=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);
    if(!library)return 2;
#define LOAD(name,type) (type)dlsym(library,name)
    const char *data=argv[2];
#endif
    initialize_fn initialize=LOAD("espeak_Initialize",initialize_fn);
    set_voice_fn set_voice=LOAD("espeak_SetVoiceByName",set_voice_fn);
    phonemes_fn phonemes=LOAD("espeak_TextToPhonemesWithTerminator",phonemes_fn);
    terminate_fn terminate=LOAD("espeak_Terminate",terminate_fn);
    set_callback_fn set_callback=LOAD("espeak_SetSynthCallback",set_callback_fn);
    synth_fn synth=LOAD("espeak_Synth",synth_fn);
    if(!initialize||!set_voice||!phonemes||!terminate||!set_callback||!synth) {
        fprintf(stderr,"eSpeak DLL lacks a required text-analysis entry point.\n");return 2;
    }
    if(initialize(2,0,data,0x8001)<=0) {
        fprintf(stderr,"Cannot initialize eSpeak language data.\n");return 2;
    }
#ifdef _WIN32
    free(data);
#endif
    set_callback(collect_events);
    int status=serve(set_voice,phonemes,synth);
    terminate();
#ifdef _WIN32
    FreeLibrary(library);
#else
    dlclose(library);
#endif
    return status;
}
