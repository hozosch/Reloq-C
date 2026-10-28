#include "reloq_duration_span.h"

#include <assert.h>
#include <stdint.h>

int main(void)
{
    const struct reloq_duration_item ordinary[] = {
        {17, RELOQ_DURATION_I16}, {23, RELOQ_DURATION_I16},
        {0, RELOQ_DURATION_NONNUMERIC}};
    assert(reloq_duration_span_query(ordinary, 3, 0, 0, 5, 1, 8, 1) == 43);
    assert(reloq_duration_span_query(ordinary, 3, 0, 1, 8, 1, 5, 1) == -43);
    assert(reloq_duration_span_query(NULL, 0, 1, 0, 5, 0, 8, 0) == 3);
    assert(reloq_duration_span_query(NULL, 0, 0, 0, 0, 0, 0, 0) ==
           RELOQ_DURATION_UNDEFINED);
    const struct reloq_duration_item zero = {0, RELOQ_DURATION_I16};
    assert(reloq_duration_span_query(&zero, 1, 0, 0, 0, 0, 0, 0) == 0);
    const struct reloq_duration_item wide = {INT64_C(4294967299), RELOQ_DURATION_I64};
    assert(reloq_duration_time_sum(&wide, 1) == 3);
    assert(reloq_duration_span_query(&wide, 1, 0, 0, 0, 0, 0, 0) == wide.value);
    const struct reloq_duration_item wrap[] = {
        {INT64_MAX, RELOQ_DURATION_I64}, {1, RELOQ_DURATION_I64}};
    assert(reloq_duration_span_query(wrap, 2, 0, 0, 0, 0, 0, 0) == INT64_MIN);
    struct reloq_duration_item rounded[] = {
        {1, RELOQ_DURATION_I16}, {1, RELOQ_DURATION_I16}, {1, RELOQ_DURATION_I16}};
    assert(!reloq_duration_redistribute(rounded, 3, 3, 2));
    assert(reloq_duration_time_sum(rounded, 3) == 3);
    struct reloq_duration_item mixed[] = {
        {3, RELOQ_DURATION_I16}, {3, RELOQ_DURATION_I64}};
    assert(!reloq_duration_redistribute(mixed, 2, 6, 3));
    assert(mixed[0].value == 2 && mixed[1].value == 1);
    const struct reloq_duration_item bad_original = {3, RELOQ_DURATION_NONNUMERIC};
    struct reloq_duration_item bad = bad_original;
    assert(reloq_duration_redistribute(&bad, 1, 3, 2) == -1);
    assert(bad.value == bad_original.value);
    assert(reloq_duration_scale_transition(17, 50) == 8);
    assert(reloq_duration_scale_transition(0, 100) == 1);
    assert(reloq_duration_scale_transition(32767, 200) == 1);
    return 0;
}
