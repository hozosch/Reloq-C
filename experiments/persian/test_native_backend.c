/* Standalone C client for the new Persian event/acoustic bridge.
 * Link the normal ECI library: no NVDA or eSpeak is loaded by this test. */
#include "reloq_eci.h"
#include "reloq_persian_backend.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct output {short buffer[2048];uint64_t hash;size_t samples;int marks[8];size_t mark_sample[8],nmarks;};

static int ECICALL callback(ECIHand h,int message,int value,void *opaque)
{
    (void)h;struct output *o=opaque;
    if(message==eciWaveformBuffer) {
        assert(value>=0 && (size_t)value<=sizeof o->buffer/sizeof *o->buffer);
        for(int i=0;i<value;++i) {
            o->hash^=(uint16_t)o->buffer[i];o->hash*=1099511628211ULL;
        }
        o->samples+=(size_t)value;
    } else if(message==eciIndexReply) {
        assert(o->nmarks<sizeof o->marks/sizeof *o->marks);
        o->marks[o->nmarks]=value;o->mark_sample[o->nmarks++]=o->samples;
    }
    return eciDataProcessed;
}

static struct output speak(const char *text,const size_t *offsets,size_t count,
    const struct reloq_fa_backend_plan *plan)
{
    struct output out={.hash=1469598103934665603ULL};
    ECIHand e=eciNewEx(0x90000);assert(e);
    eciRegisterCallback(e,callback,&out);
    assert(eciSetOutputBuffer(e,(int)(sizeof out.buffer/sizeof *out.buffer),out.buffer));
    assert(eciSetParam(e,RELOQ_ECI_PROCESS_TAGS,1)>=0);
    assert(eciSetParam(e,RELOQ_ECI_SAMPLE_RATE,3)>=0);
    size_t length=strlen(text),at=0;
    for(size_t i=0;i<count;++i) {
        assert(offsets[i]>=at && offsets[i]<=length);
        assert(eciAddTextUtf8(e,text+at,offsets[i]-at,0));
        assert(eciInsertIndex(e,(int)i+10));at=offsets[i];
    }
    assert(eciAddTextUtf8(e,text+at,length-at,0));
    int previous=eciPersianAcousticScope(1);assert(previous>=0);
    assert(!eciPersianNativeOnsets(plan?plan->onsets:NULL,plan?plan->onset_count:0));
    assert(!eciPersianNativeGeminates(plan?plan->geminates:NULL,plan?plan->geminate_count:0));
    assert(eciSynthesize(e));
    assert(!eciPersianNativeOnsets(NULL,0));
    assert(!eciPersianNativeGeminates(NULL,0));
    assert(eciPersianAcousticScope(previous)==1);
    assert(out.samples>0 && out.nmarks==count);
    for(size_t i=0;i<count;++i)assert(out.marks[i]==(int)i+10);
    eciDelete(e);return out;
}

static struct output speak_native(const char *text,int variant,const size_t *offsets,size_t count,
    int rate,int gender)
{
    struct output out={.hash=1469598103934665603ULL};
    ECIHand e=eciNewEx(0x90000);assert(e);
    eciRegisterCallback(e,callback,&out);
    assert(eciSetOutputBuffer(e,(int)(sizeof out.buffer/sizeof *out.buffer),out.buffer));
    assert(eciSetParam(e,RELOQ_ECI_PROCESS_TAGS,0)>=0);
    assert(eciSetParam(e,RELOQ_ECI_SAMPLE_RATE,rate)>=0);
    assert(eciSetParam(e,RELOQ_ECI_GENDER,gender)>=0);
    size_t bytes=strlen(text),error=0;int marks[8];assert(count<=8);
    for(size_t i=0;i<count;++i)marks[i]=(int)i+10;
    assert(!eciPersianSpeakNativeUtf8(e,text,bytes,variant,offsets,marks,count,&error));
    assert(eciGetParam(e,RELOQ_ECI_PROCESS_TAGS)==0);
    assert(out.samples && out.nmarks==count);
    for(size_t i=0;i<count;++i)assert(out.marks[i]==(int)i+10);
    eciDelete(e);return out;
}

