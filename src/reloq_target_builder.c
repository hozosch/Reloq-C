#include "reloq_target_builder.h"

#include <limits.h>
#include <math.h>
#include <string.h>

static int valid_buffer(const struct reloq_target_buffer *buffer)
{
    if (buffer->count > buffer->capacity ||
        (buffer->capacity && !buffer->points)) return 0;
    struct reloq_acoustic_curve curve = {0.0f, buffer->points, buffer->count};
    struct reloq_acoustic_cursor cursor;
    return reloq_acoustic_begin(&curve, &cursor) == 0;
}

static int valid_header(const struct reloq_target_buffer *buffer)
{
    return buffer->count <= buffer->capacity &&
           (!buffer->capacity || buffer->points);
}

/* On the supported flat-address hosts uintptr_t preserves address ordering.
 * Reject wrapped ranges before doing any overlap comparison. */
static int byte_range(const void *memory, size_t bytes,
                      uintptr_t *start, uintptr_t *end)
{
    *start = (uintptr_t)memory;
    if (bytes > UINTPTR_MAX - *start) return -1;
    *end = *start + bytes;
    return 0;
}

/* Check the reserved range, including space for points appended later. */
static int point_range(const struct reloq_target_buffer *buffer,
                       uintptr_t *start, uintptr_t *end)
{
    if (!buffer->capacity) {
        *start = *end = 0;
        return 0;
    }
    if (!buffer->points ||
        buffer->capacity > SIZE_MAX / sizeof *buffer->points) return -1;
    size_t bytes = buffer->capacity * sizeof *buffer->points;
    return byte_range(buffer->points, bytes, start, end);
}

int reloq_target_builder_begin(
    struct reloq_target_builder *builder,
    const struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT])
{
    if (!builder || !storage) return -1;
    uintptr_t starts[RELOQ_TARGET_STREAM_COUNT];
    uintptr_t ends[RELOQ_TARGET_STREAM_COUNT];
    uintptr_t builder_start, builder_end;
    if (byte_range(builder, sizeof *builder, &builder_start, &builder_end))
        return -1;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        if (!valid_header(&storage[i]) ||
            point_range(&storage[i], &starts[i], &ends[i])) return -1;
        if (storage[i].capacity &&
            starts[i] < builder_end && builder_start < ends[i]) return -1;
        for (size_t j = 0; j < i; ++j)
            if (storage[i].capacity && storage[j].capacity &&
                starts[i] < ends[j] &&
                starts[j] < ends[i]) return -1;
        if (!valid_buffer(&storage[i])) return -1;
    }
    memmove(builder->stream, storage, sizeof builder->stream);
    return 0;
}

static int add_tick(int64_t origin, int64_t offset, int64_t *tick)
{
    if ((offset > 0 && origin > INT64_MAX - offset) ||
        (offset < 0 && origin < INT64_MIN - offset)) return -1;
    *tick = origin + offset;
    return 0;
}

static int valid_transition(struct reloq_acoustic_point left,
                            struct reloq_acoustic_point right)
{
    if (right.tick < left.tick) return 0;
    if (right.tick == left.tick) return 1;
    uint64_t span = (uint64_t)right.tick - (uint64_t)left.tick;
    int64_t difference = (int64_t)right.value - left.value;
    return span <= INT64_MAX && difference >= INT32_MIN &&
           difference <= INT32_MAX;
}

