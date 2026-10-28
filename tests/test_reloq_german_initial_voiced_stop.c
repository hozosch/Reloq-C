#include "reloq_german_initial_voiced_stop.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "initial voiced stop test line %d: %s\n", __LINE__, #x); \
    return -1; \
} } while (0)

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][8];
    struct reloq_target_builder builder;
    size_t count;
};

static int begin(struct fixture *f, const char *annotation, size_t capacity)
{
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    memset(f, 0, sizeof *f);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            f->phones, 8, &info)) return -1;
    f->count = info.phone_count;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){f->points[i], capacity, 0};
    return reloq_target_builder_begin(&f->builder, buffers);
}

static int check_stream(const struct fixture *f, enum reloq_target_stream s,
                        int64_t origin, const int64_t *ticks,
                        const int32_t *values, size_t count)
{
    CHECK(f->builder.stream[s].count == count);
    for (size_t i = 0; i < count; ++i) {
        CHECK(f->points[s][i].tick == origin + ticks[i]);
        CHECK(f->points[s][i].value == values[i]);
    }
    return 0;
}

static int case_check(const char *shape, int64_t onset, int64_t burst,
                      int32_t f1, int32_t f2_0, int32_t f2_1,
                      int32_t f3_0, int32_t f3_1, int32_t af,
                      enum reloq_target_stream noise, int32_t gain)
{
    struct fixture f;
    int64_t got_onset = -7, got_burst = -7;
    CHECK(begin(&f, shape, 8) == 0);
    CHECK(reloq_german_initial_voiced_stop_append(
        f.phones, f.count, 31, &f.builder, &got_onset, &got_burst) == 0);
    CHECK(got_onset == onset && got_burst == burst);
    const int64_t span[] = {0, onset};
    const int64_t source[] = {0, burst, burst, onset};
    const int32_t f1_values[] = {f1, f1};
    const int32_t f2_values[] = {f2_0, f2_1};
    const int32_t f3_values[] = {f3_0, f3_1};
    const int32_t av[] = {20, 20, 0, 0};
    const int32_t tl[] = {35, 35, 0, 0};
    const int32_t zero[] = {0, 0, 0, 0};
    const int32_t af_values[] = {0, 0, af, af};
    const int32_t noise_values[] = {0, 0, gain, gain};
    CHECK(check_stream(&f, RELOQ_TARGET_F1, 31, span, f1_values, 2) == 0);
    CHECK(check_stream(&f, RELOQ_TARGET_F2, 31, span, f2_values, 2) == 0);
    CHECK(check_stream(&f, RELOQ_TARGET_F3, 31, span, f3_values, 2) == 0);
    CHECK(check_stream(&f, RELOQ_TARGET_AV, 31, source, av, 4) == 0);
    CHECK(check_stream(&f, RELOQ_TARGET_TL, 31, source, tl, 4) == 0);
    CHECK(check_stream(&f, RELOQ_TARGET_AH, 31, source, zero, 4) == 0);
    CHECK(check_stream(&f, RELOQ_TARGET_AF, 31, source, af_values, 4) == 0);
    CHECK(check_stream(&f, noise, 31, source, noise_values, 4) == 0);
    size_t total = 0;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        total += f.builder.stream[s].count;
    CHECK(total == 26);
    return 0;
}

static int self_test(void)
{
    CHECK(case_check("`[.1bAp].",81,70,200,1000,1000,2500,2500,56,
                     RELOQ_TARGET_AB,64) == 0);
    CHECK(case_check("`[.1bup].",106,95,200,1000,600,2500,2500,56,
                     RELOQ_TARGET_AB,64) == 0);
    CHECK(case_check("`[.1dAt].",100,90,200,1600,1600,2600,2600,53,
                     RELOQ_TARGET_A5F,55) == 0);
    CHECK(case_check("`[.1dep].",105,90,200,1600,1700,2600,2700,52,
                     RELOQ_TARGET_A5F,55) == 0);
    CHECK(case_check("`[.1gEk].",90,70,300,1500,2000,1890,2300,63,
                     RELOQ_TARGET_A2F,65) == 0);
    CHECK(case_check("`[.1gIp].",90,70,300,1500,2150,1890,2400,63,
                     RELOQ_TARGET_A3F,60) == 0);
    CHECK(case_check("`[.1gut].",90,70,300,1500,900,1890,2000,56,
                     RELOQ_TARGET_A2F,70) == 0);

    struct fixture f;
    const char *unsupported[] = {
        "`[.1pAp].", "`[.1bAl].", "`[.0dAp].", "`[.1gA.0A].",
        "`[.1A.0bA].", "`[.1bAp] `[.1dAp]."
    };
    for (size_t i = 0; i < sizeof unsupported / sizeof unsupported[0]; ++i) {
        CHECK(begin(&f, unsupported[i], 8) == 0);
        int64_t onset = -7, burst = -7;
        CHECK(reloq_german_initial_voiced_stop_append(
            f.phones, f.count, 0, &f.builder, &onset, &burst) == 1);
        CHECK(onset == -7 && burst == -7);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    CHECK(begin(&f, "`[.1bAp].", 3) == 0);
    int64_t onset = -7, burst = -7;
    CHECK(reloq_german_initial_voiced_stop_append(
        f.phones, f.count, 0, &f.builder, &onset, &burst) == -1);
    CHECK(onset == -7 && burst == -7);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        CHECK(f.builder.stream[s].count == 0);
    CHECK(begin(&f, "`[.1bAp].", 8) == 0);
    CHECK(reloq_german_initial_voiced_stop_append(
        f.phones, f.count, INT64_MAX, &f.builder, &onset, &burst) == -1);
    CHECK(onset == -7 && burst == -7);
    return 0;
}

static int query(const char *annotation)
{
    struct fixture f;
    int64_t onset, burst;
    if (begin(&f, annotation, 8) ||
        reloq_german_initial_voiced_stop_append(
            f.phones, f.count, 0, &f.builder, &onset, &burst)) return -1;
    printf("onset=%" PRId64 " burst=%" PRId64 "\n", onset, burst);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        const struct reloq_target_buffer *stream = &f.builder.stream[s];
        if (!stream->count) continue;
        printf("%s", reloq_target_stream_name((enum reloq_target_stream)s));
        for (size_t i = 0; i < stream->count; ++i)
            printf(" %" PRId64 ":%" PRId32,
                   stream->points[i].tick, stream->points[i].value);
        putchar('\n');
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) return self_test() ? EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 3 && strcmp(argv[1], "--query") == 0)
        return query(argv[2]) ? EXIT_FAILURE : EXIT_SUCCESS;
    fprintf(stderr, "usage: %s [--query SPR]\n", argv[0]);
    return EXIT_FAILURE;
}
