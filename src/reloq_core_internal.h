#ifndef RELOQ_CORE_INTERNAL_H
#define RELOQ_CORE_INTERNAL_H

#include "reloq_core.h"

/* The session exclusively owns this state and its separate sample buffers.
 * Render directly into that state, without the public API's transaction
 * copy. After ANY rendering error the owner must reset the state before
 * rendering again, as required by reloq_session.h. Input validation,
 * arithmetic, filter order and output checks are shared with the public
 * transactional entry point below. This is not a public core interface. */
int reloq_core_voiced_frame_render_owned(
    struct reloq_core_voiced_frame_state *state,
    const struct reloq_core_voiced_frame *frame,
    float *output, float *raw_af, float *scratch, uint8_t *closed_mask,
    int16_t *pcm, size_t capacity, size_t *written);

#endif
