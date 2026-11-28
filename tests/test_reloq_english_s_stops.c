/* Observe the native release and the following voice, not just PCM hashes. */
#include "reloq61.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct sample { int16_t voice, aspiration, frication; };
struct capture { struct sample samples[4096]; int count; };

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

static int prevoice_aspiration(int language,const char *text,int speed,int *voice_delay)
{
    struct capture capture={0};
    struct reloq61_request q={.language=language,.annotations=1,
        .voice={0,50,65,30,0,0,speed,100},.text=text,
        .frame=frame,.span=span,.context=&capture,.output_rate=11025};
    struct reloq61_result result={0};
    assert(!reloq61_generate(&q,&result) && !result.error && !result.unsupported);
    int first_voice=-1,best=0,run=0,last_end=0;
    for(int i=0;i<capture.count;++i) {
        if(capture.samples[i].voice>10){first_voice=i;break;}
        if(capture.samples[i].aspiration>=38) {
            ++run;if(run>best){best=run;last_end=i+1;}
        } else run=0;
    }
    assert(first_voice>=0 && last_end>0 && first_voice-last_end<=2);
    if(voice_delay) {
        int s_started=0,closure=0,burst=-1,fade=-1;
        for(int i=0;i<=first_voice && i<capture.count;++i) {
            int frication=capture.samples[i].frication;
            if(!s_started && frication>=35)s_started=1;
            else if(s_started && !closure && frication==0)closure=1;
            else if(closure && burst<0 && frication>=35)burst=i;
            else if(burst>=0 && i>burst && frication<capture.samples[i-1].frication) {
                fade=i;break;
            }
        }
        assert(burst>=0 && fade>=0);
        *voice_delay=first_voice-fade;
        assert(capture.samples[first_voice].frication<=5);
        int overlap=0;
        for(int i=fade;i<first_voice;++i)
            if(capture.samples[i].frication>0 && capture.samples[i].aspiration>=38)
                ++overlap;
        assert(overlap>0);
    }
    return best;
}

int main(void)
{
    const char *examples[]={"`[.1skWt]","`[.1spIl]","`[.1stIl]","`[.1skrYb]"};
    unsigned checks=0;
    const int speeds[]={50,100,150,200,220,250};
    for(int language=0x10000;language<=0x10001;++language)
        for(unsigned n=0;n<sizeof speeds/sizeof speeds[0];++n) {
            int speed=speeds[n];
            int longest=0;
            for(unsigned i=0;i<sizeof examples/sizeof examples[0];++i) {
                int delay=0;
                int length=prevoice_aspiration(language,examples[i],speed,&delay);
                assert(length>=1 && length<=14);
                /* The native burst fades during a brief H and ends at the
                 * following voice; no separate H delays the vowel or /r/. */
                assert(delay>=0 && delay<=9);
                if(length>longest)longest=length;
                ++checks;
            }
            /* An explicitly aspirated phone retains its native H gesture.
             * At the fastest speeds it can itself be shorter than the
             * overlapping release; the ECI test checks exact PCM equality
             * against the unmodified engine at every tested speed. */
            int explicit_length=prevoice_aspiration(language,"`[.1skhWt]",speed,NULL);
            assert(explicit_length>0);
            if(speed<=200)assert(explicit_length>longest);
            ++checks;
        }
    printf("English s+stop release: %u source/voice transition checks\n",checks);
    return 0;
}
