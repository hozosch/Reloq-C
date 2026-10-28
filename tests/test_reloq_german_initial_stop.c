#include "reloq_german_initial_stop.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "test_reloq_german_initial_stop.c:%d: %s\n", \
            __LINE__, #test); return -1; \
} } while (0)

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][8];
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
        buffers[i] = (struct reloq_target_buffer){f->points[i], 8, 0};
    return reloq_target_builder_begin(&f->builder, buffers);
}

static int check(const struct fixture *f, int64_t origin, int64_t onset,
                 int64_t burst, int32_t af, int32_t ah,
                 enum reloq_target_stream noise, int32_t gain)
{
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        size_t expected = i == RELOQ_TARGET_AV ? 2 :
            i == RELOQ_TARGET_AH ? 4 :
            i == RELOQ_TARGET_AF || i == (size_t)noise ? 5 : 0;
        CHECK(f->builder.stream[i].count == expected);
    }
    const struct reloq_acoustic_point *av = f->points[RELOQ_TARGET_AV];
    CHECK(av[0].tick == origin && av[0].value == 0);
    CHECK(av[1].tick == origin + onset && av[1].value == 0);
    const struct reloq_acoustic_point *h = f->points[RELOQ_TARGET_AH];
    CHECK(h[0].tick == origin && h[0].value == 0);
    CHECK(h[1].tick == origin + burst && h[1].value == 0);
    CHECK(h[2].tick == origin + burst && h[2].value == ah);
    CHECK(h[3].tick == origin + onset && h[3].value == ah);
    const enum reloq_target_stream streams[] = {RELOQ_TARGET_AF, noise};
    const int32_t values[] = {af, gain};
    for (size_t s = 0; s < 2; ++s) {
        const struct reloq_acoustic_point *p = f->points[streams[s]];
        const int64_t ticks[] = {0, 5, 5, burst, onset};
        const int32_t levels[] = {0, 0, values[s], values[s], 0};
        for (size_t i = 0; i < 5; ++i) {
            CHECK(p[i].tick == origin + ticks[i]);
            CHECK(p[i].value == levels[i]);
        }
    }
    return 0;
}

static int self_test(void)
{
    static const struct {
        const char *annotation;
        int64_t onset, burst;
        int32_t af, ah, gain;
        enum reloq_target_stream noise;
    } cases[] = {
        {"`[.1pA].", 65, 15, 60, 49, 64, RELOQ_TARGET_AB},
        {"`[.1pAp].", 65, 15, 60, 49, 64, RELOQ_TARGET_AB},
        {"`[.1pAs].", 65, 15, 60, 49, 64, RELOQ_TARGET_AB},
        {"`[.1pAf].", 65, 15, 60, 49, 64, RELOQ_TARGET_AB},
        {"`[.1pU].", 50, 15, 60, 49, 66, RELOQ_TARGET_AB},
        {"`[.1pu].", 59, 15, 60, 49, 66, RELOQ_TARGET_AB},
        {"`[.1pe].", 78, 15, 60, 49, 64, RELOQ_TARGET_AB},
        {"`[.1tA].", 89, 17, 59, 47, 56, RELOQ_TARGET_A5F},
        {"`[.1tE].", 95, 23, 59, 47, 56, RELOQ_TARGET_A5F},
        {"`[.1te].", 113, 23, 59, 47, 56, RELOQ_TARGET_A5F},
        {"`[.1tis].", 113, 23, 59, 47, 56, RELOQ_TARGET_A5F},
        {"`[.1tu].", 80, 17, 59, 47, 56, RELOQ_TARGET_A5F},
        {"`[.1kA].", 78, 17, 60, 45, 70, RELOQ_TARGET_A2F},
        {"`[.1kE].", 78, 17, 54, 45, 70, RELOQ_TARGET_A2F},
        {"`[.1kI].", 78, 17, 64, 45, 60, RELOQ_TARGET_A3F},
        {"`[.1ke].", 93, 17, 64, 45, 60, RELOQ_TARGET_A3F},
        {"`[.1ko].", 93, 17, 62, 45, 70, RELOQ_TARGET_A2F},
        {"`[.1kus].", 93, 17, 62, 45, 70, RELOQ_TARGET_A2F}
    };
    struct fixture f;
    int64_t onset = -7;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        CHECK(begin(&f, cases[i].annotation) == 0);
        CHECK(reloq_german_initial_stop_append(
            f.phones, f.count, 31, &f.builder, &onset) == 0);
        CHECK(onset == cases[i].onset);
        CHECK(check(&f, 31, cases[i].onset, cases[i].burst,
                    cases[i].af, cases[i].ah, cases[i].noise,
                    cases[i].gain) == 0);
    }
    const char *uncovered[] = {
        "`[.1hA].", "`[.1pA.0A].", "`[.0pA].",
        "`[.1pA] `[.1t].", "`[.1pAl].", "`[.1pAj]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        CHECK(begin(&f, uncovered[i]) == 0);
        onset = -7;
        CHECK(reloq_german_initial_stop_append(
            f.phones, f.count, 0, &f.builder, &onset) == 1);
        CHECK(onset == -7);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    CHECK(begin(&f, "`[.1pA].") == 0);
    CHECK(reloq_german_initial_stop_append(
        f.phones, f.count, -1, &f.builder, &onset) == -1);
    CHECK(reloq_german_initial_stop_append(
        f.phones, f.count, INT64_MAX, &f.builder, &onset) == -1);
    CHECK(onset == -7);
    CHECK(f.builder.stream[RELOQ_TARGET_AF].count == 0);
    return 0;
}

static int query(const char *annotation)
{
    struct fixture f;
    int64_t onset;
    if (begin(&f, annotation) || reloq_german_initial_stop_append(
            f.phones, f.count, 0, &f.builder, &onset)) return -1;
    enum reloq_target_stream noise = RELOQ_TARGET_AB;
    for (size_t s = RELOQ_TARGET_A1F; s <= RELOQ_TARGET_AB; ++s)
        if (f.builder.stream[s].count == 5) noise = (enum reloq_target_stream)s;
    printf("onset=%" PRId64 " burst=%" PRId64 " AF=%" PRId32
           " AH=%" PRId32 " noise=%s:%" PRId32 "\n",
           onset, f.points[RELOQ_TARGET_AF][3].tick,
           f.points[RELOQ_TARGET_AF][2].value,
           f.points[RELOQ_TARGET_AH][2].value,
           reloq_target_stream_name(noise), f.points[noise][2].value);
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
