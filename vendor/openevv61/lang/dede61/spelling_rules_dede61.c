#include <stdlib.h>
/* German phonetic spelling, directly from segment features. The native
 * phone stream remains authoritative; no word result is retained. */
#include "reloq61_spr_output.h"
#include "reloq61_scalar_state.h"

#include "reloq61.h"
static _Thread_local struct reloq61_german_capture *german_capture;
/* Current word traversal only; reset at every native word boundary. Number
 * liaison belongs to this shared German phone projection, including numbers
 * inside identifiers, rather than an outer text adapter. */
static _Thread_local struct {unsigned previous,before;int32_t move_r;} number_liaison;
struct reloq61_german_capture *reloq61_german_capture_scope(struct reloq61_german_capture *next)
{
    struct reloq61_german_capture *previous=german_capture;
    german_capture=next;return previous;
}
/* The native phrase-insertion rule supplies its selected phrase-boundary point.
 * Compare it with the same word locations used by the native SPR writer. No
 * speculative projection, stream mutation or text heuristic is needed. */
void reloq61_german_predicted_phrase(void *state,int32_t next_word,unsigned type)
{
    if(!german_capture||!german_capture->phrases||!next_word||type>=9)return;
    /* Resolve the native insertion boundary once, before SPR traversal.
     * Its right word point is then identical to the writer's next word. */
    int32_t key=vgetsc(state,0,0,next_word,4);
    if(key)next_word=key;
    if(german_capture->phrase_count==german_capture->phrase_capacity){german_capture->overflow=1;return;}
    german_capture->phrases_sorted=0;
    german_capture->phrases[german_capture->phrase_count++]=(struct reloq61_german_phrase){next_word,(unsigned char)type};
}
static int phrase_order(const void *left,const void *right)
{
    const struct reloq61_german_phrase *a=left,*b=right;
    return (a->next_word>b->next_word)-(a->next_word<b->next_word);
}
void reloq61_german_word_begin(void *state,int32_t location)
{
    memset(&number_liaison,0,sizeof number_liaison);
    if(!german_capture)return;
    if(german_capture->count>=german_capture->capacity){german_capture->overflow=1;return;}
    struct reloq61_german_word_facts *word=german_capture->words+german_capture->count++;
    *word=(struct reloq61_german_word_facts){0};
    int32_t at=location?EVV_AT(delta_loc *,location)->value:0;
    (void)state;
    if(!german_capture->phrases_sorted) {
        if(german_capture->phrase_count>1)
            qsort(german_capture->phrases,german_capture->phrase_count,
                sizeof *german_capture->phrases,phrase_order);
        german_capture->phrases_sorted=1;
    }
    size_t lo=0,hi=german_capture->phrase_count;
    while(lo<hi) {
        size_t mid=lo+(hi-lo)/2;
        if(german_capture->phrases[mid].next_word<at)lo=mid+1;else hi=mid;
    }
    if(lo<german_capture->phrase_count && german_capture->phrases[lo].next_word==at)
        word->phrase_before=(unsigned char)(german_capture->phrases[lo].type+1);
}
void reloq61_german_syllable_begin(void)
{
    if(!german_capture||!german_capture->count||german_capture->overflow)return;
    struct reloq61_german_word_facts *word=german_capture->words+german_capture->count-1;
    if(word->syllables>=64){german_capture->overflow=1;return;}
    ++word->syllables;
}
static void note_onset(unsigned after)
{
    if(after!=2||!german_capture||!german_capture->count||german_capture->overflow)return;
    struct reloq61_german_word_facts *word=german_capture->words+german_capture->count-1;
    if(word->syllables)word->glottal_syllables|=UINT64_C(1)<<(word->syllables-1);
}

int32_t reloq_dede61_print_ger_SPR(void *state,int32_t p0,int32_t p1,int32_t p2)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;
    if(!p1||!p2)return reloq_scalar_finish(d,94);
    int32_t begin=EVV_AT(delta_loc *,p1)->value,consumed=0;
    struct reloq61_spr_phone first,second;
    const char *spelling=NULL;
    if(begin && (*EVV_AT(int32_t *,begin+(EVV_AT(delta_vars *,d->vars)->fence_base+2)*4)&1) &&
       reloq61_spr_phone_at(d,begin,&first)) {
        note_onset(reloq61_spr_feature(d,&first,15));
        if(reloq61_spr_feature(d,&first,15)==1)print_lit(d,8,"h");
        unsigned combination=reloq61_spr_feature(d,&first,14);
        static const char *const combined[]={NULL,"P","T","C","J","'oj'","'aw'","'aj'"};
        if(combination && combination<sizeof combined/sizeof *combined &&
           reloq61_spr_phone_at(d,first.after,&second)) {
            spelling=combined[combination];consumed=second.after;
        }
        if(!spelling)switch(reloq61_spr_feature(d,&first,0)) {
        case 32:spelling="'E:'";break;
        case 41:spelling="'oe'";break;
        case 42:spelling="'OE'";break;
        case 47:spelling="'a~'";break;
        case 48:spelling="'E~'";break;
        case 49:spelling="'o~'";break;
        case 50:spelling="'oe~'";break;
        case 25:if(reloq61_spr_feature(d,&first,1)==2)spelling="r";break;
        }
        unsigned phone=reloq61_spr_feature(d,&first,0);
        if(first.node==number_liaison.move_r) {
            spelling="rU";number_liaison.move_r=0;
        } else if((phone==15||phone==25) && number_liaison.before==7 &&
                  number_liaison.previous==28) {
            struct reloq61_spr_phone u,n,t;
            if(reloq61_spr_phone_at(d,first.after,&u)&&reloq61_spr_feature(d,&u,0)==36&&
               reloq61_spr_phone_at(d,u.after,&n)&&reloq61_spr_feature(d,&n,0)==22&&
               reloq61_spr_phone_at(d,n.after,&t)&&reloq61_spr_feature(d,&t,0)==3) {
                spelling="";number_liaison.move_r=u.node;
            }
        }
        number_liaison.before=number_liaison.previous;
        number_liaison.previous=phone;
    }
    return reloq61_spr_write(d,begin,p2,spelling,consumed);
}
int32_t reloq_dede61_lang_specific_SPR(void *state,int32_t p0,int32_t p1,int32_t p2)
{
    if(!reloq_scalar_begin(state))return 94;
    int16_t mode,phonetic;
    memcpy(&mode,(unsigned char *)state+DG_BASE+3374,2);
    memcpy(&phonetic,(unsigned char *)state+DG_BASE+3358,2);
    return mode==phonetic?reloq_dede61_print_ger_SPR(state,p0,p1,p2):reloq_scalar_finish(state,0);
}
