/* Shared, direct C acoustic queue submission. */
#include <stdint.h>
extern int32_t reloq61_append_acoustic_interval(void *,int32_t,int32_t,int32_t,int32_t);
int32_t reloq_dede61_addStreamArraySsVal(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4)
{(void)p0;return reloq61_append_acoustic_interval(state,p1,p2,p3,p4);}

#include "reloq61_scalar_state.h"
#include "reloq61_acoustic_output.h"
#include "reloq_voice_filter.h"

/* The active typed model can change through inline controls. Reading its
 * values for this endpoint preserves that behavior without retaining any
 * model-dependent output between requests. */
static int16_t field16(const unsigned char *fields,unsigned offset)
{int16_t value;memcpy(&value,fields+offset,2);return value;}
/* Tenths in the native short-value lane. Retain its defined fixed-point
 * wrap/rounding for out-of-range operands, without a rule frame. */
static int16_t tenths(int16_t value,int factor)
{
    int32_t fixed=delta_muldiv((int32_t)value*factor,10000,1,0);
    fixed=delta_muldiv(fixed,10000,100000,1);
    fixed=(int32_t)((uint32_t)fixed+1);
    if(fixed<1)fixed=(int32_t)((uint32_t)fixed-2);
    return (int16_t)delta_muldiv(fixed,1,10000,0);
}
int16_t reloq_dede61_filter_endpoint(void *state,int16_t stream,int16_t value)
{
    const unsigned char *fields=(const unsigned char *)state+DG_BASE;
    int16_t rate=field16(fields,3114),gender=field16(fields,3116),head=field16(fields,3118);
    int range=field16(fields,2658)==field16(fields,2650);
    if(head!=50) {
        if(stream<6)value=(int16_t)((int64_t)value*field16(fields,3082)/100);
        else if(stream==10&&head<=40&&field16(fields,2654)==field16(fields,2650))
            value=(int16_t)((uint16_t)value<<3);
        else if(stream==9&&gender) {
            if(head<=40)value=tenths(value,16);
            if(value>3000)value=3000;
        }
    }
    if(range&&stream>0&&stream<6) {
        struct reloq_voice_filter model={50,1,11025,100,
            (field16(fields,3022)>0||field16(fields,3026)>0?1u<<3:0)|
            (field16(fields,3030)>0||field16(fields,3034)>0?1u<<4:0)};
        value=(int16_t)reloq_voice_filter_endpoint(&model,(unsigned)stream,value);
    }
    if(rate==8000) {
        if(stream==3&&value>3600)value=3600;
        else if(stream==4&&value>3900)value=3900;
        else if(stream==9) {
            value=tenths(value,15);
            if(gender==1)value=1000;
            else if(value>3000)value=3000;
        } else if(stream==8&&range) {
            int scaled=(int)value*(head<30?8:head<=40?4:2);
            int limit=head<30?3000:head<=40?1000:500;
            int small=head<=40&&head>=30?(int16_t)scaled:(uint16_t)scaled;
            value=(int16_t)(small<limit?scaled:limit);
        }
    }
    return value;
}

int32_t reloq_dede61_add_filtered_val(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5,int32_t p6)
{
    (void)p0;(void)p5;(void)p6;
    /* The two position operands were registered but never read or changed.
     * Only stream, value and native begin/end time belong to this operation. */
    return reloq61_append_filtered_interval(state,p1,p2,p3,p4,reloq_dede61_filter_endpoint);
}

int32_t reloq_dede61_run_filters(void *state,int32_t p0,int32_t p1,int32_t p2)
{
    (void)p0;delta_state *d=state;delta_loc stream={0},value={0};
    if(!reloq_scalar_begin(state))return 94;
    if(!d||!p1||!p2||get_parm(d,&stream,EVV_AT(delta_loc *,p1),DK_SHORT2)||
       get_parm(d,&value,EVV_AT(delta_loc *,p2),DK_SHORT2))return 94;
    EVV_AT(delta_loc *,p2)->field=reloq_dede61_filter_endpoint(d,stream.field,value.field);
    return reloq_scalar_finish(state,0);
}

/* One formant's two phone-boundary targets. Short phones share the rounded
 * midpoint; missing targets keep the original hold/interpolation behavior.
 * Values and native times stay scalar through filtering and queue submission. */
int32_t reloq_dede61_doF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3)
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
    memcpy(&duration,fields+2080,4);
    memcpy(&begin,fields+1048,4);memcpy(&end,fields+1056,4);
    if(left.field>0) {
        if(duration<160000 && right.field>0) {
            int32_t midpoint=delta_muldiv((int32_t)left.field+right.field,10000,1,0);
            midpoint=delta_muldiv(midpoint,5000,10000,1);
            midpoint=(int32_t)((uint32_t)midpoint+1);
            if(midpoint<1)midpoint=(int32_t)((uint32_t)midpoint-2);
            left.field=(int16_t)delta_muldiv(midpoint,1,10000,0);
            right.field=-1;
        }
        int16_t value=reloq_dede61_filter_endpoint(state,stream.field,left.field);
        reloq61_submit_interval(state,stream.field,value,begin,begin);
        if(right.field==-1)
            reloq61_submit_interval(state,stream.field,value,end,end);
    }
    if(right.field>0) {
        int16_t value=reloq_dede61_filter_endpoint(state,stream.field,right.field);
        reloq61_submit_interval(state,stream.field,value,left.field==-1?begin:end,end);
    }
    return reloq_scalar_finish(state,0);
}
