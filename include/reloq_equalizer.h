#ifndef RELOQ_EQUALIZER_H
#define RELOQ_EQUALIZER_H

#include <stddef.h>
#include <stdint.h>

struct reloq_equalizer_band {
    double b0, b1, b2, a1, a2, z1, z2;
};
#define RELOQ_EQ_FILTER_COUNT 5
struct reloq_equalizer_peak {
    int enabled,frequency,gain,width_percent;
};
struct reloq_equalizer {
    struct reloq_equalizer_band bands[3+RELOQ_EQ_FILTER_COUNT];
    double limiter_gain, limiter_release;
    int active,band_count;
};

/* Three independent gains in dB (-12..12), at the physical PCM rate.
 * A flat response is a bit-exact bypass. State is local to one utterance. */
int reloq_equalizer_init(struct reloq_equalizer *, int sample_rate,
                         int low, int mid, int high);
/* Width is 10..400 percent of an octave. Frequencies at or above Nyquist
 * are bypassed at the current rate without changing the stored values. */
int reloq_equalizer_init_extended(struct reloq_equalizer *, int sample_rate,
    int low, int mid, int high, const struct reloq_equalizer_peak *peaks);
void reloq_equalizer_process(struct reloq_equalizer *, int16_t *, size_t);

#endif
