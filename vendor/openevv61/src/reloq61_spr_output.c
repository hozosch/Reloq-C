/* Phone spelling is an output operation. It needs neither speculative
 * language tests nor a mutation/undo scope. Only the caller's requested
 * end position changes when a combined phone consumes a second segment. */
#include "reloq61_spr_output.h"
#include "reloq61_scalar_state.h"

int reloq61_spr_phone_at(delta_state *d,int32_t at,struct reloq61_spr_phone *p)
{
    if(!at)return 0;
    int32_t node=vgetsc(d,0,0,at,2);
    while(node && (*EVV_AT(int32_t *,node)&2))node=vgetsc(d,0,0,node,2);
    if(!node)return 0;
    *p=(struct reloq61_spr_phone){node,*EVV_AT(int32_t *,node+4)&~3};
    return p->after!=0;
}
unsigned reloq61_spr_feature(delta_state *d,const struct reloq61_spr_phone *p,unsigned field)
{(void)d;return *(const unsigned char *)delta_record_field(&vstmtbl[2], field, TFLDS(EVV_AT(void *,p->node)));}

int32_t reloq61_spr_write(delta_state *d,int32_t begin,int32_t end_arg,
    const char *spelling,int32_t consumed)
{
    delta_loc *end=EVV_AT(delta_loc *,end_arg);
    if(spelling)print_lit(d,8,spelling);
    else vprt_strm(d,8,begin,end->value,2,0,"");
    if(consumed)end->value=consumed;
    return reloq_scalar_finish(d,0);
}
