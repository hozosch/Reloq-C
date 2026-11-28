/* Observe the English s-stop transition, including the following voice. */
#include "reloq61.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

struct sample { int16_t voice, aspiration, frication, tract[5], a2f; };
struct capture { struct sample samples[4096]; int count; };
struct release { int silence, burst_to_voice, peak, aspiration_run, first_voice, after_voice;
    int16_t tract[5], mid_tract[5], a2f; };
struct reference { int peak, first_voice, aspiration_run; int16_t tract[5], mid_tract[5]; };

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
            (int16_t)value_at(c+8,t),
            {(int16_t)value_at(c+9,t),(int16_t)value_at(c+13,t),
             (int16_t)value_at(c+15,t),(int16_t)value_at(c+17,t),
             (int16_t)value_at(c+19,t)},(int16_t)value_at(c+36,t)};
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
    int s_end=-1,burst=-1,voice=-1,peak=0,run=0,longest=0,aspiration_start=-1;
    for(int i=1;i<capture.count;++i) {
        const struct sample s=capture.samples[i];
        if(s_end<0 && s.frication==0)s_end=i;
        if(s_end>=0 && burst<0 && s.frication>=35)burst=i;
        if(s.voice>10){voice=i;break;}
        if(s_end>=0) {
            if(s.aspiration>peak)peak=s.aspiration;
            if(s.aspiration>=38){if(aspiration_start<0)aspiration_start=i;
                if(++run>longest)longest=run;}else run=0;
        }
    }
    assert(s_end>=0 && burst>s_end && voice>burst && peak>=36);
    /* The H runs into voicing as the native burst falls away. */
    assert(capture.samples[voice].aspiration>0 && capture.samples[voice].frication<=5);
    int after=voice+8<capture.count?voice+8:voice;
    int mid=(aspiration_start+voice)/2;
    struct release out={.silence=burst-s_end,.burst_to_voice=voice-burst,.peak=peak,
        .aspiration_run=longest,.first_voice=capture.samples[voice].aspiration,
        .after_voice=capture.samples[after].aspiration,.a2f=capture.samples[voice].a2f};
    for(int i=0;i<5;++i){out.tract[i]=capture.samples[voice].tract[i];
        out.mid_tract[i]=capture.samples[mid].tract[i];}
    return out;
}

static struct reference reference_release(int language,const char *text,int speed)
{
    struct capture capture={0};
    struct reloq61_request q={.language=language,.annotations=1,
        .voice={0,50,65,30,0,0,speed,100},.text=text,
        .frame=frame,.span=span,.context=&capture,.output_rate=11025};
    struct reloq61_result result={0};
    assert(!reloq61_generate(&q,&result) && !result.error && !result.unsupported);
    int peak=0,begin=-1,voice=-1;
    for(int i=0;i<capture.count;++i) {
        if(capture.samples[i].voice>10){voice=i;break;}
        if(capture.samples[i].aspiration>=38 && begin<0)begin=i;
        if(capture.samples[i].aspiration>peak)peak=capture.samples[i].aspiration;
    }
    assert(begin>=0 && voice>begin);
    struct reference out={.peak=peak,.first_voice=capture.samples[voice].aspiration,
        .aspiration_run=voice-begin};
    int mid=(begin+voice)/2;
    for(int i=0;i<5;++i){out.tract[i]=capture.samples[voice].tract[i];
        out.mid_tract[i]=capture.samples[mid].tract[i];}
    return out;
}

int main(void)
{
    const char *examples[]={"`[.1skWt]","`[.1spIl]","`[.1stIl]","`[.1skrYb]","`[.1skAt]"};
    const int speeds[]={50,100,150,200,220,250};
    unsigned checks=0;
    for(int language=0x10000;language<=0x10001;++language)
        for(unsigned n=0;n<sizeof speeds/sizeof speeds[0];++n) {
            struct release releases[5];
            for(unsigned i=0;i<sizeof examples/sizeof examples[0];++i) {
                releases[i]=first_s_stop(language,examples[i],speeds[n]);
                /* Slow speech keeps a closure and joins the following voice. */
                if(speeds[n]==50) {
                    assert(releases[i].silence>0 && releases[i].burst_to_voice<=90);
                    assert(releases[i].aspiration_run>=20);
                    assert(releases[i].first_voice>=releases[i].after_voice);
                }
                ++checks;
            }
            assert(releases[1].peak<releases[0].peak);
            assert(releases[2].peak<releases[0].peak);
            assert(releases[0].peak<releases[3].peak);
            struct reference cat=reference_release(language,"cat",speeds[n]);
            struct reference cry=reference_release(language,"cry",speeds[n]);
            if(speeds[n]==50) {
                struct reference till=reference_release(language,"`[.1tIl]",speeds[n]);
                struct reference pill=reference_release(language,"`[.1pIl]",speeds[n]);
                int t_f2=releases[2].mid_tract[1]-till.mid_tract[1];
                int p_f2=releases[1].mid_tract[1]-pill.mid_tract[1];
                /* The softened post-s t and p carry their own native
                 * transition toward the vowel rather than the s tract. */
                assert(t_f2>=-40 && t_f2<=40);
                assert(p_f2>=-50 && p_f2<=50);
                /* The release should be recognizably sustained, while using
                 * the existing s-stop time and retaining a real closure. */
                assert(releases[4].aspiration_run>=cat.aspiration_run/3);
                assert(releases[3].aspiration_run>=cry.aspiration_run/3);
                assert(releases[4].aspiration_run<cat.aspiration_run);
                assert(releases[3].aspiration_run<cry.aspiration_run);
            }
            assert(releases[4].peak==cat.peak && releases[3].peak==cry.peak);
            assert(releases[4].first_voice==cat.first_voice);
            assert(releases[3].first_voice==cry.first_voice);
            for(int i=0;i<5;++i) {
                int dc=releases[4].tract[i]-cat.tract[i];
                int dr=releases[3].tract[i]-cry.tract[i];
                assert(dc>=-10 && dc<=10 && dr>=-10 && dr<=10);
                if(speeds[n]<=100) {
                    dc=releases[4].mid_tract[i]-cat.mid_tract[i];
                    dr=releases[3].mid_tract[i]-cry.mid_tract[i];
                    assert(dc>=-150 && dc<=150 && dr>=-150 && dr<=150);
                }
            }
            assert(releases[3].a2f<=5 && releases[4].a2f<=5);
        }
    printf("English s+stop: %u closure and connected-release checks\n",checks);
    return 0;
}
