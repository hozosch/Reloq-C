#include "reloq_german_generic_point_budget.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    struct reloq_german_generic_point_budget budget;
    assert(reloq_german_generic_point_budget(100, 7, 1,
                                             &budget) == 0);
    const size_t base = 100 * 6 + 8;
    const size_t gestural = base + 100 * 2 + 8;
    assert(budget.capacity[RELOQ_TARGET_F0] == base + 14);
    assert(budget.capacity[RELOQ_TARGET_F1] == gestural &&
           budget.capacity[RELOQ_TARGET_F2] == gestural &&
           budget.capacity[RELOQ_TARGET_F3] == gestural &&
           budget.capacity[RELOQ_TARGET_AV] == gestural &&
           budget.capacity[RELOQ_TARGET_AH] == gestural);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        if (s == RELOQ_TARGET_F0 ||
            (s >= RELOQ_TARGET_F1 && s <= RELOQ_TARGET_F3) ||
            s == RELOQ_TARGET_AV || s == RELOQ_TARGET_AH) continue;
        assert(budget.capacity[s] == base);
    }
    assert(budget.total_points ==
           RELOQ_TARGET_STREAM_COUNT * base + 5 * (100 * 2 + 8) + 14);

    struct reloq_german_generic_point_budget no_pitch = {0};
    assert(reloq_german_generic_point_budget(100, 7, 0,
                                             &no_pitch) == 0);
    assert(no_pitch.capacity[RELOQ_TARGET_F0] == base);
    assert(no_pitch.capacity[RELOQ_TARGET_F1] == gestural);
    assert(no_pitch.total_points + 14 == budget.total_points);

    /* Arithmetic must fail before any caller allocation or modification. */
    struct reloq_german_generic_point_budget saved = budget;
    assert(reloq_german_generic_point_budget(SIZE_MAX, 7, 1,
                                             &budget) == -1);
    assert(memcmp(&budget, &saved, sizeof budget) == 0);
    assert(reloq_german_generic_point_budget(100, SIZE_MAX, 1,
                                             &budget) == -1);
    assert(memcmp(&budget, &saved, sizeof budget) == 0);
    assert(reloq_german_generic_point_budget(100, 7, 2,
                                             &budget) == -1);
    assert(reloq_german_generic_point_budget(100, 7, 1,
                                             NULL) == -1);
    assert(reloq_german_generic_point_budget(
               SIZE_MAX / (RELOQ_TARGET_STREAM_COUNT * 6), 0, 0,
               &budget) == -1);
    puts("selective stage point budget: OK");
    return 0;
}
