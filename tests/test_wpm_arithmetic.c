/* The slow-rate fix must preserve ordinary two-step fixed-point rounding. */
#include "reloq61_duration_math.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint32_t random=0x367812a3u;
    unsigned checks=0;
    for(unsigned i=0;i<200000;++i) {
        random=random*1664525u+1013904223u;
        int32_t duration=(int32_t)(random%20000001u)-10000000;
        random=random*1664525u+1013904223u;
        int32_t percent=(int32_t)(random%4000001u);
        int64_t product=(int64_t)duration*percent;
        int64_t magnitude=product<0?-product:product;
        int64_t intermediate=(magnitude+5000)/10000;
        if(intermediate<=INT32_MAX) {
            int32_t old=(int32_t)((intermediate+50)/100);
            if(product<0)old=-old;
            assert(delta_scale_percent(duration,percent)==old);
            ++checks;
        }
    }
    /* 400 ms at 5 WPM: the old intermediate exceeded signed 32-bit. */
    assert(delta_scale_percent(4000000,35600000)==142400000);
    assert(delta_scale_percent(-4000000,35600000)==-142400000);
    assert(delta_scale_percent(500000,22250000)==11125000);
    assert(checks>10000);
    printf("WPM duration arithmetic: %u normal-rounding checks and three wide cases passed\n",checks);
    return 0;
}
