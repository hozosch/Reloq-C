#ifndef RELOQ_PARAMETER_STREAM_H
#define RELOQ_PARAMETER_STREAM_H

#include <stddef.h>
#include <stdint.h>

/* A platform-independent timeline of numeric synthesis controls. Time is in
 * caller-defined integer ticks; a later layer chooses the tick duration and
 * maps the controls to typed synthesis-frame fields. No engine object layout
 * or annotation syntax is part of this interface. */
enum { RELOQ_PARAMETER_TRACKS = 62 };

struct reloq_parameter_knot {
    int64_t tick;
    int32_t value;
};

struct reloq_parameter_track {
    float baseline;
    const struct reloq_parameter_knot *knots;
    size_t knot_count;
};

struct reloq_parameter_schedule {
    int64_t first_tick;
    int64_t end_tick; /* exclusive */
    uint32_t frame_ticks;
    struct reloq_parameter_track tracks[RELOQ_PARAMETER_TRACKS];
};

struct reloq_parameter_cursor {
    const struct reloq_parameter_schedule *schedule;
    int64_t next_tick;
    size_t knot_indices[RELOQ_PARAMETER_TRACKS];
};

struct reloq_parameter_frame {
    int64_t tick;
    uint32_t duration_ticks;
    float parameters[RELOQ_PARAMETER_TRACKS];
};

/* Validate once before reading frames. Keep the schedule and its knot arrays
 * immutable and alive until the cursor is finished. Knots must be strictly
 * increasing in time. A baseline applies before the first knot; the last
 * knot's value holds after it. Return -1 for invalid inputs or ranges whose
 * interpolation is not representable without integer overflow. */
int reloq_parameter_begin(const struct reloq_parameter_schedule *schedule,
                          struct reloq_parameter_cursor *cursor);

/* Return 1 and advance one frame, 0 at the end, -1 on invalid state. On
 * error, neither the cursor nor the output frame changes. Between knots,
 * interpolate with single-precision arithmetic and truncate toward zero.
 * A final frame may be shorter than frame_ticks. */
int reloq_parameter_next(struct reloq_parameter_cursor *cursor,
                         struct reloq_parameter_frame *frame);

#endif
