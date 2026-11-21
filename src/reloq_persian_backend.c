/* Temporary acoustic bridge for the independent Persian event plan.
 * No NVDA, eSpeak, global process state or platform-specific code. */
#include "reloq_persian_backend.h"
#include "reloq_persian_phones.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct buffer { char *data; size_t used,capacity; };

static int append(struct buffer *b,const char *s,size_t n)
{
    if(n>SIZE_MAX-b->used-1)return -2;
    size_t need=b->used+n+1;
    if(need>b->capacity) {
        size_t cap=b->capacity?b->capacity:128;
        while(cap<need)cap=cap>SIZE_MAX/2?need:cap*2;
        void *next=realloc(b->data,cap);
        if(!next)return -2;
        b->data=next;b->capacity=cap;
    }
    memcpy(b->data+b->used,s,n);b->used+=n;b->data[b->used]=0;
    return 0;
}
static int gap(struct buffer *b)
{
    if(!b->used || b->data[b->used-1]==' ' || b->data[b->used-1]=='~')return 0;
    return append(b," ",1);
}
static char symbol(uint32_t id)
{
    static const char phones[]="AaeioupbtdkgRwfvszSZGhCJmnlrj";
    if(id>=1 && id<=RELOQ_FA_PHONE_COUNT)return phones[id-1];
    switch(id) {
    case RELOQ_FA_PHONE_FRONT_ROUNDED_O:return 'O';
    case RELOQ_FA_PHONE_FRONT_ROUNDED_U:return 'y';
    case RELOQ_FA_PHONE_OFFGLIDE_I:return 'I';
    case RELOQ_FA_PHONE_OFFGLIDE_U:return 'U';
    case RELOQ_FA_PHONE_OFFGLIDE_FRONT_U:return 'Y';
    default:return 0;
    }
}
static int vowel(uint32_t id)
{
    return (id>=RELOQ_FA_PHONE_AE && id<=RELOQ_FA_PHONE_U) ||
           id==RELOQ_FA_PHONE_FRONT_ROUNDED_O || id==RELOQ_FA_PHONE_FRONT_ROUNDED_U;
}
static char punctuation(uint32_t cp)
{
    if(cp==0x060c)return ',';
    if(cp==0x061b)return ';';
    if(cp==0x061f)return '?';
    if(cp=='\n' || (cp<128 && strchr(".,?!:;()[]{}-+/%",(int)cp)))return (char)cp;
    return 0;
}

void reloq_fa_backend_clear(struct reloq_fa_backend_plan *p)
{
    if(!p)return;
    free(p->text);free(p->offsets);free(p->onsets);free(p->geminates);
    *p=(struct reloq_fa_backend_plan){0};
}

static int add_mark(uint32_t **marks,size_t *count,uint32_t value)
{
    if(*count>=32767 || *count+1>SIZE_MAX/sizeof **marks)return -2;
    void *next=realloc(*marks,(*count+1)*sizeof **marks);
    if(!next)return -2;
    *marks=next;(*marks)[(*count)++]=value;return 0;
}

