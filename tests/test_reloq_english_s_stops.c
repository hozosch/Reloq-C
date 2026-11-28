/* Compare each English s-stop release with the ordinary native stop. */
#include "reloq61.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

struct sample { int16_t voice, aspiration, frication; };
struct capture { struct sample samples[4096]; int count; };
struct release { int duration, level, frication_peak; };

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

static struct release first_release(int language,const char *text,int speed)
{
    struct capture capture={0};
    struct reloq61_request q={.language=language,.annotations=1,
        .voice={0,50,65,30,0,0,speed,100},.text=text,
        .frame=frame,.span=span,.context=&capture,.output_rate=11025};
    struct reloq61_result result={0};
    assert(!reloq61_generate(&q,&result) && !result.error && !result.unsupported);
    int begin=-1,end=-1,level=0,voice=-1,peak=0;
    for(int i=0;i<capture.count;++i) {
        const struct sample s=capture.samples[i];
        if(begin<0) {
            if(s.voice<=10 && s.aspiration>=38)begin=i;
            else continue;
        }
        if(s.voice>10 || s.aspiration<38) {end=i;break;}
        if(s.aspiration>level)level=s.aspiration;
    }
    assert(begin>=0 && end>begin);
    for(int i=begin;i>=0;--i) {
        int frication=capture.samples[i].frication;
        if(i<begin && frication==0)break;
        if(frication>peak)peak=frication;
    }
    for(int i=end;i<capture.count && i<=end+2;++i)
        if(capture.samples[i].voice>10){voice=i;break;}
    /* Native frication fades out with the H; voice follows immediately. */
    assert(voice>=0 && capture.samples[voice].frication<=5);
    return (struct release){end-begin,level,peak};
}

int main(void)
{
    const struct { const char *normal,*after_s; int same_frication; } pairs[]={
        {"American","`[.0skXn]",1}, {"cat","scat",1}, {"cry","scry",1},
        {"take","stake",1}, {"opinion","`[.1spIl]",1}, {"pay","spay",0}};
    const int speeds[]={50,100,150,200,220,250};
    unsigned checks=0;
    for(int language=0x10000;language<=0x10001;++language)
        for(unsigned n=0;n<sizeof speeds/sizeof speeds[0];++n)
            for(unsigned i=0;i<sizeof pairs/sizeof pairs[0];++i) {
                struct release normal=first_release(language,pairs[i].normal,speeds[n]);
                struct release after=first_release(language,pairs[i].after_s,speeds[n]);
                assert(after.level==normal.level);
                if(pairs[i].same_frication) {
                    int difference=after.frication_peak-normal.frication_peak;
                    assert(difference>=-5 && difference<=5);
                }
                assert(after.duration<=normal.duration);
                if(normal.duration>=3)assert(after.duration<normal.duration);
                ++checks;
            }
    printf("English s+stop: %u native-reference and connected-release checks\n",checks);
    return 0;
}
