#include "reloq_acoustic_curve.h"

#include <limits.h>
#include <math.h>

int reloq_acoustic_begin(const struct reloq_acoustic_curve *curve,
                         struct reloq_acoustic_cursor *cursor)
{
    if (!curve || !cursor || !isfinite(curve->default_value) ||
        (curve->point_count && !curve->points)) return -1;
    for (size_t i = 1; i < curve->point_count; ++i) {
        const struct reloq_acoustic_point *left = &curve->points[i - 1];
        const struct reloq_acoustic_point *right = &curve->points[i];
        if (left->tick > right->tick) return -1;
        if (left->tick == right->tick) continue;
        uint64_t span = (uint64_t)right->tick - (uint64_t)left->tick;
        int64_t difference = (int64_t)right->value - left->value;
        if (span > INT64_MAX || difference < INT32_MIN ||
            difference > INT32_MAX) return -1;
    }
    const struct reloq_acoustic_cursor initial = {curve, 0, 0, 0};
    *cursor = initial;
    return 0;
}

int reloq_acoustic_value(struct reloq_acoustic_cursor *cursor,
                         int64_t tick, float *output)
{
    if (!cursor || !cursor->curve || !output ||
        (cursor->have_last_tick && tick < cursor->last_tick)) return -1;
    const struct reloq_acoustic_curve *curve = cursor->curve;
    if (curve->point_count && !curve->points) return -1;
    struct reloq_acoustic_cursor next = *cursor;
    float value = curve->default_value;
    if (curve->point_count) {
        const struct reloq_acoustic_point *points = curve->points;
        if (tick < points[0].tick) {
            value = (float)points[0].value;
        } else {
            size_t i = next.left_index;
            if (i >= curve->point_count) return -1;
            /* Reach the first target at a duplicated tick, then step over
             * its later same-time replacements on the next frame tick. */
            while (i + 1 < curve->point_count &&
                   points[i + 1].tick < tick) ++i;
            if (i + 1 < curve->point_count &&
                points[i].tick < tick && points[i + 1].tick == tick) ++i;
            next.left_index = i;
            value = (float)points[i].value;
            if (i + 1 < curve->point_count &&
                points[i + 1].tick > tick) {
                const struct reloq_acoustic_point *left = &points[i];
                const struct reloq_acoustic_point *right = &points[i + 1];
                uint64_t span = (uint64_t)right->tick -
                                (uint64_t)left->tick;
                uint64_t elapsed = (uint64_t)tick -
                                   (uint64_t)left->tick;
                if (!span || span > INT64_MAX || elapsed > INT64_MAX)
                    return -1;
                int32_t difference = (int32_t)((int64_t)right->value -
                                               left->value);
                float offset = (float)(int64_t)elapsed;
                offset *= (float)difference;
                offset /= (float)(int64_t)span;
                value += offset;
                if (!isfinite(value) ||
                    (double)value < (double)INT32_MIN ||
                    (double)value >= 2147483648.0) return -1;
                value = (float)(int32_t)value;
            }
        }
    }
    next.last_tick = tick;
    next.have_last_tick = 1;
    *cursor = next;
    *output = value;
    return 0;
}
