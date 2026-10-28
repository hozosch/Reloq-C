#ifndef RELOQ_ACOUSTIC_CURVE_H
#define RELOQ_ACOUSTIC_CURVE_H

#include <stddef.h>
#include <stdint.h>

/* One acoustic target curve before its numeric Klatt frames are emitted.
 * Ordered points may share a tick: at that tick the first point wins; on the
 * following tick the last point governs interpolation onward. This models
 * the reference's frame-step boundary without inventing a fractional time.
 * The caller owns the points and retains them while a cursor is active. */
struct reloq_acoustic_point {
    int64_t tick;
    int32_t value;
};

struct reloq_acoustic_curve {
    float default_value; /* Used only if there are no points. */
    const struct reloq_acoustic_point *points;
    size_t point_count;
};

struct reloq_acoustic_cursor {
    const struct reloq_acoustic_curve *curve;
    size_t left_index;
    int64_t last_tick;
    int have_last_tick;
};

/* Return -1 on invalid input and leave the cursor unchanged. A duplicated
 * tick is valid; descending ticks and an unrepresentable difference/span
 * are not. */
int reloq_acoustic_begin(const struct reloq_acoustic_curve *curve,
                         struct reloq_acoustic_cursor *cursor);

/* Query at nondecreasing integer ticks. Return the interpolated numeric
 * control after single-precision arithmetic and truncation toward zero.
 * Before the first point the first value holds; after the last the last
 * value holds. On error the cursor and output stay unchanged. */
int reloq_acoustic_value(struct reloq_acoustic_cursor *cursor,
                         int64_t tick, float *output);

#endif
