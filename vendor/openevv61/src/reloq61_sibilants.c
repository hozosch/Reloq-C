/* Shared S/ts source balance. The adapters retain each language's native
 * phone, timing and stream layout; only active S-source endpoints gain 4 dB.
 * Existing affricate releases retain their native reference level. */
#include "reloq61_segment_rules.h"
#include "reloq61_scalar_state.h"
#include "reloq61_spr_output.h"

/* Spanish (Latin America) and Portuguese keep phone records in slot 4;
 * their slot 2 contains words. Query the actual phone layout explicitly. */
static int phone4_gain(delta_state *d,int32_t at,unsigned first)
{
    if(!at)return 0;
    int32_t node=vgetsc(d,0,0,at,4);
    while(node&&(*EVV_AT(int32_t *,node)&2))node=vgetsc(d,0,0,node,4);
    if(!node||!(*EVV_AT(int32_t *,node+4)&~3))return 0;
    unsigned name=*(const unsigned char *)delta_record_field(&vstmtbl[4],0,TFLDS(EVV_AT(void *,node)));
    return name==first||name==first+1?4:0;
}

int32_t reloq_engb61_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;if(!reloq_scalar_begin(state))return 94;
    static const struct reloq61_segment_layout layout={.af_stream=1042,.phone_boundary=720,.phone_end=1152};
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,4);
    struct reloq61_spr_phone phone;int gain=0;
    if(reloq61_spr_phone_at(state,at,&phone)){unsigned name=reloq61_spr_feature(state,&phone,0);
        if(name==13||name==14)gain=4;}
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,gain);
}

int32_t reloq_esmx61_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;if(!reloq_scalar_begin(state))return 94;
    static const struct reloq61_segment_layout layout={.af_stream=946,.phone_boundary=720,.phone_end=1056};
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,4);
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,phone4_gain(state,at,12));
}

int32_t reloq_frfr61_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;if(!reloq_scalar_begin(state))return 94;
    static const struct reloq61_segment_layout layout={.af_stream=1042,.phone_boundary=720,.phone_end=1152};
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,4);
    struct reloq61_spr_phone phone;int gain=0;
    if(reloq61_spr_phone_at(state,at,&phone)){unsigned name=reloq61_spr_feature(state,&phone,0);
        if(name==11||name==12)gain=4;}
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,gain);
}

int32_t reloq_frca61_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;if(!reloq_scalar_begin(state))return 94;
    static const struct reloq61_segment_layout layout={.af_stream=1042,.phone_boundary=720,.phone_end=1152};
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,4);
    struct reloq61_spr_phone phone;int gain=0;
    if(reloq61_spr_phone_at(state,at,&phone)){unsigned name=reloq61_spr_feature(state,&phone,0);
        if(name==11||name==12)gain=4;}
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,gain);
}

int32_t reloq_itit61_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;if(!reloq_scalar_begin(state))return 94;
    static const struct reloq61_segment_layout layout={.af_stream=952,.phone_boundary=720,.phone_end=1064};
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,4);
    struct reloq61_spr_phone phone;int gain=0;
    if(reloq61_spr_phone_at(state,at,&phone)){unsigned name=reloq61_spr_feature(state,&phone,0);
        if(name==9||name==10)gain=4;}
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,gain);
}

int32_t reloq_ptbr_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;if(!reloq_scalar_begin(state))return 94;
    static const struct reloq61_segment_layout layout={.af_stream=946,.phone_boundary=720,.phone_end=1056};
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,4);
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,phone4_gain(state,at,9));
}

int32_t reloq_fifi_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;if(!reloq_scalar_begin(state))return 94;
    static const struct reloq61_segment_layout layout={.af_stream=946,.phone_boundary=720,.phone_end=1056};
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,4);
    struct reloq61_spr_phone phone;int gain=0;
    if(reloq61_spr_phone_at(state,at,&phone)){unsigned name=reloq61_spr_feature(state,&phone,0);
        if(name==9||name==10)gain=4;}
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,gain);
}

int32_t reloq_zhcn61_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;if(!reloq_scalar_begin(state))return 94;
    static const struct reloq61_segment_layout layout={.af_stream=954,.phone_boundary=720,.phone_end=1064};
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,4);
    struct reloq61_spr_phone phone;int gain=0;
    if(reloq61_spr_phone_at(state,at,&phone)){unsigned name=reloq61_spr_feature(state,&phone,0);
        if(name==13||name==14)gain=4;}
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,gain);
}

int32_t reloq_kokr61_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;if(!reloq_scalar_begin(state))return 94;
    static const struct reloq61_segment_layout layout={.af_stream=954,.phone_boundary=720,.phone_end=1064};
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,4);
    struct reloq61_spr_phone phone;int gain=0;
    if(reloq61_spr_phone_at(state,at,&phone)){unsigned name=reloq61_spr_feature(state,&phone,0);
        if(name==17||name==18)gain=4;}
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,gain);
}
