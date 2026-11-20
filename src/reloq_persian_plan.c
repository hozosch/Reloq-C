/* Persian-owned linguistic event planner. This module does not call a
 * Finnish, English, eSpeak or other language runtime. The existing lexicon's
 * serialized phone data is decoded here as an interim representation. */
#include "reloq_persian_plan.h"
#include "reloq_persian_direct.h"
#include "reloq_persian_phones.h"
#include "reloq_persian_timing.h"
#include "reloq_utf8.h"
#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct plan {struct reloq_fa_plan_event *events;size_t n,cap;};

static int push(struct plan *p,uint32_t kind,uint32_t value,uint32_t flags,
                uint32_t syllable,uint32_t stress)
{
    if(p->n==p->cap) {
        size_t cap=p->cap?p->cap*2:128;
        if(cap<p->cap || cap>SIZE_MAX/sizeof *p->events)return -2;
        void *next=realloc(p->events,cap*sizeof *p->events);
        if(!next)return -2;
        p->events=next;p->cap=cap;
    }
    p->events[p->n++]=(struct reloq_fa_plan_event){kind,value,flags,syllable,stress};
    return 0;
}

static uint32_t phone_id(char c)
{
    switch(c) {
    case 'A':return RELOQ_FA_PHONE_AE; case 'a':return RELOQ_FA_PHONE_BACK_A;
    case 'e':return RELOQ_FA_PHONE_E; case 'i':return RELOQ_FA_PHONE_I;
    case 'o':return RELOQ_FA_PHONE_O; case 'u':return RELOQ_FA_PHONE_U;
    case 'p':return RELOQ_FA_PHONE_P; case 'b':return RELOQ_FA_PHONE_B;
    case 't':return RELOQ_FA_PHONE_T; case 'd':return RELOQ_FA_PHONE_D;
    case 'k':return RELOQ_FA_PHONE_K; case 'g':return RELOQ_FA_PHONE_G;
    case 'R':case 'q':return RELOQ_FA_PHONE_UVULAR;
    case 'w':case '?':return RELOQ_FA_PHONE_GLOTTAL_STOP;
    case 'f':return RELOQ_FA_PHONE_F; case 'v':return RELOQ_FA_PHONE_V;
    case 's':return RELOQ_FA_PHONE_S; case 'z':return RELOQ_FA_PHONE_Z;
    case 'S':return RELOQ_FA_PHONE_SH; case 'Z':return RELOQ_FA_PHONE_ZH;
    case 'G':case 'x':return RELOQ_FA_PHONE_KH; case 'h':return RELOQ_FA_PHONE_H;
    case 'C':return RELOQ_FA_PHONE_CH; case 'J':return RELOQ_FA_PHONE_J;
    case 'm':return RELOQ_FA_PHONE_M; case 'n':return RELOQ_FA_PHONE_N;
    case 'l':return RELOQ_FA_PHONE_L; case 'r':return RELOQ_FA_PHONE_R;
    case 'j':return RELOQ_FA_PHONE_Y;
    case 'O':return RELOQ_FA_PHONE_FRONT_ROUNDED_O;
    case 'y':return RELOQ_FA_PHONE_FRONT_ROUNDED_U;
    case 'I':return RELOQ_FA_PHONE_OFFGLIDE_I;
    case 'U':return RELOQ_FA_PHONE_OFFGLIDE_U;
    case 'Y':return RELOQ_FA_PHONE_OFFGLIDE_FRONT_U;
    default:return 0;
    }
}
static int nucleus(uint32_t id)
{return (id>=RELOQ_FA_PHONE_AE && id<=RELOQ_FA_PHONE_U) ||
        id==RELOQ_FA_PHONE_FRONT_ROUNDED_O || id==RELOQ_FA_PHONE_FRONT_ROUNDED_U;}
static int offglide(uint32_t id)
{return id>=RELOQ_FA_PHONE_OFFGLIDE_I && id<=RELOQ_FA_PHONE_OFFGLIDE_FRONT_U;}

/* Describe the approved timing independently of the temporary sound
 * backend. A long vowel retains its identity when its nucleus is shortened. */
