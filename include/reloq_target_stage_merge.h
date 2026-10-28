#ifndef RELOQ_TARGET_STAGE_MERGE_H
#define RELOQ_TARGET_STAGE_MERGE_H

#include "reloq_target_builder.h"

#include <stdint.h>

/* A phonetic gesture owns the explicitly selected streams. Its source points
 * are in local nominal ticks; it does not own the destination's settings or
 * index callbacks. A zero bit deliberately discards that source stream, for
 * example a local F0 when a connected utterance uses global phrase F0. */
#define RELOQ_TARGET_STAGE_ALL_STREAMS \
    ((UINT32_C(1) << RELOQ_TARGET_STREAM_COUNT) - UINT32_C(1))

struct reloq_target_stage {
    const struct reloq_target_builder *source;
    int64_t origin;
    uint32_t stream_mask;
};

/* Apply output controls exactly once while copying nominal targets. NULL is
 * identity. Use time_scale=1 and f0_scale=1 for the reference C composer;
 * attenuation affects AV, AH, AF and frication sources but not formants.
 * The same time transform must be used for word/index landmarks. */
struct reloq_target_stage_transform {
    double time_scale;
    double f0_scale;
    int source_attenuation;
    int mute;
};

int reloq_target_stage_scale_tick(
    int64_t nominal_tick, const struct reloq_target_stage_transform *transform,
    int64_t *output_tick);

/* Stable composition: points are sorted per stream by transformed
 * tick, preserving source-stage order and same-stage insertion order when
 * ticks tie. The destination must already be bound with builder_begin and
 * have capacity for every selected point. A selected source range must not
 * overlap any destination storage. Earlier destination points precede tied
 * stage points. Returns zero on success, -1 on invalid input, range,
 * overlap, transition, or capacity error. Failure leaves all destination
 * counts and points unchanged. point_count receives only appended points. */
int reloq_target_stage_merge(
    struct reloq_target_builder *destination,
    const struct reloq_target_stage *stages, size_t stage_count,
    const struct reloq_target_stage_transform *transform,
    size_t *point_count);

#endif
