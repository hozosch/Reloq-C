#ifndef RELOQ61_DURATION_MATH_H
#define RELOQ61_DURATION_MATH_H
#include <stdint.h>

/* A provider-owned CJK request can cap a very slow native factor while
 * retaining its original value for the streaming renderer. */
int32_t reloq61_cjk_duration_factor(void *state, int32_t factor);
int32_t reloq61_cjk_wpm_factor(void *state, int32_t factor, int32_t wpm);

/* Preserve both fixed-point rounding steps without narrowing the intermediate. */
static inline int32_t delta_scale_percent(int32_t value, int32_t percent)
{
    int64_t product=(int64_t)value*percent;
    uint64_t magnitude=(uint64_t)(product<0?-product:product);
    uint64_t intermediate=(magnitude+5000u)/10000u;
    uint64_t result=(intermediate+50u)/100u;
    return (int32_t)(product<0?-(int64_t)result:(int64_t)result);
}
#endif