static void mark_vowel_timing(struct plan *p,size_t first,size_t last)
{
    int before_vowel=0,stressed=0;
    unsigned later=0;
    struct reloq_fa_plan_event *initial=NULL;
    for(size_t i=first;i<last;++i) {
        struct reloq_fa_plan_event *phone=p->events+i;
        if(phone->kind!=RELOQ_FA_PLAN_PHONE || !nucleus(phone->value))continue;
        if(initial && before_vowel) {
            if(later<UINT_MAX)++later;
            if(phone->stress==1 || phone->stress==2)stressed=1;
        }
        if(phone->stress==0) {
            if(phone->value==RELOQ_FA_PHONE_I || phone->value==RELOQ_FA_PHONE_BACK_A) {
                int left=i>first && p->events[i-1].kind==RELOQ_FA_PLAN_PHONE &&
                         nucleus(p->events[i-1].value);
                int right=i+1<last && p->events[i+1].kind==RELOQ_FA_PLAN_PHONE &&
                          nucleus(p->events[i+1].value) && p->events[i+1].stress==1;
                unsigned kind=phone->value==RELOQ_FA_PHONE_I ? RELOQ_FA_HIATUS_I :
                    RELOQ_FA_HIATUS_SHORT_BACK_A;
                int hiatus=reloq_fa_hiatus_timing_percent(kind,
                    !!(phone->flags&RELOQ_FA_PHONE_LONG),left,right)<100;
                if(hiatus)phone->flags|=RELOQ_FA_PLAN_COMPACT_HIATUS;
                if(phone->value==RELOQ_FA_PHONE_I && (phone->flags&RELOQ_FA_PHONE_LONG) && hiatus)
                    phone->flags|=RELOQ_FA_PLAN_SHORT_NUCLEUS;
            }
            if(!before_vowel && (phone->flags&RELOQ_FA_PHONE_LONG) &&
               (phone->value==RELOQ_FA_PHONE_I || phone->value==RELOQ_FA_PHONE_AE ||
                phone->value==RELOQ_FA_PHONE_U)) {
                /* The approved /u/ before /t/ plus a voiced stop keeps
                 * its separate, less aggressive cluster timing. */
                int cluster=phone->value==RELOQ_FA_PHONE_U && i+2<last &&
                    p->events[i+1].kind==RELOQ_FA_PLAN_PHONE && p->events[i+1].value==RELOQ_FA_PHONE_T &&
                    p->events[i+2].kind==RELOQ_FA_PLAN_PHONE &&
                    (p->events[i+2].value==RELOQ_FA_PHONE_B || p->events[i+2].value==RELOQ_FA_PHONE_D ||
                     p->events[i+2].value==RELOQ_FA_PHONE_G);
                if(!cluster)initial=phone;
            }
        }
        before_vowel=1;
    }
    if(initial && (initial->value==RELOQ_FA_PHONE_I ?
       reloq_fa_short_initial_i_context(1,later,stressed) :
       reloq_fa_short_initial_other_context(1,later)))
        initial->flags|=RELOQ_FA_PLAN_SHORT_NUCLEUS;
}

/* Parse a complete explicit group without the old acoustic backend. Each
 * syllable has a nucleus. A final legacy H carrier is removed only when the
 * group came from the existing internal lexicon, never from user input. */
