#include "reloq_parameter_stream.h"

#include <limits.h>
#include <math.h>

int reloq_parameter_begin(const struct reloq_parameter_schedule *schedule,
                          struct reloq_parameter_cursor *cursor)
{
    if (!schedule || !cursor || schedule->frame_ticks == 0 ||
        schedule->first_tick > schedule->end_tick)
        return -1;
    for (size_t p = 0; p < RELOQ_PARAMETER_TRACKS; ++p) {
        const struct reloq_parameter_track *track = &schedule->tracks[p];
        if (!isfinite(track->baseline) ||
            (track->knot_count != 0 && !track->knots))
            return -1;
        for (size_t i = 1; i < track->knot_count; ++i) {
            if (track->knots[i - 1].tick >= track->knots[i].tick)
                return -1;
            uint64_t span = (uint64_t)track->knots[i].tick -
                            (uint64_t)track->knots[i - 1].tick;
            if (span > INT64_MAX) return -1;
            int64_t difference = (int64_t)track->knots[i].value -
                                 (int64_t)track->knots[i - 1].value;
            if (difference < INT32_MIN || difference > INT32_MAX)
                return -1;
        }
    }
    struct reloq_parameter_cursor initial = {0};
    initial.schedule = schedule;
    initial.next_tick = schedule->first_tick;
    *cursor = initial;
    return 0;
}

static int track_value(const struct reloq_parameter_track *track,
                       size_t *index, int64_t tick, float *result)
{
    if (!track->knot_count || tick < track->knots[0].tick) {
        *result = track->baseline;
        return 0;
    }
    size_t i = *index;
    while (i + 1 < track->knot_count && track->knots[i + 1].tick <= tick)
        ++i;
    *index = i;
    if (i + 1 == track->knot_count) {
        *result = (float)track->knots[i].value;
        return 0;
    }
    const struct reloq_parameter_knot *left = &track->knots[i];
    const struct reloq_parameter_knot *right = &track->knots[i + 1];
    /* Subtract in unsigned arithmetic after ordering so even a span from
     * INT64_MIN to INT64_MAX is defined. Refuse spans that cannot be used
     * as a signed time difference in the interpolation. */
    uint64_t span = (uint64_t)right->tick - (uint64_t)left->tick;
    uint64_t elapsed = (uint64_t)tick - (uint64_t)left->tick;
    if (span > INT64_MAX || elapsed > INT64_MAX) return -1;
    int32_t difference = (int32_t)((int64_t)right->value - left->value);
    float offset = (float)(int64_t)elapsed;
    offset *= (float)difference;
    offset /= (float)(int64_t)span;
    float value = (float)left->value + offset;
    /* C conversion from an out-of-range float to int is undefined. The
     * upper endpoint rounds to 2^31 in float, hence the exclusive bound. */
    if (!isfinite(value) || (double)value < (double)INT32_MIN ||
        (double)value >= 2147483648.0)
        return -1;
    *result = (float)(int32_t)value;
    return 0;
}

int reloq_parameter_next(struct reloq_parameter_cursor *cursor,
                         struct reloq_parameter_frame *frame)
{
    if (!cursor || !frame || !cursor->schedule) return -1;
    const struct reloq_parameter_schedule *schedule = cursor->schedule;
    if (cursor->next_tick < schedule->first_tick ||
        cursor->next_tick > schedule->end_tick || !schedule->frame_ticks)
        return -1;
    if (cursor->next_tick == schedule->end_tick) return 0;

    /* Work on copies so a malformed range never advances a partial frame. */
    struct reloq_parameter_cursor next = *cursor;
    struct reloq_parameter_frame output = {0};
    output.tick = next.next_tick;
    uint64_t remaining = (uint64_t)schedule->end_tick -
                         (uint64_t)next.next_tick;
    output.duration_ticks = remaining < schedule->frame_ticks ?
                            (uint32_t)remaining : schedule->frame_ticks;
    for (size_t p = 0; p < RELOQ_PARAMETER_TRACKS; ++p) {
        const struct reloq_parameter_track *track = &schedule->tracks[p];
        if (track->knot_count &&
            (next.knot_indices[p] >= track->knot_count || !track->knots))
            return -1;
        if (track_value(track, &next.knot_indices[p], output.tick,
                        &output.parameters[p]))
            return -1;
    }
    /* The duration is bounded by end_tick - next_tick, so signed addition
     * is safe even when a timeline starts below zero. */
    next.next_tick += output.duration_ticks;
    *cursor = next;
    *frame = output;
    return 1;
}
