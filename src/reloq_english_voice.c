#include "reloq_english_voice.h"
#include "reloq_english_nucleus_duration.h"
#include "reloq_english_source.h"
#include "reloq_english_tract.h"
#include "reloq_english_transition.h"
#include "reloq_german_voice_defaults.h"
#include "reloq_german_voice_model.h"
#include "reloq_voice_controls.h"
#include "reloq_curve_pipeline.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if defined(RELOQ_ENGLISH_DIAGNOSTICS) || defined(RELOQ_ENGLISH_PHONE_TRACE)
#include <stdio.h>
#endif

struct english_landmark {unsigned char stream;short tick,value;};
struct english_gesture {unsigned first;unsigned short count,contact,duration,release,head;};
#include "reloq_english_gestures.inc"
#include "reloq_english_timing84.inc"
#include "reloq_english_timing89.inc"

static int measured_timing(const struct english_timing_node *tree,const int f[35])
{
    unsigned i=0;
    for(unsigned depth=0;depth<26;++depth) {
        const struct english_timing_node *n=tree+i;
        if(n->feature==255)return n->value;
        int yes=n->equality?f[n->feature]==n->threshold:f[n->feature]<=n->threshold;
        i=yes?n->yes:n->no;
    }
    return -1;
}

static int supported_timing89(const struct english_timing_node *tree,
    const struct english_timing_node *fallback,const int f[35])
{
    unsigned i=0;
    for(unsigned depth=0;depth<26;++depth) {
        const struct english_timing_node *n=tree+i;
        if(n->feature==255)
            return n->threshold>=2?n->value:measured_timing(fallback,f);
        i=(n->equality?f[n->feature]==n->threshold:f[n->feature]<=n->threshold)?n->yes:n->no;
    }
    return measured_timing(fallback,f);
}

struct unit {
    unsigned char phone,stress,weak,aspirated,prominent,before_h;
    size_t word,syllable,source_begin,source_end;
    int pause,question,continuation,speed,pitch,volume,final_syllable,accent;
    double scale;
    int64_t start,contact,end,release,head;
    const struct english_gesture *incoming,*outgoing;
    int timing[35];
    size_t event_begin;
    unsigned source_profile;
    int pitch_declination;
};
struct event {int64_t tick;int32_t value;size_t serial;unsigned char stream;};
struct events {struct event *p;size_t n,cap;};
extern int reloq_english_analyze(const unsigned char *,size_t,int,
    unsigned char **,size_t **,size_t *,size_t *);
static int vowel(unsigned char c){return c&&strchr("iIeEAXHxauUocRYWONM",c)!=NULL;}
static int number(unsigned char c)
{const char *p=strchr(english_inventory,c);return p&&c?(int)(p-english_inventory):-1;}
/* Context supplied to the measured duration rules. All features are local
 * phonetic facts; spelling and word identities are deliberately absent. */
static void timing_features(const struct unit *u,size_t count,size_t i,
    const struct english_gesture *in,const struct english_gesture *out,int f[35])
{
    size_t first=i,last=i,sfirst=i,slast=i;
    while(first&&u[first-1].word==u[i].word)--first;
    while(last+1<count&&u[last+1].word==u[i].word)++last;
    while(sfirst&&u[sfirst-1].syllable==u[i].syllable)--sfirst;
    while(slast+1<count&&u[slast+1].syllable==u[i].syllable)++slast;
    f[0]=number(u[i].phone);
    f[1]=i&&!u[i-1].pause?number(u[i-1].phone):-1;
    f[2]=i+1<count&&!u[i].pause?number(u[i+1].phone):-1;
    f[3]=i>1&&!u[i-1].pause&&!u[i-2].pause?number(u[i-2].phone):-1;
    f[4]=i+2<count&&!u[i].pause&&!u[i+1].pause?number(u[i+2].phone):-1;
    f[5]=u[i].stress;f[6]=u[i].weak;f[7]=u[i].aspirated;
    f[8]=u[i].final_syllable;f[9]=u[i].accent+1;
    f[10]=first==i;f[11]=last==i;f[12]=sfirst==i;f[13]=slast==i;
    f[14]=!i||u[i-1].pause;f[15]=u[i].pause!=0;
    f[16]=(int)(i-sfirst);f[17]=(int)(slast-i);f[18]=f[19]=0;
    for(size_t j=sfirst;j<i;++j)f[18]+=vowel(u[j].phone);
    for(size_t j=i+1;j<=slast;++j)f[19]+=vowel(u[j].phone);
    f[20]=(int)(u[i].syllable-u[first].syllable);
    f[21]=(int)(u[last].syllable-u[first].syllable+1);
    f[22]=u[last].pause!=0;f[23]=(int)(i-first);f[24]=(int)(last-i);
    f[25]=f[1]<0?-1:u[i-1].stress;f[26]=f[2]<0?-1:u[i+1].stress;
    f[27]=in->contact;f[28]=in->duration;f[29]=in->release;
    f[30]=out->contact;f[31]=out->duration;f[32]=out->release;
    f[33]=u[i].question;f[34]=u[i].continuation;
}

/* Native nucleus budget and interval distribution. A monosyllabic word may
 * have one ordinary coda, whose incoming contact is the nucleus's outgoing
 * interval. Composite public diphthongs retain their existing internal
 * composer until their member clocks are separately exposed. */
