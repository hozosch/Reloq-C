#ifndef RELOQ_TARGET_STAGE_ROUTER_H
#define RELOQ_TARGET_STAGE_ROUTER_H

#include "reloq_target_stage_merge.h"

/* A phrase can plan the vocal source, noise source and low vocal tract on
 * separate timelines. Each target stream has exactly one owner, so its
 * duplicate-tick step order is retained when the stages are merged. F0 is
 * reserved for the phrase melody, appended to the destination afterward. */
enum reloq_target_stage_role {
    RELOQ_STAGE_TRACT = 0,      /* F1, F2, F3 */
    RELOQ_STAGE_VOICED,         /* AV, AH */
    RELOQ_STAGE_NOISE,          /* AF, A1F..AB */
    RELOQ_STAGE_REMAINDER,      /* other profile streams, including OQ */
    RELOQ_STAGE_ROLE_COUNT
};

struct reloq_target_stage_router {
    struct reloq_target_builder stage[RELOQ_STAGE_ROLE_COUNT];
    struct reloq_acoustic_point *storage;
    size_t storage_capacity;
    uint32_t mask[RELOQ_STAGE_ROLE_COUNT];
    int ready;
    int merged;
    int borrowed;
};

/* Allocate staging point storage from the destination's remaining capacity.
 * A zero-initialized router may be reused after dispose. Once begun, the
 * destination must not change before merge. */
int reloq_target_stage_router_begin(
    struct reloq_target_stage_router *router,
    const struct reloq_target_builder *destination);

/* Plan into an empty destination's contiguous point storage. Stage counts
 * remain private until merge; dispose leaves the caller's storage owned by
 * the caller. Discard its uncommitted points after a planning failure.
 * All non-F0 buffers must be contiguous in stream order. This avoids a
 * second phrase-sized point allocation and the final point copy. */
int reloq_target_stage_router_begin_borrowed(
    struct reloq_target_stage_router *router,
    const struct reloq_target_builder *destination);

/* Gesture events may be interleaved between roles in any order, but points
 * within each stream retain target_builder's nondecreasing-tick rule. An
 * invalid batch changes neither private stages nor destination. */
int reloq_target_stage_router_append(
    struct reloq_target_stage_router *router, int64_t origin,
    const struct reloq_target_event *events, size_t event_count);

/* Append one landmark with an independent origin for each target stage.
 * An event's absolute tick is origin_by_role[owner(event.stream)] +
 * event.offset. The tract, voiced source and noise source may therefore
 * begin or end their transition at different ticks. Each stream still has
 * one owner and must receive nondecreasing targets. The whole batch is
 * validated before any stage changes; on failure it is discarded.
 *
 * For the original shared-timeline geometry, use append() above; that
 * wrapper supplies the same origin for every role. F0 remains reserved for
 * the phrase melody in both entry points. */
int reloq_target_stage_router_append_at(
    struct reloq_target_stage_router *router,
    const int64_t origin_by_role[RELOQ_STAGE_ROLE_COUNT],
    const struct reloq_target_event *events, size_t event_count);

int reloq_target_stage_router_merge(
    struct reloq_target_stage_router *router,
    struct reloq_target_builder *destination);

void reloq_target_stage_router_dispose(
    struct reloq_target_stage_router *router);

#endif
