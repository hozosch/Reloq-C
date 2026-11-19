/* Request-local control frames used for Persian language continuations. */
#ifndef RELOQ_PERSIAN_FRAMES_H
#define RELOQ_PERSIAN_FRAMES_H

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

/* Provider syllable edges, in the native 1/10000-ms clock. */
struct reloq_fa_syllable {
    int32_t from, to;
};

/* One verified U-to-A word boundary in the captured frame clock. */
struct reloq_fa_glide_candidate {
    uint32_t word;
    int64_t left, boundary, right;
};

int reloq_fa_apply_glides(struct reloq_fa_frame *rows,size_t count,
    const struct reloq_fa_glide_candidate *candidates,size_t candidate_count,
    int pitch,size_t *applied);

#endif