static int plan_nuclei(struct unit *u,size_t count,int64_t *utterance_end,int native_clock)
{
    for(size_t i=0;i<count;++i) {
        if(!vowel(u[i].phone)||strchr("YWONM",u[i].phone))continue;
        struct reloq_english_nucleus_phone p[3]={{
            .phone=u[i].phone=='N'?'n':u[i].phone=='M'?'m':u[i].phone,
            .syllabic=1,.quantity=u[i].weak?0:1}};
        size_t available=1;
        while(available<3&&i+available<count && !u[i+available-1].pause &&
              u[i+available].word==u[i].word&&u[i+available].syllable==u[i].syllable &&
              u[i+available].scale==u[i].scale &&
              strchr(available==1?"lrwy":"lr",u[i+available].phone)) {
            p[available].phone=u[i+available].phone;++available;
        }
        struct reloq_english_nucleus nucleus;
        if(reloq_english_nucleus_find(p,available,&nucleus)!=1)continue;
        size_t last=i+nucleus.count-1;
        size_t coda=last;
        if(!u[last].pause && last+1<count && u[last+1].word==u[i].word &&
           u[last+1].syllable==u[i].syllable &&
           (last+2==count || u[last+2].word!=u[i].word) &&
           strchr("pbtdkgTDfvszSZmnG",u[last+1].phone))coda=last+1;
        const int unaspirated=i>=2&&!u[i-2].pause&&!u[i-1].pause&&
            u[i-2].phone=='s'&&u[i-2].word==u[i-1].word;
        const int word_end=coda+1==count||u[coda+1].word!=u[i].word;
        const int terminal=u[coda].pause!=0;
        const int described=word_end&&!u[coda].continuation&&!u[i].weak&&
            !u[i].aspirated&&u[i].accent==1&&u[i].timing[21]==1&&
            !u[i].timing[18]&&!u[i].timing[19]&&
            (!i||u[i-1].pause||!strchr("ptk",u[i-1].phone)||unaspirated);
        if(!described)continue;
        p[0].closed=coda!=last;
        p[0].following=coda!=last?u[coda].phone:0;
        if(reloq_english_nucleus_base(p,nucleus.count,&nucleus)!=1)continue;
        const size_t outgoing=!u[last].pause&&last+1<count&&!u[last+1].aspirated?last+1:last;
        const int hiatus=outgoing!=last&&u[outgoing].word!=u[last].word&&vowel(u[outgoing].phone);
        int32_t hiatus_out=0,hiatus_in=0;
        if(hiatus) {
            unsigned char right=u[outgoing].phone;
            if(right=='Y'||right=='W')right='a';else if(right=='O')right='c';
            if(u[outgoing].scale!=u[i].scale ||
               reloq_english_nucleus_hiatus_ticks(u[last].phone,right,&hiatus_out,&hiatus_in)!=1)continue;
        }
        /* The original duration stages receive an integer percentage clock,
         * rather than the old whole-utterance fitted time ratio. */
        const double rate=native_clock?reloq_english_nucleus_rate_percent(u[i].speed):floor(u[i].scale*100.0+.5);
        if(rate<1)return -1;
        int64_t old_end=u[last].end,tail=u[last].release-old_end;
        if(tail<0||tail>INT32_MAX)return -1;
        struct reloq_english_nucleus_interval intervals[6];
        size_t fixed_count=0;
        int32_t entrance=(int32_t)(u[i].contact-u[i].start);
        if(!i||u[i-1].pause)entrance=0;
        else if(u[i-1].word==u[i].word&&u[i-1].syllable==u[i].syllable&&
                !strchr("ptk",u[i-1].phone)) {
            int nominal=reloq_english_nucleus_liquid_onset_ticks(u[i-1].phone,u[i].phone);
            /* A simple l onset has the lateral entrance clock. The
             * cluster l branch above is the separate fl/pl/... gesture. */
            if(u[i-1].phone=='l'&&(i==1||u[i-2].word!=u[i].word))
                nominal=u[i].phone=='i'?50:strchr("IeEA",u[i].phone)?40:30;
            if(nominal<0)nominal=u[i].incoming->contact;
            entrance=(int32_t)llround(nominal*rate/100.0);
        }
        intervals[fixed_count++]=(struct reloq_english_nucleus_interval){
            entrance,0};
        for(size_t j=1;j<nucleus.count;++j) {
            intervals[fixed_count++]=(struct reloq_english_nucleus_interval){
                (int32_t)llround(nucleus.transition_ticks[j-1]*rate/100.0),1};
            intervals[fixed_count++]=(struct reloq_english_nucleus_interval){
                (int32_t)llround(nucleus.closing_ticks[j-1]*rate/100.0),1};
        }
        int32_t exit_ticks=0;
        if(outgoing!=last) {
            exit_ticks=(int32_t)(u[outgoing].contact-u[outgoing].start);
            int nominal=reloq_english_nucleus_exit_ticks(u[last].phone,u[outgoing].phone);
            if(u[last].phone=='r'&&u[outgoing].phone=='t'&&u[last].word!=u[outgoing].word)nominal=25;
            if(nominal>=0)exit_ticks=(int32_t)llround(nominal*rate/100.0);
            if(hiatus)exit_ticks=(int32_t)llround(hiatus_out*rate/100.0);
        }
        intervals[fixed_count++]=(struct reloq_english_nucleus_interval){exit_ticks,0};
        double budget;
        int planned=terminal?reloq_english_nucleus_terminal_budget(p,nucleus.count,
            coda!=last?u[coda].phone:0,&budget):
            reloq_english_nucleus_medial_budget(p,nucleus.count,
            coda!=last?u[coda].phone:0,
            coda==last&&outgoing!=last?u[outgoing].phone:0,&budget);
        if(planned!=1)continue;
        budget=budget*rate/100.0;
        int32_t fixed[6],free_ticks;
        const int32_t minimum=3;
        if(reloq_english_nucleus_distribute(intervals,fixed_count,budget,
                minimum,fixed,&free_ticks))return -1;
        u[i].contact=u[i].start+fixed[0];u[i].end=u[i].contact+free_ticks;
        for(size_t j=1;j<nucleus.count;++j) {
            struct unit *before=u+i+j-1,*next=before+1;
            next->start=before->end;
            next->contact=next->start+fixed[1+2*(j-1)];
            next->end=next->contact+fixed[2+2*(j-1)];
            before->release=next->contact;
        }
        u[last].release=u[last].end+(outgoing==last?tail:fixed[fixed_count-1]);
        int64_t delta=u[last].end-old_end;
        if(outgoing!=last) {
            int64_t old_coda_end=u[outgoing].end;
            int64_t body=u[outgoing].end-u[outgoing].contact;
            int64_t release=u[outgoing].release-u[outgoing].end;
            u[outgoing].start=hiatus?u[last].release:u[last].end;
            u[outgoing].contact=hiatus?u[outgoing].start+(int64_t)llround(hiatus_in*rate/100.0):u[last].release;
            u[outgoing].end=u[outgoing].contact+body;u[outgoing].release=u[outgoing].end+release;
            delta=u[outgoing].end-old_coda_end;
        }
        for(size_t j=outgoing+1;delta&&j<count;++j) {
            u[j].start+=delta;u[j].contact+=delta;u[j].end+=delta;u[j].release+=delta;
        }
        *utterance_end+=delta;
#ifdef RELOQ_ENGLISH_PHONE_TRACE
        fprintf(stderr,"nucleus_plan %zu %zu %d %.8g %d\n",i,nucleus.count,
            nucleus.base_ticks,budget,terminal);
#endif
        i=last;
    }
    return 0;
}
static int add(struct events *e,unsigned stream,int64_t t,int value)
{
    if(t<0)return -1;
    if(e->n==e->cap) {
        size_t cap=e->cap?e->cap*2:1024;
        if(cap<e->cap||cap>SIZE_MAX/sizeof *e->p)return -1;
        void *p=realloc(e->p,cap*sizeof *e->p);if(!p)return -1;e->p=p;e->cap=cap;
    }
    e->p[e->n]=(struct event){t,value,e->n,(unsigned char)stream};++e->n;return 0;
}
#include "reloq_english_acoustics87.h"
#include "reloq_english_source88.h"
#include "reloq_english_source_schedule.h"

static int contract_tract_targets(struct events *events,const struct unit *u)
{
    if(u->end<=u->contact||u->end-u->contact>=16)return 0;
    for(unsigned stream=RELOQ_TARGET_F1;stream<=RELOQ_TARGET_F5;++stream) {
        size_t first=SIZE_MAX,last=SIZE_MAX;int other=0;
        for(size_t j=u->event_begin;j<events->n;++j) {
            const struct event *p=events->p+j;
            if(p->stream!=stream)continue;
            if(p->tick==u->contact&&first==SIZE_MAX)first=j;
            else if(p->tick==u->end&&last==SIZE_MAX)last=j;
            else other=1;
        }
        /* Composite nuclei and extra release/head targets retain their
         * separate gestures. This contract owns a simple two-target body. */
        if(other||first==SIZE_MAX||last==SIZE_MAX)continue;
        struct reloq_english_tract_span plan[2];size_t count;
        if(reloq_english_tract_targets(events->p[first].value,
                events->p[last].value,u->contact,u->end,plan,&count))return -1;
        if(count==2) {
            events->p[first].value=plan[0].value;
            events->p[last].value=plan[1].value;
        }
    }
    return 0;
}

