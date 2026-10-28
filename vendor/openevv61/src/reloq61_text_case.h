/* Typed C boundary for case normalization in the existing text projection. */
#ifndef RELOQ61_TEXT_CASE_H
#define RELOQ61_TEXT_CASE_H
#include "reloq61_scalar_state.h"
#include "reloq61_text_case.inc"

static inline int32_t reloq61_text_lowercase(void *state,int32_t first,int32_t last,unsigned language)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;
    if(!first||!last)return reloq_scalar_finish(d,94);
    const delta_loc *from=EVV_AT(const delta_loc *,first),*to=EVV_AT(const delta_loc *,last);
    if(from->kind!=DK_SYNC||to->kind!=DK_SYNC||!from->value||!to->value)
        return reloq_scalar_finish(d,94);
    int32_t at=EVV_REF(vmover(d,EVV_AT(int32_t *,from->value),1));
    int32_t end=EVV_REF(vmover(d,EVV_AT(int32_t *,to->value),1));
    for(size_t steps=0;at!=end && steps<200000;++steps) {
        int32_t node=vgetsc(d,0,0,at,1);
        if(!node)break;
        if(*EVV_AT(int32_t *,node)&2){at=node;continue;}
        void *fields=TFLDS(EVV_AT(void *,node));
        /* Only uppercase letter records change. Other input features and
         * intervening lowercase letters remain intact. */
        if(*(unsigned char *)delta_record_field(&vstmtbl[1], 3, fields)==1&&
           *(unsigned char *)delta_record_field(&vstmtbl[1], 1, fields)==2) {
            unsigned name=*(unsigned char *)delta_record_field(&vstmtbl[1], 0, fields);
            unsigned char lower=input_lowercase[language][name];
            if(lower!=name) {
                vstmtbl[1].put[0](fields,&lower);
                EVV_AT(delta_owner *,d->owner)->changed=1;
            }
        }
        int32_t after=*EVV_AT(int32_t *,node+4)&~3;
        if(!after)return reloq_scalar_finish(d,94);
        at=EVV_REF(vmover(d,EVV_AT(int32_t *,after),1));
    }
    return reloq_scalar_finish(d,0);
}
#endif
