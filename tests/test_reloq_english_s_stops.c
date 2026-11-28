/* Observe the English s-stop transition, including the following voice. */
#include "reloq61.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

struct sample { int16_t voice, aspiration, frication; };
struct capture { struct sample samples[4096]; int count; };
struct release { int silence, burst_to_voice, peak, first_voice, after_voice; };

static int32_t value_at(const struct reloq61_curve *c,int32_t t)
{
    if(c->at_l==c->at_r || c->val_l==c->val_r)return c->val_l;
    float value=(float)(t-c->at_l)*(float)(c->val_r-c->val_l);
    value/=(float)(c->at_r-c->at_l);
    return (int32_t)((float)c->val_l+value);
}

static int span(void *context,int32_t from,int32_t to,
    const struct reloq61_curve c[62],int32_t volume)
{
    struct capture *capture=context;(void)volume;
    for(int32_t t=from;t<to;++t) {
        assert(capture->count<(int)(sizeof capture->samples/sizeof capture->samples[0]));
        capture->samples[capture->count++]=(struct sample){
            (int16_t)value_at(c+2,t),(int16_t)value_at(c+7,t),
            (int16_t)value_at(c+8,t)};
    }
    return 0;
}
static int frame(void *context,const int32_t c[62],int32_t volume)
{(void)context;(void)c;(void)volume;assert(!"Expected sparse spans");return 1;}

static struct release first_s_stop(int language,const char *text,int speed)
{
    struct capture capture={0};
    struct reloq61_request q={.language=language,.annotations=1,
        .voice={0,50,65,30,0,0,speed,100},.text=text,
        .frame=frame,.span=span,.context=&capture,.output_rate=11025};
    struct reloq61_result result={0};
    assert(!reloq61_generate(&q,&result) && !result.error && !result.unsupported);
    int s_end=-1,burst=-1,voice=-1,peak=0;
    for(int i=1;i<capture.count;++i) {
        const struct sample s=capture.samples[i];
        if(s_end<0 && s.frication==0)s_end=i;
        if(s_end>=0 && burst<0 && s.frication>=35)burst=i;
        if(s.voice>10){voice=i;break;}
        if(s_end>=0 && s.aspiration>peak)peak=s.aspiration;
    }
    assert(s_end>=0 && burst>s_end && voice>burst && peak>=36);
    /* The ordinary voice begins while the release noise is still present. */
    assert(capture.samples[voice].aspiration>0);
    int after=voice+8<capture.count?voice+8:voice;
    return (struct release){burst-s_end,voice-burst,peak,
        capture.samples[voice].aspiration,capture.samples[after].aspiration};
}

int main(void)
{
    const char *examples[]={"`[.1skWt]","`[.1spIl]","`[.1stIl]","`[.1skrYb]"};
    const int speeds[]={50,100,150,200,220,250};
    unsigned checks=0;
    for(int language=0x10000;language<=0x10001;++language)
        for(unsigned n=0;n<sizeof speeds/sizeof speeds[0];++n) {
            struct release releases[4];
            for(unsigned i=0;i<sizeof examples/sizeof examples[0];++i) {
                releases[i]=first_s_stop(language,examples[i],speeds[n]);
                /* Slow speech keeps a closure and joins the following voice. */
                if(speeds[n]==50) {
                    assert(releases[i].silence>0 && releases[i].burst_to_voice<=30);
                    assert(releases[i].first_voice>=releases[i].after_voice);
                }
                ++checks;
            }
            assert(releases[1].peak<releases[0].peak);
            assert(releases[2].peak<releases[0].peak);
            assert(releases[0].peak<releases[3].peak);
        }
    printf("English s+stop: %u closure and connected-release checks\n",checks);
    return 0;
}
