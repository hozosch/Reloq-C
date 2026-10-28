#include "reloq_target_builder.h"

#include <limits.h>
#include <math.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) return __LINE__; } while (0)

static int value_at(const struct reloq_acoustic_curve *curve,
                    int64_t tick, float expected)
{
    struct reloq_acoustic_cursor cursor;
    float value = -1.0f;
    return reloq_acoustic_begin(curve, &cursor) == 0 &&
           reloq_acoustic_value(&cursor, tick, &value) == 0 &&
           value == expected;
}

int main(void)
{
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][8];
    memset(points, 0x5a, sizeof points);
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        storage[i] = (struct reloq_target_buffer){points[i], 8, 0};
    struct reloq_target_builder builder = {0};
    CHECK(reloq_target_builder_begin(&builder, storage) == 0);

    const struct reloq_target_event gesture[] = {
        {RELOQ_TARGET_F0, -2, 1200},
        {RELOQ_TARGET_AV, 0, 51},
        {RELOQ_TARGET_F1, 0, 700},
        {RELOQ_TARGET_AV, 0, 49},
        {RELOQ_TARGET_AV, 3, 46},
        {RELOQ_TARGET_F0, 4, 1000},
        {RELOQ_TARGET_A1F, 0, 30}
    };
    CHECK(reloq_target_builder_append(&builder, 100, gesture,
                                      sizeof gesture / sizeof gesture[0]) == 0);
    CHECK(builder.stream[RELOQ_TARGET_F0].count == 2 &&
          points[RELOQ_TARGET_F0][0].tick == 98 &&
          points[RELOQ_TARGET_F0][1].tick == 104);
    CHECK(builder.stream[RELOQ_TARGET_AV].count == 3 &&
          points[RELOQ_TARGET_AV][0].tick == 100 &&
          points[RELOQ_TARGET_AV][1].tick == 100 &&
          points[RELOQ_TARGET_AV][2].tick == 103);

    const uint8_t field[RELOQ_TARGET_STREAM_COUNT] = {
        1, 9, 13, 15, 17, 19, 10, 14, 16, 18, 20,
        27, 29, 31, 33, 4, 3, 5, 6, 2, 7, 8,
        35, 36, 37, 38, 39, 43
    };
    float defaults[RELOQ_PARAMETER_TRACKS] = {0};
    defaults[0] = 1.0f;
    struct reloq_acoustic_curve curves[RELOQ_PARAMETER_TRACKS];
    unsigned char mapped[RELOQ_PARAMETER_TRACKS];
    CHECK(reloq_target_builder_export(&builder, field, defaults,
                                      curves, mapped) == 0);
    CHECK(mapped[1] == 1 && mapped[2] == 1 &&
          mapped[0] == 0 && mapped[11] == 0);
    CHECK(curves[1].point_count == 2 && curves[2].point_count == 3 &&
          curves[11].point_count == 0 && curves[0].default_value == 1.0f);
    CHECK(value_at(&curves[1], 100, 1133.0f));
    CHECK(value_at(&curves[2], 100, 51.0f));
    CHECK(value_at(&curves[2], 101, 48.0f));

    struct reloq_target_builder before = builder;
    struct reloq_acoustic_point old_points[RELOQ_TARGET_STREAM_COUNT][8];
    memcpy(old_points, points, sizeof points);
    const struct reloq_target_event too_many[] = {
        {RELOQ_TARGET_F0, 4, 1000}, {RELOQ_TARGET_F0, 5, 1000},
        {RELOQ_TARGET_F0, 6, 1000}, {RELOQ_TARGET_F0, 7, 1000},
        {RELOQ_TARGET_F0, 8, 1000}, {RELOQ_TARGET_F0, 9, 1000},
        {RELOQ_TARGET_F0, 10, 1000}
    };
    CHECK(reloq_target_builder_append(&builder, 100, too_many,
              sizeof too_many / sizeof too_many[0]) == -1);
    CHECK(memcmp(&builder, &before, sizeof builder) == 0 &&
          memcmp(points, old_points, sizeof points) == 0);
    const struct reloq_target_event backwards[] = {
        {RELOQ_TARGET_AV, 4, 44}, {RELOQ_TARGET_F0, 3, 800}
    };
    CHECK(reloq_target_builder_append(&builder, 100, backwards, 2) == -1);
    CHECK(memcmp(&builder, &before, sizeof builder) == 0 &&
          memcmp(points, old_points, sizeof points) == 0);
    const struct reloq_target_event overflow = {RELOQ_TARGET_F0, 1, 1};
    CHECK(reloq_target_builder_append(&builder, INT64_MAX, &overflow, 1) == -1);
    CHECK(memcmp(&builder, &before, sizeof builder) == 0 &&
          memcmp(points, old_points, sizeof points) == 0);
    CHECK(reloq_target_builder_append(&builder, 100, NULL, 0) == 0);

    uint8_t duplicate[RELOQ_TARGET_STREAM_COUNT];
    memcpy(duplicate, field, sizeof field);
    duplicate[RELOQ_TARGET_AV] = duplicate[RELOQ_TARGET_AH];
    struct reloq_acoustic_curve saved[RELOQ_PARAMETER_TRACKS];
    unsigned char saved_map[RELOQ_PARAMETER_TRACKS];
    memcpy(saved, curves, sizeof saved);
    memcpy(saved_map, mapped, sizeof saved_map);
    CHECK(reloq_target_builder_export(&builder, duplicate, defaults,
                                      curves, mapped) == -1);
    CHECK(memcmp(saved, curves, sizeof saved) == 0 &&
          memcmp(saved_map, mapped, sizeof saved_map) == 0);
    defaults[0] = NAN;
    CHECK(reloq_target_builder_export(&builder, field, defaults,
                                      curves, mapped) == -1);

    struct reloq_target_buffer invalid[RELOQ_TARGET_STREAM_COUNT];
    memcpy(invalid, storage, sizeof invalid);
    invalid[0].count = 9;
    CHECK(reloq_target_builder_begin(&builder, invalid) == -1);
    CHECK(memcmp(&builder, &before, sizeof builder) == 0);
    memcpy(invalid, storage, sizeof invalid);
    invalid[1].points = points[0] + 1;
    invalid[1].capacity = 7; /* overlaps the first stream's future writes */
    CHECK(reloq_target_builder_begin(&builder, invalid) == -1);
    CHECK(memcmp(&builder, &before, sizeof builder) == 0);
    invalid[1].points = points[1];
    invalid[1].capacity = SIZE_MAX / sizeof *points[1] + 1;
    CHECK(reloq_target_builder_begin(&builder, invalid) == -1);
    CHECK(memcmp(&builder, &before, sizeof builder) == 0);
    memcpy(invalid, storage, sizeof invalid);
    invalid[0].points = (struct reloq_acoustic_point *)(void *)&builder;
    invalid[0].capacity = 1;
    CHECK(reloq_target_builder_begin(&builder, invalid) == -1);
    CHECK(memcmp(&builder, &before, sizeof builder) == 0);

    struct reloq_target_event aliased_events[] = {
        {RELOQ_TARGET_F0, 0, 1200}, {RELOQ_TARGET_F1, 1, 700}
    };
    struct reloq_target_event original_events[2];
    memcpy(original_events, aliased_events, sizeof original_events);
    struct reloq_target_buffer aliased_storage[RELOQ_TARGET_STREAM_COUNT];
    memcpy(aliased_storage, storage, sizeof aliased_storage);
    aliased_storage[RELOQ_TARGET_F0].points =
        (struct reloq_acoustic_point *)(void *)&aliased_events[1];
    aliased_storage[RELOQ_TARGET_F0].capacity = 1;
    struct reloq_target_builder aliased_builder = {0};
    CHECK(reloq_target_builder_begin(&aliased_builder, aliased_storage) == 0);
    struct reloq_target_builder original_builder = aliased_builder;
    CHECK(reloq_target_builder_append(&aliased_builder, 100, aliased_events,
                                      2) == -1);
    CHECK(memcmp(&aliased_builder, &original_builder,
                 sizeof aliased_builder) == 0 &&
          memcmp(aliased_events, original_events,
                 sizeof aliased_events) == 0);
    CHECK(strcmp(reloq_target_stream_name(RELOQ_TARGET_AH), "AH") == 0 &&
          reloq_target_stream_name((enum reloq_target_stream)-1) == NULL);
    return 0;
}
