#ifndef RELOQ_EQUALIZER_H
#define RELOQ_EQUALIZER_H

#include <stddef.h>
#include <stdint.h>

struct reloq_equalizer_band {
    double b0, b1, b2, a1, a2, z1, z2;
};
struct reloq_equalizer {
    struct reloq_equalizer_band bands[3];
    double preamp;
    int active;
};

/* Three independent gains in dB (-12..12), at the physical PCM rate.
 * A flat response is a bit-exact bypass. State is local to one utterance. */
int reloq_equalizer_init(struct reloq_equalizer *, int sample_rate,
                         int low, int mid, int high);
void reloq_equalizer_process(struct reloq_equalizer *, int16_t *, size_t);

#endif
