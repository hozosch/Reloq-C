/* Do not carry whole-word da-particle stress into a compound constituent
 * whose own morphology starts with a root rather than a da prefix. */
#ifndef RELOQ61_GERMAN_COMPOUND_STRESS_H
#define RELOQ61_GERMAN_COMPOUND_STRESS_H
#include "delta.h"
#include <string.h>

static inline int reloq61_german_da_is_partial_root(void *state)
{
    delta_state *d=state;int32_t start,end;
    if(!d || !d->vars || deltaErrorThrown(d))return 0;
    const unsigned char *fields=(const unsigned char *)state+DG_BASE;
    memcpy(&start,fields+584,4);memcpy(&end,fields+592,4);
    if(!start || !end)return 0;
    int32_t word=vgetsc(d,0,0,start,4);
    if(!word || (*EVV_AT(const int32_t *,word)&2) ||
       (*EVV_AT(const int32_t *,word+4)&~3)==end)return 0;
    int32_t morph=vgetsc(d,0,0,start,3);
    if(!morph || (*EVV_AT(const int32_t *,morph)&2))return 0;
    void *record=TFLDS(EVV_AT(void *,morph));
    return *(const unsigned char *)delta_record_field(&vstmtbl[3],0,record)==3;
}
#endif