int reloq_target_builder_append(
    struct reloq_target_builder *builder, int64_t origin,
    const struct reloq_target_event *events, size_t event_count)
{
    if (!builder || (event_count && !events)) return -1;
    if (event_count) {
        uintptr_t event_start, event_end, storage_start, storage_end;
        if (event_count > SIZE_MAX / sizeof *events ||
            byte_range(events, event_count * sizeof *events,
                       &event_start, &event_end) ||
            byte_range(builder, sizeof *builder,
                       &storage_start, &storage_end)) return -1;
        if (event_start < storage_end && storage_start < event_end)
            return -1;
        for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
            if (point_range(&builder->stream[i], &storage_start,
                            &storage_end)) return -1;
            if (builder->stream[i].capacity &&
                event_start < storage_end && storage_start < event_end)
                return -1;
        }
    }
    size_t additions[RELOQ_TARGET_STREAM_COUNT] = {0};
    struct reloq_acoustic_point last[RELOQ_TARGET_STREAM_COUNT];
    unsigned char has_last[RELOQ_TARGET_STREAM_COUNT] = {0};
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        const struct reloq_target_buffer *buffer = &builder->stream[i];
        if (!valid_header(buffer)) return -1;
        if (buffer->count) {
            last[i] = buffer->points[buffer->count - 1];
            has_last[i] = 1;
        }
    }
    for (size_t i = 0; i < event_count; ++i) {
        enum reloq_target_stream stream = events[i].stream;
        if (stream < 0 || stream >= RELOQ_TARGET_STREAM_COUNT) return -1;
        size_t index = (size_t)stream;
        const struct reloq_target_buffer *buffer = &builder->stream[index];
        if (additions[index] >= buffer->capacity - buffer->count) return -1;
        struct reloq_acoustic_point point;
        if (add_tick(origin, events[i].offset, &point.tick)) return -1;
        point.value = events[i].value;
        if (has_last[index] && !valid_transition(last[index], point))
            return -1;
        last[index] = point;
        has_last[index] = 1;
        ++additions[index];
    }
    for (size_t i = 0; i < event_count; ++i) {
        enum reloq_target_stream stream = events[i].stream;
        struct reloq_target_buffer *buffer = &builder->stream[(size_t)stream];
        struct reloq_acoustic_point *point = &buffer->points[buffer->count++];
        (void)add_tick(origin, events[i].offset, &point->tick);
        point->value = events[i].value;
    }
    return 0;
}

int reloq_target_builder_export(
    const struct reloq_target_builder *builder,
    const uint8_t field_for_stream[RELOQ_TARGET_STREAM_COUNT],
    const float defaults[RELOQ_PARAMETER_TRACKS],
    struct reloq_acoustic_curve curves[RELOQ_PARAMETER_TRACKS],
    unsigned char mapped[RELOQ_PARAMETER_TRACKS])
{
    if (!builder || !field_for_stream || !defaults || !curves || !mapped)
        return -1;
    struct reloq_acoustic_curve next[RELOQ_PARAMETER_TRACKS];
    unsigned char selected[RELOQ_PARAMETER_TRACKS] = {0};
    for (size_t i = 0; i < RELOQ_PARAMETER_TRACKS; ++i) {
        if (!isfinite(defaults[i])) return -1;
        next[i] = (struct reloq_acoustic_curve){defaults[i], NULL, 0};
    }
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        size_t field = field_for_stream[i];
        if (!field || field >= RELOQ_PARAMETER_TRACKS || selected[field] ||
            !valid_buffer(&builder->stream[i])) return -1;
        selected[field] = 1;
        next[field].points = builder->stream[i].points;
        next[field].point_count = builder->stream[i].count;
    }
    memcpy(curves, next, sizeof next);
    memcpy(mapped, selected, sizeof selected);
    return 0;
}

const char *reloq_target_stream_name(enum reloq_target_stream stream)
{
    static const char *const names[RELOQ_TARGET_STREAM_COUNT] = {
        "F0", "F1", "F2", "F3", "F4", "F5",
        "B1", "B2", "B3", "B4", "B5",
        "FNP", "FNZ", "FTP", "FTZ",
        "TL", "OQ", "FL", "DI", "AV", "AH", "AF",
        "A1F", "A2F", "A3F", "A4F", "A5F", "AB"
    };
    return stream >= 0 && stream < RELOQ_TARGET_STREAM_COUNT ?
           names[(size_t)stream] : NULL;
}
