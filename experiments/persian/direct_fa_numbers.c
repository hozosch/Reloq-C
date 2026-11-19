/* Direct Persian cardinal/decimal experiment, GPL-3.0-or-later. */
#include "direct_fa_numbers.h"
#include <stdint.h>
#include <string.h>
#include "direct_fa_numbers.inc"

struct number_output { char *out;size_t used,capacity; };

static int digit(char c) { return c>='0' && c<='9'; }
static size_t arabic_mark(const char *text,size_t length,size_t at,unsigned char last)
{ return at+1<length && (unsigned char)text[at]==0xd9 && (unsigned char)text[at+1]==last ? 2:0; }

/* A cardinal group has final lexical prominence, including a group made of
 * several native tags (e.g. one hundred and one). Keep the reference's
 * secondary stresses and phones, then apply its existing phrase weakening. */
static int emit(struct number_output *b,const char *text,int weaken,int final_stress,
                int short_hundred)
{
    size_t n=strlen(text),space=b->used?1:0;
    /* In 100–199, the source table lengthens the vowel of صد. Persian صد
     * uses the same short /æ/ as the independently spoken word. Match the
     * first syllable only, preserving the remaining number and its stress. */
    int shorten=short_hundred && n>=9 && text[0]=='`' && text[1]=='[' &&
                text[2]=='.' && text[3]>='0' && text[3]<='2' && text[4]=='s' &&
                !memcmp(text+5,"'A:'",4);
    size_t written=n-(shorten?3u:0u);
    if(written>SIZE_MAX-b->used-space-1 || b->used+space+written>=b->capacity)return -2;
    if(space)b->out[b->used++]=' ';
    size_t last=SIZE_MAX;
    if(final_stress)for(size_t i=1;i<n;++i)
        if(text[i-1]=='.' && text[i]>='0' && text[i]<='2')last=i;
    for(size_t i=0;i<n;++i) {
        if(shorten && i==5) {b->out[b->used++]='A';i+=3;continue;}
        char value=text[i];
        if(last!=SIZE_MAX && i && text[i-1]=='.' && value>='0' && value<='2') {
            if(i==last)value='1';
            else if(value=='1')value='0';
        }
        if(weaken && value=='1' && i && text[i-1]=='.')value=weaken==1?'0':'2';
        b->out[b->used++]=value;
    }
    b->out[b->used]=0;return 0;
}

/* First construct linguistic events, without ECI controls or an acoustic
 * language. The legacy renderer below is a temporary compatibility bridge. */
struct number_plan {
    struct reloq_fa_number_event events[48];
    size_t used;
};

static int add_event(struct number_plan *p,uint32_t kind,uint32_t value,
                     uint32_t linked,uint32_t weaken,uint32_t final_stress)
{
    if(p->used>=sizeof p->events/sizeof p->events[0])return -2;
    p->events[p->used++]=(struct reloq_fa_number_event){kind,value,linked,weaken,final_stress};
    return 0;
}

static int plan_cardinal(struct number_plan *p,const char *digits,size_t n,int following)
{
    if(n>1 && digits[0]=='0') {
        /* Preserve leading zeroes (identifiers and phone-like strings). */
        for(size_t i=0;i<n;++i)
            if(add_event(p,RELOQ_FA_NUMBER_GROUP,(uint32_t)(digits[i]-'0'),0,
                         i+1<n?1:following?2:0,1))return -2;
        return 0;
    }
    unsigned groups[4]={0};size_t group=0;
    while(n) {
        size_t first=n>3?n-3:0;unsigned value=0;
        for(size_t i=first;i<n;++i)value=value*10+(unsigned)(digits[i]-'0');
        groups[group++]=value;n=first;
    }
    if(group==1 && groups[0]==0)
        return add_event(p,RELOQ_FA_NUMBER_GROUP,0,0,following?2:0,1);
    while(group) {
        size_t position=--group;
        if(!groups[position])continue;
        int lower=0;
        for(size_t j=0;j<position;++j)if(groups[j])lower=1;
        if(add_event(p,RELOQ_FA_NUMBER_GROUP,groups[position],0,
                     position?(lower?2:1):(following?2:0),1))return -2;
        /* The linked form has an unstressed final linking vowel. */
        if(position && add_event(p,RELOQ_FA_NUMBER_SCALE,(uint32_t)(position-1),
                                 lower?1:0,following&&!lower?2:0,!lower))return -2;
    }
    return 0;
}

/* Dotted version components are cardinal groups, not successive decimal
 * fractions. Keep every point in the Persian number plan, including when
 * NVDA has separated an alphanumeric prefix from the first digit. */
