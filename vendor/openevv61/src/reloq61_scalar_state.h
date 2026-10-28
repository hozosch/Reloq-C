/* Shared scalar language state operations. These routines neither create a
 * rule scope nor touch scan positions, fences or backtracking stacks. */
#ifndef RELOQ61_SCALAR_STATE_H
#define RELOQ61_SCALAR_STATE_H
#include "delta.h"
#include <string.h>

static inline int reloq_scalar_begin(void *state)
{
    delta_state *d=state;
    if(!d || !d->vars || deltaErrorThrown(d))return 0;
    d->unknown_3c=0;
    return 1;
}
static inline int32_t reloq_scalar_finish(void *state,int32_t status)
{
    delta_state *d=state;
    EVV_AT(delta_vars *,d->vars)->unknown_11e8=0;
    return status;
}

struct reloq_default_voice {
    int16_t gender,pitch,speed,volume,head,inflection,roughness,breathiness;
};
static inline void reloq_scalar_voice_defaults(void *fields)
{
    static const struct reloq_default_voice voices[]={
        {0,65,50,92,50,30,0,0},{0,69,50,92,50,30,0,0},
        {1,81,50,100,50,30,0,50},{1,93,50,90,22,35,0,0},
        {0,61,50,90,30,44,18,20},{1,68,50,90,45,30,3,40},
        {0,56,50,93,86,47,0,0},{0,69,70,92,50,34,0,0},
        {1,89,70,95,56,35,0,40},{0,74,50,92,50,35,0,0}
    };
    /* The compatibility record interleaves each value with its type tag.
     * Only the values are initialized; adjacent tags remain caller-owned. */
    unsigned char *out=fields;
    for(size_t voice=0;voice<sizeof voices/sizeof *voices;++voice)
        for(size_t field=0;field<8;++field)
            memcpy(out+voice*32+field*4,
                (const unsigned char *)(voices+voice)+field*2,2);
}
static inline uint16_t reloq_scalar_next_file(uint16_t number)
{
    return number>INT16_MAX || (number>0 && number<999)||(number>1000 && number<1999)?
        (uint16_t)(number+1):number;
}
static inline int16_t reloq_scalar_next_accent(int16_t current,
    int16_t first,int16_t second,int16_t third,int16_t ordinary,int16_t last)
{
    if(current==first)return second;
    if(current==second)return third;
    return first==ordinary && first==last?first:current;
}
static inline void reloq_scalar_pitch_window(int16_t inflection,int32_t midline,
    int32_t *window,int32_t *base)
{
    /* The product before division can exceed int32 at high pitch.
       A single wide ratio preserves the rounding of the final target. */
    int32_t extent=delta_muldiv(inflection,midline,100,1);
    int32_t low=delta_muldiv(extent,-9,10,1);
    *window=extent;
    *base=(int32_t)((uint32_t)midline+(uint32_t)low);
}

static inline int32_t reloq_scalar_focus_duration(void *state,int32_t arg,
    int plain,int focused)
{
    delta_loc duration={0};
    if(!arg||get_parm(state,&duration,EVV_AT(delta_loc *,arg),DK_FIXED))
        return reloq_scalar_finish(state,94);
    if(!plain&&!focused)return reloq_scalar_finish(state,94);
    if(!plain) {
        duration.value=delta_muldiv(duration.value,120000,10000,1);
        duration.value=delta_muldiv(duration.value,10000,100000,1);
    }
    EVV_AT(delta_loc *,arg)->value=duration.value;
    return reloq_scalar_finish(state,0);
}
#endif
