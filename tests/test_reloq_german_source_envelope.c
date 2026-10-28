#include "reloq_german_source_envelope.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct observation {enum reloq_target_stream stream;int64_t tick;int32_t value;size_t phone;};
struct capture {struct observation points[128];size_t count,limit;};
static int emit(void *context,enum reloq_target_stream stream,int64_t tick,int32_t value,size_t phone)
{
    struct capture *c=context;
    if (c->count>=c->limit) return -1;
    c->points[c->count++]=(struct observation){stream,tick,value,phone};
    return 0;
}
static size_t selected(const struct capture *c,enum reloq_target_stream stream,
                       size_t phone,struct observation *out)
{
    size_t n=0;
    for (size_t i=0;i<c->count;++i)
        if (c->points[i].stream==stream && c->points[i].phone==phone)
            out[n++]=c->points[i];
    return n;
}
static void sparse_bridges(void)
{
    struct reloq_german_phone_node p[3]={
        {.phone=RELOQ_DE_PHONE_A,.stress=1},
        {.phone=RELOQ_DE_PHONE_n,.left_phone=RELOQ_DE_PHONE_A,
         .right_phone=RELOQ_DE_PHONE_A,.same_word_left=1,.same_syllable_right=1},
        {.phone=RELOQ_DE_PHONE_A}
    };
    struct reloq_german_source_segment s[3]={
        {.begin=0,.end=90,.av=57,.av_end=49,.outgoing_ticks=40,.kind=RELOQ_SOURCE_NUCLEUS},
        {.begin=90,.end=120,.kind=RELOQ_SOURCE_NASAL_BRIDGE},
        {.begin=120,.end=217,.av=54,.av_end=44,.incoming_ticks=25,
         .kind=RELOQ_SOURCE_NUCLEUS,.phrase_final=1}
    };
    assert(reloq_german_source_consonant_ticks(p,3,1)==30);
    assert(reloq_german_source_classify(p,3,1,0,0)==RELOQ_SOURCE_NASAL_BRIDGE);
    p[1].same_word_left=0;
    assert(reloq_german_source_classify(p,3,1,0,0)==RELOQ_SOURCE_NASAL_BRIDGE);
    p[1].same_word_left=1;
    struct capture c={.limit=128};struct observation points[16];
    assert(!reloq_german_source_envelope_append(p,s,3,emit,&c));
    assert(selected(&c,RELOQ_TARGET_AV,1,points)==0);
    assert(selected(&c,RELOQ_TARGET_AV,0,points)==2);
    assert(points[0].tick==1 && points[0].value==57);
    assert(points[1].tick==90 && points[1].value==49);
    c.count=0;
    assert(!reloq_german_resonance_append(RELOQ_DE_PHONE_n,90,120,200,1,emit,&c));
    assert(selected(&c,RELOQ_TARGET_FNZ,1,points)==1);
    assert(points[0].tick==105 && points[0].value==350);
    p[1].phone=RELOQ_DE_PHONE_v;s[1].kind=RELOQ_SOURCE_PARTIAL_VOICE;
    c.count=0;
    assert(!reloq_german_source_envelope_append(p,s,3,emit,&c));
    assert(selected(&c,RELOQ_TARGET_AV,1,points)==1);
    assert(points[0].tick==105 && points[0].value==30);
    c.count=0;c.limit=1;
    assert(reloq_german_source_envelope_append(p,s,3,emit,&c)==-1);
}
static void coda_ownership(void)
{
    struct reloq_german_phone_node p[2]={
        {.phone=RELOQ_DE_PHONE_A,.stress=1},
        {.phone=RELOQ_DE_PHONE_l,.stress=1,.same_syllable_left=1}
    };
    struct reloq_german_source_segment s[2]={
        {.begin=0,.end=134,.av=58,.av_end=47,.kind=RELOQ_SOURCE_NUCLEUS},
        {.begin=134,.end=174,.kind=RELOQ_SOURCE_CARRY,.phrase_final=1}
    };
    struct capture c={.limit=128};struct observation points[16];
    assert(!reloq_german_source_envelope_append(p,s,2,emit,&c));
    assert(selected(&c,RELOQ_TARGET_AV,0,points)==2);
    assert(points[1].tick==174 && points[1].value==47);
    assert(selected(&c,RELOQ_TARGET_AV,1,points)==1);
    assert(points[0].tick==204 && points[0].value==0);
    p[1].phone=RELOQ_DE_PHONE_n;s[0].end=107;s[0].outgoing_ticks=40;
    s[1].begin=107;s[1].end=227;s[1].kind=RELOQ_SOURCE_NASAL_FINAL;
    c.count=0;
    assert(!reloq_german_source_envelope_append(p,s,2,emit,&c));
    assert(selected(&c,RELOQ_TARGET_AV,1,points)==6);
    assert(points[0].tick==203 && points[0].value==42);
    assert(points[1].tick==227 && points[1].value==40);
    assert(points[3].tick==242 && points[3].value==0);
    s[0].outgoing_ticks=0;
    c.count=0;
    assert(!reloq_german_resonance_append(RELOQ_DE_PHONE_n,107,227,200,1,emit,&c));
    assert(selected(&c,RELOQ_TARGET_FNZ,1,points)==1);
    assert(points[0].tick==167 && points[0].value==350);
}
static void initial_gestures(void)
{
    struct reloq_german_phone_node p[2]={
        {.phone=RELOQ_DE_PHONE_r,.stress=1,.right_phone=RELOQ_DE_PHONE_A,.same_syllable_right=1},
        {.phone=RELOQ_DE_PHONE_A,.stress=1}
    };
    struct reloq_german_source_segment s[2]={
        {.begin=0,.end=75,.kind=RELOQ_SOURCE_R_INITIAL},
        {.begin=75,.end=182,.av=57,.av_end=47,.incoming_ticks=15,.kind=RELOQ_SOURCE_NUCLEUS}
    };
    struct capture c={.limit=128};struct observation points[16];
    assert(reloq_german_source_consonant_ticks(p,2,0)==75);
    assert(!reloq_german_source_envelope_append(p,s,2,emit,&c));
    assert(selected(&c,RELOQ_TARGET_AH,0,points)==8);
    const int times[]={0,15,16,38,39,54,55,75};
    const int levels[]={27,27,48,48,27,27,52,52};
    for (size_t i=0;i<8;++i) assert(points[i].tick==times[i] && points[i].value==levels[i]);
    p[0].phone=RELOQ_DE_PHONE_m;s[0].kind=RELOQ_SOURCE_NASAL_ONSET;
    s[0].end=85;s[1].begin=85;
    c.count=0;
    assert(!reloq_german_resonance_append(RELOQ_DE_PHONE_m,0,85,200,0,emit,&c));
    assert(selected(&c,RELOQ_TARGET_FNZ,0,points)==1);
    assert(points[0].tick==42 && points[0].value==350);
    assert(!reloq_german_resonance_append(RELOQ_DE_PHONE_Q_a_TILDE,100,180,350,1,emit,&c));
    assert(selected(&c,RELOQ_TARGET_FNZ,1,points)==2);
    assert(points[0].tick==100 && points[0].value==350);
    s[1].begin=80;
    assert(reloq_german_source_envelope_append(p,s,2,emit,&c)==-1);
}
int main(void)
{
    sparse_bridges();coda_ownership();initial_gestures();
    puts("source envelopes: sparse voicing, shared coda, releases and nasal resonance passed");
    return 0;
}
