/* Standalone smoke test for the static Apple ECI library; no app or NVDA. */
#include "reloq_eci.h"
#include "reloq_persian_us_proxy.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct audio {short buffer[2048];short *samples;size_t count,capacity;};
static int ECICALL receive(ECIHand voice,int message,int value,void *context)
{
    (void)voice;struct audio *a=context;
    if(message!=eciWaveformBuffer)return eciDataProcessed;
    if(value<0 || (size_t)value>sizeof a->buffer/sizeof a->buffer[0] ||
       (size_t)value>SIZE_MAX-a->count)return eciDataAbort;
    if(value==0)return eciDataProcessed;
    size_t need=a->count+(size_t)value;
    if(need>a->capacity) {
        if(need>SIZE_MAX/sizeof *a->samples)return eciDataAbort;
        void *next=realloc(a->samples,need*sizeof *a->samples);
        if(!next)return eciDataAbort;
        a->samples=next;a->capacity=need;
    }
    memcpy(a->samples+a->count,a->buffer,(size_t)value*sizeof *a->samples);
    a->count=need;return eciDataProcessed;
}
static ECIHand create(struct audio *a,int dialect)
{
    ECIHand h=eciNewEx(dialect);if(!h)return NULL;
    eciRegisterCallback(h,receive,a);
    if(!eciSetOutputBuffer(h,(int)(sizeof a->buffer/sizeof a->buffer[0]),a->buffer) ||
       eciSetParam(h,RELOQ_ECI_SAMPLE_RATE,3)<0) {eciDelete(h);return NULL;}
    return h;
}
static int add(ECIHand h,const char *s) {return eciAddTextUtf8(h,s,strlen(s),0);}
int main(void)
{
    int status=1;struct audio expected={0},actual={0},persian={0};
    struct reloq_fa_owned_capture capture={0},donor={0};
    ECIHand a=create(&expected,0x10000),b=create(&actual,0x10000),fa=create(&persian,0x90000);
    /* The CI runner should identify a failed step even when its compiler and
     * library are platform-specific and the large build log is suppressed. */
#define REQUIRE(condition) do { if(!(condition)) { \
    fprintf(stderr,"Apple ECI smoke failed at line %d: %s\n",__LINE__,#condition); \
    goto done; \
} } while(0)
    REQUIRE(a && b && fa);
    REQUIRE(eciSetParam(a,RELOQ_ECI_PROCESS_TAGS,1)>=0);
    REQUIRE(eciSetParam(b,RELOQ_ECI_PROCESS_TAGS,0)>=0);
    REQUIRE(eciSetVoiceParam(a,0,2,60)>=0 && eciSetVoiceParam(b,0,2,60)>=0);
    REQUIRE(add(a,"Batteries, `vb65 90 percent charged.") && eciSynthesize(a));
    REQUIRE(add(b,"Batteries, ") && eciSetVoiceParam(b,0,2,65)>=0);
    REQUIRE(add(b,"90 percent charged.") && eciSynthesize(b));
    REQUIRE(actual.count && expected.count==actual.count);
    REQUIRE(!memcmp(expected.samples,actual.samples,actual.count*sizeof *actual.samples));
    struct reloq_fa_us_proxy proxy={0};
    const char *tags="`[.1sefr] `[.0ket.1Ab].";
    if(eciPersianUsProxyUtf8(b,tags,strlen(tags),&proxy) || !proxy.text ||
       strcmp(proxy.text,"`[.1sEfs] `[.0kEt.1Ab].") || proxy.word_count!=2) {
        fprintf(stderr,"Apple ECI smoke: US proxy failed or differed.\n");
        eciPersianUsProxyRelease(&proxy);goto done;
    }
    eciPersianUsProxyRelease(&proxy);
    const char *text="سلام دنیا.";size_t error=0;
    REQUIRE(!eciPersianSpeakNativeUtf8(fa,text,strlen(text),0,NULL,NULL,0,&error));
    REQUIRE(persian.count);
    size_t before=persian.count,offsets[]={strlen("سلام ")};
    REQUIRE(!eciPersianCaptureNativeUtf8(fa,text,strlen(text),0,offsets,1,30000,&capture,&error));
    REQUIRE(capture.frame_count && capture.sample_count && capture.mark_count==1);
    REQUIRE(persian.count==before);
    before=actual.count;
    REQUIRE(!eciPersianCaptureUsProxyUtf8(b,tags,strlen(tags),30000,&donor));
    REQUIRE(donor.frame_count && donor.sample_count && donor.mark_count==1);
    REQUIRE(actual.count==before && eciGetParam(b,RELOQ_ECI_PROCESS_TAGS)==0);
    before=persian.count;
    REQUIRE(!eciPersianReplayUsCapture(fa,&capture,&donor,NULL,NULL,0));
    REQUIRE(persian.count>before);
    status=0;puts("Apple ECI smoke: voice changes, Persian captures and replay passed.");
done:
    eciPersianReleaseOwnedCapture(&capture);
    eciPersianReleaseOwnedCapture(&donor);
    if(a)eciDelete(a);
    if(b)eciDelete(b);
    if(fa)eciDelete(fa);
    free(expected.samples);free(actual.samples);free(persian.samples);
    if(status)fputs("Apple ECI smoke failed.\n",stderr);
#undef REQUIRE
    return status;
}
