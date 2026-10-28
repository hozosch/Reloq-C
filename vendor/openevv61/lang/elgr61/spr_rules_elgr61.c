/* A Greek phoneme word ends at its closing bracket. The surrounding
   input records retain their original adjacency, so character names,
   apostrophe elision and punctuation use the native context rules. */
#include "delta.h"
int reloq_elgr61_spr_word_end(void *state,int32_t token_ref)
{
    delta_state *d=state;
    if(!d||!d->vars||!token_ref)return 0;
    const delta_loc *token=EVV_AT(const delta_loc *,token_ref);
    if(token->kind!=DK_SYNC||!token->value)return 0;
    delta_node *at=vmovel(d,EVV_AT(delta_node *,token->value),1);
    int32_t record=vgetsc(d,1,0,EVV_REF(at),1);
    if(!record||(*EVV_AT(const int32_t *,record)&2))return 0;
    const unsigned char *name=delta_record_field(&vstmtbl[1],0,
        TFLDS(EVV_AT(void *,record)));
    return *name==81; /* native input code for ] */
}
