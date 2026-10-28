#ifndef RELOQ_VOICE_FILTER_H
#define RELOQ_VOICE_FILTER_H
#include <stdint.h>

/* A scalar endpoint operation, before interpolation. The text/phone planner
 * supplies the two upper-pole ownership bits; the filter needs no language
 * machine, token arena, callback, allocation or saved instruction state. */
struct reloq_voice_filter {
    int head_size, female, sample_rate, scale_percent;
    unsigned explicit_poles;
};
/* Native acoustic parameter numbers: F1-F5=1..5, B3=8, B4=9, B5=10.
 * All other endpoints pass through unchanged. */
int32_t reloq_voice_filter_endpoint(const struct reloq_voice_filter *model,
    unsigned parameter,int32_t value);
#endif
