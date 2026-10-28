/* Native anatomy endpoints use the common bounded C arithmetic. */
#include "reloq61_scalar_state.h"
#include "reloq_voice_filter.h"
int32_t reloq_zhcn61_run_filters(void *state,int32_t p0,int32_t p1,int32_t p2)
{
    (void)p0;
    delta_loc stream={0},value={0};
    if(!state||!p1||!p2||get_parm(state,&stream,EVV_AT(delta_loc *,p1),DK_SHORT2)||
       get_parm(state,&value,EVV_AT(delta_loc *,p2),DK_SHORT2))return 94;
    const unsigned char *fields=(const unsigned char *)state+DG_BASE;
    int16_t rate,female,head,scale,f4a,f4b,f5a,f5b;
    memcpy(&rate,fields+3120+10,2);memcpy(&female,fields+3120+12,2);
    memcpy(&head,fields+3120+14,2);memcpy(&scale,fields+3098,2);
    memcpy(&f4a,fields+3038,2);memcpy(&f4b,fields+3042,2);
    memcpy(&f5a,fields+3046,2);memcpy(&f5b,fields+3050,2);
    const struct reloq_voice_filter model={head,female,rate,scale,
        (f4a>0||f4b>0?1u<<3:0)|(f5a>0||f5b>0?1u<<4:0)};
    EVV_AT(delta_loc *,p2)->field=(int16_t)reloq_voice_filter_endpoint(&model,
        (unsigned)stream.field,value.field);
    return 0;
}

/* Word tone percentages act on integer targets. Keeping the percentage as
 * one ratio avoids expanding a high female target past signed 32 bits. */
int32_t reloq_zhcn61_chi_word_stress_tone(void *state,int32_t p0,int32_t p1)
{
    (void)p0;
    delta_loc tone={0};uint16_t stress;
    if(!reloq_scalar_begin(state)||!p1||
       get_parm(state,&tone,EVV_AT(delta_loc *,p1),DK_SHORT2))return 94;
    memcpy(&stress,(const char *)state+DG_BASE+222,sizeof stress);
    static const unsigned percent[]={95,95,100,110,120,120};
    int value=tone.field;
    if(stress<sizeof percent/sizeof *percent)
        value=(int)((int64_t)value*percent[stress]/100);
    EVV_AT(delta_loc *,p1)->field=(int16_t)value;
    return reloq_scalar_finish(state,0);
}
