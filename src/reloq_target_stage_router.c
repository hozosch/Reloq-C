#include "reloq_target_stage_router.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static enum reloq_target_stage_role owner(enum reloq_target_stream stream)
{
    if (stream >= RELOQ_TARGET_F1 && stream <= RELOQ_TARGET_F3)
        return RELOQ_STAGE_TRACT;
    if (stream == RELOQ_TARGET_AV || stream == RELOQ_TARGET_AH)
        return RELOQ_STAGE_VOICED;
    if (stream >= RELOQ_TARGET_AF && stream <= RELOQ_TARGET_AB)
        return RELOQ_STAGE_NOISE;
    return RELOQ_STAGE_REMAINDER;
}

static int begin(
    struct reloq_target_stage_router *router,
    const struct reloq_target_builder *destination,int borrowed)
{
    if (!router || !destination || router->ready) return -1;
    size_t capacity[RELOQ_TARGET_STREAM_COUNT];
    size_t total = 0;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        const struct reloq_target_buffer *dest = &destination->stream[i];
        if (dest->count > dest->capacity ||
            (dest->capacity && !dest->points) ||
            (borrowed && dest->count)) return -1;
        /* The generic phrase melody alone owns F0. */
        capacity[i] = i == RELOQ_TARGET_F0 ? 0 :
            dest->capacity - dest->count;
        if (capacity[i] > SIZE_MAX - total) return -1;
        total += capacity[i];
    }
    if (total > SIZE_MAX / sizeof *router->storage) return -1;
    struct reloq_acoustic_point *storage = NULL;
    if (borrowed) {
        uintptr_t end=0;
        for(size_t i=1;i<RELOQ_TARGET_STREAM_COUNT;++i) {
            if(!capacity[i])continue;
            uintptr_t start=(uintptr_t)destination->stream[i].points;
            size_t bytes=capacity[i]*sizeof *storage;
            if(bytes>UINTPTR_MAX-start || (storage && start!=end))return -1;
            if(!storage)storage=destination->stream[i].points;
            end=start+bytes;
        }
    } else if(total)storage=malloc(total*sizeof *storage);
    if (total && !storage) return -1;
    if(borrowed && total) {
        uintptr_t start=(uintptr_t)storage,object=(uintptr_t)router;
        size_t bytes=total*sizeof *storage;
        if(object>UINTPTR_MAX-sizeof *router ||
            (start<object+sizeof *router && object<start+bytes))return -1;
    }
    struct reloq_target_stage_router next = {0};
    next.storage = storage;
    next.storage_capacity = total;
    next.borrowed=borrowed;
    struct reloq_target_buffer buffers[RELOQ_STAGE_ROLE_COUNT]
                                      [RELOQ_TARGET_STREAM_COUNT] = {{{0}}};
    size_t offset = 0;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        enum reloq_target_stage_role role = owner((enum reloq_target_stream)i);
        next.mask[role] |= UINT32_C(1) << i;
        if (!capacity[i]) continue;
        buffers[role][i] = (struct reloq_target_buffer){
            storage + offset, capacity[i], 0};
        offset += capacity[i];
    }
    for (size_t r = 0; r < RELOQ_STAGE_ROLE_COUNT; ++r)
        if (reloq_target_builder_begin(&next.stage[r], buffers[r])) {
            if(!borrowed)free(storage);
            return -1;
        }
    next.ready = 1;
    *router = next;
    return 0;
}

int reloq_target_stage_router_begin(
    struct reloq_target_stage_router *router,
    const struct reloq_target_builder *destination)
{return begin(router,destination,0);}

int reloq_target_stage_router_begin_borrowed(
    struct reloq_target_stage_router *router,
    const struct reloq_target_builder *destination)
{return begin(router,destination,1);}

