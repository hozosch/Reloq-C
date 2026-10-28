#include "reloq_german_initial_nasal.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "test_reloq_german_initial_nasal.c:%d: %s\n", \
            __LINE__, #test); return -1; \
} } while (0)

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][4];
    struct reloq_target_builder builder;
    size_t count;
};

static int begin(struct fixture *f, const char *annotation)
{
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    memset(f, 0, sizeof *f);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            f->phones, 8, &info)) return -1;
    f->count = info.phone_count;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){f->points[i], 4, 0};
    return reloq_target_builder_begin(&f->builder, buffers);
}

static int self_test(void)
{
    static const struct {
        const char *annotation;
        int64_t end;
        int32_t f1, f2, f3, f4, f5, b2, b3, av;
    } cases[] = {
        {"`[.1mA].", 85, 300, 1000, 2500, 3600, 3900,
         150, 105, 51},
        {"`[.1mOt].", 85, 300, 700, 2300, 3600, 3900,
         150, 105, 51},
        {"`[.1me].", 85, 300, 1300, 2500, 3600, 3900,
         150, 105, 51},
        {"`[.1nAp].", 70, 200, 1600, 2500, 3350, 3800,
         200, 80, 48},
        {"`[.1nEt].", 70, 200, 1700, 2600, 3350, 3800,
         200, 80, 48},
        {"`[.1nIk].", 70, 200, 1850, 2800, 3400, 3800,
         200, 80, 48},
        {"`[.1nOt].", 70, 200, 1500, 2500, 3350, 3800,
         200, 80, 48},
        {"`[.1nU].", 70, 200, 1500, 2500, 3350, 3800,
         200, 80, 48},
        {"`[.1no].", 70, 200, 1400, 2500, 3350, 3800,
         200, 80, 48}
    };
    struct fixture f;
    int64_t first_send = -7;
    for (size_t c = 0; c < sizeof cases / sizeof cases[0]; ++c) {
        CHECK(begin(&f, cases[c].annotation) == 0);
        CHECK(reloq_german_initial_nasal_append(
            f.phones, f.count, 31, &f.builder, &first_send) == 0);
        CHECK(first_send == cases[c].end);
        size_t count = 0;
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
            size_t expected = s == RELOQ_TARGET_FNZ ? 1 :
                s == RELOQ_TARGET_F0 || s == RELOQ_TARGET_FL ||
                s == RELOQ_TARGET_FTP || s == RELOQ_TARGET_FTZ ? 0 : 2;
            CHECK(f.builder.stream[s].count == expected);
            count += expected;
            if (expected == 2) {
                CHECK(f.points[s][0].tick == 31);
                CHECK(f.points[s][1].tick == 31 + cases[c].end);
                CHECK(f.points[s][0].value == f.points[s][1].value);
            }
        }
        CHECK(count == 47);
        CHECK(f.points[RELOQ_TARGET_FNZ][0].tick ==
              31 + cases[c].end / 2);
        CHECK(f.points[RELOQ_TARGET_FNZ][0].value == 350);
        const enum reloq_target_stream streams[] = {
            RELOQ_TARGET_F1, RELOQ_TARGET_F2, RELOQ_TARGET_F3,
            RELOQ_TARGET_F4, RELOQ_TARGET_F5, RELOQ_TARGET_B2,
            RELOQ_TARGET_B3, RELOQ_TARGET_AV};
        const int32_t values[] = {cases[c].f1, cases[c].f2, cases[c].f3,
                                  cases[c].f4, cases[c].f5, cases[c].b2,
                                  cases[c].b3, cases[c].av};
        for (size_t s = 0; s < sizeof streams / sizeof streams[0]; ++s)
            CHECK(f.points[streams[s]][0].value == values[s]);
    }

    const char *uncovered[] = {
        "`[.1lA].", "`[.1rA].", "`[.1mA.0nA].", "`[.0mA].",
        "`[.1mAs].", "`[.1mAf].", "`[.1mY]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        CHECK(begin(&f, uncovered[i]) == 0);
        first_send = -7;
        CHECK(reloq_german_initial_nasal_append(
            f.phones, f.count, 0, &f.builder, &first_send) == 1);
        CHECK(first_send == -7);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    CHECK(begin(&f, "`[.1mA].") == 0);
    CHECK(reloq_german_initial_nasal_append(
        f.phones, f.count, -1, &f.builder, &first_send) == -1);
    CHECK(reloq_german_initial_nasal_append(
        f.phones, f.count, INT64_MAX, &f.builder, &first_send) == -1);
    CHECK(f.builder.stream[RELOQ_TARGET_AV].count == 0);
    f.builder.stream[RELOQ_TARGET_AV].capacity = 1;
    CHECK(reloq_german_initial_nasal_append(
        f.phones, f.count, 0, &f.builder, &first_send) == -1);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        CHECK(f.builder.stream[s].count == 0);
    return 0;
}

static int dump(const char *annotation)
{
    struct fixture f;
    int64_t first_send;
    if (begin(&f, annotation) || reloq_german_initial_nasal_append(
            f.phones, f.count, 0, &f.builder, &first_send)) return -1;
    printf("first_send %" PRId64 "\n", first_send);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        for (size_t i = 0; i < f.builder.stream[s].count; ++i)
            printf("%s %" PRId64 " %" PRId32 "\n",
                   reloq_target_stream_name((enum reloq_target_stream)s),
                   f.points[s][i].tick, f.points[s][i].value);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) return self_test() ? EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 3 && strcmp(argv[1], "--points") == 0)
        return dump(argv[2]) ? EXIT_FAILURE : EXIT_SUCCESS;
    fprintf(stderr, "usage: %s [--points ANNOTATION]\n", argv[0]);
    return EXIT_FAILURE;
}