static int plan_version(const char *text,size_t length,size_t *consumed,
                        struct number_plan *plan)
{
    size_t at=0,points=0;int too_long=0;
    while(at<length && digit(text[at])) {
        size_t start=at;
        while(at<length && digit(text[at]))++at;
        if(at-start>12)too_long=1;
        size_t width=at<length && text[at]=='.'?1:arabic_mark(text,length,at,0xab);
        if(!width || at+width>=length || !digit(text[at+width]))break;
        ++points;at+=width;
    }
    if(points<2)return 1;
    if(too_long)return 2;
    /* Do not consume a version prefix of a malformed grouped number. */
    if(at<length) {
        size_t width=text[at]==','?1:
                     (arabic_mark(text,length,at,0xab)||arabic_mark(text,length,at,0xac));
        if(width && at+width<length && digit(text[at+width]))return 2;
    }
    size_t cursor=0;
    while(cursor<at) {
        size_t start=cursor;
        while(cursor<at && digit(text[cursor]))++cursor;
        int following=cursor<at;
        if(plan_cardinal(plan,text+start,cursor-start,following))return -2;
        if(following) {
            if(add_event(plan,RELOQ_FA_NUMBER_DECIMAL,0,0,0,0))return -2;
            cursor+=text[cursor]=='.'?1:arabic_mark(text,at,cursor,0xab);
        }
    }
    *consumed=at;
    return 0;
}

int reloq_fa_number_events(const char *text,size_t length,size_t *consumed,
    struct reloq_fa_number_event *events,size_t capacity,size_t *count)
{
    if(consumed)*consumed=0;
    if(count)*count=0;
    if(!text || !length || !consumed || !count || (!events && capacity) || !digit(text[0]))return -1;
    struct number_plan version={0};size_t version_used=0;
    int version_status=plan_version(text,length,&version_used,&version);
    if(version_status!=1) {
        if(version_status)return version_status;
        if(events && capacity<version.used)return -2;
        if(events)memcpy(events,version.events,version.used*sizeof *events);
        *consumed=version_used;*count=version.used;return 0;
    }
    char integer[12],fraction[32];size_t ni=0,nf=0,at=0;
    while(at<length && digit(text[at])) {
        if(ni==sizeof integer)return 2;
        integer[ni++]=text[at++];
    }
    size_t group_width=0;int group_kind=0;
    while(at<length) {
        int kind=text[at]==','?1:arabic_mark(text,length,at,0xac)?2:0;
        if(!kind)break;
        size_t width=kind==1?1:2;
        if(at+width>=length || !digit(text[at+width])) {
            if(kind==1)break; /* a normal comma after a number */
            return 2;
        }
        if(!group_kind) {if(ni>3)return 2;group_kind=kind;}
        if(group_kind!=kind)return 2;
        at+=width;group_width=0;
        while(at<length && digit(text[at])) {
            if(ni==sizeof integer || group_width==3)return 2;
            integer[ni++]=text[at++];++group_width;
        }
        if(group_width!=3)return 2;
    }
    size_t decimal_width=at<length && text[at]=='.'?1:arabic_mark(text,length,at,0xab);
    if(decimal_width) {
        if(at+decimal_width>=length || !digit(text[at+decimal_width])) {
            if(decimal_width==2)return 2;
            decimal_width=0; /* sentence-final full stop */
        } else {
            at+=decimal_width;
            while(at<length && digit(text[at])) {
                if(nf==sizeof fraction)return 2;
                fraction[nf++]=text[at++];
            }
        }
    }
    if(nf && at<length) {
        size_t width=text[at]=='.'||text[at]==','?1:
                     (arabic_mark(text,length,at,0xab)||arabic_mark(text,length,at,0xac));
        if(width && at+width<length && digit(text[at+width]))return 2;
    }
    struct number_plan plan={0};
    if(plan_cardinal(&plan,integer,ni,nf!=0))return -2;
    if(nf) {
        if(add_event(&plan,RELOQ_FA_NUMBER_DECIMAL,0,0,0,0))return -2;
        for(size_t i=0;i<nf;++i)
            if(add_event(&plan,RELOQ_FA_NUMBER_GROUP,(uint32_t)(fraction[i]-'0'),0,
                         i+1<nf?1:0,1))return -2;
    }
    if(events && capacity<plan.used)return -2;
    if(events)memcpy(events,plan.events,plan.used*sizeof *events);
    *consumed=at;*count=plan.used;return 0;
}

int reloq_fa_direct_number(const char *text,size_t length,size_t *consumed,
                           char *output,size_t capacity)
{
    if(consumed)*consumed=0;
    if(output && capacity)output[0]=0;
    if(!output || !capacity || !consumed)return -1;
    struct reloq_fa_number_event events[48];size_t count=0,used=0;
    int status=reloq_fa_number_events(text,length,&used,events,
                                      sizeof events/sizeof events[0],&count);
    if(status)return status;
    struct number_output b={output,0,capacity};
    for(size_t i=0;i<count;++i) {
        const struct reloq_fa_number_event *event=&events[i];
        const char *value=event->kind==RELOQ_FA_NUMBER_GROUP?fa_number_groups[event->value]:
            event->kind==RELOQ_FA_NUMBER_SCALE?fa_number_scales[event->value][event->linked]:
            fa_number_decimal;
        int hundred=event->kind==RELOQ_FA_NUMBER_GROUP && event->value>=100 && event->value<200;
        if(emit(&b,value,(int)event->weaken,(int)event->final_stress,hundred))return -2;
    }
    *consumed=used;return 0;
}
