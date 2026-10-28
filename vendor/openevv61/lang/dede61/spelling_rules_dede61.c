/* German phonetic spelling, directly from segment features. The native
 * phone stream remains authoritative; no word result is retained. */
#include "reloq61_spr_output.h"
#include "reloq61_scalar_state.h"

#include "reloq61.h"
static _Thread_local struct reloq61_german_capture *german_capture;
struct reloq61_german_capture *reloq61_german_capture_scope(struct reloq61_german_capture *next)
{
    struct reloq61_german_capture *previous=german_capture;
    german_capture=next;return previous;
}
void reloq61_german_word_begin(void)
{
    if(!german_capture)return;
    if(german_capture->count>=german_capture->capacity){german_capture->overflow=1;return;}
    german_capture->words[german_capture->count++]=(struct reloq61_german_word_facts){0};
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