static int compare(const void *a,const void *b)
{
    const struct event *x=a,*y=b;
    if(x->stream!=y->stream)return x->stream<y->stream?-1:1;
    if(x->tick!=y->tick)return x->tick<y->tick?-1:1;
    return x->serial<y->serial?-1:x->serial>y->serial;
}
static int parse(const unsigned char *s,size_t n,const size_t *map,int tags,
    const struct reloq_german_generic_settings *settings,int native_volume,struct unit **result,size_t *count)
{
    if(n>SIZE_MAX/sizeof **result)return -2;
    struct unit *out=calloc(n?n:1,sizeof *out);if(!out)return -2;
    size_t used=0,word=0,syllable=0;int weak=0,accent=1;
    int speed=settings->native_speed?settings->native_speed:50;
    int pitch=settings->native_pitch_plus_one?settings->native_pitch_plus_one-1:65;
    int volume=native_volume;
    for(size_t i=0;i<n;) {
        if(s[i]=='`'&&i+1<n) {
            if(s[i+1]=='[') {
                size_t end=i+2;while(end<n&&s[end]!=']')++end;
                if(end==n)goto invalid;
                struct reloq_english_phone *p=NULL;size_t k=0;
                if(reloq_english_parse(s+i,end-i+1,&p,&k,NULL))goto invalid;
                unsigned aspirated=0;size_t before=used,syllables=0;
                for(size_t j=0;j<k;++j) {
                    unsigned char c=p[j].syllabic?(p[j].phone=='n'?'N':'M'):p[j].phone;
                    if(c=='h'){aspirated=1;continue;}
                    if(number(c)<0||(aspirated&&!english_aspirated[0][number(c)])){free(p);goto invalid;}
                    out[used++]=(struct unit){.phone=c,.stress=p[j].stress,
                        .weak=(unsigned char)weak,.aspirated=(unsigned char)aspirated,
                        .word=word,.syllable=syllable+p[j].syllable,
                        .source_begin=map?map[i]:i,.source_end=map?map[end]:end+1,
                        .speed=speed,.pitch=pitch,.volume=volume,.accent=accent};
                    aspirated=0;syllables=p[j].syllable+1;
                }
                free(p);if(aspirated||before==used)goto invalid;
                ++word;syllable+=syllables;weak=0;accent=1;i=end+1;continue;
            }
            if(s[i+1]>='0'&&s[i+1]<='2') {
                weak=s[i+1]=='0'&&i+2<n&&s[i+2]=='0';
                accent=weak?-1:s[i+1]-'0';i+=weak?3:2;continue;
            }
            if(!tags)goto invalid;
            size_t k=i+1;while(k<n&&s[k]>='a'&&s[k]<='z')++k;
            size_t end=k;int v=0;
            while(end<n&&s[end]>='0'&&s[end]<='9') {
                if(v>100000)goto invalid;
                v=10*v+s[end++]-'0';
            }
            if(k==end||k-i!=3)goto invalid;
            if(!memcmp(s+i+1,"vs",2)){if(v<40||v>250)goto invalid;speed=v;}
            else if(!memcmp(s+i+1,"vb",2)){if(v>100)goto invalid;pitch=v;}
            else if(!memcmp(s+i+1,"vv",2)){if(v>100)goto invalid;volume=v;}
            else if(!memcmp(s+i+1,"rp",2)){if(v>30000||!used)goto invalid;out[used-1].pause=v+100000;}
            else goto invalid;
            i=end;continue;
        }
        if(strchr(".,!?;:()\034\035\036",s[i])) {
            if(used) {
                unsigned char mark=s[i];
                int medial=mark=='('||mark==')'||mark==034||mark==035||mark==036;
                int pause=mark==','||medial?150:mark==';'?250:400;
                if(out[used-1].pause<100000&&out[used-1].pause<pause)out[used-1].pause=pause;
                out[used-1].question=out[used-1].question||mark=='?'||mark==034;
                out[used-1].continuation=mark==','||medial||mark==';'||mark==':';
            }
        } else if(s[i]!=' '&&s[i]!='\t'&&s[i]!='\r'&&s[i]!='\n')goto invalid;
        ++i;
    }
    if(used&&!out[used-1].pause)out[used-1].pause=400;
    /* Public SPR normalization observed in the English module. These are
     * nucleus/rhotic identities, not German vocalic-r substitutions. */
    for(size_t i=0;i+1<used;++i)if(!out[i].pause&&out[i].word==out[i+1].word&&
        out[i+1].phone=='r') {
        if(out[i].phone=='I')out[i].phone='i';
        if(out[i].phone=='E'||out[i].phone=='A')out[i].phone='e';
        if(out[i].phone=='o')out[i].phone='c';
        if(out[i].phone=='x'&&out[i].syllable==out[i+1].syllable) {
            out[i].phone='R';out[i].pause=out[i+1].pause;
            out[i].question=out[i+1].question;out[i].source_end=out[i+1].source_end;
            out[i].continuation=out[i+1].continuation;
            memmove(out+i+1,out+i+2,(used-i-2)*sizeof *out);--used;
        }
    }
    for(size_t i=1;i+1<used;++i)if((out[i].phone=='t'||out[i].phone=='d')&&
        !out[i].stress&&!out[i-1].pause&&vowel(out[i-1].phone)&&vowel(out[i+1].phone)&&
        out[i-1].word==out[i].word&&out[i].word==out[i+1].word)out[i].phone='F';
    /* The original assigns phrase accents to primary stress; secondary
     * stress changes duration but cannot steal a word's nuclear accent. */
    for(size_t first=0;first<used;) {
        size_t last=first;int primary=0;
        while(last<used&&out[last].word==out[first].word) {
            if(out[last].stress==1)primary=1;
            ++last;
        }
        size_t accented=SIZE_MAX;
        for(size_t j=first;j<last;++j) {
            out[j].prominent=out[j].stress==(primary?1:2);
            if(vowel(out[j].phone)&&out[j].prominent) {
                if(out[j].syllable==accented)out[j].prominent=0;
                else accented=out[j].syllable;
            }
        }
        first=last;
    }
    size_t last_syllable=SIZE_MAX;int question=0,continuation=0;
    for(size_t i=used;i-->0;) {
        if(out[i].pause){last_syllable=SIZE_MAX;question=out[i].question;continuation=out[i].continuation;}
        if(last_syllable==SIZE_MAX&&vowel(out[i].phone))last_syllable=out[i].syllable;
        out[i].final_syllable=last_syllable==SIZE_MAX||out[i].syllable==last_syllable;
        out[i].question=question;
        out[i].continuation=continuation;
    }
    for(size_t i=0;i+1<used;++i)
        out[i].before_h=!out[i].pause && out[i+1].aspirated &&
            strchr("iIeEAXHxauUocR",out[i+1].phone)!=NULL;
    *result=out;*count=used;return 0;
invalid:free(out);return -1;
}
static const struct english_gesture *pair(unsigned mode,int a,int b,int side)
{
    if(a<0||b<0)return NULL;
    unsigned id=english_pairs[mode][a][b][side];
    /* A reduced function word cannot contain the synthetic two-nucleus
     * SPR shapes rejected by the reference. At an actual word boundary
     * the dedicated weak-hiatus observations supply those connections. */
    if(!id)return NULL;
    return english_gestures+id;
}
static const struct english_gesture *isolated(int p)
{return p<0||!english_isolated[p]?NULL:english_gestures+english_isolated[p];}
static int64_t retime(int t,const struct english_gesture *g,const struct unit *u)
{
    if(t<=g->contact&&g->contact)
        return u->start+(int64_t)llround((double)t*(u->contact-u->start)/g->contact);
    if(t<=g->duration) {
        int body=g->duration-g->contact;
        return u->contact+(body?(int64_t)llround((double)(t-g->contact)*(u->end-u->contact)/body):0);
    }
    return u->end+(g->release?(int64_t)llround((double)(t-g->duration)*(u->release-u->end)/g->release):0);
}
static int emit_side(struct events *events,const struct english_gesture *g,
    const struct unit *u,int side,int terminal,int nasal,int tract)
{
    if(!g)return -1;
    int64_t split=u->contact+(u->end-u->contact)/2;
    const int stop=strchr("bpdtkg?F",u->phone)!=NULL;
    for(unsigned i=0;i<g->count;++i) {
        const struct english_landmark *p=english_landmarks+g->first+i;
        if(p->tick<0)continue; /* resolved after following phone timing */
        if(tract && p->stream>=RELOQ_TARGET_F1 &&
            p->stream<=(tract==1?RELOQ_TARGET_F3:RELOQ_TARGET_F5))continue;
        if(nasal&&p->stream>=RELOQ_TARGET_B1&&p->stream<=RELOQ_TARGET_B5)continue;
        if(!terminal && stop && p->stream>=RELOQ_TARGET_AH)continue;
        if(!terminal&&p->tick>g->duration && p->stream<RELOQ_TARGET_TL)continue;
        int64_t tick=retime(p->tick,g,u);
        int source=p->stream>=RELOQ_TARGET_TL;
        if(source && p->tick==g->contact+1)tick=u->contact+1;
        if(source && u->before_h && tick>u->end)tick=u->end;
        int64_t seam=source&&stop?u->end:split;
        if(side==0&&tick>seam)continue;
        if(side==1&&tick<=seam)continue;
        /* AV/AH endpoints beyond a nucleus are its handoff to the next
         * constriction, not a pause. Dropping them made the previous
         * envelope interpolate toward an unrelated later word. */
        if(!terminal && p->tick>g->duration && vowel(u->phone) &&
           (p->stream<RELOQ_TARGET_TL||p->stream>RELOQ_TARGET_AH))continue;
        if(add(events,p->stream,tick,p->value))return -1;
    }
    return 0;
}

/* The incoming stop supplies closure voicing; the following phone owns
 * release noise. Keep the observed burst interval at the end of the
 * closure instead of stretching a 5/10-ms onset burst over the closure. */
static int emit_stop_release(struct events *e,const struct english_gesture *g,
    const struct unit *u)
{
    for(unsigned stream=RELOQ_TARGET_AH;stream<RELOQ_TARGET_STREAM_COUNT;++stream) {
        if(add(e,stream,u->contact,0))return -1;
        for(unsigned i=0;i<g->count;++i) {
            const struct english_landmark *p=english_landmarks+g->first+i;
            if(p->stream!=stream || p->tick<0)continue;
            int64_t t=p->tick<=g->duration?
                u->end-(int64_t)llround((g->duration-p->tick)*u->scale):retime(p->tick,g,u);
            if(t<u->contact)t=u->contact;
            if(add(e,stream,t,p->value))return -1;
        }
    }
    return 0;
}

/* The closure's source level is a phonetic decision, independent of a
 * prototype's burst and duration. In particular, a source envelope selected
 * for n+d must not impose its 35-unit voice bar before a fricative. Keep
 * the selected zero crossings, burst, partial intervals and release; only
 * the positive closure level and its tilt belong to this contract. */
static void stop_closure_source(struct events *e,const struct unit *u,
    unsigned char previous,unsigned char following)
{
    int level=reloq_english_stop_closure_av(u->phone,previous,following);
    if(level<0)return;
    for(size_t j=u->event_begin;j<e->n;++j) {
        struct event *p=e->p+j;
        if(p->tick<u->contact || p->tick>u->end)continue;
        /* A final voiced release starts with its own AV at this same
         * endpoint; it is outside the closure even though clocks touch. */
        if(u->pause && p->tick==u->end)continue;
        if(p->stream==RELOQ_TARGET_AV && p->value>0 &&
           (p->tick<u->end || p->value==20 || p->value==35))p->value=level;
        if(p->stream==RELOQ_TARGET_TL && p->value>0)p->value=level?35:0;
    }
}

/* h is a separately timed aspiration in the original, with the following
 * nucleus's tract. It must not inherit a vowel-to-vowel glottal gesture or
 * prematurely voiced source from the incoming pair approximation. */
