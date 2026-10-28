#ifndef RELOQ_NOISE_PROFILE_H
#define RELOQ_NOISE_PROFILE_H
#include <stddef.h>
/* AF-only extension. The oral cascade, aspiration, phoneme levels and
 * release clocks remain owned by the reference synthesis path. */
struct reloq_noise_profile {
    double coefficient[6][5], target[6][5], step[6][5], delay[6][2];
    double upper_gain, upper_target, upper_step;
    float rate;
    float upper_frequency[3], upper_width[3];
    int kind, ready;
    unsigned remaining;
};
void reloq_noise_profile_prepare(struct reloq_noise_profile *, float rate,
    long av, long af, long ab, float centers[8], float widths[8], long levels[8],
    long *count);
void reloq_noise_profile_process(struct reloq_noise_profile *,float *,size_t);
#endif
