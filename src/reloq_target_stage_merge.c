#include "reloq_target_stage_merge.h"

#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct stage_point {
    int64_t tick;
    int32_t value;
};

struct merge_head {
    struct stage_point point;
    size_t stage, next;
};

static int valid_transform(const struct reloq_target_stage_transform *t)
{
    return !t || (isfinite(t->time_scale) && t->time_scale > 0.0 &&
                  isfinite(t->f0_scale) && t->f0_scale > 0.0 &&
                  t->source_attenuation >= 0 &&
                  (t->mute == 0 || t->mute == 1));
}

int reloq_target_stage_scale_tick(
    int64_t nominal_tick, const struct reloq_target_stage_transform *transform,
    int64_t *output_tick)
{
    if (!output_tick || nominal_tick < 0 || !valid_transform(transform))
        return -1;
    if (!transform || transform->time_scale == 1.0) {
        *output_tick = nominal_tick;
        return 0;
    }
    double scaled = (double)nominal_tick * transform->time_scale;
    if (!isfinite(scaled) || scaled < 0.0 ||
        scaled >= (double)INT64_MAX - 1.0) return -1;
    *output_tick = (int64_t)llround(scaled);
    return 0;
}

static int scaled_point(const struct reloq_acoustic_point *source,
                        enum reloq_target_stream stream,
                        const struct reloq_target_stage *stage,
                        const struct reloq_target_stage_transform *transform,
                        struct stage_point *out)
{
    int64_t tick = source->tick;
    if ((stage->origin > 0 && tick > INT64_MAX - stage->origin) ||
        (stage->origin < 0 && tick < INT64_MIN - stage->origin) ||
        reloq_target_stage_scale_tick(tick + stage->origin, transform,
                                      &out->tick)) return -1;
    int32_t value = source->value;
    if (transform && stream == RELOQ_TARGET_F0) {
        double f0 = (double)value * transform->f0_scale;
        if (!isfinite(f0) || f0 < 0.0 || f0 > INT32_MAX - 1.0)
            return -1;
        value = (int32_t)lround(f0);
    } else if (transform && stream >= RELOQ_TARGET_AV && value > 0) {
        value = transform->mute ? 0 :
            value < transform->source_attenuation ? 0 :
            value - transform->source_attenuation;
    }
    out->value = value;
    return 0;
}

static int precedes(const struct merge_head *left,
                    const struct merge_head *right)
{
    return left->point.tick < right->point.tick ||
        (left->point.tick == right->point.tick && left->stage < right->stage);
}

static void sift_down(struct merge_head *heap, size_t count, size_t at)
{
    struct merge_head head = heap[at];
    while (at < count / 2) {
        size_t child = at * 2 + 1;
        if (child + 1 < count && precedes(heap + child + 1, heap + child))
            ++child;
        if (!precedes(heap + child, &head)) break;
        heap[at] = heap[child];
        at = child;
    }
    heap[at] = head;
}

static int valid_step(int64_t tick, int32_t value,
                      int64_t last_tick, int32_t last_value)
{
    if (tick < last_tick) return 0;
    if (tick == last_tick) return 1;
    uint64_t span = (uint64_t)tick - (uint64_t)last_tick;
    int64_t delta = (int64_t)value - last_value;
    return span <= INT64_MAX && delta >= INT32_MIN && delta <= INT32_MAX;
}

/* Inputs are already ordered, and positive time scaling cannot reverse
 * them. Keep only the next point of each stage; stage rank breaks ties.
 * The validation pass completes before any destination is written. */
static int merge_stream(struct reloq_target_buffer *dest, size_t stream,
    const struct reloq_target_stage *stages, size_t stage_count,
    const struct reloq_target_stage_transform *transform,
    struct merge_head *heap, int commit)
{
    size_t count = 0, used = 0;
    for (size_t i = 0; i < stage_count; ++i) {
        const struct reloq_target_buffer *src = &stages[i].source->stream[stream];
        if (!(stages[i].stream_mask & (UINT32_C(1) << stream)) || !src->count)
            continue;
        struct merge_head *head = heap + count++;
        head->stage = i;
        head->next = 1;
        if (scaled_point(src->points, (enum reloq_target_stream)stream,
                         stages + i, transform, &head->point)) return -1;
    }
    for (size_t i = count / 2; i; --i) sift_down(heap, count, i - 1);
    int64_t last_tick = dest->count ? dest->points[dest->count - 1].tick : 0;
    int32_t last_value = dest->count ? dest->points[dest->count - 1].value : 0;
    while (count) {
        struct merge_head head = heap[0];
        if ((dest->count || used) &&
            !valid_step(head.point.tick, head.point.value, last_tick, last_value))
            return -1;
        if (commit) dest->points[dest->count + used] =
            (struct reloq_acoustic_point){head.point.tick, head.point.value};
        ++used;
        last_tick = head.point.tick;
        last_value = head.point.value;
        const struct reloq_target_stage *stage = stages + head.stage;
        const struct reloq_target_buffer *src = &stage->source->stream[stream];
        if (head.next < src->count) {
            if (scaled_point(src->points + head.next++,
                (enum reloq_target_stream)stream, stage, transform, &head.point))
                return -1;
            heap[0] = head;
        } else heap[0] = heap[--count];
        if (count) sift_down(heap, count, 0);
    }
    if (commit) dest->count += used;
    return 0;
}