static int aspirated_source90(struct events *e,struct unit *u,
    const struct english_gesture *in,const struct english_gesture *out,int stop)
{
    if((!u->aspirated && !stop) || !strchr("iIeEAXHxauUocRYWO",u->phone))return 0;
    /* h plus a diphthong already has a dedicated measured source shape;
     * the following repair extends only the stop-to-diphthong boundary. */
    if(u->aspirated && strchr("YWO",u->phone))return 0;
    if(stop && stop!=2 && !u->aspirated && !u->source_profile)return 0;
    int ah=0;
    for(unsigned j=0;j<in->count;++j) {
        const struct english_landmark *p=english_landmarks+in->first+j;
        if(p->stream==RELOQ_TARGET_AH && p->tick>=0 && p->tick<in->contact && p->value>0) {
            ah=p->value;break;
        }
    }
    if(!ah && (!stop || u->aspirated))return 0;
    if(ah && !u->stress)ah-=3;
    int early=stop==2;
    if(stop && !u->aspirated)for(unsigned j=0;j<in->count;++j) {
        const struct english_landmark *p=english_landmarks+in->first+j;
        if(p->stream==RELOQ_TARGET_AV && p->value>0 &&
            p->tick>=0 && p->tick<in->contact)early=1;
    }
    size_t keep=u->event_begin;
    for(size_t j=keep;j<e->n;++j)if(e->p[j].stream<RELOQ_TARGET_TL) {
        e->p[keep]=e->p[j];e->p[keep].serial=keep;++keep;
    }
    e->n=keep;u->source_profile=0;
    for(unsigned j=0;j<out->count;++j) {
        const struct english_landmark *p=english_landmarks+out->first+j;
        if(p->stream<RELOQ_TARGET_TL || p->tick<0 ||
           (!early && p->tick<out->contact))continue;
        if(p->tick==out->contact &&
           (p->stream==RELOQ_TARGET_AV || p->stream==RELOQ_TARGET_AH || p->stream==RELOQ_TARGET_TL) &&
           (!early || !p->value || p->stream==RELOQ_TARGET_TL))continue;
        if(early && p->tick<=out->contact+1 && !p->value &&
           (p->stream==RELOQ_TARGET_AV || p->stream==RELOQ_TARGET_AH))continue;
        int64_t t=(p->tick==out->contact+1 ||
            (early && p->tick<=out->contact &&
             (p->stream==RELOQ_TARGET_AV || p->stream==RELOQ_TARGET_AH)))?
            (early?u->start:u->contact)+1:retime(p->tick,out,u);
        if(u->before_h && t>u->end)t=u->end;
        if(add(e,p->stream,t,p->value))return -1;
    }
    /* After a voiceless stop, its release owns aspiration through the
     * vowel's contact. Reuse the nucleus's complete outgoing source
     * gesture; a learned onset at start+1 would voice the aspiration.
     * Keep the stop's existing targets and absolute contact+1 onset. */
    if(stop && !u->aspirated)return 0;
    if(add(e,RELOQ_TARGET_AV,u->start,0) || add(e,RELOQ_TARGET_AV,u->contact,0) ||
       add(e,RELOQ_TARGET_AF,u->start,0) || add(e,RELOQ_TARGET_AF,u->contact,0) ||
       add(e,RELOQ_TARGET_AH,u->start,ah) || add(e,RELOQ_TARGET_AH,u->contact,ah))return -1;
    return 0;
}

/* A nasal before an obstruent owns its AV plateau. An envelope observed
 * between sonorants may omit AV because its nucleus supplies that span;
 * importing that omission before a closure creates a falling interpolation
 * into silence. Recover the complete outgoing pair's plateau only when
 * the selected profile has no AV targets of its own. */
static int nasal_source93(struct events *e,const struct unit *u,
    const struct unit *next,size_t next_limit)
{
    if(u->pause || !u->source_profile || !strchr("mnG",u->phone) ||
       !strchr("bpdtkgF?CJszSZfvTD",next->phone))return 0;
    for(size_t j=u->event_begin;j<next->event_begin;++j)
        if(e->p[j].stream==RELOQ_TARGET_AV)return 0;
    /* The original intentionally leaves AV open into a voiced closure.
     * Preserve that inherited ramp; only a zero closure needs the nasal's
     * own plateau. The following envelope is known after phone planning. */
    int64_t first=INT64_MAX;int closure_av=-1;
    for(size_t j=next->event_begin;j<next_limit;++j) {
        const struct event *p=e->p+j;
        if(p->stream==RELOQ_TARGET_AV && p->tick<first) {
            first=p->tick;closure_av=p->value;
        }
    }
    if(closure_av!=0)return 0;
    const struct english_gesture *g=u->outgoing;
    for(unsigned j=0;j<g->count;++j) {
        const struct english_landmark *p=english_landmarks+g->first+j;
        if(p->stream==RELOQ_TARGET_AV && p->tick>=g->contact && p->tick<=g->duration) {
            if(add(e,p->stream,retime(p->tick,g,u),p->value))return -1;
            e->p[e->n-1].serial=u->event_begin;
        }
    }
    return 0;
}

/* Original p/t/k+w contracts: aspiration continues through /w/ at a
 * primary-stressed onset (also an isolated monosyllable), whereas medial
 * weak/secondary /w/ voices at contact. S+stop suppresses this aspiration
 * and starts the voiced glide immediately. Word boundaries do not share
 * this onset. The 45 stress/boundary contrasts are saved with release 93. */
static int stop_glide_source93(struct events *e,struct unit *u,int unaspirated,
    int syllables)
{
    int devoiced=!unaspirated && (syllables==1 || u->stress==1);
    int64_t onset=unaspirated?u->start:u->contact;
    size_t keep=u->event_begin;
    for(size_t j=keep;j<e->n;++j)
        if(e->p[j].stream!=RELOQ_TARGET_AV && e->p[j].stream!=RELOQ_TARGET_AH) {
            e->p[keep]=e->p[j];e->p[keep].serial=keep;++keep;
        }
    e->n=keep;u->source_profile=0;
    return add(e,RELOQ_TARGET_AV,onset,devoiced?0:50) ||
        add(e,RELOQ_TARGET_AV,u->end,devoiced?0:50) ||
        add(e,RELOQ_TARGET_AH,onset,devoiced?55:34) ||
        add(e,RELOQ_TARGET_AH,u->end,devoiced?55:34);
}

static int pitch_point(struct events *e,const struct unit *u,
    const struct reloq_german_generic_settings *settings,int64_t tick,int value)
{
    int inflection=settings->native_inflection_plus_one?
        settings->native_inflection_plus_one-1:30;
    double pitch=reloq_voice_pitch_f0_scale(u->pitch);
    if(inflection==30)value=(int)lround(value*pitch);
    else {
        /* Original enu.so keeps the lower register at half the selected
         * base pitch. The old 30-Hz clamp let high-inflection tails fall
         * an octave too low. Phrase declination is an independent offset,
         * not another excursion to multiply by the inflection setting. */
        double base=1189.15,decline=u->pitch_declination;
        double target=inflection?base+(value+decline-base)*inflection/30.0-decline:base;
        value=(int)fmax(trunc(594.575*pitch),trunc(target*pitch));
    }
    return add(e,RELOQ_TARGET_F0,tick,value);
}

/* A phrase has a nuclear accent, followed by its postaccentual tail.
 * Lexical stress and the public `0/`2/`00 controls are independent.
 * In particular a final schwa or weak function word must not replace the
 * falling/rising nuclear movement with a flat 104.6-Hz plateau.
 * Anchors derive from the saved 0.2.79 tone contrasts; arbitrary connected
 * phrases remain a compositional approximation, not exact replay. */
