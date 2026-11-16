#ifndef RELOQ_PERSIAN_FRAME_INTERNAL_H
#define RELOQ_PERSIAN_FRAME_INTERNAL_H

#include <stddef.h>
#include <stdint.h>
#include "reloq_persian_frames.h"

/* Capture storage is thread-local and released after each request. */
int reloq_fa_capture_begin(size_t maximum);
const struct reloq_fa_frame *reloq_fa_capture_end(size_t *count, int *failed);
void reloq_fa_capture_release(void);
int reloq_fa_capture_set_controls_only(int enabled);
int reloq_fa_capture_controls_only(void);
int reloq_fa_capture_active(void);
int reloq_fa_capture_advance_samples(size_t count);
size_t reloq_fa_capture_sample_count(void);
int reloq_fa_capture_append(int64_t tick, const float controls[62],
    float duration, float gain, int formants, int head, int gender,
    int s_frication, int limit_frication);

#endif
