/* Shared, direct C acoustic queue submission. */
#include <stdint.h>
#include "reloq61_scalar_state.h"
#include "reloq61_acoustic_output.h"
#include "reloq61_spr_output.h"
#include "reloq_voice_filter.h"
#include "reloq61_german_final.h"
extern int reloq_dede61_consonant_r(void *);
extern int reloq_dede61_number_four_r(void *);
static int16_t consonant_r_source(void *state,int16_t stream,int16_t value)
{
    int16_t aspiration;memcpy(&aspiration,(const unsigned char *)state+DG_BASE+942,2);
    return stream==aspiration&&value>0&&reloq_dede61_consonant_r(state)?(int16_t)(value+1):value;
}
int32_t reloq_dede61_addStreamArraySsVal(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4)
{(void)p0;return reloq61_append_filtered_interval(state,p1,p2,p3,p4,consonant_r_source);}

static int starts_field(delta_state *d,const struct reloq61_spr_phone *p,unsigned field)
{
    int32_t left=*EVV_AT(const int32_t *,p->node)&~3;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    return (*EVV_AT(const int32_t *,left+(v->fence_base+field)*4)&1)!=0;
}
/* The source-dependent terminal-a preference is identified while the
 * original spelling is still present, then consumed by acoustic planning. */
int reloq_dede61_final_text_a(void *state)
{
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone phone;
    return reloq61_spr_phone_at(state,at,&phone)&&
        reloq61_german_final_matches(state,phone.node);
}
static int16_t final_a_f2(delta_state *d)
{
    int32_t at;memcpy(&at,(const unsigned char *)d+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone p,previous;
    if(!reloq61_spr_phone_at(d,at,&p))return 1220;
    int32_t node=vgetsc(d,1,0,*EVV_AT(const int32_t *,p.node)&~3,2);
    while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,1,0,node,2);
    if(!node)return 1220;
    previous=(struct reloq61_spr_phone){node,EVV_AT(const int32_t *,node)[1]&~3};
    unsigned prior=reloq61_spr_feature(d,&previous,0);
    if(prior==1||prior==2||prior==7||prior==8||prior==21||prior==27)return 1100;
    if(prior==3||prior==4||prior==10)return 1250;
    if(prior==5||prior==6)return 1280;
    return 1220;
}
/* The old German composer used these distinct onset-r resonances.  Retain
 * the neighbouring vowel and onset-cluster decisions when submitting them
 * into the native planner, before the ordinary model/anatomy filter. */
static int consonant_r_tract(delta_state *d,int16_t left[5],int16_t right[5])
{
    if(!reloq_dede61_consonant_r(d))return 0;
    int32_t at;memcpy(&at,(const unsigned char *)d+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone p,next,previous;
    if(!reloq61_spr_phone_at(d,at,&p)||!reloq61_spr_phone_at(d,p.after,&next)||
       (starts_field(d,&next,7)&&!reloq_dede61_number_four_r(d))||
       reloq61_spr_feature(d,&next,1)!=1)return 0;
    unsigned vowel=reloq61_spr_feature(d,&next,0),prior=0;
    int32_t node=vgetsc(d,1,0,*EVV_AT(const int32_t *,p.node)&~3,2);
    while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,1,0,node,2);
    if(node) {
        previous=(struct reloq61_spr_phone){node,EVV_AT(const int32_t *,node)[1]&~3};
        prior=reloq61_spr_feature(d,&previous,0);
    }
    int cluster=!starts_field(d,&p,7)&&!reloq_dede61_number_four_r(d),initial=starts_field(d,&p,4);
    const int16_t base[]={400,1320,2100,3600,3900};
    memcpy(left,base,sizeof base);
    for(unsigned i=0;i<5;++i)right[i]=-1;
    if(cluster) {
        int labial=prior==1||prior==2||prior==7||prior==8;
        if(vowel==28||vowel==30||vowel==32) {left[1]=1670;left[2]=2300;}
        else if(vowel==29||vowel==31) {left[1]=1450;left[2]=2200;}
        else if(vowel==35||vowel==36||vowel==37||vowel==38||vowel==39||vowel==40) {
            left[2]=2000;
            left[1]=labial?(vowel==35||vowel==37?1000:vowel==39||vowel==40?1200:900):1320;
            if(labial&&(vowel==35||vowel==37))right[1]=850;
        }
    } else {
        if(!initial) {
            if(prior==31||prior==32||prior==48)left[2]=2200;
            else if(prior==38||prior==36||prior==49) {left[1]=1200;left[2]=2000;}
            else if(prior==37||prior==35) {left[1]=850;left[2]=2000;}
            else if(prior==27||prior==21||prior==1)left[1]=1000;
        }
        if(vowel==28||vowel==30||vowel==32) {right[1]=1670;right[2]=2300;}
        else if(vowel==29||vowel==31) {right[1]=1450;right[2]=2200;}
        else if(vowel==35||vowel==37) {right[1]=840;right[2]=2000;}
        else if(vowel==36||vowel==38||vowel==39) {right[1]=1200;right[2]=2000;}
        if(initial)for(unsigned i=1;i<=2;++i)if(right[i]>0)left[i]=right[i];
    }
    return 1;
}