#include "reloq_english_melody86.h"
static int phrase_pitch(struct events *e,struct unit *u,size_t count,
    const struct reloq_german_generic_settings *settings)
{
    for(size_t begin=0;begin<count;) {
        size_t end=begin;
        while(end+1<count&&!u[end].pause)++end;
        int measured=measured_phrase_pitch86(e,u,begin,end,settings);
        if(measured<0)return -1;
        if(!measured){begin=end+1;continue;}
        for(size_t j=begin;j<=end;++j) {
            size_t syllables=u[j].syllable-u[begin].syllable;
            u[j].pitch_declination=syllables>50?150:(int)syllables*3;
        }
        size_t nucleus=SIZE_MAX,accents=0,first_vowel=SIZE_MAX,last_vowel=SIZE_MAX;
        for(size_t j=begin;j<=end;++j)if(vowel(u[j].phone)) {
            if(first_vowel==SIZE_MAX)first_vowel=j;
            last_vowel=j;
            if(u[j].prominent&&!u[j].weak&&u[j].accent>0){nucleus=j;++accents;}
        }
        if(first_vowel==SIZE_MAX){begin=end+1;continue;}
        if(nucleus==SIZE_MAX)nucleus=last_vowel;
        size_t onset=nucleus;
        while(onset>begin&&u[onset-1].syllable==u[nucleus].syllable)--onset;
        int64_t finish=u[last_vowel].end;
        for(size_t j=last_vowel+1;j<=end;++j) {
            if(strchr("mnGrlywNM",u[j].phone))finish=u[j].end;
            else {finish=u[j].contact;break;}
        }
        if(finish<=u[nucleus].start)finish=u[nucleus].end;
        int declination=(int)(u[nucleus].syllable-u[begin].syllable)*3;
        if(declination>150)declination=150;
        int final_declination=(int)(u[last_vowel].syllable-u[begin].syllable)*3;
        if(final_declination>150)final_declination=150;
        int aspirated=u[nucleus].aspirated || (nucleus>begin &&
            strchr("ptk",u[nucleus-1].phone));
        int64_t voice_start=aspirated?u[nucleus].contact:u[nucleus].start;
        int64_t initial=u[begin].start+(u[first_vowel].start-u[begin].start)/(accents>1?20:10);
        int initial_value=accents>1?1153:1046;
        if(u[first_vowel].weak){initial=u[begin].start;initial_value=1117;}
        else if(u[end].question&&accents==2)initial_value=1117;
        if(accents<=1 && first_vowel==begin && !u[first_vowel].aspirated&&!u[first_vowel].weak) {
            initial=u[begin].start+(finish-u[begin].start)*2/100;initial_value=1217;
        } else if(accents<=1 && u[first_vowel].aspirated) {
            initial=u[begin].start+(u[first_vowel].head-u[begin].start)/10;
            initial_value=1049;
        }
        if(pitch_point(e,u+begin,settings,initial,initial_value))return -1;
        unsigned ordinal=0;
        for(size_t j=begin;j<nucleus;++j)if(vowel(u[j].phone)&&u[j].prominent&&
            !u[j].weak&&u[j].accent>0) {
            size_t next=j+1;
            while(next<nucleus&&!(vowel(u[next].phone)&&u[next].prominent&&
                !u[next].weak&&u[next].accent>0))++next;
            int final_pre=next==nucleus;
            /* Declination saturates in long phrases. An unbounded accent
             * counter eventually produced a negative, invalid F0 target. */
            int high_drop=ordinal>7?150:(int)lround(24.5*(ordinal?ordinal-1:0));
            int low_drop=ordinal>11?150:(int)lround(13.6*ordinal);
            int value=ordinal?1221-high_drop:
                (final_pre||u[end].question?1224:1331);
            int64_t t=u[j].end+(int64_t)llround((ordinal?-35:25)*u[j].scale);
            if(t>=u[next].contact)t=u[next].start;
            if(!(u[end].question&&final_pre)&&pitch_point(e,u+j,settings,t,value))return -1;
            if(!final_pre) {
                int64_t low=u[next].start-(int64_t)llround(40*u[j].scale);
                if(low>t&&pitch_point(e,u+j,settings,low,1132-low_drop))return -1;
            }
            ++ordinal;
        }
        if(accents>1 || nucleus>first_vowel) {
            int64_t t=u[onset].contact+(u[nucleus].start-u[onset].contact)/10;
            if(pitch_point(e,u+nucleus,settings,t,1046-declination))return -1;
        }
        int64_t span=finish-voice_start;
        int64_t tail=voice_start+span*99/100;
        if(tail<u[nucleus].start)tail=u[nucleus].start;
        int64_t peak=voice_start+(last_vowel==nucleus?span:
            u[nucleus].end-voice_start)*(accents>1?15:35)/100;
        int64_t low=voice_start+span*70/100;
        if(u[end].question) {
            peak=voice_start+span*(accents>1?20:60)/100;
            low=voice_start+span*90/100;
        }
        if(peak>tail)peak=tail;
        if(low>tail)low=tail;
        int first=u[end].question?957:(accents>1?1224:1296);
        int middle=u[end].question?1224:u[end].continuation?939:796;
        int last=u[end].question?1403:u[end].continuation?868:743;
        if(pitch_point(e,u+nucleus,settings,peak,first-declination)||
           pitch_point(e,u+last_vowel,settings,low,middle-final_declination)||
           pitch_point(e,u+last_vowel,settings,tail,last-final_declination))return -1;
        /* An explicit capital-pitch mark still changes an unaccented
         * letter/word; it is independent of automatic prominence. */
        for(size_t j=begin+1;j<=end;++j)if(u[j].pitch!=u[j-1].pitch &&
            (!u[j].stress||u[j].accent<=0) &&
            pitch_point(e,u+j,settings,u[j].start,1046-declination))return -1;
        begin=end+1;
    }
    return 0;
}
struct delivery {
    reloq_session_pcm_sink sink;reloq_german_generic_index_sink index_sink;void *context;
    const size_t *samples;size_t indices,next,emitted;
    const struct unit *units;size_t unit_count,unit_next;unsigned clock;
    double generic_gain;
};
static int deliver_pcm(struct delivery *d,const int16_t *pcm,size_t n)
{
    while(d->unit_next+1<d->unit_count&&
          (size_t)d->units[d->unit_next+1].start*d->clock<=d->emitted)++d->unit_next;
    double gain=d->generic_gain*reloq_voice_volume_gain(d->units[d->unit_next].volume);
    if(gain==1)return d->sink(d->context,pcm,n);
    int16_t block[128];
    while(n) {
        size_t k=n>128?128:n;
        for(size_t j=0;j<k;++j)block[j]=(int16_t)lround(pcm[j]*gain);
        if(d->sink(d->context,block,k))return -1;
        pcm+=k;n-=k;
    }
    return 0;
}
static int deliver(void *context,const int16_t *pcm,size_t n)
{
    struct delivery *d=context;size_t at=0;
    while(d->next<d->indices&&d->samples[d->next]<=d->emitted+n) {
        size_t count=d->samples[d->next]>d->emitted+at?d->samples[d->next]-d->emitted-at:0;
        if(count&&deliver_pcm(d,pcm+at,count))return -1;
        at+=count;
        if(d->index_sink&&d->index_sink(d->context,d->next))return -1;
        ++d->next;
    }
    if(at<n&&deliver_pcm(d,pcm+at,n-at))return -1;
    d->emitted+=n;return 0;
}
struct output_profile {struct reloq_german_voice_model model;};
static void frame_profile(void *context,float c[62])
{
    struct output_profile *p=context;
    reloq_german_voice_model_frame(&p->model,c);
}

