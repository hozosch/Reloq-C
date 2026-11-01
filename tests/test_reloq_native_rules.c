/* Observe the real native number rules and sparse F0 curves. No renderer
 * mock, output audio or stored synthesis result is used by these tests. */
#include "reloq61.h"
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct pitch_plan {
    int32_t pair[2048][2];size_t count;
    uint64_t acoustic_hash;int64_t ticks;
};
static int span(void *context,int32_t from,int32_t to,
    const struct reloq61_curve c[62],int32_t volume)
{
    struct pitch_plan *p=context;(void)from;(void)to;(void)volume;
    int32_t a=c[1].val_l,b=c[1].val_r;
    if((a||b)&&(!p->count||a!=p->pair[p->count-1][0]||b!=p->pair[p->count-1][1])) {
        assert(p->count<2048);p->pair[p->count][0]=a;p->pair[p->count++][1]=b;
    }
    for(unsigned j=2;j<62;++j) {
        p->acoustic_hash=(p->acoustic_hash^(uint32_t)c[j].val_l)*UINT64_C(1099511628211);
        p->acoustic_hash=(p->acoustic_hash^(uint32_t)c[j].val_r)*UINT64_C(1099511628211);
    }
    return 0;
}
static int unexpected_frame(void *context,const int32_t frame[62],int32_t volume)
{(void)context;(void)frame;(void)volume;assert(!"Expected sparse spans");return 1;}
static struct reloq61_request request(int language,const char *text)
{
    struct reloq61_request q={.language=language,.annotations=1,.voice={0,50,65,30,0,0,75,100},
        .text=text,.frame=unexpected_frame,.span=span,.output_rate=11025};return q;
}
static struct pitch_plan pitch(const char *text,int speed,int gender,int head,int setting)
{
    struct pitch_plan p={.acoustic_hash=UINT64_C(1469598103934665603)};
    struct reloq61_request q=request(0x60000,text);struct reloq61_result result={0};
    q.context=&p;q.voice[0]=gender;q.voice[1]=head;q.voice[2]=setting;q.voice[6]=speed;
    assert(!reloq61_generate(&q,&result)&&!result.error&&!result.unsupported);
    assert(p.count&&result.ticks>0);p.ticks=result.ticks;return p;
}
static unsigned chinese(void)
{
    const char *texts[]={"妈。","你好，这是中文语音测试。","你好，今天的天气很好。",
        "Hello world. This is a test.","你好，Hello world，这是一个 test。"};
    const int speeds[]={40,75,150};unsigned checks=0;
    for(unsigned t=0;t<sizeof texts/sizeof *texts;++t)
        for(int head=29;head<=50;head+=21)for(int setting=65;setting<=98;setting+=33) {
            struct pitch_plan reference=pitch(texts[t],75,0,head,setting);
            for(unsigned s=0;s<3;++s)for(int gender=0;gender<2;++gender) {
                struct pitch_plan actual=pitch(texts[t],speeds[s],gender,head,setting);
                assert(actual.count==reference.count);
                assert(!memcmp(actual.pair,reference.pair,actual.count*sizeof *actual.pair));
                if(speeds[s]!=75)assert(actual.ticks!=reference.ticks);
                if(gender)assert(actual.acoustic_hash!=reference.acoustic_hash);
                ++checks;
            }
        }
    return checks;
}
static void phones(int language,const char *text,char normalized[4096])
{
    char raw[16384]={0};struct reloq61_request q=request(language,text);
    struct reloq61_result result={0};q.phonemes_only=1;q.phonemes=raw;q.phonemes_capacity=sizeof raw;
    assert(!reloq61_generate(&q,&result)&&!result.error&&!result.unsupported);
    size_t used=0;int inside=0;
    for(size_t i=0;i<result.phonemes_length;++i) {
        unsigned char c=(unsigned char)raw[i];
        if(c=='['){inside=1;continue;}if(c==']'){inside=0;continue;}
        if(!inside||isdigit(c)||c=='.')continue;
        /* US t/flap is correctly contextual across the clock fields. */
        if(language==0x10000&&c=='F')c='t';
        assert(used+1<4096);normalized[used++]=(char)c;
    }
    assert(used);normalized[used]=0;
}
static unsigned clocks(void)
{
    unsigned checks=0;
    const char *minutes[]={"28","02","20"};
    for(int lang=0x10000;lang<=0x10001;++lang)
        for(unsigned m=0;m<3;++m)for(unsigned s=0;s<60;++s) {
            char clock[64],prefix[32],suffix[32],a[4096],b[4096],got[4096];
            snprintf(prefix,sizeof prefix,"13:%s",minutes[m]);snprintf(suffix,sizeof suffix,"%02u",s);
            snprintf(clock,sizeof clock,"%s:%s",prefix,suffix);
            phones(lang,prefix,a);phones(lang,suffix,b);assert(strlen(a)+strlen(b)<sizeof a);strcat(a,b);
            phones(lang,clock,got);
            if(strcmp(a,got)){fprintf(stderr,"Missing or changed clock field: %s (%x)\n",clock,lang);abort();}
            ++checks;
        }
    const char *examples[][2]={
        {"00:20:48.526","zero twenty forty eight point five two six"},
        {"13:00:25","thirteen oh zero twenty five"},
        {"1:00:03","one oh zero zero three"},
        {"00:00:00","zero oh zero zero zero"},
        {"12:00:30 pm","twelve oh zero thirty p m"},
    };
    for(int lang=0x10000;lang<=0x10001;++lang)for(unsigned i=0;i<sizeof examples/sizeof *examples;++i) {
        char a[4096],b[4096];phones(lang,examples[i][0],a);phones(lang,examples[i][1],b);
        if(strcmp(a,b)){fprintf(stderr,"Clock edge case: %s (%x)\n%s\n%s\n",examples[i][0],lang,a,b);abort();}++checks;
    }
    return checks;
}
int main(void)
{
    unsigned c=clocks(),p=chinese();
    printf("Native rules: %u English seconds/zero/fraction cases, %u full Chinese F0 sequences; tempo, models, heads and pitch controls\n",c,p);
    return 0;
}
