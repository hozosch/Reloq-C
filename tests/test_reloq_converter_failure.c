/* Force native audio-format setup to fail during provider construction.
 * Previously the failed converter left a freed format in ST_OUTFMT; the
 * constructor's next attempt freed it again and crashed in the manager. */
#define RELOQ_ECI_STATIC
#include "reloq_eci.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(__i386__)
#define CALL __attribute__((thiscall))
#else
#define CALL
#endif

static int inject_failure, failures;
extern int32_t CALL __real_stw_createAudioConverter(void *,void *);
int32_t CALL __wrap_stw_createAudioConverter(void *thread,void *format)
{
    if(inject_failure) {++failures;return -2;}
    return __real_stw_createAudioConverter(thread,format);
}

#define CHECK(x) do {if(!(x)){fprintf(stderr,"%s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}} while(0)
struct engine {ECIHand handle;short pcm[1024];unsigned samples;};
static int ECICALL callback(ECIHand handle,int type,int count,void *data)
{
    struct engine *e=data;
    if(handle!=e->handle)return eciDataAbort;
    if(type==eciWaveformBuffer&&count>0)e->samples+=(unsigned)count;
    return eciDataProcessed;
}
static int setup(struct engine *e)
{
    memset(e,0,sizeof *e);
    e->handle=eciNewEx(RELOQ_ECI_ENGLISH);
    if(!e->handle)return 0;
    eciRegisterCallback(e->handle,callback,e);
    return eciSetOutputBuffer(e->handle,1024,e->pcm);
}
static int speak(struct engine *e,const char *text)
{
    return eciAddTextUtf8(e->handle,text,strlen(text),0)&&
        eciSynthesize(e->handle)&&eciSynchronize(e->handle);
}
int main(void)
{
    struct engine first,other;
    CHECK(setup(&first));
    CHECK(speak(&first,"A first working voice."));
    CHECK(first.samples>0);
    for(unsigned i=0;i<16;++i) {
        CHECK(setup(&other));
        inject_failure=1;failures=0;
        CHECK(!speak(&other,"A filename requiring a new language engine."));
        CHECK(failures>0);
        CHECK(!eciIsBeingReentered(other.handle));
        CHECK(eciClearInput(other.handle));
        inject_failure=0;
        CHECK(speak(&other,"The next file is readable."));
        CHECK(other.samples>0);
        CHECK(speak(&first,"The other voice remains usable."));
        eciDelete(other.handle);
    }
    eciDelete(first.handle);
    puts("Forced audio-format failures: 16 clean refusals and recovery on two handles");
    return 0;
}
