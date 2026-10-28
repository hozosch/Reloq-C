#include "reloq_german_voiced_stop_tract.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "voiced-stop tract line %d: %s\n", __LINE__, #condition); \
    return -1; \
} } while (0)

struct fixture {
    struct reloq_german_phone_node phones[4];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][6];
    struct reloq_target_builder builder;
    size_t phone_count;
};

static int begin(struct fixture *f, const char *annotation, size_t capacity)
{
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    memset(f, 0, sizeof *f);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            f->phones, 4, &info)) return -1;
    f->phone_count = info.phone_count;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        storage[s] = (struct reloq_target_buffer){f->points[s], capacity, 0};
    return reloq_target_builder_begin(&f->builder, storage);
}

static int pair(const struct fixture *f, enum reloq_target_stream stream,
                int64_t first, int64_t second, int32_t value)
{
    const struct reloq_target_buffer *b = &f->builder.stream[stream];
    CHECK(b->count == 2);
    CHECK(b->points[0].tick == first && b->points[0].value == value);
    CHECK(b->points[1].tick == second && b->points[1].value == value);
    return 0;
}

static int reference_case(const char *annotation, int64_t onset,
                          int64_t target, int64_t hold, int64_t closure)
{
    struct fixture f;
    CHECK(begin(&f, annotation, 6) == 0);
    CHECK(reloq_german_voiced_stop_tract_append(
        f.phones, f.phone_count, onset, target, hold, closure,
        &f.builder) == 0);
    CHECK(pair(&f, RELOQ_TARGET_F4, target, hold, 3600) == 0);
    CHECK(pair(&f, RELOQ_TARGET_F5, target, hold, 3900) == 0);
    static const int32_t widths[5] = {120, 100, 150, 330, 260};
    for (size_t i = 0; i < 5; ++i)
        CHECK(pair(&f, (enum reloq_target_stream)(RELOQ_TARGET_B1+i),
                   target, hold, widths[i]) == 0);
    CHECK(pair(&f, RELOQ_TARGET_FNP, target, hold, 200) == 0);
    CHECK(pair(&f, RELOQ_TARGET_FNZ, target, hold, 200) == 0);
    CHECK(pair(&f, RELOQ_TARGET_TL, onset+1, closure, 0) == 0);
    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s)
        CHECK(pair(&f, s, target, hold, 0) == 0);
    CHECK(pair(&f, RELOQ_TARGET_AF, target, hold, 0) == 0);
    CHECK(pair(&f, RELOQ_TARGET_DI, target, hold, 0) == 0);
    CHECK(pair(&f, RELOQ_TARGET_OQ, target, hold, 56) == 0);
    size_t point_count = 0;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        point_count += f.builder.stream[s].count;
    CHECK(point_count == 38);
    CHECK(f.builder.stream[RELOQ_TARGET_F0].count == 0);
    CHECK(f.builder.stream[RELOQ_TARGET_F1].count == 0);
    CHECK(f.builder.stream[RELOQ_TARGET_F2].count == 0);
    CHECK(f.builder.stream[RELOQ_TARGET_F3].count == 0);
    CHECK(f.builder.stream[RELOQ_TARGET_AV].count == 0);
    CHECK(f.builder.stream[RELOQ_TARGET_AH].count == 0);
    return 0;
}

static int invalid_cases(void)
{
    const char *uncovered[] = {
        "`[.1pAp].", "`[.1bA].", "`[.1gAs].", "`[.0dEt].",
        "`[.1A.0bA]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        struct fixture f;
        if (begin(&f, uncovered[i], 6)) {
            fprintf(stderr, "invalid test annotation: %s\n", uncovered[i]);
            return -1;
        }
        CHECK(reloq_german_voiced_stop_tract_append(
            f.phones, f.phone_count, 81, 96, 148, 178,
            &f.builder) == 1);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    struct fixture f;
    CHECK(begin(&f, "`[.1bAp].", 1) == 0);
    CHECK(reloq_german_voiced_stop_tract_append(
        f.phones, f.phone_count, 81, 96, 148, 178, &f.builder) == -1);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        CHECK(f.builder.stream[s].count == 0);
    CHECK(begin(&f, "`[.1bAp].", 6) == 0);
    CHECK(reloq_german_voiced_stop_tract_append(
        f.phones, f.phone_count, INT64_MAX, INT64_MAX, INT64_MAX,
        INT64_MAX, &f.builder) == -1);
    CHECK(reloq_german_voiced_stop_tract_append(
        f.phones, f.phone_count, 81, 96, 95, 178, &f.builder) == -1);
    CHECK(f.builder.stream[RELOQ_TARGET_TL].count == 0);
    return 0;
}

int main(void)
{
    /* Independent reference observations: initial place and coda alter
     * timing but preserve these 38 sustained controls. */
    if (reference_case("`[.1bAp].", 81, 96, 148, 178) ||
        reference_case("`[.1dEt].", 100, 125, 180, 205) ||
        reference_case("`[.1gUk].", 90, 115, 134, 174) ||
        reference_case("`[.1dUt].", 100, 142, 145, 184) ||
        invalid_cases()) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
