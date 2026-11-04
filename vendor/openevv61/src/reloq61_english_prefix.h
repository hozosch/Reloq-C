/* Do not strip the only vowel of a remaining English root as a medial
 * prefix. That leaves a consonant-only stem and a spurious final syllable. */
#ifndef RELOQ61_ENGLISH_PREFIX_H
#define RELOQ61_ENGLISH_PREFIX_H
#include "delta.h"
#include <string.h>

static inline int reloq61_english_medial_needs_vowel(void *state)
{
    delta_state *d=state;
    int32_t word,start,end;
    if(!d || !d->vars || deltaErrorThrown(d))return 0;
    const unsigned char *fields=(const unsigned char *)state+DG_BASE;
    memcpy(&word,fields+584,4);  /* w44: word start */
    memcpy(&start,fields+744,4); /* w64: remaining root start */
    memcpy(&end,fields+752,4);   /* w65: remaining root end */
    if(!word || !start || !end || start==word)return 0;
    int32_t at=start;unsigned letters=0,vowels=0;
    while(at!=end && letters<256) {
        int32_t node=vgetsc(d,0,0,at,1);
        if(!node || (*EVV_AT(const int32_t *,node)&2))return 0;
        void *input=TFLDS(EVV_AT(void *,node));
        unsigned category=*(const unsigned char *)delta_record_field(&vstmtbl[1],4,input);
        if(category<1 || category>3)return 0;
        if(category==1 || category==3)++vowels;
        int32_t next=*EVV_AT(const int32_t *,node+4)&~3;
        if(!next || next==at)return 0;
        at=next;++letters;
    }
    return at==end && letters>=3 && vowels==1;
}

/* A T-rule may collapse a final spelling cluster into one consonant. If it
 * would also consume the word's only vowel, leave that final input letter
 * for the ordinary next diaphone rule. Return the old end for backtracking. */
static inline int32_t reloq61_english_reserve_final_vowel(void *state)
{
    delta_state *d=state;int32_t word,end,phone_start,phone_end;
    if(!d || !d->vars || deltaErrorThrown(d))return 0;
    unsigned char *fields=(unsigned char *)state+DG_BASE;
    memcpy(&word,fields+584,4);memcpy(&end,fields+592,4);
    memcpy(&phone_start,fields+712,4);memcpy(&phone_end,fields+720,4);
    if(!word || !end || !phone_start || phone_end!=end ||
       d->lpta.node!=phone_start || d->rpta.node!=phone_end)return 0;
    int32_t at=word,last=0;unsigned vowels=0,letters=0;int in_phone=0;
    while(at!=end && letters<256) {
        int32_t node=vgetsc(d,0,0,at,1);
        if(!node || (*EVV_AT(const int32_t *,node)&2))return 0;
        void *input=TFLDS(EVV_AT(void *,node));
        unsigned category=*(const unsigned char *)delta_record_field(&vstmtbl[1],4,input);
        if(category<1 || category>3)return 0;
        if(at==phone_start)in_phone=1;
        int32_t next=*EVV_AT(const int32_t *,node+4)&~3;
        if(!next || next==at)return 0;
        if(category==1 || category==3) {
            ++vowels;
            if(in_phone && at!=phone_start && next==end)last=at;
        }
        at=next;++letters;
    }
    if(at!=end || vowels!=1 || !last)return 0;
    memcpy(fields+720,&last,4);
    return phone_end;
}
#endif
