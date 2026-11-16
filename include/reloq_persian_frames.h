/* Request-local control frames used for Persian language continuations. */
#ifndef RELOQ_PERSIAN_FRAMES_H
#define RELOQ_PERSIAN_FRAMES_H

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

#endif
