#include "reloq_target_stage_merge.h"

#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) return __LINE__; } while (0)

struct stage_storage {
    struct reloq_target_builder builder;
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][12];
};

static int begin(struct stage_storage *stage, size_t capacity)
{
    memset(stage, 0, sizeof *stage);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        stage->buffers[i] = (struct reloq_target_buffer){
            stage->points[i], capacity, 0};
    return reloq_target_builder_begin(&stage->builder, stage->buffers);
}

int main(void)
{
    struct stage_storage first, second, output;
    CHECK(begin(&first, 12) == 0 && begin(&second, 12) == 0 &&
          begin(&output, 12) == 0);
    const struct reloq_target_event a[] = {
        {RELOQ_TARGET_F0, 0, 1100},
        {RELOQ_TARGET_F1, 0, 500},
        {RELOQ_TARGET_AV, 0, 0},
        {RELOQ_TARGET_AV, 10, 52},
        {RELOQ_TARGET_AV, 10, 50},
        {RELOQ_TARGET_F0, 20, 1000}
    };
    const struct reloq_target_event b[] = {
        {RELOQ_TARGET_F0, 0, 900},
        {RELOQ_TARGET_AV, 10, 47},
        {RELOQ_TARGET_AV, 14, 0},
        {RELOQ_TARGET_AF, 10, 55}
    };
    CHECK(reloq_target_builder_append(&first.builder, 0, a,
          sizeof a / sizeof a[0]) == 0);
    CHECK(reloq_target_builder_append(&second.builder, 0, b,
          sizeof b / sizeof b[0]) == 0);
    const struct reloq_target_event old = {RELOQ_TARGET_F0, 0, 850};
    CHECK(reloq_target_builder_append(&output.builder, 0, &old, 1) == 0);
    const struct reloq_target_stage stages[] = {
        {&first.builder, 20, RELOQ_TARGET_STAGE_ALL_STREAMS},
        {&second.builder, 20,
         RELOQ_TARGET_STAGE_ALL_STREAMS &
             ~(UINT32_C(1) << RELOQ_TARGET_F0)}
    };
    const struct reloq_target_stage_transform transform = {2.0, 1.5, 2, 0};
    size_t copied = 999;
    CHECK(reloq_target_stage_merge(&output.builder, stages, 2,
                                   &transform, &copied) == 0 && copied == 9);
    const struct reloq_target_buffer *f0 =
        &output.builder.stream[RELOQ_TARGET_F0];
    CHECK(f0->count == 3 && f0->points[0].tick == 0 &&
          f0->points[0].value == 850 &&
          f0->points[1].tick == 40 && f0->points[1].value == 1650 &&
          f0->points[2].tick == 80 && f0->points[2].value == 1500);
    const struct reloq_target_buffer *av =
        &output.builder.stream[RELOQ_TARGET_AV];
    CHECK(av->count == 5 && av->points[0].tick == 40 &&
          av->points[0].value == 0 &&
          av->points[1].tick == 60 && av->points[1].value == 50 &&
          av->points[2].tick == 60 && av->points[2].value == 48 &&
          av->points[3].tick == 60 && av->points[3].value == 45 &&
          av->points[4].tick == 68 && av->points[4].value == 0);
    CHECK(output.builder.stream[RELOQ_TARGET_F1].points[0].tick == 40 &&
          output.builder.stream[RELOQ_TARGET_F1].points[0].value == 500 &&
          output.builder.stream[RELOQ_TARGET_AF].points[0].tick == 60 &&
          output.builder.stream[RELOQ_TARGET_AF].points[0].value == 53);
    int64_t word_start = -1;
    CHECK(reloq_target_stage_scale_tick(30, &transform, &word_start) == 0 &&
          word_start == 60);

    /* Bad inputs and capacity exhaustion have no partial writes. */
    struct reloq_target_builder snapshot = output.builder;
    struct reloq_acoustic_point saved[RELOQ_TARGET_STREAM_COUNT][12];
    memcpy(saved, output.points, sizeof saved);
    struct reloq_target_stage earlier = {&second.builder, 0,
        RELOQ_TARGET_STAGE_ALL_STREAMS};
    copied = 999;
    CHECK(reloq_target_stage_merge(&output.builder, &earlier, 1,
                                  NULL, &copied) == -1 && copied == 999);
    struct reloq_target_stage huge = {&second.builder, INT64_MAX,
        RELOQ_TARGET_STAGE_ALL_STREAMS};
    CHECK(reloq_target_stage_merge(&output.builder, &huge, 1,
                                  NULL, &copied) == -1);
    struct reloq_target_stage alias = {&output.builder, 0,
        RELOQ_TARGET_STAGE_ALL_STREAMS};
    CHECK(reloq_target_stage_merge(&output.builder, &alias, 1,
                                  NULL, &copied) == -1);
    const struct reloq_target_stage_transform invalid = {NAN, 1.0, 0, 0};
    CHECK(reloq_target_stage_merge(&output.builder, stages, 2,
                                  &invalid, &copied) == -1);
    CHECK(memcmp(&snapshot, &output.builder, sizeof snapshot) == 0 &&
          memcmp(saved, output.points, sizeof saved) == 0);

    struct stage_storage small;
    CHECK(begin(&small, 1) == 0);
    CHECK(reloq_target_stage_merge(&small.builder, stages, 2,
                                   NULL, NULL) == -1);
    CHECK(small.builder.stream[RELOQ_TARGET_AV].count == 0 &&
          small.builder.stream[RELOQ_TARGET_F0].count == 0);

    /* Opt out of a local phrase F0 when a later global stage owns melody. */
    struct stage_storage global;
    CHECK(begin(&global, 12) == 0);
    const struct reloq_target_event melody[] = {
        {RELOQ_TARGET_F0, 0, 1230},
        {RELOQ_TARGET_F0, 50, 1100}
    };
    CHECK(reloq_target_builder_append(&global.builder, 0, melody, 2) == 0);
    struct reloq_target_stage de_f0 = {
        &global.builder, 0, UINT32_C(1) << RELOQ_TARGET_F0};
    CHECK(reloq_target_stage_merge(&small.builder, &de_f0, 1,
                                   NULL, NULL) == -1);
    struct stage_storage routed;
    CHECK(begin(&routed, 12) == 0);
    struct reloq_target_stage selected[] = {
        {&first.builder, 0,
         RELOQ_TARGET_STAGE_ALL_STREAMS &
             ~(UINT32_C(1) << RELOQ_TARGET_F0)},
        {&global.builder, 0, UINT32_C(1) << RELOQ_TARGET_F0}
    };
    CHECK(reloq_target_stage_merge(&routed.builder, selected, 2,
                                   NULL, NULL) == 0);
    CHECK(routed.builder.stream[RELOQ_TARGET_F0].count == 2 &&
          routed.builder.stream[RELOQ_TARGET_F0].points[0].value == 1230 &&
          routed.builder.stream[RELOQ_TARGET_AV].count == 3);

    struct stage_storage muted;
    CHECK(begin(&muted, 12) == 0);
    const struct reloq_target_stage_transform quiet = {1.0, 1.0, 0, 1};
    CHECK(reloq_target_stage_merge(&muted.builder, stages, 2,
                                   &quiet, NULL) == 0);
    CHECK(muted.builder.stream[RELOQ_TARGET_AV].points[1].value == 0 &&
          muted.builder.stream[RELOQ_TARGET_F1].points[0].value == 500);
    return 0;
}
