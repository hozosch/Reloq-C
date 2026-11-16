/* Cross-platform planning of the selected Persian/English frame transitions.
 * The caller owns the input, output and all request-local timing data. */
#ifndef RELOQ_PERSIAN_TRANSITION_H
#define RELOQ_PERSIAN_TRANSITION_H

#include <stddef.h>
#include <stdint.h>

struct reloq_fa_frame {
    int64_t tick;
    float controls[62];
    float gain;
    int32_t formants, head, gender;
    int32_t s_frication, limit_frication;
};

struct reloq_fa_frame_mark {
    uint32_t frame;
    int32_t value;
};

/* ends contains one cumulative frame count per segment, times contains
 * frame_count+1 cumulative times, and skip/changed contain segment_count
 * bytes. The caller preselects the eligible English-to-Persian boundaries.
 * Input and output must not overlap. Output is populated only when at least
 * one boundary changes. No results survive this call. */
int reloq_persian_bridge_continuation(const struct reloq_fa_frame *input,
    struct reloq_fa_frame *output, const double *times, const size_t *ends,
    const unsigned char *skip, size_t frame_count, size_t segment_count,
    double window_ms, unsigned char *changed);

#endif
