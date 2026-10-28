#include "reloq_german_terminal_timing.h"

#include <assert.h>
#include <stddef.h>

int main(void)
{
    const int32_t expected[] = {460, 460, 460, 460, 460, 310, 210, 210, 210, 210};
    for (int mark = RELOQ_DE_TERMINAL_IMPLICIT;
         mark <= RELOQ_DE_TERMINAL_EXCLAMATION_CONTINUE; ++mark) {
        struct reloq_german_terminal_timing timing = {0};
        assert(reloq_german_terminal_timing(
            (enum reloq_german_terminal_mark)mark, &timing) == 0);
        assert(timing.window_ticks == expected[mark]);
        assert(timing.source_release_ticks == 61);
        assert(timing.zero_source_ticks == expected[mark] - 61);
    }
    struct reloq_german_terminal_timing unchanged = {12, 13, 14};
    assert(reloq_german_terminal_timing(
        (enum reloq_german_terminal_mark)-1, &unchanged) == -1);
    assert(reloq_german_terminal_timing(
        (enum reloq_german_terminal_mark)(RELOQ_DE_TERMINAL_CONTROL+1), &unchanged) == -1);
    assert(reloq_german_terminal_timing(
        RELOQ_DE_TERMINAL_PERIOD, NULL) == -1);
    assert(unchanged.window_ticks == 12 &&
           unchanged.source_release_ticks == 13 &&
           unchanged.zero_source_ticks == 14);
    return 0;
}
