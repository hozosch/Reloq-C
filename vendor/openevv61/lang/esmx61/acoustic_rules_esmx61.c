/* Native anatomy endpoints use the common bounded C arithmetic. */
#include "reloq61_scalar_state.h"
#include "reloq_voice_filter.h"
int32_t reloq_esmx61_run_filters(void *state,int32_t p0,int32_t p1,int32_t p2)
{
    (void)p0;
    delta_loc stream={0},value={0};
    if(!state||!p1||!p2||get_parm(state,&stream,EVV_AT(delta_loc *,p1),DK_SHORT2)||
       get_parm(state,&value,EVV_AT(delta_loc *,p2),DK_SHORT2))return 94;
    const unsigned char *fields=(const unsigned char *)state+DG_BASE;
    int16_t rate,female,head,scale,f4a,f4b,f5a,f5b;
    memcpy(&rate,fields+3108+10,2);memcpy(&female,fields+3108+12,2);
    memcpy(&head,fields+3108+14,2);memcpy(&scale,fields+3086,2);
    memcpy(&f4a,fields+3026,2);memcpy(&f4b,fields+3030,2);
    memcpy(&f5a,fields+3034,2);memcpy(&f5b,fields+3038,2);
    const struct reloq_voice_filter model={head,female,rate,scale,
        (f4a>0||f4b>0?1u<<3:0)|(f5a>0||f5b>0?1u<<4:0)};
    EVV_AT(delta_loc *,p2)->field=(int16_t)reloq_voice_filter_endpoint(&model,
        (unsigned)stream.field,value.field);
    return 0;
}