static int group(struct plan *p,const char *s,size_t n,int internal)
{
    if(n<4 || s[0]!='`' || s[1]!='[' || s[n-1]!=']')return 1;
    size_t mark=p->n,at=2,end=n-1,syllable_start=0,last_nucleus=0;
    uint32_t syl=0,stress=1;int began=0,segment_vowel=0;
    int result=push(p,RELOQ_FA_PLAN_WORD_BEGIN,0,0,0,0);
    if(result)return result;
    while(at<end) {
        if(s[at]=='.') {
            if(at+1>=end || s[at+1]<'0' || s[at+1]>'2' ||
               (began && (p->n==syllable_start || (internal && !segment_vowel))))
                {result=1;break;}
            if(began)++syl;
            stress=(uint32_t)(s[at+1]-'0');at+=2;began=1;segment_vowel=0;
            syllable_start=p->n;last_nucleus=0;continue;
        }
        if(!began){began=1;syllable_start=p->n;}
        uint32_t flags=0,id=0;
        if(s[at]=='\'') {
            if(at+3>=end || s[at+2]!=':' || s[at+3]!='\'' ||
               !nucleus(id=phone_id(s[at+1]))){result=1;break;}
            flags=RELOQ_FA_PHONE_LONG;at+=4;
        } else {
            id=phone_id(s[at++]);
            if(!id){result=1;break;}
        }
        if(offglide(id) && (!segment_vowel || last_nucleus+1!=p->n))
            {result=1;break;}
        if(nucleus(id)) {
            if(segment_vowel) {
                ++syl;stress=0;
                /* Consonants between nuclei start the following syllable. */
                size_t first=last_nucleus+1;
                for(size_t i=first;i<p->n;++i) {
                    if(offglide(p->events[i].value))continue;
                    p->events[i].syllable=syl;p->events[i].stress=stress;
                }
                syllable_start=first;
            }
            segment_vowel=1;last_nucleus=p->n;
        }
        if(internal && id==RELOQ_FA_PHONE_H && at+1==end && s[at]=='w')++at;
        result=push(p,RELOQ_FA_PLAN_PHONE,id,flags,syl,stress);
        if(result)break;
        if(offglide(id))last_nucleus=p->n-1;
    }
    if(!result && (!began || p->n==syllable_start || (internal && !segment_vowel)))result=1;
    if(!result) {
        mark_vowel_timing(p,mark+1,p->n);
        result=push(p,RELOQ_FA_PLAN_WORD_END,0,0,0,0);
    }
    if(result)p->n=mark;
    return result;
}

/* Decode the projected, internal lexicon data into the public event format.
 * This decoder never forwards a control or unknown text to another language. */
static int projected(struct plan *p,const char *s,size_t bytes,size_t **mapping)
{
    if(bytes==SIZE_MAX || bytes+1>SIZE_MAX/sizeof(size_t))return -2;
    size_t *map=calloc(bytes+1,sizeof *map);
    if(!map)return -2;
    map[0]=p->n;
    size_t at=0;int result=0;
    while(at<bytes && !result) {
        size_t first=at,before=p->n;
        if(s[at]=='`' && at+1<bytes && s[at+1]=='[') {
            at+=2;while(at<bytes && s[at]!=']')++at;
            if(at==bytes){result=1;break;}
            ++at;result=group(p,s+first,at-first,1);
        } else if(s[at]=='`' && at+2<bytes && s[at+1]=='0' && s[at+2]=='0') {
            at+=3;result=push(p,RELOQ_FA_PLAN_WEAK_WORD,0,0,0,0);
        } else if(s[at]=='~') {
            ++at;result=push(p,RELOQ_FA_PLAN_GLOTTAL_ONSET,0,0,0,0);
        } else {
            uint32_t cp=0;
            if(reloq_utf8_next((const unsigned char *)s,bytes,&at,&cp)!=1)
                {result=-1;break;}
            if(cp==' ' || cp=='\t' || cp=='\r') { /* no event */ }
            else if(cp=='\n' || (cp<128 && strchr(".,?!:;()[]{}-+/%",(int)cp)) ||
                    cp==0x060c || cp==0x061b || cp==0x061f)
                result=push(p,RELOQ_FA_PLAN_PUNCTUATION,cp,0,0,0);
            else result=1;
        }
        for(size_t i=first;i<at;++i)map[i]=before;
        map[at]=p->n;
    }
    if(result){free(map);return result;}
    *mapping=map;return 0;
}

