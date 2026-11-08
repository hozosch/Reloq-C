#ifndef RELOQ_SAPI_PITCH_H
#define RELOQ_SAPI_PITCH_H
#include "reloq_voice_controls.h"
#include <math.h>

/* SAPI supplies a pitch offset in semitones. The native pitch/F0 mapping is
 * strictly increasing, so only the two neighbours surrounding the target
 * can win the original logarithmic nearest-pitch comparison. */
static int sapi_native_pitch(int base,long adjustment)
{
    if(!adjustment)return base;
    double target=reloq_voice_pitch_f0_scale(base)*pow(2.0,adjustment/10.0);
    if(!(target>0.0)||!isfinite(target))return 0;
    int low=0,high=100;
    while(low<high){
        int mid=low+(high-low)/2;
        if(reloq_voice_pitch_f0_scale(mid)<target)low=mid+1;
        else high=mid;
    }
    if(!low)return 0;
    double upper=fabs(log(reloq_voice_pitch_f0_scale(low)/target));
    double lower=fabs(log(reloq_voice_pitch_f0_scale(low-1)/target));
    return upper<lower?low:low-1;
}
/* Preserve SAPI's relative pitch adjustment around an exact Hertz base.
 * At the application's neutral value, adjustment is zero. */
static int sapi_hertz_pitch(int base,long adjustment)
{
    if(adjustment>200)return 24000;
    if(adjustment< -200)return 1;
    double wanted=(double)base*pow(2.0,adjustment/10.0);
    if(wanted<=1.0)return 1;
    if(wanted>=24000.0)return 24000;
    return (int)lround(wanted);
}
#endif
