#ifndef RELOQ_PITCH_GLIDE_H
#define RELOQ_PITCH_GLIDE_H

#include <math.h>

/* Per-utterance transition of the pitch multiplier. A first target is
 * applied immediately; later targets start at the actually reached value.
 * Interpolation in log frequency uses a bounded smoothstep, so the target is
 * reached after the requested time without overshoot or a slope kink. */
struct reloq_pitch_glide {
    double start_log, target_log, target, elapsed_ms;
    int initialized;
};

static inline double reloq_pitch_glide_value(struct reloq_pitch_glide *g,
                                             double target, int duration_ms)
{
    if(!g->initialized) {
        g->initialized=1;g->target=target;g->target_log=log(target);
        g->start_log=g->target_log;g->elapsed_ms=duration_ms;
        return target;
    }
    double current;
    if(g->elapsed_ms>=duration_ms)current=g->target;
    else {
        double u=g->elapsed_ms/duration_ms;
        double blend=u*u*(3.0-2.0*u);
        current=exp(g->start_log+(g->target_log-g->start_log)*blend);
    }
    if(target!=g->target) {
        g->start_log=log(current);g->target_log=log(target);
        g->target=target;g->elapsed_ms=0;
    }
    return current;
}

static inline void reloq_pitch_glide_advance(struct reloq_pitch_glide *g,
                                               double milliseconds,int duration_ms)
{
    if(g->initialized && g->elapsed_ms<duration_ms) {
        g->elapsed_ms+=milliseconds;
        if(g->elapsed_ms>duration_ms)g->elapsed_ms=duration_ms;
    }
}

#endif