/* A syllable-final r after long /oe/ has its own approved vocalic colour.
 * Use phone and syllable records, so the same rule applies to SPR input. */
static int r_after_long_oe(delta_state *d)
{
    int32_t at;memcpy(&at,(const unsigned char *)d+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone current,previous;
    if(!reloq61_spr_phone_at(d,at,&current))return 0;
    unsigned name=reloq61_spr_feature(d,&current,0);
    if(name!=15 && !(name==25&&reloq61_spr_feature(d,&current,1)==2))return 0;
    int32_t left=*EVV_AT(const int32_t *,current.node)&~3;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    if(*EVV_AT(const int32_t *,left+(v->fence_base+7)*4)&1)return 0;
    int32_t node=vgetsc(d,1,0,left,2);
    while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,1,0,node,2);
    if(!node)return 0;
    previous=(struct reloq61_spr_phone){node,EVV_AT(const int32_t *,node)[1]&~3};
    return previous.after&&reloq61_spr_feature(d,&previous,0)==41;
}

/* The reduced -er nucleus after /y/ or /o/, as in "früher" and
 * "lower", keeps the previous voice's neutral oral targets. */
static int reduced_r_after_y_o(delta_state *d)
{
    int32_t at;memcpy(&at,(const unsigned char *)d+DG_BASE+712,sizeof at);
    struct reloq61_spr_phone current,previous,next;
    if(!reloq61_spr_phone_at(d,at,&current)||
       reloq61_spr_feature(d,&current,0)!=25||
       reloq61_spr_feature(d,&current,1)!=1)return 0;
    int32_t left=*EVV_AT(const int32_t *,current.node)&~3;
    delta_vars *v=EVV_AT(delta_vars *,d->vars);
    if(*EVV_AT(const int32_t *,left+(v->fence_base+4)*4)&1)return 0;
    int32_t node=vgetsc(d,1,0,left,2);
    while(node&&(*EVV_AT(const int32_t *,node)&2))node=vgetsc(d,1,0,node,2);
    if(!node)return 0;
    previous=(struct reloq61_spr_phone){node,EVV_AT(const int32_t *,node)[1]&~3};
    unsigned prior=reloq61_spr_feature(d,&previous,0);
    if(prior!=39&&prior!=37)return 0;
    if(reloq61_spr_phone_at(d,current.after,&next)&&
       reloq61_spr_feature(d,&next,0)!=51) {
        int32_t edge=*EVV_AT(const int32_t *,next.node)&~3;
        if(!(*EVV_AT(const int32_t *,edge+(v->fence_base+4)*4)&1))return 0;
    }
    return 1;
}

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
    if(stream==8&&value>0&&r_after_long_oe(state))value=200;
    if(stream>=6&&stream<=10&&value>0) {
        int16_t left[5],right[5];
        if(consonant_r_tract(state,left,right)) {
            static const int16_t bandwidth[]={120,100,150,330,260};
            value=bandwidth[stream-6];
        }
    }
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
    if(stream.field>=1&&stream.field<=5) {
        int16_t entrance[5],departure[5];
        if(consonant_r_tract(d,entrance,departure)) {
            unsigned i=(unsigned)stream.field-1;
            if(left.field>0)left.field=entrance[i];
            if(departure[i]>0&&left.field>0)right.field=departure[i];
            else if(right.field>0)right.field=departure[i]>0?departure[i]:left.field>0?-1:entrance[i];
        }
    }
    if(stream.field>=1&&stream.field<=3&&reloq_dede61_final_text_a(d)) {
        static const int16_t target[]={800,1220,2500};
        if(left.field>0)left.field=stream.field==2?final_a_f2(d):target[stream.field-1];
        if(right.field>0||(stream.field==2&&left.field>0))right.field=target[stream.field-1];
    }
    if(stream.field>=1&&stream.field<=3&&r_after_long_oe(d)) {
        static const int16_t target[]={600,1320,2300};
        if(left.field>0)left.field=target[stream.field-1];
        if(right.field>0)right.field=target[stream.field-1];
    }
    if(stream.field>=1&&stream.field<=3&&reduced_r_after_y_o(d)) {
        static const int16_t entrance[]={600,1360,2280};
        static const int16_t departure[]={600,1300,2280};
        if(left.field>0)left.field=entrance[stream.field-1];
        if(right.field>0)right.field=departure[stream.field-1];
    }
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
