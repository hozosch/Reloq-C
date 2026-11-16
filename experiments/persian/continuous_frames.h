/* Private, request-scoped native frame capture for Persian sentence planning. */
#ifndef RELOQ_PERSIAN_CONTINUOUS_FRAMES_H
#define RELOQ_PERSIAN_CONTINUOUS_FRAMES_H

#include "../../src/reloq_persian_transition.h"

/* The capture is private to the current synthesis thread. No callbacks cross
 * the language boundary; rows remain valid until the next begin/release. */
int reloq_fa_capture_append(int64_t tick, const float controls[62],
    float duration, float gain, int formants, int head, int gender,
    int s_frication, int limit_frication);
int reloq_fa_capture_controls_only(void);
int reloq_fa_capture_advance_samples(size_t count);

#endif
