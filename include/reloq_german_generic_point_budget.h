#ifndef RELOQ_GERMAN_GENERIC_POINT_BUDGET_H
#define RELOQ_GERMAN_GENERIC_POINT_BUDGET_H

#include "reloq_target_builder.h"

#include <stddef.h>

struct reloq_german_generic_point_budget {
    size_t capacity[RELOQ_TARGET_STREAM_COUNT];
    size_t total_points;
};

/* The base profile count reserves up to six targets per phoneme plus a
 * boundary margin. F1..F3 and AV/AH can additionally carry two independent
 * gestural targets per phoneme without enlarging the unrelated source
 * streams. Also reserves word-level F0 modifications when requested.
 * All additions and byte counts are checked before allocation. */
int reloq_german_generic_point_budget(
    size_t phone_count, size_t word_count, int word_pitch,
    struct reloq_german_generic_point_budget *result);

#endif