int reloq_fa_backend_prepare(const char *source,size_t source_bytes,
    const struct reloq_fa_plan_event *events,size_t count,
    struct reloq_fa_backend_plan *result,size_t *error_event)
{
    if(error_event)*error_event=0;
    if(!result || !error_event || (!source && source_bytes) || (!events && count) ||
       count==SIZE_MAX || count+1>SIZE_MAX/sizeof(size_t))return -1;
    *result=(struct reloq_fa_backend_plan){0};
    size_t *offsets=malloc((count+1)*sizeof *offsets);
    if(!offsets)return -2;
    struct buffer b={0};int status=append(&b,"",0),in_word=0;
    uint32_t syllable=UINT32_MAX,stress=0,previous_phone=0;
    uint32_t word=0,j_ordinal=0,*onsets=NULL,*geminates=NULL;
    size_t word_phones=0,onset_count=0,geminate_count=0;
    int ambiguous_word=0,skip_pair=0,english=0,explicit_input=0;
    for(size_t i=0;i<count;++i)if(events[i].kind==RELOQ_FA_PLAN_ENGLISH_TEXT)english=1;
    for(size_t i=0;i+1<source_bytes;++i)
        if(source[i]=='`' && source[i+1]=='['){explicit_input=1;break;}
    for(size_t i=0;!status && i<count;++i) {
        const struct reloq_fa_plan_event *e=events+i;
        offsets[i]=b.used;
        switch(e->kind) {
        case RELOQ_FA_PLAN_WORD_BEGIN:
            if(in_word){status=1;break;}
            if((status=gap(&b)) || (status=append(&b,"`[",2)))break;
            if(++word>32767){status=1;break;}
            in_word=1;syllable=UINT32_MAX;previous_phone=0;word_phones=0;
            j_ordinal=0;skip_pair=0;ambiguous_word=0;
            for(size_t j=i+1;j+1<count && events[j].kind==RELOQ_FA_PLAN_PHONE;++j)
                if(events[j].value==RELOQ_FA_PHONE_D && events[j+1].kind==RELOQ_FA_PLAN_PHONE &&
                   events[j+1].value==RELOQ_FA_PHONE_ZH)ambiguous_word=1;
            break;
        case RELOQ_FA_PLAN_PHONE: {
            char c=symbol(e->value);
            if(!in_word || !c || e->stress>2 ||
               (e->flags & ~(RELOQ_FA_PHONE_LONG|RELOQ_FA_PLAN_SHORT_NUCLEUS|
                             RELOQ_FA_PLAN_COMPACT_HIATUS)) ||
               ((e->flags & RELOQ_FA_PHONE_LONG) && !vowel(e->value))) {status=1;break;}
            if(e->syllable!=syllable) {
                if((syllable==UINT32_MAX && e->syllable!=0) ||
                   (syllable!=UINT32_MAX && e->syllable!=syllable+1)) {status=1;break;}
                char marker[2]={'.',(char)('0'+e->stress)};
                if((status=append(&b,marker,2)))break;
                syllable=e->syllable;stress=e->stress;
            } else if(e->stress!=stress){status=1;break;}
            int doubled=e->value==RELOQ_FA_PHONE_J && !explicit_input && !ambiguous_word && !skip_pair &&
                i+1<count && events[i+1].kind==RELOQ_FA_PLAN_PHONE &&
                events[i+1].value==RELOQ_FA_PHONE_J;
            if(doubled) {
                if(english || j_ordinal>=65535){status=1;break;}
                status=add_mark(&geminates,&geminate_count,(word<<16)|(j_ordinal+1));
                skip_pair=1;previous_phone=e->value;++word_phones;break;
            }
            if(e->value==RELOQ_FA_PHONE_J){++j_ordinal;skip_pair=0;}
            if(e->flags & RELOQ_FA_PHONE_LONG) {
                char long_phone[4]={'\'',c,':','\''};status=append(&b,long_phone,4);
            } else status=append(&b,&c,1);
            previous_phone=e->value;++word_phones;break;
        }
        case RELOQ_FA_PLAN_WORD_END:
            if(!in_word || !word_phones){status=1;break;}
            if(previous_phone==RELOQ_FA_PHONE_H && (status=append(&b,"w",1)))break;
            status=append(&b,"]",1);in_word=0;break;
        case RELOQ_FA_PLAN_PUNCTUATION: {
            char c=punctuation(e->value);
            if(in_word || !c){status=1;break;}
            status=append(&b,&c,1);break;
        }
        case RELOQ_FA_PLAN_WEAK_WORD:
            if(in_word){status=1;break;}
            if((status=gap(&b)))break;
            status=append(&b,"`00 ",4);break;
        case RELOQ_FA_PLAN_GLOTTAL_ONSET:
            if(in_word || english || !b.used || b.data[b.used-1]!=']' || word>=32767)
                {status=1;break;}
            if((status=add_mark(&onsets,&onset_count,word+1)))break;
            status=append(&b," ",1);break;
        case RELOQ_FA_PLAN_INPUT_VARIANT:
            if(in_word || e->value>1)status=1;
            break;
        case RELOQ_FA_PLAN_ENGLISH_TEXT: {
            size_t from=e->value,bytes=e->flags;
            if(in_word || !bytes || from>source_bytes || bytes>source_bytes-from ||
               memchr(source+from,0,bytes)){status=1;break;}
            if((status=gap(&b)) || (status=append(&b,"`l1.0 ",6)) ||
               (status=append(&b,source+from,bytes)) ||
               (status=append(&b," `l9.0 ",7)))break;
            break;
        }
        default:status=1;break;
        }
        if(status)*error_event=i;
    }
    if(!status && in_word){status=1;*error_event=count;}
    if(status){free(b.data);free(offsets);free(onsets);free(geminates);return status;}
    offsets[count]=b.used;
    *result=(struct reloq_fa_backend_plan){b.data,b.used,offsets,
        onsets,geminates,onset_count,geminate_count};return 0;
}