static int segment(struct plan *p,const char *text,size_t bytes,int variant,int kind,
                   size_t *source_map,size_t *error)
{
    if(!bytes){source_map[0]=p->n;return 0;}
    size_t before=p->n;int result=0;char *tagged=NULL;size_t *input_map=NULL,*output_map=NULL;
    if(kind==2) {
        result=group(p,text,bytes,0);
        if(!result) {
            for(size_t i=0;i<bytes;++i)source_map[i]=before;
            source_map[bytes]=p->n;return 0;
        }
        if(result<0)return result;
        /* An invalid phonetic group is spelled in Persian, including its
         * delimiters; it is never reinterpreted as ordinary foreign text. */
        result=reloq_fa_spell_for_plan(text,bytes,&tagged,&input_map,error);
    } else if(kind==1) {
        result=reloq_fa_spell_for_plan(text,bytes,&tagged,&input_map,error);
    } else {
        if(bytes==SIZE_MAX || bytes+1>SIZE_MAX/sizeof(size_t))return -2;
        size_t num=bytes+1,ends[1]={bytes};unsigned char modes[1]={0};
        size_t *positions=malloc(num*sizeof *positions);
        unsigned char *indexes=malloc(num);
        input_map=malloc(num*sizeof *input_map);
        if(!positions || !indexes || !input_map) {
            free(positions);free(indexes);free(input_map);return -2;
        }
        for(size_t i=0;i<num;++i)positions[i]=i;
        memset(indexes,1,num);
        result=reloq_fa_project_runs(text,bytes,variant,ends,modes,1,
            positions,indexes,num,&tagged,input_map,error);
        if(result==1 && variant) {
            int punctuation_only=1;
            for(size_t i=0;i<bytes;++i)
                if(!strchr(" \t\r\n.,?!:;()[]{}-+/%",text[i])){punctuation_only=0;break;}
            if(punctuation_only)result=reloq_fa_project_runs(text,bytes,0,ends,modes,1,
                positions,indexes,num,&tagged,input_map,error);
        }
        free(positions);free(indexes);
    }
    if(!result) {
        size_t length=strlen(tagged);
        result=projected(p,tagged,length,&output_map);
        if(!result)for(size_t i=0;i<=bytes;++i) {
            if(input_map[i]>length){result=-1;break;}
            source_map[i]=output_map[input_map[i]];
        }
    }
    if(result)p->n=before;
    free(tagged);free(input_map);free(output_map);return result;
}

/* Read a numbered ECI language command. An unrelated command is not
 * consumed; an unsupported dialect is reported to the host, not guessed. */
static int selector(const char *s,size_t bytes,size_t *consumed,int *variant)
{
    if(bytes<3 || s[0]!='`' || s[1]!='l' || s[2]<'0' || s[2]>'9')return 0;
    size_t at=2;uint32_t major=0,minor=0;
    while(at<bytes && s[at]>='0' && s[at]<='9') {
        unsigned d=(unsigned)(s[at++]-'0');
        if(major>(65535u-d)/10u)return -1;
        major=major*10u+d;
    }
    if(at+1<bytes && s[at]=='.' && s[at+1]>='0' && s[at+1]<='9') {
        ++at;
        while(at<bytes && s[at]>='0' && s[at]<='9') {
            unsigned d=(unsigned)(s[at++]-'0');
            if(minor>(65535u-d)/10u)return -1;
            minor=minor*10u+d;
        }
    }
    if(major!=(RELOQ_FA_DIALECT_PERSIAN>>16) || minor>1)return -1;
    *consumed=at;*variant=(int)minor;return 1;
}

static int ascii_letter(unsigned char c)
{return (c>='A' && c<='Z') || (c>='a' && c<='z');}

/* Latin words in Persian text belong to English; in Pinglish they stay in
 * the Persian analyzer whenever its native word rules accept them. Keep the
 * source slice rather than attempting to pronounce English here. */
static int english_run(const char *text,size_t at,size_t end,int variant,size_t *last)
{
    if(at>=end || !ascii_letter((unsigned char)text[at]))return 0;
    size_t p=at;
    while(p<end && ascii_letter((unsigned char)text[p]))++p;
    size_t letters=p-at;
    int numeric=0;
    while(p<end && ((text[p]>='0' && text[p]<='9') ||
          (text[p]=='.' && p+1<end && text[p+1]>='0' && text[p+1]<='9'))) {
        numeric=1;++p;
    }
    *last=p;
    if(variant && !numeric) {
        if(letters==1)return 0; /* the shared Pinglish letter-name rule */
        char phone[8193];
        int found=reloq_fa_pinglish_word(text+at,letters,phone,sizeof phone);
        if(found<0)return found;
        if(found==1)return 0;
    }
    return 1;
}

