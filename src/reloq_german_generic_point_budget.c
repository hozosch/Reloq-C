#include "reloq_german_generic_point_budget.h"

#include <stdint.h>

int reloq_german_generic_point_budget(
    size_t phone_count, size_t word_count, int word_pitch,
    struct reloq_german_generic_point_budget *result)
{
    if (!result || (word_pitch != 0 && word_pitch != 1) ||
        phone_count > (SIZE_MAX - 8) / 6 ||
        phone_count > (SIZE_MAX - 8) / 2 ||
        (word_pitch && word_count > SIZE_MAX / 2)) return -1;
    size_t base = phone_count * 6 + 8;
    size_t extra = phone_count * 2 + 8;
    size_t pitch = word_pitch ? word_count * 2 : 0;
    if (extra > SIZE_MAX - base || pitch > SIZE_MAX - base)
        return -1;
    struct reloq_german_generic_point_budget next = {0};
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        size_t capacity = base;
        if ((i >= RELOQ_TARGET_F1 && i <= RELOQ_TARGET_F3) ||
            i == RELOQ_TARGET_AV || i == RELOQ_TARGET_AH)
            capacity += extra;
        else if (i == RELOQ_TARGET_F0)
            capacity += pitch;
        if (capacity > SIZE_MAX - next.total_points) return -1;
        next.capacity[i] = capacity;
        next.total_points += capacity;
    }
    if (next.total_points > SIZE_MAX / sizeof(struct reloq_acoustic_point))
        return -1;
    *result = next;
    return 0;
}
