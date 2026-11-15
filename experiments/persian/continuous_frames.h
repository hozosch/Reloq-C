/* Private, request-scoped native frame capture for Persian sentence planning. */
#ifndef RELOQ_PERSIAN_CONTINUOUS_FRAMES_H
#define RELOQ_PERSIAN_CONTINUOUS_FRAMES_H

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

/* The capture is private to the current synthesis thread. No callbacks cross
 * the language boundary; rows remain valid until the next begin/release. */
int reloq_fa_capture_append(int64_t tick, const float controls[62],
    float duration, float gain, int formants, int head, int gender,
    int s_frication, int limit_frication);
int reloq_fa_capture_controls_only(void);
int reloq_fa_capture_advance_samples(size_t count);

#endif
