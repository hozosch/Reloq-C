/* A proposed inner split must stay within the constituent being analysed. */
#ifndef RELOQ61_GERMAN_COMPOUND_BOUNDS_H
#define RELOQ61_GERMAN_COMPOUND_BOUNDS_H
#include "delta.h"
#include <string.h>

static inline int reloq61_german_split_past_end(void *state, int32_t start,
                                                int32_t split, int32_t end)
{
    delta_state *d=state;int32_t current_start,current_end;
    if(!d || !d->vars || !start || !split || !end || deltaErrorThrown(d))return 0;
    const unsigned char *fields=(const unsigned char *)state+DG_BASE;
    memcpy(&current_start,fields+584,4);memcpy(&current_end,fields+592,4);
    if(start!=current_start || end!=current_end)return 0;

    /* Follow the actual input projection; arena references are not ordered
     * addresses. Reject only when the end is reached before the split.
     * If the projection cannot be traversed, leave the existing rule alone. */
    int32_t at=start;
    for(unsigned steps=0;steps<256;++steps) {
        if(at==split)return 0;
        if(at==end)return 1;
        int32_t node=vgetsc(d,0,0,at,1);
        if(!node)return 0;
        int32_t next=(*EVV_AT(const int32_t *,node)&2)?node:
            (*EVV_AT(const int32_t *,node+4)&~3);
        if(!next || next==at)return 0;
        at=next;
    }
    return 0;
}

/* A successful compound search must actually cover the constituent. When
 * the first projected item is already its end, no morpheme was created;
 * the caller can use its ordinary whole-root fallback instead. */
static inline int reloq61_german_compound_empty(void *state, int32_t start,
                                                int32_t end)
{
    delta_state *d=state;
    if(!d || !d->vars || !start || !end || start==end || deltaErrorThrown(d))
        return 0;
    return vgetsc(d,0,0,start,3)==end;
}
#endif