int reloq_english_render_text(const unsigned char *text,size_t length,int tags,int native_volume,
    const struct reloq_german_generic_settings *settings,
    const struct reloq_german_generic_index *indices,size_t index_count,
    const struct reloq_german_generic_pitch_mark *pitches,size_t pitch_count,
    struct reloq_session *session,reloq_session_pcm_sink sink,
    reloq_german_generic_index_sink index_sink,void *context)
{
    if(!text||!settings||!session||!sink||(index_count&&!indices)||(pitch_count&&!pitches)||native_volume<0||native_volume>100)return RELOQ_DE_UTTERANCE_INVALID;
    if(settings->rate<0||settings->rate>100||settings->pitch<0||settings->pitch>100||
       settings->volume<0||settings->volume>100||settings->pause_mode<0||settings->pause_mode>2||
       (settings->native_speed&&(settings->native_speed<40||settings->native_speed>250))||
       settings->native_pitch_plus_one<0||settings->native_pitch_plus_one>101||
       settings->native_inflection_plus_one<0||settings->native_inflection_plus_one>101||
       settings->head_size_plus_one<0||settings->head_size_plus_one>101||
       settings->roughness<0||settings->roughness>100||settings->breathiness<0||settings->breathiness>100||
       settings->gender<0||settings->gender>1)return RELOQ_DE_UTTERANCE_INVALID;
    for(size_t j=0;j<index_count;++j)
        if(indices[j].annotation_offset>length||(j&&indices[j].annotation_offset<indices[j-1].annotation_offset))return RELOQ_DE_UTTERANCE_INVALID;
    for(size_t j=0;j<pitch_count;++j)
        if(pitches[j].native_pitch<0||pitches[j].native_pitch>100||pitches[j].annotation_offset>length||
           (j&&pitches[j].annotation_offset<pitches[j-1].annotation_offset))return RELOQ_DE_UTTERANCE_INVALID;
    unsigned char *annotation=NULL;size_t *map=NULL,n=0,count=0;struct unit *u=NULL;
    struct events events={0};struct reloq_acoustic_point *points=NULL,*sibilant_points=NULL;size_t *samples=NULL;
    int result=RELOQ_DE_UTTERANCE_INVALID;
    if(reloq_english_analyze(text,length,tags,&annotation,&map,&n,NULL))goto done;
    if(parse(annotation,n,map,tags,settings,native_volume,&u,&count))goto done;
    if(reloq_session_cancelled(session)){result=RELOQ_DE_UTTERANCE_CANCELLED;goto done;}
    int64_t now=0;size_t next_pitch=0;
    for(size_t i=0;i<count;++i) {
        if(reloq_session_cancelled(session)){result=RELOQ_DE_UTTERANCE_CANCELLED;goto done;}
        int fresh=i==0||u[i-1].pause,terminal=u[i].pause!=0;
        int a=number(u[i].phone),left=fresh?-1:number(u[i-1].phone),right=terminal?-1:number(u[i+1].phone);
        int final_syllable=u[i].final_syllable;
        unsigned mode=u[i].weak?3:!u[i].stress?2:final_syllable?0:1;
        const struct english_gesture *in=left<0?NULL:pair(mode,left,a,1);
        const struct english_gesture *out=right<0?NULL:pair(mode,a,right,0);
        if(mode<3&&left>=0&&u[i-1].phone=='x'&&u[i].phone=='r'&&u[i-1].syllable!=u[i].syllable)
            in=english_gestures+english_cross_xr[mode][1];
        if(mode<3&&right>=0&&u[i].phone=='x'&&u[i+1].phone=='r'&&u[i].syllable!=u[i+1].syllable)
            out=english_gestures+english_cross_xr[mode][0];
        /* Original observations distinguish word boundaries from the
         * within-syllable transitions used by the earlier composer. */
        if(left>=0&&mode<2&&u[i-1].word!=u[i].word&&
           u[i-1].stress==1&&!u[i-1].weak&&u[i].stress==1) {
            unsigned id=english_boundaries83[mode][left][a][1];
            if(id)in=english_gestures+id;
        }
        int word_end=right>=0&&mode==1&&u[i].word!=u[i+1].word&&
            u[i].stress==1&&u[i+1].stress==1&&!u[i+1].weak;
        if(word_end) {
            unsigned next_mode=u[i+1].final_syllable?0:1;
            unsigned id=english_boundaries83[next_mode][a][right][0];
            if(id)out=english_gestures+id;
        }
        /* A legal cross-word sequence can normalize away in an artificial
         * single-syllable probe (notably weak schwa followed by /r/).
         * Use the measured word boundary instead of aborting the utterance. */
        if(!in&&left>=0&&u[i-1].word!=u[i].word) {
            unsigned id=english_boundaries83[!final_syllable][left][a][1];
            if(id)in=english_gestures+id;
        }
        if(!out&&right>=0&&u[i].word!=u[i+1].word) {
            unsigned id=english_boundaries83[!u[i+1].final_syllable][a][right][0];
            if(id)out=english_gestures+id;
        }
        const struct english_gesture *base=isolated(a);
        /* A terminal vowel keeps the incoming context's duration and
         * reduction. An isolated stressed nucleus is only a true fallback
         * for an isolated phone, never a replacement for a weak ending. */
        if(!out)out=terminal?(in?in:base):NULL;
        if(!out&&fresh)out=pair(mode,a,number('a'),0);
        if(!in)in=fresh?out:NULL;
        if(!in||!out) {
#ifdef RELOQ_ENGLISH_DIAGNOSTICS
            fprintf(stderr,"%u %c %c %c %d %d\n",mode,left<0?'_':english_inventory[left],u[i].phone,right<0?'_':english_inventory[right],in!=NULL,out!=NULL);
#endif
            result=RELOQ_DE_UTTERANCE_UNCOVERED;goto done;
        }
        if(u[i].aspirated) {
            if(!english_aspirated[!final_syllable][a]){result=RELOQ_DE_UTTERANCE_UNCOVERED;goto done;}
            in=english_gestures+english_aspirated[!final_syllable][a];
            if(terminal)out=in;
        }
        int timing[35];timing_features(u,count,i,in,out,timing);
#ifdef RELOQ_ENGLISH_PHONE_TRACE
        fputs("features",stderr);
        for(unsigned j=0;j<35;++j)fprintf(stderr," %d",timing[j]);
        fputc('\n',stderr);
#endif
        double scale=settings->native_speed?reloq_voice_speed_time_scale(u[i].speed):(150-settings->rate)/100.0;
        if(tags)scale=reloq_voice_speed_time_scale(u[i].speed);
        if(scale<=0)goto done;
        u[i].scale=scale;
        int incoming=in->contact;
        double duration=out->duration;
        if(vowel(u[i].phone) && !terminal) {
            int aspiration=u[i].aspirated||(left>=0&&strchr("ptk",u[i-1].phone));
            if(base&&left>=0&&strchr("ptk",u[i-1].phone)) {
                /* The following constriction belongs to the vowel's full
                 * timing budget. Shorten that budget before removing the
                 * transition, rather than shortening only its steady part. */
                if(right>=0&&!vowel(u[i+1].phone))
                    duration=floor((duration+out->release)*.9)-out->release;
                else duration*=.9;
            }
            if(aspiration)duration+=incoming;
            if(duration<=incoming)duration=incoming+1;
        } else if(!vowel(u[i].phone)) {
            duration=in->duration;
            if(fresh||word_end)duration=out->duration;
            if(duration<=incoming)duration=incoming+1;
        }
        int release=out->release;
        /* Pair timing is sufficient for a simple isolated syllable. Complex
         * syllables, reductions and multi-syllable words need their own
         * observed contact/body/release budgets, independent of spelling. */
        int complex=timing[21]>1||timing[6]||timing[16]+timing[17]>=3||
            (i&&!u[i-1].pause&&u[i-1].weak)||(i+1<count&&!terminal&&u[i+1].weak);
        if(complex
#ifdef RELOQ_ENGLISH_TIMING_BASELINE
            &&0
#endif
        ) {
            int contact=supported_timing89(english_timing_nodes89+english_timing_roots89[0][a],english_timing_contact,timing);
            int body=supported_timing89(english_timing_nodes89+english_timing_roots89[1][a],english_timing_body,timing);
            int tail=supported_timing89(english_timing_nodes89+english_timing_roots89[2][a],english_timing_release,timing);
            if(contact>=0&&contact<=250&&body>=0&&body<=1000&&tail>=0&&tail<=500) {
                incoming=contact;duration=contact+(body?body:1);release=tail;
            }
        }
        if(u[i].aspirated && timing[21]==1 && strchr("iIeEAXHxauUocR",u[i].phone)) {
            int h_ticks=u[i].stress?80:50;
            double body=out->duration;
            if(out==in || u[i].phone=='a')body-=out->contact;
            duration=h_ticks+(body>0?body:1);release=out->release;
            incoming=h_ticks;
        } else if(fresh && u[i].weak && !terminal && timing[21]==1 &&
                  strchr("ieAXxauocRYW",u[i].phone)) {
            /* Weak phrase starts have no preceding vowel and therefore
             * no inherited 65-tick hiatus. Most nuclei retain their full
             * budget; a/ay/aw carry that contact outside their budget. */
            incoming=i?1:0;
            duration=out->duration-(strchr("aYW",u[i].phone)?out->contact:0)+incoming;
            release=out->release;
        }
        if(u[i].before_h && u[i].word!=u[i+1].word) {
            size_t last=i+1;
            while(last+1<count && u[last+1].word==u[i+1].word)++last;
            if(u[last].syllable==u[i+1].syllable)release=u[i+1].stress?80:50;
        }
        /* The s+stop family has its own closure and release budgets.
         * It cannot reuse an isolated p/t/k onset, even when the source
         * selector returns no replacement profile. Original contrasts
         * distinguish a new word beginning in the stop from a stop which
         * closes the preceding word. Vowel identity is immaterial here. */
        if(u[i].phone=='s' && right>=0 && strchr("ptk",u[i+1].phone) &&
           i+2<count && !u[i+1].pause && vowel(u[i+2].phone) &&
           (fresh || (left>=0 && vowel(u[i-1].phone)))) {
            if(fresh)incoming=0;
            int body=u[i].word==u[i+1].word &&
                u[i+1].word!=u[i+2].word?60:
                u[i].word==u[i+1].word && u[i].syllable!=u[i+1].syllable &&
                u[i+1].stress!=1?80:90;
            duration=incoming+body;
        }
        if(left>=0 && u[i-1].phone=='s' && strchr("ptk",u[i].phone) &&
           right>=0 && vowel(u[i+1].phone)) {
            incoming=1;
            int body=u[i].phone=='p'?75:u[i].phone=='t'?60:
                !u[i].stress && timing[20]>0?70:65;
            duration=incoming+body;
        }
        if(i>=2 && vowel(u[i].phone) && !u[i].aspirated &&
           strchr("ptk",u[i-1].phone) && u[i-2].phone=='s' &&
           !u[i-1].pause && !u[i-2].pause) {
            const int unaspirated=u[i-2].word==u[i-1].word;
            incoming=unaspirated?
                (u[i-1].phone=='t' && u[i-1].word!=u[i].word?20:
                 u[i-1].phone=='p' && u[i].phone=='R' && !u[i].stress && timing[20]>0?40:30):
                u[i-1].phone=='p'?50:70;
            /* An unaspirated transition is inside the nucleus budget.
             * Do not apply the .9 aspirated-onset shortening and then
             * add an independent 50/70-tick aspiration to that budget. */
            if(timing[21]==1 && !u[i].weak && unaspirated) {
                duration=terminal && base?base->duration:out->duration;
                if(duration<=incoming)duration=incoming+1;
            } else if(timing[21]==1 && terminal && base && !unaspirated) {
                duration=floor(base->duration*.9)+incoming;
            }
        }
        if(left>=0&&vowel(u[i].phone)&&!u[i].aspirated) {
            int contact=reloq_english_source_boundary_contact(u[i-1].phone,
                u[i-1].word!=u[i].word);
            if(contact>=0){duration+=contact-incoming;incoming=contact;}
        }
        /* The nasal family returns nominal intervals. A nucleus shortened
         * to its free-part minimum and a multipart nucleus already own a
         * distributed interval budget; replacing just one part would undo
         * that distribution. Retain their allocated incoming interval. */
        if(left>=0 && !u[i].aspirated && duration-incoming>3 &&
           !strchr("YWO",u[i].phone)) {
            int contact=reloq_english_nasal_transition_ticks(u[i-1].phone,u[i].phone,
                u[i-1].word!=u[i].word);
            if(contact>=0){duration+=contact-incoming;incoming=contact;}
        }
        if(terminal) {
            int tail=reloq_english_nasal_transition_ticks(u[i].phone,0,1);
            if(tail>=0)release=tail;
        }
        u[i].start=now;u[i].contact=now+(int64_t)llround(incoming*scale);
        u[i].end=now+(int64_t)llround(duration*scale);
        if(u[i].end<=u[i].contact)u[i].end=u[i].contact+1;
        u[i].release=u[i].end+(int64_t)llround(release*scale);
        u[i].incoming=in;u[i].outgoing=out;
        memcpy(u[i].timing,timing,sizeof timing);
        now=u[i].end;
        if(terminal) {
            int pause=u[i].pause;
            if(pause>=100000)pause-=100000;
            else if(settings->pause_mode==2||(settings->pause_mode==1&&i+1==count))pause=40;
            now=u[i].release+pause;
        }
    }
    if(plan_nuclei(u,count,&now,settings->native_speed||tags))goto done;
    /* A connected transition has one clock, shared by the outgoing
     * gesture and the next phone's incoming gesture. Resolve it before
     * generating any source, tract, nasal or noise targets. Independent
     * release/contact estimates must not create competing handoffs.
     * Successive vowel members and postvocalic liquid/glide continuations
     * retain the separate nucleus budget used by the original. */
    for(size_t i=0;i+1<count;++i)if(!u[i].pause &&
        !(vowel(u[i+1].phone) &&
          (vowel(u[i].phone)||strchr("lrwy",u[i].phone))))
        u[i].release=u[i+1].contact;
    for(size_t i=0;i<count;++i) {
        if(reloq_session_cancelled(session)){result=RELOQ_DE_UTTERANCE_CANCELLED;goto done;}
        const int terminal=u[i].pause!=0;
        const int left=i&&!u[i-1].pause?number(u[i-1].phone):-1;
        const int right=terminal?-1:number(u[i+1].phone);
        const struct english_gesture *in=u[i].incoming,*out=u[i].outgoing;
        const int *timing=u[i].timing;
        u[i].head=retime(in->head,in,u+i);u[i].event_begin=events.n;
#ifdef RELOQ_ENGLISH_PHONE_TRACE
        fprintf(stderr,"phone %c %zu %zu %u %u %lld %lld %lld %lld\n",
            u[i].phone,u[i].word,u[i].syllable,u[i].stress,u[i].weak,
            (long long)u[i].start,(long long)u[i].contact,
            (long long)u[i].end,(long long)u[i].release);
        fprintf(stderr,"gesture %u %u %u %u %u %u\n",in->contact,in->duration,in->release,
            out->contact,out->duration,out->release);
        if(getenv("RELOQ_ENGLISH_GESTURE_TRACE"))for(unsigned side=0;side<2;++side) {
            const struct english_gesture *g=side?out:in;
            for(unsigned j=0;j<g->count;++j) {
                const struct english_landmark *p=english_landmarks+g->first+j;
                fprintf(stderr,"landmark %c %u %s %d %d %lld\n",u[i].phone,side,
                    reloq_target_stream_name(p->stream),p->tick,p->value,
                    (long long)retime(p->tick,g,u+i));
            }
        }
#endif
        int nasal=vowel(u[i].phone)&&right>=0&&strchr("mnG",u[i+1].phone);
        int tract=right>=0 && u[i].word==u[i+1].word &&
            vowel(u[i].phone) && u[i+1].phone=='r'?1:0;
        if(right>=0 && left>=0 && u[i].phone=='l' && vowel(u[i+1].phone) &&
           vowel(u[i-1].phone) && u[i].word==u[i+1].word &&
           u[i].syllable==u[i+1].syllable)tract=2;
        if(emit_side(&events,in,u+i,0,terminal,nasal,tract)||
           emit_side(&events,out,u+i,1,terminal,nasal,tract))goto memory;
        /* Rhotic anticipation affects the whole vowel, as nasal widening
         * does. A syllable-initial l likewise keeps its light-l tract;
         * splicing in the coda half had imposed an unrelated dark onset. */
        if(tract)for(unsigned j=0;j<out->count;++j) {
            const struct english_landmark *p=english_landmarks+out->first+j;
            if(p->stream>=RELOQ_TARGET_F1 &&
               p->stream<=(tract==1?RELOQ_TARGET_F3:RELOQ_TARGET_F5) &&
               p->tick>=0 && p->tick<=out->duration &&
               add(&events,p->stream,retime(p->tick,out,u+i),p->value))goto memory;
        }
        /* Anticipatory nasalization widens the whole preceding nucleus;
         * joining two half gestures had turned it into an unintended ramp. */
        if(nasal)for(unsigned j=0;j<out->count;++j) {
            const struct english_landmark *p=english_landmarks+out->first+j;
            if(p->stream>=RELOQ_TARGET_B1&&p->stream<=RELOQ_TARGET_B5&&p->tick<=out->duration&&
               add(&events,p->stream,retime(p->tick,out,u+i),p->value))goto memory;
        }
        if(!terminal && strchr("bpdtkg?F",u[i].phone) &&
           emit_stop_release(&events,out,u+i))goto memory;
        if(measured_acoustics87(&events,u+i,timing))goto memory;
        if(contract_tract_targets(&events,u+i))goto memory;
        if(measured_source88(&events,u+i,timing))goto memory;
        stop_closure_source(&events,u+i,left<0?0:u[i-1].phone,
            right<0?0:u[i+1].aspirated?'h':u[i+1].phone);
        int aspirated_stop=left>=0 && strchr("ptk",u[i-1].phone)!=NULL;
        /* In s+stop clusters the nucleus is voiced from its start. The
         * isolated stop pair alone would insert an aspirated onset. */
        if(aspirated_stop && i>=2 && u[i-2].phone=='s' && !u[i-2].pause &&
            u[i-2].word==u[i-1].word)aspirated_stop=2;
        if((timing[21]==1 || aspirated_stop || u[i].aspirated) &&
           aspirated_source90(&events,u+i,in,out,aspirated_stop))goto memory;
        if(u[i].phone=='w' && aspirated_stop && right>=0 && vowel(u[i+1].phone) &&
           u[i-1].word==u[i].word &&
           stop_glide_source93(&events,u+i,aspirated_stop==2,timing[21]))goto memory;
#ifdef RELOQ_ENGLISH_PHONE_TRACE
        if(getenv("RELOQ_ENGLISH_SOURCE_TRACE")) {
            fprintf(stderr,"source_profile %zu %c %u\n",i,u[i].phone,u[i].source_profile);
            for(size_t j=u[i].event_begin;j<events.n;++j)if(events.p[j].stream>=RELOQ_TARGET_TL)
                fprintf(stderr,"source_event %zu %c %s %lld %d\n",i,u[i].phone,
                    reloq_target_stream_name(events.p[j].stream),(long long)events.p[j].tick,events.p[j].value);
        }
#endif
        /* Bound source and tract ownership at a phone's own boundaries.
         * Sparse pair targets are interpolated once across the entire
         * sentence; neither source history nor audio filters are reset. */
        while(next_pitch<pitch_count&&pitches[next_pitch].annotation_offset<=u[i].source_begin)
            u[i].pitch=pitches[next_pitch++].native_pitch;
        if(next_pitch)u[i].pitch=pitches[next_pitch-1].native_pitch;
        now=u[i].end;
        if(terminal) {
            now=u[i].release;
            int pause=u[i].pause;
            if(pause>=100000)pause-=100000;
            else if(settings->pause_mode==2||(settings->pause_mode==1&&i+1==count))pause=40;
            for(unsigned stream=RELOQ_TARGET_AV;stream<RELOQ_TARGET_STREAM_COUNT;++stream)
                if(add(&events,stream,now,0)||add(&events,stream,now+pause,0))goto memory;
            /* The pause tract returns the upper resonances to the observed
             * neutral values after a sibilant or alveolar release. Retaining
             * the consonant's F4/F5 changed the still-audible release. */
            if(add(&events,RELOQ_TARGET_F4,now,3600)||
               add(&events,RELOQ_TARGET_F4,now+pause,3600)||
               add(&events,RELOQ_TARGET_F5,now,3900)||
               add(&events,RELOQ_TARGET_F5,now+pause,3900))goto memory;
            static const unsigned char neutral_streams[]={RELOQ_TARGET_B1,
                RELOQ_TARGET_B2,RELOQ_TARGET_B3,RELOQ_TARGET_B4,RELOQ_TARGET_B5,
                RELOQ_TARGET_FNP,RELOQ_TARGET_FNZ,RELOQ_TARGET_TL,RELOQ_TARGET_OQ};
            static const short neutral_values[]={120,100,150,330,260,200,200,0,56};
            for(unsigned j=0;j<sizeof neutral_streams;++j)
                if(add(&events,neutral_streams[j],now,neutral_values[j])||
                   add(&events,neutral_streams[j],now+pause,neutral_values[j]))goto memory;
            now+=pause;
        }
    }
    const size_t planned_events=events.n;
    /* The unaspirated s+stop cluster ends its AV/AH closure at the
     * following nucleus's start. A reused aspirated stop envelope must
     * not place a second zero inside that already voiced nucleus. This
     * also holds across a word boundary (original: "ist ein"). */
    for(size_t i=2;i<count;++i)if(vowel(u[i].phone) &&
        strchr("ptk",u[i-1].phone) && u[i-2].phone=='s' &&
        !u[i-1].pause && !u[i-2].pause && u[i-2].word==u[i-1].word) {
        int early=0;
        size_t limit=i+1<count?u[i+1].event_begin:events.n;
        for(size_t j=u[i].event_begin;j<limit;++j)
            if(events.p[j].stream==RELOQ_TARGET_AV && events.p[j].value>0 &&
               events.p[j].tick<=u[i].start+1)early=1;
        if(early)for(size_t j=u[i-1].event_begin;j<u[i].event_begin;++j) {
            struct event *p=events.p+j;
            if((p->stream==RELOQ_TARGET_AV || p->stream==RELOQ_TARGET_AH) &&
               p->tick>=u[i].start) {
                p->tick=u[i].start;p->value=0;
            }
        }
    }
    for(size_t i=0;i+1<count;++i)if(!u[i].pause&&vowel(u[i].phone)&&
        u[i].word==u[i+1].word&&strchr("lrwy",u[i+1].phone)) {
        const struct english_gesture *g=u[i].outgoing;
        for(unsigned j=0;j<g->count;++j) {
            const struct english_landmark *p=english_landmarks+g->first+j;
            if(p->tick>=0 || (u[i].source_profile && p->stream>=RELOQ_TARGET_TL))continue;
            int64_t t=u[i+1].pause?u[i+1].end:u[i+1].release;
            if(u[i].before_h)t=u[i].end;
            if(add(&events,p->stream,t,p->value))goto memory;
            /* Preserve original source ownership at a shared endpoint:
             * the following closure's value comes after the vowel's. */
            events.p[events.n-1].serial=u[i].event_begin;
        }
    }
    for(size_t i=0;i+1<count;++i)
        if(nasal_source93(&events,u+i,u+i+1,
            i+2<count?u[i+2].event_begin:planned_events))goto memory;
    for(size_t i=0;i+1<count;++i)
        if(deferred_source88(&events,u+i,u+i+1))goto memory;
    if(coordinate_source(&events,u,count,planned_events))goto memory;
    if(phrase_pitch(&events,u,count,settings))goto memory;
    if(!count) {
        for(size_t i=0;i<index_count;++i)if(index_sink&&index_sink(context,i)){result=RELOQ_DE_UTTERANCE_SINK;goto done;}
        result=RELOQ_DE_UTTERANCE_OK;goto done;
    }
    qsort(events.p,events.n,sizeof *events.p,compare);
    points=malloc(events.n*sizeof *points);if(!points)goto memory;
    struct reloq_target_buffer buffers[28];size_t at=0;
    struct reloq_german_voice_model model={settings->head_size_plus_one?settings->head_size_plus_one-1:50,
        settings->roughness,settings->breathiness,settings->gender,reloq_session_sample_rate(session)};
    for(unsigned stream=0;stream<28;++stream) {
        size_t begin=at;
        while(at<events.n&&events.p[at].stream==stream) {
            /* Shared voice anatomy is applied to endpoints. Phonetic
             * identities and target generation remain wholly English. */
            int value=events.p[at].value;
            if(stream!=RELOQ_TARGET_F0)value=reloq_german_voice_model_target(&model,NULL,NULL,(enum reloq_target_stream)stream,value);
            if(value>0 && (stream==RELOQ_TARGET_AV ||
               stream==RELOQ_TARGET_AH || stream==RELOQ_TARGET_AF)) {
                /* Serials retain their phonetic owner even when source
                 * coordination defers a knot or sorting interleaves phones.
                 * Do not infer S/Z from a shared dental noise filter. */
                size_t first=0,last=count;
                while(first+1<last) {
                    size_t middle=first+(last-first)/2;
                    if(u[middle].event_begin<=events.p[at].serial)first=middle;
                    else last=middle;
                }
                unsigned char phone=u[first].phone;
                /* Match the accepted German S balance after anatomy and
                 * breath controls. Only frication gains four source dB;
                 * voiced /z/ retains its existing periodic excitation. */
                if(stream==RELOQ_TARGET_AF && (phone=='s' || phone=='z'))value+=4;
                /* English encodes /h/ on its following nucleus. The same
                 * nucleus holds a stop's aspiration until contact. Boost
                 * that consonant-owned AH without raising vowel breath. */
                int release=stream==RELOQ_TARGET_AH &&
                    events.p[at].tick<=u[first].contact &&
                    (u[first].aspirated || (first && !u[first-1].pause &&
                     strchr("ptk",u[first-1].phone) &&
                     !(first>1 && u[first-2].phone=='s' &&
                       !u[first-2].pause && u[first-2].word==u[first-1].word)));
                if((phone && !vowel(phone) && phone!='s' && phone!='z') || release)++value;
            }
            points[at]=(struct reloq_acoustic_point){events.p[at].tick,(float)value};++at;
        }
        buffers[stream]=(struct reloq_target_buffer){points+begin,at-begin,at-begin};
    }
    struct reloq_target_builder builder;
    if(reloq_target_builder_begin(&builder,buffers))goto done;
    float defaults[62];uint8_t fields[28];struct reloq_session_config config;
    if(reloq_german_voice_defaults(defaults,fields,&config))goto done;
    struct reloq_curve_schedule schedule={.first_tick=0,.end_tick=now,.frame_ticks=1};unsigned char mapped[62];
    if(reloq_target_builder_export(&builder,fields,defaults,schedule.tracks,mapped))goto done;
    if(index_count) {
        if(index_count>SIZE_MAX/sizeof *samples)goto memory;
        samples=malloc(index_count*sizeof *samples);if(!samples)goto memory;
        for(size_t i=0;i<index_count;++i) {
            size_t offset=indices[i].annotation_offset;int64_t tick=now;
            for(size_t j=0;j<count;++j) {
                if(offset<=u[j].source_begin){tick=u[j].start;break;}
                if(offset<=u[j].source_end) {
                    while(j+1<count&&u[j+1].word==u[j].word)++j;
                    tick=j+1<count?u[j+1].start:now;break;
                }
            }
            /* Match the renderer's integer samples-per-millisecond clock,
             * including 11,025, 22,050 and 44,100 Hz. */
            samples[i]=(size_t)tick*(reloq_session_sample_rate(session)/1000);
        }
    }
    struct delivery d={.sink=sink,.index_sink=index_sink,.context=context,.samples=samples,
        .indices=index_count,.units=u,.unit_count=count,
        .clock=reloq_session_sample_rate(session)/1000,.generic_gain=settings->volume/100.0};
    struct reloq_acoustic_curve sibilant_curve={0};
    if(reloq_session_sample_rate(session)>=16000) {
        if(count>SIZE_MAX/2/sizeof *sibilant_points)goto memory;
        sibilant_points=malloc(2*count*sizeof *sibilant_points);if(!sibilant_points)goto memory;
        size_t used=0;int previous=0;
        for(size_t i=0;i<count;++i)if(!vowel(u[i].phone)) {
            int active=u[i].phone=='s'||u[i].phone=='z'||u[i].phone=='t';
            if(u[i].start>0)sibilant_points[used++]=(struct reloq_acoustic_point){u[i].start-1,(float)previous};
            sibilant_points[used++]=(struct reloq_acoustic_point){u[i].start,(float)active};previous=active;
        }
        sibilant_curve=(struct reloq_acoustic_curve){0,sibilant_points,used};
        schedule.s_frication=&sibilant_curve;
    }
    struct output_profile profile={model};size_t completed=0;
    int status=reloq_curve_pipeline_render_profile(&schedule,reloq_german_voice_model_formants(&model),
        session,deliver,&d,&completed,frame_profile,&profile);
    if(reloq_session_cancelled(session))result=RELOQ_DE_UTTERANCE_CANCELLED;
    else if(status)result=status==RELOQ_SESSION_INVALID?RELOQ_DE_UTTERANCE_RENDER:RELOQ_DE_UTTERANCE_SINK;
    else {
        result=RELOQ_DE_UTTERANCE_OK;
        while(d.next<d.indices) {
            if(index_sink&&index_sink(context,d.next)){result=RELOQ_DE_UTTERANCE_SINK;break;}++d.next;
        }
    }
    goto done;
memory:result=RELOQ_DE_UTTERANCE_MEMORY;
done:free(annotation);free(map);free(u);free(events.p);free(points);free(samples);free(sibilant_points);return result;
}
