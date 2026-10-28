/* Direct language bindings for shared segment and duration decisions. */
#include "reloq61_segment_rules.h"
#include "reloq61_spr_output.h"
#include "reloq61_scalar_state.h"
#include "reloq61.h"
extern int16_t reloq_enus61_filter_endpoint(void *,int16_t,int16_t);
static const struct reloq61_segment_layout layout={
    .pause_class=9,.speed_field=10,
    .speed_before=464,
    .speed_after=472,
    .nucleus=184,
    .unscaled=478,
    .short_pause=426,
    .pause=418,
    .first_count=222,
    .stress=374,
    .ordinary_stress=354,
    .word_type=210,
    .phone_boundary=720,
    .voice=3196,
    .force_voicing=2132,
    .point_voicing=1192,
    .av_stream=1034,
    .af_stream=1042,
    .pole_stream=1002,
    .zero_stream=1006,
    .phone_end=1152,
    .minimum=2342,
    .short_minimum=2346,
    .pause_duration=3162,
    .focus=3474,
    .neutral=3450,
    .previous_focus=3462,
    .syllable_count=1886
};
int32_t reloq_enus61_addStreamArrayPtVal(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_point(state,p1,p2,p3,p4,p5);}
int32_t reloq_enus61_doAV(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_voicing(state,&layout,p1,p2,p3,p4,p5);}
int32_t reloq_enus61_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{
    (void)p0;
    if(!reloq_scalar_begin(state))return 94;
    /* Source ownership comes from the current phonetic segment, before
     * endpoints enter the ordered queue. Match the German S gain without
     * raising dental bursts, aspiration, voicing or parallel-pole levels. */
    int32_t at;memcpy(&at,(const unsigned char *)state+DG_BASE+712,4);
    struct reloq61_spr_phone phone;int gain=0;
    if(reloq61_spr_phone_at(state,at,&phone)) {
        unsigned name=reloq61_spr_feature(state,&phone,0);
        if(name==13 || name==14)gain=4; /* /z/, /s/ */
        else if(name==11 || name==12)gain=reloq61_labiodental_gain(state); /* /v, f/ */
    }
    return reloq61_segment_frication_gain(state,&layout,p1,p2,p3,p4,p5,gain);
}
int32_t reloq_enus61_doFNZ(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_nasal_zero(state,&layout,reloq_enus61_filter_endpoint,p1,p2,p3,p4,p5);}
int32_t reloq_enus61_doFNP(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_nasal_pole(state,&layout,reloq_enus61_filter_endpoint,p1,p2,p3,p4,p5);}
int32_t reloq_enus61_apply_speed_anno(void *state,int32_t p0,int32_t p1,int32_t p2)
{(void)p0;return reloq61_segment_speed(state,&layout,p1,p2);}
int32_t reloq_enus61_insert_dur(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4)
{(void)p0;return reloq61_segment_duration(state,&layout,p1,p2,p3,p4);}
