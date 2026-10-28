#ifndef RELOQ_TARGET_BUILDER_H
#define RELOQ_TARGET_BUILDER_H

#include "reloq_acoustic_curve.h"
#include "reloq_parameter_stream.h"

#include <stddef.h>
#include <stdint.h>

/* Named, ordered acoustic controls in the observed 28-stream map, whose
 * names and field placement matched ten tested European language modules.
 * Names are descriptive Klatt controls. A caller supplies the actual mapping
 * to its 62 renderer fields; the builder contains no language rule or value
 * table. */
enum reloq_target_stream {
    RELOQ_TARGET_F0 = 0,
    RELOQ_TARGET_F1, RELOQ_TARGET_F2, RELOQ_TARGET_F3,
    RELOQ_TARGET_F4, RELOQ_TARGET_F5,
    RELOQ_TARGET_B1, RELOQ_TARGET_B2, RELOQ_TARGET_B3,
    RELOQ_TARGET_B4, RELOQ_TARGET_B5,
    RELOQ_TARGET_FNP, RELOQ_TARGET_FNZ,
    RELOQ_TARGET_FTP, RELOQ_TARGET_FTZ,
    RELOQ_TARGET_TL, RELOQ_TARGET_OQ, RELOQ_TARGET_FL,
    RELOQ_TARGET_DI, RELOQ_TARGET_AV, RELOQ_TARGET_AH,
    RELOQ_TARGET_AF,
    RELOQ_TARGET_A1F, RELOQ_TARGET_A2F, RELOQ_TARGET_A3F,
    RELOQ_TARGET_A4F, RELOQ_TARGET_A5F, RELOQ_TARGET_AB,
    RELOQ_TARGET_STREAM_COUNT
};

struct reloq_target_buffer {
    struct reloq_acoustic_point *points; /* caller-owned mutable storage */
    size_t capacity;
    size_t count;
};

struct reloq_target_builder {
    struct reloq_target_buffer stream[RELOQ_TARGET_STREAM_COUNT];
};

/* One event relative to an independently chosen word/gesture origin tick.
 * Negative offsets permit anticipatory control targets. A batch is validated
 * in its given order, so repeated ticks retain insertion order. */
struct reloq_target_event {
    enum reloq_target_stream stream;
    int64_t offset;
    int32_t value;
};

/* Bind caller-owned, nonoverlapping point arrays, optionally containing
 * existing sorted points. Their full reserved ranges must also be disjoint
 * from the builder object. Do not mutate their contents or capacity outside
 * this builder while it is in use. Returns -1 on invalid storage and leaves
 * builder unchanged. */
int reloq_target_builder_begin(
    struct reloq_target_builder *builder,
    const struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT]);

/* Append an entire gesture batch. Every event's absolute tick is origin +
 * offset. Per-stream times may repeat but never decrease, and nonzero spans
 * and value differences must be valid for reloq_acoustic_value. On any error
 * (capacity, overflow, invalid event/order), neither builder counts nor
 * caller point arrays change. Events must not overlap any bound point array
 * or the builder object. Empty batches are valid with events == NULL. */
int reloq_target_builder_append(
    struct reloq_target_builder *builder, int64_t origin,
    const struct reloq_target_event *events, size_t event_count);

/* Build the 62 curves consumed by the existing scheduler and PCM pipeline.
 * field_for_stream contains 28 unique field indices in 1..61; field zero
 * remains a fixed frame-duration default. Defaults must be finite. Returned
 * point pointers borrow the builder's storage and remain valid until the
 * next append or caller storage mutation. On error outputs stay unchanged. */
int reloq_target_builder_export(
    const struct reloq_target_builder *builder,
    const uint8_t field_for_stream[RELOQ_TARGET_STREAM_COUNT],
    const float defaults[RELOQ_PARAMETER_TRACKS],
    struct reloq_acoustic_curve curves[RELOQ_PARAMETER_TRACKS],
    unsigned char mapped[RELOQ_PARAMETER_TRACKS]);

const char *reloq_target_stream_name(enum reloq_target_stream stream);

#endif