static void exercise(const char *source,int variant,int compare)
{
    size_t bytes=strlen(source),end=bytes,source_marks[4]={0,bytes};size_t mark_count=2;
    if(!strncmp(source,"سلام ",strlen("سلام "))) {
        source_marks[1]=strlen("سلام");source_marks[2]=strlen("سلام ");
        source_marks[3]=bytes;mark_count=4;
    }
    unsigned char mode=0,kind[4]={1,1,1,1};
    size_t event_marks[4]={0},count=0,error=0;
    struct reloq_fa_plan_event *events=NULL;
    int status=eciPersianAnalyzeRunsCompleteUtf8(source,bytes,variant,&end,&mode,1,
        source_marks,kind,mark_count,&events,&count,event_marks,&error);
    assert(status==0 && events && count);
    struct reloq_fa_backend_plan plan={0};
    assert(!reloq_fa_backend_prepare(source,bytes,events,count,&plan,&error));
    assert(plan.text && plan.length==strlen(plan.text));
    if(!strcmp(source,"تعجب")) {
        assert(plan.geminate_count==1 && plan.geminates[0]==(1u<<16|1u));
        assert(!strcmp(plan.text,"`[.0tA.0wA.1Job]"));
    }
    if(!strcmp(source,"سوال")) {
        assert(plan.onset_count==1 && plan.onsets[0]==2);
        assert(!strcmp(plan.text,"`[.1so] `[.0al]"));
    }
    for(size_t i=0;i<count;++i)assert(plan.offsets[i]<=plan.offsets[i+1]);
    size_t marks[4]={0};
    for(size_t i=0;i<mark_count;++i)marks[i]=plan.offsets[event_marks[i]];
    struct output actual=speak(plan.text,marks,mark_count,&plan);
    struct output native=speak_native(source,variant,source_marks,mark_count,3,0);
    assert(actual.samples==native.samples && actual.hash==native.hash);
    for(size_t i=0;i<mark_count;++i)assert(actual.mark_sample[i]==native.mark_sample[i]);
    if(compare) {
        char *reference=NULL;size_t ref_marks[4]={0};
        assert(!eciPersianProjectTextCompleteUtf8(source,bytes,variant,source_marks,
            kind,mark_count,&reference,ref_marks,&error));
        assert(reference);
        struct output expected=speak(reference,ref_marks,mark_count,NULL);
        assert(actual.samples==expected.samples && actual.hash==expected.hash);
        for(size_t i=0;i<mark_count;++i)assert(actual.mark_sample[i]==expected.mark_sample[i]);
        eciPersianRelease(reference);
    }
    printf("%s: %zu samples, %zu native events\n",source,actual.samples,count);
    reloq_fa_backend_clear(&plan);eciPersianRelease(events);
}

int main(void)
{
    exercise("سلام دنیا.",0,1);
    exercise("کتاب را.",0,1);
    exercise("این عدد صفر است.",0,1);
    exercise("salam donya.",1,1);
    /* A word absent from the lexicon uses the C orthographic rules. */
    exercise("ژژژژژ",0,1);
    exercise("تعجب",0,0);
    exercise("سوال",0,0);
    /* A real English run is routed using the normal ECI language command. */
    exercise("سلام NVDA دنیا؟",0,0);
    for(int rate=0;rate<7;++rate)for(int gender=0;gender<2;++gender) {
        const char *text="برای راه‌اندازی رایانه باید این گزینه انتخاب شود.";
        size_t offsets[]={0,strlen(text)};
        (void)speak_native(text,0,offsets,2,rate,gender);
    }
    /* Reject unimplemented controls and ambiguous mixed onset routing before
     * emitting samples or modifying the caller's tag setting or queue. */
    {
        struct output out={.hash=1469598103934665603ULL};size_t error=0;
        ECIHand e=eciNewEx(0x90000);assert(e);
        eciRegisterCallback(e,callback,&out);
        assert(eciSetOutputBuffer(e,(int)(sizeof out.buffer/sizeof *out.buffer),out.buffer));
        assert(eciSetParam(e,RELOQ_ECI_PROCESS_TAGS,0)>=0);
        const char *tag="`vs70 سلام";
        assert(eciPersianSpeakNativeUtf8(e,tag,strlen(tag),0,NULL,NULL,0,&error)==1);
        assert(eciPersianSpeakNativeUtf8(e,"x",SIZE_MAX,0,NULL,NULL,0,&error)==-1);
        const char bad[]={ (char)0xff,0 };
        assert(eciPersianSpeakNativeUtf8(e,bad,1,0,NULL,NULL,0,&error)==-1);
        size_t outside[]={99};int value[]={5};
        assert(eciPersianSpeakNativeUtf8(e,"سلام",strlen("سلام"),0,outside,value,1,&error)==-1);
        const char *mixed="سوال NVDA";
        assert(eciPersianSpeakNativeUtf8(e,mixed,strlen(mixed),0,NULL,NULL,0,&error)==1);
        assert(eciPersianSpeakNativeUtf8(e,"3,14",4,0,NULL,NULL,0,&error)==1);
        assert(!out.samples && eciGetParam(e,RELOQ_ECI_PROCESS_TAGS)==0);
        assert(eciPersianAcousticScope(1)==0);
        assert(eciPersianSpeakNativeUtf8(e,"سلام",strlen("سلام"),0,NULL,NULL,0,&error)==-1);
        assert(eciPersianAcousticScope(0)==1);
        assert(eciAddTextUtf8(e,"hello",5,0));
        assert(eciPersianSpeakNativeUtf8(e,"سلام",strlen("سلام"),0,NULL,NULL,0,&error)==-1);
        assert(eciClearInput(e));eciDelete(e);
    }
    puts("Passed standalone Persian C analysis, synthesis, indices and mixed text.");
    return 0;
}
