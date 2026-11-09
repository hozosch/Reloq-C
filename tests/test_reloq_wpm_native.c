/* Compare ECI audio duration with the original native WPM rule directly. */
#include "reloq61.h"
#include "reloq_eci.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct capture { size_t samples; unsigned indices; };
static int ECICALL audio(ECIHand h,int message,int value,void *context)
{
    struct capture *c=context;(void)h;
    if(message==RELOQ_ECI_WAVEFORM_BUFFER){assert(value>0);c->samples+=(size_t)value;}
    else if(message==RELOQ_ECI_INDEX_REPLY)++c->indices;
    return 1;
}
static int span(void *context,int32_t from,int32_t to,const struct reloq61_curve curves[62],int32_t volume)
{(void)context;(void)from;(void)to;(void)curves;(void)volume;return 0;}
static int frame(void *context,const int32_t curves[62],int32_t volume)
{(void)context;(void)curves;(void)volume;return 0;}

int main(void)
{
    const int languages[]={0x40000,0x10000};
    const char *sentences[]={"Hallo Welt.","Hello world."};
    const int values[]={50,100,149,150,151,178,200,300,1000};
    unsigned checks=0;
    for(unsigned l=0;l<sizeof languages/sizeof *languages;++l) {
        ECIHand engine=eciNewEx(languages[l]);assert(engine);
        short buffer[1100];struct capture capture={0};
        eciRegisterCallback(engine,audio,&capture);assert(eciSetOutputBuffer(engine,1100,buffer));
        assert(eciSetParam(engine,RELOQ_ECI_VOICE_MODE,RELOQ_ECI_VOICE_GENERIC)>=0);
        assert(eciSetParam(engine,RELOQ_ECI_PROCESS_TAGS,1)>=0);
        assert(eciSetParam(engine,RELOQ_ECI_SAMPLE_RATE,1)>=0);
        assert(eciSetParam(engine,RELOQ_ECI_NATIVE_SPEED,50)>=0);
        assert(eciSetParam(engine,RELOQ_ECI_NATIVE_PITCH,65)>=0);
        assert(eciSetParam(engine,RELOQ_ECI_NATIVE_INFLECTION,0)>=0);
        struct reloq61_request q={.language=languages[l],.annotations=1,
            .voice={0,50,65,0,0,0,50,100},.frame=frame,.span=span,.output_rate=11025};
        for(unsigned i=0;i<sizeof values/sizeof *values;++i) {
            char text[128];snprintf(text,sizeof text,"`vwpm%d %s",values[i],sentences[l]);
            q.text=text;struct reloq61_result result={0};
            assert(!reloq61_generate(&q,&result)&&!result.error&&!result.unsupported&&result.ticks>0);
            capture.samples=capture.indices=0;
            assert(eciAddTextUtf8(engine,text,strlen(text),0)&&eciSynthesize(engine));
            if(capture.samples!=(size_t)result.ticks*11)
                fprintf(stderr,"language=%x wpm=%d ECI=%zu native=%lld\n",languages[l],values[i],capture.samples,(long long)result.ticks*11);
            assert(capture.samples==(size_t)result.ticks*11);++checks;
        }
        eciDelete(engine);
    }
    printf("VWPM: %u ECI durations match the native language rule\n",checks);
    return 0;
}