static int range(const void *pointer, size_t count, size_t unit,
                 uintptr_t *start, uintptr_t *end)
{
    if (count > SIZE_MAX / unit) return -1;
    *start = (uintptr_t)pointer;
    if (count * unit > UINTPTR_MAX - *start) return -1;
    *end = *start + count * unit;
    return 0;
}

static int safe_add_count(size_t a, size_t b, size_t *sum)
{
    if (a > SIZE_MAX - b) return -1;
    *sum = a + b;
    return 0;
}

static int valid_stream(const struct reloq_target_buffer *stream)
{
    if (stream->count > stream->capacity ||
        (stream->capacity && !stream->points)) return 0;
    struct reloq_acoustic_curve curve = {0.0f, stream->points, stream->count};
    struct reloq_acoustic_cursor cursor;
    return reloq_acoustic_begin(&curve, &cursor) == 0;
}

int reloq_target_stage_merge(
    struct reloq_target_builder *destination,
    const struct reloq_target_stage *stages, size_t stage_count,
    const struct reloq_target_stage_transform *transform,
    size_t *point_count)
{
    if (!destination || (stage_count && !stages) ||
        !valid_transform(transform)) return -1;
    size_t selected[RELOQ_TARGET_STREAM_COUNT] = {0};
    size_t appended = 0;
    size_t single[RELOQ_TARGET_STREAM_COUNT];
    int needs_merge = 0;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        const struct reloq_target_buffer *dest = &destination->stream[s];
        if (!valid_stream(dest)) return -1;
        single[s] = SIZE_MAX;
        for (size_t i = 0; i < stage_count; ++i) {
            const struct reloq_target_stage *stage = stages + i;
            if (!stage->source || stage->source == destination ||
                (stage->stream_mask & ~RELOQ_TARGET_STAGE_ALL_STREAMS))
                return -1;
            if (!(stage->stream_mask & (UINT32_C(1) << s))) continue;
            const struct reloq_target_buffer *src =
                &stage->source->stream[s];
            if (!valid_stream(src) ||
                safe_add_count(selected[s], src->count, &selected[s]))
                return -1;
            if (!src->count) continue;
            single[s] = single[s] == SIZE_MAX ? i : stage_count;
            uintptr_t begin, end;
            if (range(src->points, src->count, sizeof *src->points,
                      &begin, &end)) return -1;
            for (size_t d = 0; d < RELOQ_TARGET_STREAM_COUNT; ++d) {
                const struct reloq_target_buffer *other =
                    &destination->stream[d];
                uintptr_t dest_begin, dest_end;
                if (range(other->points, other->capacity,
                          sizeof *other->points,
                          &dest_begin, &dest_end)) return -1;
                if (other->capacity && begin < dest_end &&
                    dest_begin < end) return -1;
            }
        }
        if (selected[s] > dest->capacity - dest->count ||
            safe_add_count(appended, selected[s], &appended)) return -1;
        if (!selected[s]) continue;
        if (single[s] < stage_count && !transform &&
            stages[single[s]].origin == 0) {
            const struct reloq_target_buffer *src =
                &stages[single[s]].source->stream[s];
            if (src->points[0].tick < 0 || (dest->count &&
                !valid_step(src->points[0].tick, src->points[0].value,
                    dest->points[dest->count - 1].tick,
                    dest->points[dest->count - 1].value))) return -1;
        } else {
            single[s] = stage_count;
            needs_merge = 1;
        }
    }
    struct merge_head local[8];
    struct merge_head *heap = local;
    if (needs_merge && stage_count > sizeof local / sizeof *local) {
        if (stage_count > SIZE_MAX / sizeof *heap) return -1;
        heap = malloc(stage_count * sizeof *heap);
        if (!heap) return -1;
    }

    /* First pass validates every stream without changing destination; the
     * second pass cannot fail because source storage must remain immutable. */
    for (int pass = 0; pass < 2; ++pass) {
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
            if (!selected[s]) continue;
            struct reloq_target_buffer *dest = &destination->stream[s];
            if (single[s] < stage_count) {
                if (pass) {
                    const struct reloq_target_buffer *src =
                        &stages[single[s]].source->stream[s];
                    memcpy(dest->points + dest->count, src->points,
                           src->count * sizeof *src->points);
                    dest->count += src->count;
                }
            } else if (merge_stream(dest, s, stages, stage_count,
                                     transform, heap, pass)) {
                goto failed;
            }
        }
    }
    if (heap != local) free(heap);
    if (point_count) *point_count = appended;
    return 0;
failed:
    if (heap != local) free(heap);
    return -1;
}
