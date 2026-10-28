/* Shared, direct C acoustic queue submission. */
#include <stdint.h>
extern int32_t reloq61_append_acoustic_interval(void *,int32_t,int32_t,int32_t,int32_t);
int32_t reloq_enus61_addStreamArraySsVal(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4)
{(void)p0;return reloq61_append_acoustic_interval(state,p1,p2,p3,p4);}

#include "reloq61_scalar_state.h"
#include "reloq61_acoustic_output.h"
#include "reloq_voice_filter.h"

/* The active typed model can change through inline controls. Reading its
 * values for this endpoint preserves that behavior without retaining any
 * model-dependent output between requests. */
int16_t reloq_enus61_filter_endpoint(void *state,int16_t stream,int16_t value)
{
    const unsigned char *fields=(const unsigned char *)state+DG_BASE;
    int16_t rate,gender,head,scale,f4a,f4b,f5a,f5b;
    memcpy(&rate,fields+3196+10,2);memcpy(&gender,fields+3196+12,2);
    memcpy(&head,fields+3196+14,2);memcpy(&scale,fields+3174,2);
    memcpy(&f4a,fields+3114,2);memcpy(&f4b,fields+3118,2);
    memcpy(&f5a,fields+3122,2);memcpy(&f5b,fields+3126,2);
    struct reloq_voice_filter model={head,gender,rate,scale,
        (f4a>0||f4b>0?1u<<3:0)|(f5a>0||f5b>0?1u<<4:0)};
    return (int16_t)reloq_voice_filter_endpoint(&model,(unsigned)stream,value);
}

int32_t reloq_enus61_add_filtered_val(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5,int32_t p6)
{
    (void)p0;(void)p5;(void)p6;
    /* The two position operands were registered but never read or changed.
     * Only stream, value and native begin/end time belong to this operation. */
    return reloq61_append_filtered_interval(state,p1,p2,p3,p4,reloq_enus61_filter_endpoint);
}

int32_t reloq_enus61_run_filters(void *state,int32_t p0,int32_t p1,int32_t p2)
{
    (void)p0;delta_state *d=state;delta_loc stream={0},value={0};
    if(!d||!p1||!p2||get_parm(d,&stream,EVV_AT(delta_loc *,p1),DK_SHORT2)||
       get_parm(d,&value,EVV_AT(delta_loc *,p2),DK_SHORT2))return 94;
    EVV_AT(delta_loc *,p2)->field=reloq_enus61_filter_endpoint(d,stream.field,value.field);
    return 0;
}

/* One formant's two phone-boundary targets. Short phones share the rounded
 * midpoint; missing targets keep the original hold/interpolation behavior.
 * Values and native times stay scalar through filtering and queue submission. */
int32_t reloq_enus61_doF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    delta_loc stream={0},left={0},right={0};
    delta_state *d=state;
    if(!p1||!p2||!p3||get_parm(d,&stream,EVV_AT(delta_loc *,p1),DK_SHORT2)||
        get_parm(d,&left,EVV_AT(delta_loc *,p2),DK_SHORT2)||
        get_parm(d,&right,EVV_AT(delta_loc *,p3),DK_SHORT2))
        return reloq_scalar_finish(state,94);
    const unsigned char *fields=(const unsigned char *)state+DG_BASE;
    int32_t duration,begin,end;
    memcpy(&duration,fields+2172,4);
    memcpy(&begin,fields+1144,4);memcpy(&end,fields+1152,4);
    if(left.field>0) {
        if(duration<160000 && right.field>0) {
            int32_t midpoint=delta_muldiv((int32_t)left.field+right.field,10000,1,0);
            midpoint=delta_muldiv(midpoint,5000,10000,1);
            midpoint=(int32_t)((uint32_t)midpoint+1);
            if(midpoint<1)midpoint=(int32_t)((uint32_t)midpoint-2);
            left.field=(int16_t)delta_muldiv(midpoint,1,10000,0);
            right.field=-1;
        }
        int16_t value=reloq_enus61_filter_endpoint(state,stream.field,left.field);
        reloq61_submit_interval(state,stream.field,value,begin,begin);
        if(right.field==-1)
            reloq61_submit_interval(state,stream.field,value,end,end);
    }
    if(right.field>0) {
        int16_t value=reloq_enus61_filter_endpoint(state,stream.field,right.field);
        reloq61_submit_interval(state,stream.field,value,left.field==-1?begin:end,end);
    }
    return reloq_scalar_finish(state,0);
}