int reloq_target_stage_router_append_at(
    struct reloq_target_stage_router *router,
    const int64_t origin_by_role[RELOQ_STAGE_ROLE_COUNT],
    const struct reloq_target_event *events, size_t event_count)
{
    if (!router || !router->ready || router->merged || !origin_by_role ||
        (event_count && !events)) return -1;
    if (event_count > SIZE_MAX / sizeof *events) return -1;
    /* An origin array supplied from a mutable stage buffer must not change
     * between validation and commit. Keep the complete gesture geometry
     * in local storage, just as event batches exclude stage-buffer aliases. */
    int64_t origins[RELOQ_STAGE_ROLE_COUNT];
    memcpy(origins, origin_by_role, sizeof origins);
    /* The storage is private to this router, so each stream's owner and
     * buffer were checked at begin. Validate one complete batch before
     * committing it. This also avoids four independent full-builder scans
     * per 1-ms profile gesture in long NVDA requests. */
    uintptr_t event_start = (uintptr_t)events;
    size_t bytes = event_count * sizeof *events;
    uintptr_t own_start = (uintptr_t)router->storage;
    size_t own_bytes = router->storage_capacity * sizeof *router->storage;
    uintptr_t router_start = (uintptr_t)router;
    if (bytes > UINTPTR_MAX - event_start ||
        own_bytes > UINTPTR_MAX - own_start ||
        (bytes && ((own_bytes && event_start < own_start + own_bytes &&
                    own_start < event_start + bytes) ||
                   (event_start < router_start + sizeof *router &&
                    router_start < event_start + bytes)))) return -1;
    /* Source envelopes submit single knots very frequently. Validate that
     * knot directly instead of clearing and traversing per-stream batch
     * bookkeeping. The same capacity, overflow and transition rules apply. */
    if (event_count == 1) {
        enum reloq_target_stream stream = events[0].stream;
        if (stream <= RELOQ_TARGET_F0 || stream >= RELOQ_TARGET_STREAM_COUNT)
            return -1;
        enum reloq_target_stage_role role = owner(stream);
        struct reloq_target_buffer *buffer = &router->stage[role].stream[stream];
        int64_t origin = origins[role], offset = events[0].offset;
        if (buffer->count == buffer->capacity ||
            (offset > 0 && origin > INT64_MAX - offset) ||
            (offset < 0 && origin < INT64_MIN - offset)) return -1;
        struct reloq_acoustic_point point = {origin + offset, events[0].value};
        if (buffer->count) {
            const struct reloq_acoustic_point *prior = buffer->points + buffer->count - 1;
            if (point.tick < prior->tick) return -1;
            if (point.tick != prior->tick) {
                uint64_t span = (uint64_t)point.tick - (uint64_t)prior->tick;
                int64_t difference = (int64_t)point.value - prior->value;
                if (span > INT64_MAX || difference < INT32_MIN ||
                    difference > INT32_MAX) return -1;
            }
        }
        buffer->points[buffer->count++] = point;
        return 0;
    }
    size_t additions[RELOQ_TARGET_STREAM_COUNT] = {0};
    struct reloq_acoustic_point last[RELOQ_TARGET_STREAM_COUNT];
    unsigned char has_last[RELOQ_TARGET_STREAM_COUNT] = {0};
    for (size_t i = 0; i < event_count; ++i) {
        enum reloq_target_stream stream = events[i].stream;
        if (stream <= RELOQ_TARGET_F0 || stream >= RELOQ_TARGET_STREAM_COUNT)
            return -1;
        size_t s = (size_t)stream;
        struct reloq_target_buffer *buffer =
            &router->stage[owner(stream)].stream[s];
        int64_t origin = origins[owner(stream)];
        if (additions[s] >= buffer->capacity - buffer->count ||
            (events[i].offset > 0 && origin > INT64_MAX - events[i].offset) ||
            (events[i].offset < 0 && origin < INT64_MIN - events[i].offset))
            return -1;
        struct reloq_acoustic_point point = {
            origin + events[i].offset, events[i].value};
        struct reloq_acoustic_point prior;
        if (has_last[s]) prior = last[s];
        else if (buffer->count) prior = buffer->points[buffer->count - 1];
        else goto first_point;
        if (point.tick < prior.tick) return -1;
        if (point.tick != prior.tick) {
            uint64_t span = (uint64_t)point.tick - (uint64_t)prior.tick;
            int64_t difference = (int64_t)point.value - prior.value;
            if (span > INT64_MAX || difference < INT32_MIN ||
                difference > INT32_MAX) return -1;
        }
first_point:
        last[s] = point;
        has_last[s] = 1;
        ++additions[s];
    }
    for (size_t i = 0; i < event_count; ++i) {
        enum reloq_target_stream stream = events[i].stream;
        struct reloq_target_buffer *buffer =
            &router->stage[owner(stream)].stream[(size_t)stream];
        int64_t origin = origins[owner(stream)];
        buffer->points[buffer->count++] =
            (struct reloq_acoustic_point){
                origin + events[i].offset, events[i].value};
    }
    return 0;
}

int reloq_target_stage_router_append(
    struct reloq_target_stage_router *router, int64_t origin,
    const struct reloq_target_event *events, size_t event_count)
{
    const int64_t origin_by_role[RELOQ_STAGE_ROLE_COUNT] = {
        origin, origin, origin, origin};
    return reloq_target_stage_router_append_at(
        router, origin_by_role, events, event_count);
}

int reloq_target_stage_router_merge(
    struct reloq_target_stage_router *router,
    struct reloq_target_builder *destination)
{
    if (!router || !router->ready || router->merged || !destination)
        return -1;
    if(router->borrowed) {
        /* Each stream has one owner. Its data is already in its final
         * buffer, and append validated every transition. Verify all
         * bindings before publishing any of the destination counts. */
        for(size_t i=0;i<RELOQ_TARGET_STREAM_COUNT;++i) {
            const struct reloq_target_buffer *src=
                &router->stage[owner((enum reloq_target_stream)i)].stream[i];
            const struct reloq_target_buffer *dest=&destination->stream[i];
            if(dest->count)return -1;
            if(i==RELOQ_TARGET_F0)continue;
            if(dest->capacity!=src->capacity || src->count>src->capacity ||
                (src->capacity && dest->points!=src->points) ||
                (src->count && src->points[0].tick<0))return -1;
        }
        for(size_t i=1;i<RELOQ_TARGET_STREAM_COUNT;++i)
            destination->stream[i].count=
                router->stage[owner((enum reloq_target_stream)i)].stream[i].count;
        router->merged=1;
        return 0;
    }
    struct reloq_target_stage stages[RELOQ_STAGE_ROLE_COUNT];
    for (size_t r = 0; r < RELOQ_STAGE_ROLE_COUNT; ++r)
        stages[r] = (struct reloq_target_stage){
            &router->stage[r], 0, router->mask[r]};
    if (reloq_target_stage_merge(destination, stages,
                                 RELOQ_STAGE_ROLE_COUNT, NULL, NULL))
        return -1;
    router->merged = 1;
    return 0;
}

void reloq_target_stage_router_dispose(
    struct reloq_target_stage_router *router)
{
    if (!router) return;
    if(!router->borrowed)free(router->storage);
    memset(router, 0, sizeof *router);
}
