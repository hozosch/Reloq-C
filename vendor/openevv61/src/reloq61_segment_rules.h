/* Language bindings describe only the layout of the existing voice state. */
#ifndef RELOQ61_SEGMENT_RULES_H
#define RELOQ61_SEGMENT_RULES_H
#include "reloq61_acoustic_output.h"
#include <stdint.h>
struct reloq61_segment_layout {
    uint16_t voice,force_voicing,point_voicing;
    uint16_t av_stream,af_stream,pole_stream,zero_stream,phone_boundary,phone_end;
    uint16_t speed_before,speed_after,nucleus,unscaled;
    uint16_t minimum,short_minimum,short_pause,pause,pause_duration;
    uint16_t focus,neutral,previous_focus,syllable_count,first_count;
    uint16_t stress,ordinary_stress,word_type;
    uint8_t pause_class,speed_field;
};
int32_t reloq61_segment_point(void *,int32_t,int32_t,int32_t,int32_t,int32_t);
int32_t reloq61_segment_voicing(void *,const struct reloq61_segment_layout *,int32_t,int32_t,int32_t,int32_t,int32_t);
int32_t reloq61_segment_frication(void *,const struct reloq61_segment_layout *,int32_t,int32_t,int32_t,int32_t,int32_t);
int32_t reloq61_segment_frication_gain(void *,const struct reloq61_segment_layout *,int32_t,int32_t,int32_t,int32_t,int32_t,int);
int32_t reloq61_segment_nasal_zero(void *,const struct reloq61_segment_layout *,reloq61_endpoint_filter,int32_t,int32_t,int32_t,int32_t,int32_t);
int32_t reloq61_segment_nasal_pole(void *,const struct reloq61_segment_layout *,reloq61_endpoint_filter,int32_t,int32_t,int32_t,int32_t,int32_t);
int32_t reloq61_segment_speed(void *,const struct reloq61_segment_layout *,int32_t,int32_t);
int32_t reloq61_segment_duration(void *,const struct reloq61_segment_layout *,int32_t,int32_t,int32_t,int32_t);
#endif