int reloq_fa_analyze_runs(const char *text,size_t bytes,int mode,
    const size_t *run_ends,const unsigned char *run_modes,size_t run_count,
    const size_t *source_offsets,const unsigned char *is_index,size_t event_count,
    struct reloq_fa_plan_event **events,size_t *count,size_t *target_events,size_t *error_offset)
{
    if(events)*events=NULL;
    if(count)*count=0;
    if(error_offset)*error_offset=0;
    if(!text || !events || !count || !error_offset || (mode!=0 && mode!=1) ||
       (run_count && (!run_ends || !run_modes)) || (!!run_count != !!bytes) ||
       (event_count && (!source_offsets || !is_index || !target_events)) ||
       bytes==SIZE_MAX || bytes+1>SIZE_MAX/sizeof(size_t) ||
       event_count>SIZE_MAX/sizeof(size_t) || memchr(text,0,bytes))return -1;
    size_t previous=0;
    for(size_t i=0;i<run_count;++i) {
        size_t end=run_ends[i];
        if(end<=previous || end>bytes || run_modes[i]>2 ||
           (end<bytes && ((unsigned char)text[end]&0xc0)==0x80))return -1;
        previous=end;
    }
    if(previous!=bytes)return -1;
    size_t *map=calloc(bytes+1,sizeof *map);
    size_t *mapped=event_count?malloc(event_count*sizeof *mapped):NULL;
    if(!map || (event_count&&!mapped)){free(map);free(mapped);return -2;}
    struct plan p={0};int result=0;size_t start=0;
    for(size_t r=0;!result && r<run_count;) {
        size_t end=run_ends[r];int kind=run_modes[r++];
        while(kind!=2 && r<run_count && run_modes[r]==kind)end=run_ends[r++];
        if(kind!=0) {
            size_t err=0;result=segment(&p,text+start,end-start,mode,kind,map+start,&err);
            if(result)*error_offset=start+err;
        } else {
            size_t from=start,at=start;
            while(at<end && !result) {
                if(text[at]=='`' && at+1<end && text[at+1]=='[') {
                    size_t err=0,closing=at+2;
                    while(closing<end && text[closing]!=']' &&
                          text[closing]!=' ' && text[closing]!='\t' && text[closing]!='\n')++closing;
                    if(closing<end && text[closing]==']')++closing;
                    result=segment(&p,text+from,at-from,mode,0,map+from,&err);
                    if(result){*error_offset=from+err;break;}
                    result=segment(&p,text+at,closing-at,mode,2,map+at,&err);
                    if(result){*error_offset=at+err;break;}
                    at=closing;from=at;continue;
                }
                size_t used=0;int next=mode;
                int parsed=selector(text+at,end-at,&used,&next);
                if(!parsed) {
                    size_t last=at;
                    int foreign=english_run(text,at,end,mode,&last);
                    if(foreign<0){*error_offset=at;result=foreign;break;}
                    if(!foreign){at=last>at?last:at+1;continue;}
                    size_t err=0;
                    result=segment(&p,text+from,at-from,mode,0,map+from,&err);
                    if(result){*error_offset=from+err;break;}
                    if(at>UINT32_MAX || last-at>UINT32_MAX){result=-1;*error_offset=at;break;}
                    size_t before=p.n;
                    result=push(&p,RELOQ_FA_PLAN_ENGLISH_TEXT,(uint32_t)at,(uint32_t)(last-at),0,0);
                    for(size_t j=at;j<last;++j)map[j]=before;
                    at=last;map[at]=p.n;from=at;continue;
                }
                if(parsed<0){*error_offset=at;result=1;break;}
                size_t err=0;
                result=segment(&p,text+from,at-from,mode,0,map+from,&err);
                if(result){*error_offset=from+err;break;}
                size_t before=p.n;
                result=push(&p,RELOQ_FA_PLAN_INPUT_VARIANT,(uint32_t)next,0,0,0);
                for(size_t j=at;j<at+used;++j)map[j]=before;
                at+=used;map[at]=p.n;from=at;mode=next;
            }
            if(!result) {
                size_t err=0;result=segment(&p,text+from,end-from,mode,0,map+from,&err);
                if(result)*error_offset=from+err;
            }
        }
        start=end;
    }
    previous=0;
    for(size_t i=0;!result && i<event_count;++i) {
        size_t at=source_offsets[i];
        if(at>bytes || (i && at<previous) || is_index[i]>1){result=3;break;}
        if(!is_index[i] && at>0 && at<bytes && map[at]==map[at-1]) {
            *error_offset=at;result=3;break;
        }
        mapped[i]=map[at];previous=at;
    }
    if(!result) {
        if(event_count)memcpy(target_events,mapped,event_count*sizeof *mapped);
        *events=p.events;*count=p.n;p.events=NULL;
    }
    free(p.events);free(map);free(mapped);return result;
}
