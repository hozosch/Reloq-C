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
    int32_t max_diplophonia,max_aspiration;
    int32_t aj_to_und_start,aj_to_und_end,continuous_until;
    int i_e_r_target,i_e_target,unstressed_er_target,initial_s_nasal_target,final_a_target,comma_a_transition,kamera_a_target,oe_er_target,oe_r_lowered,weak_verb_rise,direct_weak_verb_fall,j_glide_target,ei_glide_target,j_onset_target;
    unsigned r_phase_mask,r_phase_anywhere;
    int r_continuous,r_medial;
};
static int span(void *context,int32_t from,int32_t to,
    const struct reloq61_curve c[62],int32_t volume)
{
    struct pitch_plan *p=context;(void)from;(void)to;(void)volume;
    int32_t a=c[1].val_l,b=c[1].val_r;
    if(a==968&&b==990)p->weak_verb_rise=1;
    if(a==968&&b==751)p->direct_weak_verb_fall=1;
    if((a||b)&&(!p->count||a!=p->pair[p->count-1][0]||b!=p->pair[p->count-1][1])) {
        assert(p->count<2048);p->pair[p->count][0]=a;p->pair[p->count++][1]=b;
    }
    for(unsigned j=2;j<62;++j) {
        p->acoustic_hash=(p->acoustic_hash^(uint32_t)c[j].val_l)*UINT64_C(1099511628211);
        p->acoustic_hash=(p->acoustic_hash^(uint32_t)c[j].val_r)*UINT64_C(1099511628211);
    }
    if(c[6].val_l>p->max_diplophonia)p->max_diplophonia=c[6].val_l;
    if(c[6].val_r>p->max_diplophonia)p->max_diplophonia=c[6].val_r;
    if(c[7].val_l>p->max_aspiration)p->max_aspiration=c[7].val_l;
    if(c[7].val_r>p->max_aspiration)p->max_aspiration=c[7].val_r;
    if(c[9].val_l==775&&c[13].val_l==1205)p->final_a_target=1;
    if(c[9].val_l==775&&c[13].val_l==1112&&c[13].val_r==1202)p->comma_a_transition=1;
    if(c[9].val_l==775&&c[13].val_l==1215&&c[15].val_l==2450)p->kamera_a_target=1;
    if(c[9].val_l==600&&c[13].val_l==1360&&c[15].val_l==2280)p->oe_er_target=1;
    if(c[2].val_l==57&&c[2].val_r==53)p->oe_r_lowered=1;
    if(!p->aj_to_und_start&&c[9].val_l==420&&c[9].val_r==390&&
       c[13].val_l==1900&&c[13].val_r==930) {
        p->aj_to_und_start=from;p->aj_to_und_end=to;
    }
    if(from<50&&c[7].val_l==28&&c[7].val_r==49)p->r_phase_mask|=1u;
    if(from<50&&c[7].val_l==49&&c[7].val_r==28)p->r_phase_mask|=2u;
    if(from<50&&c[7].val_l==28&&c[7].val_r==53)p->r_phase_mask|=4u;
    if(from>0&&c[2].val_l==41&&c[2].val_r==41&&c[6].val_l==0&&c[6].val_r==0&&
       c[7].val_l==53&&c[7].val_r==53)p->r_continuous=1;
    if(c[2].val_l==41&&c[2].val_r==41&&c[7].val_l==53&&c[7].val_r==53&&
       to>p->continuous_until)p->continuous_until=to;
    if(c[7].val_l==28&&c[7].val_r==49)p->r_phase_anywhere|=1u;
    if(c[7].val_l==49&&c[7].val_r==28)p->r_phase_anywhere|=2u;
    if(c[7].val_l==28&&c[7].val_r==53)p->r_phase_anywhere|=4u;
    if(c[2].val_l==0&&c[2].val_r==28&&c[7].val_l==28&&c[7].val_r==53)p->r_medial=1;
    if(c[9].val_l==400&&c[13].val_l==2050)p->i_e_r_target=1;
    if(c[9].val_l==370&&c[13].val_l==2150)p->i_e_target=1;
    if(c[9].val_l==400&&c[13].val_l==2150&&c[15].val_l==2700)p->unstressed_er_target=1;
    if(c[8].val_l==62&&c[39].val_l==50&&c[9].val_l==300&&
       c[13].val_l==1500&&c[15].val_l==2600)p->initial_s_nasal_target=1;
    /* Default softness lowers the /Ej/ third formant by 200 Hz. */
    if(c[9].val_l==285&&c[13].val_l==2100&&c[15].val_l==2650)p->j_glide_target=1;
    if(c[9].val_l==420&&c[13].val_l==1900&&c[15].val_l==2450)p->ei_glide_target=1;
    if(c[9].val_l==300&&c[13].val_l==2100&&c[15].val_l==2900)p->j_onset_target=1;
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

/* German's F0 insertion used to overflow before its fixed-point division.
 * At high pitch and large inflection its maxima could become lower when the
 * pitch setting increased. Exercise both normal voice settings and the
 * inline inflection command through the real sparse acoustic plans. */
static int32_t max_pitch(const struct pitch_plan *p)
{
    int32_t value=0;
    for(size_t i=0;i<p->count;++i)
        for(unsigned side=0;side<2;++side)
            if(p->pair[i][side]>value)value=p->pair[i][side];
    return value;
}
static struct pitch_plan german_pitch(const char *text,int setting,int gender,int speed)
{
    struct pitch_plan p={.acoustic_hash=UINT64_C(1469598103934665603)};
    struct reloq61_request q=request(0x40000,text);struct reloq61_result result={0};
    q.context=&p;q.voice[0]=gender;q.voice[2]=setting;q.voice[3]=100;q.voice[6]=speed;
    assert(!reloq61_generate(&q,&result)&&!result.error&&!result.unsupported);
    assert(p.count&&result.ticks>0);p.ticks=result.ticks;return p;
}
static unsigned german_high_pitch(void)
{
    const char *texts[]={"Hallo! Wie geht das?", "Alle Texte enden unterschiedlich. Wirklich?",
        "`vf100 Barriere wie eben. Eine Frage?"};
    const int speeds[]={50,150};unsigned checks=0;
    for(unsigned t=0;t<sizeof texts/sizeof *texts;++t)
        for(unsigned s=0;s<sizeof speeds/sizeof *speeds;++s)
            for(int gender=0;gender<2;++gender) {
                struct pitch_plan lower=german_pitch(texts[t],90,gender,speeds[s]);
                struct pitch_plan higher=german_pitch(texts[t],98,gender,speeds[s]);
                const int32_t lo=max_pitch(&lower),hi=max_pitch(&higher);
                assert(lo>0&&hi>0&&hi*10>lo*12);
                ++checks;
            }
    return checks;
}
static unsigned german_phone_targets(void)
{
    struct pitch_plan p;
    p=german_pitch("Rot",65,0,75);
    assert(p.max_aspiration==53&&p.r_phase_mask==7&&p.max_diplophonia==0);
    p=german_pitch("vierund",65,0,75);
    assert(p.max_diplophonia==0&&p.r_continuous&&!p.r_phase_anywhere);
    p=german_pitch("`[.1rja]",65,0,75);
    assert(p.max_diplophonia==0&&p.r_continuous&&!p.r_phase_anywhere&&
           p.continuous_until>=25&&p.continuous_until<=30);
    p=german_pitch("`[.0A.1rja]",65,0,75);
    assert(p.r_continuous&&!(p.r_phase_anywhere&3u));
    p=german_pitch("brennen",65,0,75);
    assert(p.max_diplophonia==40&&!p.r_phase_anywhere);
    p=german_pitch("Stra\337e",65,0,75);
    assert(p.max_diplophonia==60&&!p.r_phase_anywhere);
    p=german_pitch("B\374ro",65,0,75);
    assert(p.max_diplophonia==0&&p.r_phase_anywhere==7);
    p=german_pitch("Karriere",65,0,75);
    assert(p.max_diplophonia==0&&p.r_phase_anywhere==7);
    /* The old composer treats a stressed R after unstressed E as a long
     * four-phase onset even before short U; vierund stays continuous. */
    p=german_pitch("herunter",65,0,75);
    assert(p.max_diplophonia==0&&p.r_phase_anywhere==7);
    p=german_pitch("Parade",65,0,75);
    assert(p.max_diplophonia==0&&p.r_phase_anywhere==7);
    p=german_pitch("die Rose",65,0,75);
    assert(p.max_diplophonia==0&&p.r_phase_anywhere==7);
    p=german_pitch("Wort Rose",65,0,75);
    assert(p.r_medial&&!(p.r_phase_anywhere&3u));
    p=german_pitch("Druck",65,0,75);
    assert(p.max_diplophonia==40&&!p.r_phase_anywhere);
    p=german_pitch("Grippe",65,0,75);
    assert(p.max_diplophonia==60&&!p.r_phase_anywhere);
    p=german_pitch("Pizza.",65,0,75);
    assert(p.final_a_target);
    p=german_pitch("Komma.",65,0,75);
    assert(p.comma_a_transition);
    p=german_pitch("Kamera.",65,0,75);
    assert(p.kamera_a_target);
    p=german_pitch("h\366her",65,0,75);
    assert(p.oe_er_target&&!p.oe_r_lowered);
    p=german_pitch("H\366rbuch",65,0,75);
    assert(p.oe_r_lowered);
    p=german_pitch("Barriere",65,0,75);
    assert(p.i_e_r_target);
    p=german_pitch("wie eben",65,0,75);
    assert(p.i_e_target);
    p=german_pitch("Kann er",65,0,75);
    assert(p.unstressed_er_target);
    p=german_pitch("Kann er das?",65,0,75);
    assert(p.unstressed_er_target);
    p=german_pitch("`[.0er]",65,0,75);
    assert(p.unstressed_er_target);
    /* Word-initial /sm/ and /sn/ use the former planner's AF=62 while
     * keeping its A5F=50 and the native duration/transition. */
    const char *s_nasals[]={"Smiley","Snob","ein Smiley","ein Snob",
                            "`[.1smI]","`[.1snOp]"};
    for(size_t i=0;i<sizeof(s_nasals)/sizeof(*s_nasals);++i) {
        p=german_pitch(s_nasals[i],65,0,75);
        assert(p.initial_s_nasal_target);
    }
    /* The original native /Ej/ target, shifted slightly back, applies in
     * text and explicit phonemes; German /aj/ keeps its established target. */
    p=german_pitch("Homepage",65,0,75);
    assert(p.j_glide_target);
    p=german_pitch("`[.1pEjC]",65,0,75);
    assert(p.j_glide_target);
    p=german_pitch("drei",65,0,75);
    assert(p.ei_glide_target&&!p.j_glide_target);
    p=german_pitch("Projekt",65,0,75);
    assert(p.j_onset_target);
    /* The standalone connected words and the same unit in a full number
     * retain the old glide length, without a new glottal interruption. */
    struct pitch_plan two=german_pitch("zweiund",65,0,75);
    struct pitch_plan twentytwo=german_pitch("22",65,0,75);
    struct pitch_plan three=german_pitch("dreiund",65,0,75);
    struct pitch_plan twentythree=german_pitch("23",65,0,75);
    assert(two.aj_to_und_start&&twentytwo.aj_to_und_start&&
           abs(two.aj_to_und_start-twentytwo.aj_to_und_start)<=2);
    assert(three.aj_to_und_start&&twentythree.aj_to_und_start&&
           abs(three.aj_to_und_start-twentythree.aj_to_und_start)<=2);
    assert(two.max_diplophonia<100&&three.max_diplophonia<100&&
           twentytwo.max_diplophonia<100&&twentythree.max_diplophonia<100);
    /* The earlier planner retains secondary stress in 42/43, but starts
     * and finishes the diphthong/und transition at these separate times. */
    struct pitch_plan fortytwo=german_pitch("42",65,0,75);
    struct pitch_plan fortythree=german_pitch("43",65,0,75);
    assert(abs(fortytwo.aj_to_und_start-190)<=2&&abs(fortytwo.aj_to_und_end-209)<=2);
    assert(abs(fortythree.aj_to_und_start-178)<=2&&abs(fortythree.aj_to_und_end-198)<=2);
    assert(fortytwo.max_diplophonia<100&&fortythree.max_diplophonia<100);
    p=german_pitch("82",65,0,75);
    assert(p.max_diplophonia==100); /* Glottal onset of achtzig. */
    return 32;
}

static int pitch_pair(const struct pitch_plan *p,int32_t left,int32_t right)
{
    for(size_t i=0;i<p->count;++i)
        if(p->pair[i][0]==left&&p->pair[i][1]==right)return 1;
    return 0;
}
static unsigned german_question_alignment(void)
{
    struct pitch_plan p={.acoustic_hash=UINT64_C(1469598103934665603)};
    struct reloq61_request q=request(0x40000,"Wie geht das?");
    struct reloq61_result result={0};q.context=&p;
    assert(!reloq61_generate(&q,&result)&&!result.error&&!result.unsupported);
    /* Actual old-composer targets: a low nucleus before the two-part rise. */
    assert(pitch_pair(&p,1043,954)&&pitch_pair(&p,954,1218)&&pitch_pair(&p,1218,1397));
    return 1;
}
static unsigned german_postfocus_melody(void)
{
    struct pitch_plan p={.acoustic_hash=UINT64_C(1469598103934665603)};
    struct reloq61_request q=request(0x40000,"Der Task wird mit Administratorberechtigungen erstellt.");
    struct reloq61_result result={0};q.context=&p;
    assert(!reloq61_generate(&q,&result)&&!result.error&&!result.unsupported);
    /* The earlier planner proceeds from the preceding postnuclear target to
     * the falling phrase tone, without the extra high target on the verb. */
    assert(!p.weak_verb_rise&&p.direct_weak_verb_fall);
    return 1;
}
static unsigned german_question_contours(void)
{
    const char *texts[]={"Wie geht das?", "Warum funktioniert das nicht?",
        "M\366chtest du Kaffee oder Tee?", "Ist das richtig?", "Das geht?",
        "Was?", "Wird der Task mit Administratorberechtigungen erstellt?",
        "Wann kommt das n\344chste Paket?", "Ist das richtig? Ja. Warum nicht?"};
    const int speeds[]={40,75,150,200},pitches[]={65,98};unsigned checks=0;
    for(unsigned t=0;t<sizeof texts/sizeof *texts;++t)
        for(unsigned s=0;s<sizeof speeds/sizeof *speeds;++s)
            for(unsigned h=0;h<sizeof pitches/sizeof *pitches;++h)
                for(int gender=0;gender<2;++gender) {
                    struct pitch_plan p={.acoustic_hash=UINT64_C(1469598103934665603)};
                    struct reloq61_request q=request(0x40000,texts[t]);
                    struct reloq61_result result={0};
                    q.context=&p;q.voice[0]=gender;q.voice[2]=pitches[h];q.voice[6]=speeds[s];
                    assert(!reloq61_generate(&q,&result)&&!result.error&&!result.unsupported);
                    size_t i=p.count;
                    while(i&&p.pair[i-1][0]==p.pair[i-1][1])--i;
                    assert(i&&p.pair[i-1][1]>p.pair[i-1][0]);++checks;
                }
    /* A user-selected zero inflection stays flat for questions as well. */
    struct pitch_plan p={.acoustic_hash=UINT64_C(1469598103934665603)};
    struct reloq61_request q=request(0x40000,"Wie geht das?");
    struct reloq61_result result={0};q.context=&p;q.voice[3]=0;
    assert(!reloq61_generate(&q,&result)&&!result.error&&!result.unsupported&&p.count);
    for(size_t i=0;i<p.count;++i)assert(p.pair[i][0]==p.pair[i][1]);
    return checks+1;
}
static unsigned english_question_contours(void)
{
    const char *questions[]={"How does it work?", "Is this right?", "Where are we going?",
        "Would you like coffee or tea?", "What?", "Are you ready?", "What is this?",
        "This is a statement. Is that a question?"};
    const int speeds[]={40,75,150,200},pitches[]={65,98};unsigned checks=0;
    for(unsigned t=0;t<sizeof questions/sizeof *questions;++t)
        for(unsigned s=0;s<sizeof speeds/sizeof *speeds;++s)
            for(unsigned h=0;h<sizeof pitches/sizeof *pitches;++h)
                for(int gender=0;gender<2;++gender) {
                    struct pitch_plan p={.acoustic_hash=UINT64_C(1469598103934665603)};
                    struct reloq61_request q=request(0x10000,questions[t]);struct reloq61_result result={0};
                    q.context=&p;q.voice[0]=gender;q.voice[2]=pitches[h];q.voice[6]=speeds[s];
                    assert(!reloq61_generate(&q,&result)&&!result.error&&!result.unsupported);
                    size_t i=p.count;while(i&&p.pair[i-1][0]==p.pair[i-1][1])--i;
                    if(!i||p.pair[i-1][1]<=p.pair[i-1][0]) {
                        fprintf(stderr,"Question did not rise: %s, speed %d, pitch %d, gender %d\n",questions[t],speeds[s],pitches[h],gender);abort();
                    }
                    ++checks;
                }
    struct pitch_plan p={.acoustic_hash=UINT64_C(1469598103934665603)};
    struct reloq61_request q=request(0x10000,"How does it work?");struct reloq61_result result={0};
    q.context=&p;q.voice[3]=0;
    assert(!reloq61_generate(&q,&result)&&!result.error&&!result.unsupported&&p.count);
    for(size_t i=0;i<p.count;++i)assert(p.pair[i][0]==p.pair[i][1]);
    return checks+1;
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
    const char *minutes[]={"28","02","20","00"};
    for(int lang=0x10000;lang<=0x10001;++lang)
        for(unsigned m=0;m<3;++m)for(unsigned s=0;s<60;++s) {
            char clock[64],prefix[32],expected[96],a[4096],got[4096];
            snprintf(prefix,sizeof prefix,"13:%s",minutes[m]);
            snprintf(clock,sizeof clock,"%s:%02u",prefix,s);
            snprintf(expected,sizeof expected,"%s and %u second%s",prefix,s,s==1?"":"s");
            phones(lang,expected,a);
            phones(lang,clock,got);
            if(strcmp(a,got)){fprintf(stderr,"Missing or changed clock field: %s (%x)\n",clock,lang);abort();}
            ++checks;
        }
    const char *examples[][2]={
        {"13:28:01.","thirteen twenty eight and one second."},
        {"13:28:00.","thirteen twenty eight and zero seconds."},
        {"13:28:01.5","thirteen twenty eight and one point five seconds"},
        {"00:20:48.526","zero twenty and forty eight point five two six seconds"},
        {"13:00:25","13:00 and 25 seconds"},
        {"1:00:03","1:00 and 3 seconds"},
        {"00:00:00","00:00 and 0 seconds"},
        {"12:00:30 pm","12:00 and 30 seconds pm"},
    };
    for(int lang=0x10000;lang<=0x10001;++lang)for(unsigned i=0;i<sizeof examples/sizeof *examples;++i) {
        char a[4096],b[4096];phones(lang,examples[i][0],a);phones(lang,examples[i][1],b);
        if(strcmp(a,b)){fprintf(stderr,"Clock edge case: %s (%x)\n%s\n%s\n",examples[i][0],lang,a,b);abort();}++checks;
    }
    return checks;
}
int main(void)
{
    unsigned c=clocks(),p=chinese(),g=german_high_pitch(),a=german_phone_targets(),m=german_postfocus_melody(),q=german_question_contours(),qa=german_question_alignment(),enq=english_question_contours();
    printf("Native rules: %u English seconds/zero/fraction cases, %u full Chinese F0 sequences, %u German high-pitch contours, %u rhotic/e/A targets, %u post-focus melodies, %u German question contours, %u old-path question alignment and %u US question contours; tempo, models, heads and pitch controls\n",c,p,g,a,m,q,qa,enq);
    return 0;
}
