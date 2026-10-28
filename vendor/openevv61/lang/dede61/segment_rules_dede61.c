/* Direct language bindings for shared segment and duration decisions. */
#include "reloq61_segment_rules.h"
extern int16_t reloq_dede61_filter_endpoint(void *,int16_t,int16_t);
static const struct reloq61_segment_layout layout={
    .pause_class=8,.speed_field=15,
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
    .voice=3104,
    .force_voicing=2040,
    .point_voicing=1096,
    .av_stream=938,
    .af_stream=946,
    .pole_stream=906,
    .zero_stream=910,
    .phone_end=1056,
    .minimum=2250,
    .short_minimum=2254,
    .pause_duration=3070,
    .focus=3382,
    .neutral=3358,
    .previous_focus=3370,
    .syllable_count=1794
};
int32_t reloq_dede61_addStreamArrayPtVal(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_point(state,p1,p2,p3,p4,p5);}
int32_t reloq_dede61_doAV(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_voicing(state,&layout,p1,p2,p3,p4,p5);}
int32_t reloq_dede61_doAF(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_frication(state,&layout,p1,p2,p3,p4,p5);}
int32_t reloq_dede61_doFNZ(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_nasal_zero(state,&layout,reloq_dede61_filter_endpoint,p1,p2,p3,p4,p5);}
int32_t reloq_dede61_doFNP(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4,int32_t p5)
{(void)p0;return reloq61_segment_nasal_pole(state,&layout,reloq_dede61_filter_endpoint,p1,p2,p3,p4,p5);}
int32_t reloq_dede61_apply_speed_anno(void *state,int32_t p0,int32_t p1,int32_t p2)
{(void)p0;return reloq61_segment_speed(state,&layout,p1,p2);}
int32_t reloq_dede61_insert_dur(void *state,int32_t p0,int32_t p1,int32_t p2,int32_t p3,int32_t p4)
{(void)p0;return reloq61_segment_duration(state,&layout,p1,p2,p3,p4);}
