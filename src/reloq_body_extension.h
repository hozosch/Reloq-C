#ifndef RELOQ_BODY_EXTENSION_H
#define RELOQ_BODY_EXTENSION_H

#include <math.h>

/* Complementary split: B + amount * HP(calibrated A - B).
 * The accepted Classic body remains the DC/low-band foundation. Only the
 * difference above 2.5 kHz enters progressively; no block-boundary reset,
 * lookahead, signal-dependent gain or additional output delay is involved. */
struct reloq_body_extension {
    double previous_difference, previous_high;
};

static inline float reloq_body_extension_sample(struct reloq_body_extension *s,
    float classic, float variant_a, float amount, double gain, double pole)
{
    double difference=.8*(double)variant_a-classic;
    double high=gain*(difference-s->previous_difference)+pole*s->previous_high;
    s->previous_difference=difference;
    s->previous_high=high;
    return (float)(amount*high);
}

#endif
