#ifndef RELOQ_BODY_EXTENSION_H
#define RELOQ_BODY_EXTENSION_H
#include <math.h>

/* Add A's periodic upper body, retaining the complete Classic foundation.
 * A two-pole 2.5-kHz split keeps the addition out of the lower formants.
 * Its state belongs to the utterance, never a callback/frame boundary. */
struct reloq_body_extension { double c[5],delay[2]; };
static inline void reloq_body_extension_initialize(struct reloq_body_extension *s,double rate)
{
    double w=2*3.14159265358979323846*2500/rate,c=cos(w),alpha=sin(w)/sqrt(2),d=1+alpha;
    s->c[0]=(1+c)/(2*d);s->c[1]=-(1+c)/d;s->c[2]=s->c[0];
    s->c[3]=-2*c/d;s->c[4]=(1-alpha)/d;
    s->delay[0]=s->delay[1]=0;
}
static inline float reloq_body_extension_sample(struct reloq_body_extension *s,float input)
{
    double y=s->c[0]*input+s->delay[0];
    s->delay[0]=s->c[1]*input-s->c[3]*y+s->delay[1];
    s->delay[1]=s->c[2]*input-s->c[4]*y;
    return (float)y;
}
#endif
