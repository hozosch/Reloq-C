/* Direct submission of an acoustic hold to the existing ordered stream queue.
 * Compact cells are decoded once at this boundary. The operation has no rule
 * frame, speculative tests, saved registers or backtracking state. */
#include "delta.h"
#include "reloq61_scalar_state.h"
#include "reloq61_acoustic_output.h"

extern int32_t addStreamArrayPtValC(void *,const int16_t *,const int16_t *,const int32_t *);
extern int32_t addStreamArraySsValC(void *,const int16_t *,const int16_t *,const int32_t *,const int32_t *);

void reloq61_submit_interval(void *state,int16_t stream,int16_t value,
    int32_t begin,int32_t end)
{
    const delta_loc s={DK_SHORT2,stream,0},v={DK_SHORT2,value,0};
    const delta_loc a={DK_FIXED,0,begin},b={DK_FIXED,0,end};
    if(begin==end)
        (void)addStreamArrayPtValC(state,(const int16_t *)&s,
            (const int16_t *)&v,(const int32_t *)&a);
    else
        (void)addStreamArraySsValC(state,(const int16_t *)&s,
            (const int16_t *)&v,(const int32_t *)&a,(const int32_t *)&b);
}

static int32_t append_interval(void *state,int32_t stream_arg,
    int32_t value_arg,int32_t begin_arg,int32_t end_arg,reloq61_endpoint_filter filter)
{
    if(!reloq_scalar_begin(state))return 94;
    delta_state *d=state;
    delta_loc stream={0},value={0},begin={0},end={0};
    if(!stream_arg || !value_arg || !begin_arg || !end_arg ||
        get_parm(d,&stream,EVV_AT(delta_loc *,stream_arg),DK_SHORT2) ||
        get_parm(d,&value,EVV_AT(delta_loc *,value_arg),DK_SHORT2) ||
        get_parm(d,&begin,EVV_AT(delta_loc *,begin_arg),DK_FIXED) ||
        get_parm(d,&end,EVV_AT(delta_loc *,end_arg),DK_FIXED))
        return reloq_scalar_finish(state,94);
    if(filter)value.field=filter(state,stream.field,value.field);
    /* Compare the native times before truncating them to queue ticks. Two
     * distinct sub-tick times still submit two ordered points, as before.
     * The queue owns duplicate/rejected-time handling and partial appends. */
    reloq61_submit_interval(d,stream.field,value.field,begin.value,end.value);
    return reloq_scalar_finish(state,0);
}

int32_t reloq61_append_acoustic_interval(void *state,int32_t stream,
    int32_t value,int32_t begin,int32_t end)
{return append_interval(state,stream,value,begin,end,NULL);}

int32_t reloq61_append_filtered_interval(void *state,int32_t stream,
    int32_t value,int32_t begin,int32_t end,reloq61_endpoint_filter filter)
{return append_interval(state,stream,value,begin,end,filter);}
