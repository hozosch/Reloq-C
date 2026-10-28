#include "reloq_german_stop_fricative_vowel.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); \
    return -1; \
} } while (0)

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][4];
    struct reloq_target_builder builder;
    size_t count;
};

static int begin(struct fixture *f, const char *annotation, size_t capacity)
{
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    memset(f, 0, sizeof *f);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            f->phones, 8, &info)) return -1;
    f->count = info.phone_count;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        storage[i] = (struct reloq_target_buffer){
            f->points[i], capacity, 0};
    return reloq_target_builder_begin(&f->builder, storage);
}

static int check_four(const struct fixture *f, enum reloq_target_stream s,
                      int64_t onset, int64_t burst, int64_t hold,
                      const int32_t values[4])
{
    const int64_t ticks[4] = {0, burst, onset, hold};
    CHECK(f->builder.stream[s].count == 4);
    for (size_t i = 0; i < 4; ++i)
        CHECK(f->points[s][i].tick == ticks[i] &&
              f->points[s][i].value == values[i]);
    return 0;
}

static int reference_cases(void)
{
    static const struct {
        const char *annotation;
        int64_t onset, burst, closure, hold;
        int32_t f[3][4], av;
    } cases[] = {
        {"`[.1pAs].", 65,15,96,121,
         {{350,350,780,780},{900,900,1150,1150},
          {2500,2500,2500,2500}},51},
        {"`[.1pAf].", 65,15,96,136,
         {{350,350,780,780},{900,900,1150,1150},
          {2500,2500,2500,2500}},51},
        {"`[.1tis].", 113,23,166,254,
         {{300,300,265,265},{1600,1700,2150,2150},
          {2500,2700,2900,2900}},53},
        {"`[.1kus].", 93,17,166,219,
         {{300,300,320,320},{1500,1250,750,820},
          {2300,2000,2400,2400}},55},
        {"`[.1pus].", 59,15,166,185,
         {{350,350,320,320},{800,800,750,820},
          {2500,2500,2400,2400}},55},
        {"`[.1pUs].", 50,15,83,93,
         {{350,350,420,420},{800,800,900,900},
          {2500,2500,2400,2400}},58},
        {"`[.1pef].", 78,15,180,233,
         {{350,350,370,370},{1250,1250,2000,2150},
          {2500,2500,2700,2700}},51},
        {"`[.1kAf].", 78,17,96,149,
         {{300,300,780,780},{1500,1600,1250,1250},
          {2300,2100,2400,2400}},51}
    };
    for (size_t c = 0; c < sizeof cases / sizeof cases[0]; ++c) {
        struct fixture f;
        int64_t closure = -17, burst = -17, hold = -17;
        CHECK(begin(&f, cases[c].annotation, 4) == 0);
        CHECK(reloq_german_stop_fricative_vowel_append(
            f.phones, f.count, cases[c].onset, &f.builder,
            &closure, &burst, &hold) == 0);
        CHECK(closure == cases[c].closure && burst == cases[c].burst &&
              hold == cases[c].hold);
        for (size_t i = 0; i < 3; ++i)
            CHECK(check_four(&f, (enum reloq_target_stream)
                             (RELOQ_TARGET_F1 + (int)i),
                             cases[c].onset, burst, hold,
                             cases[c].f[i]) == 0);
        const struct reloq_acoustic_point *av = f.points[RELOQ_TARGET_AV];
        const struct reloq_acoustic_point *ah = f.points[RELOQ_TARGET_AH];
        CHECK(f.builder.stream[RELOQ_TARGET_AV].count == 2 &&
              f.builder.stream[RELOQ_TARGET_AH].count == 2);
        CHECK(av[0].tick == cases[c].onset + 1 &&
              av[0].value == cases[c].av &&
              av[1].tick == cases[c].onset + closure &&
              av[1].value == cases[c].av - 2);
        CHECK(ah[0].tick == cases[c].onset + 1 &&
              ah[0].value == 34 &&
              ah[1].tick == cases[c].onset + closure &&
              ah[1].value == 34);
        for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
            if (i < RELOQ_TARGET_F1 || i > RELOQ_TARGET_F3)
                if (i != RELOQ_TARGET_AV && i != RELOQ_TARGET_AH)
                    CHECK(f.builder.stream[i].count == 0);
    }
    return 0;
}

static int failure_cases(void)
{
    struct fixture f;
    const char *uncovered[] = {
        "`[.1pAp].", "`[.1hAs].", "`[.1pA].", "`[.0tIs].",
        "`[.1pA.0s].", "`[.1pAs] `[.1kAf].", "`[.1pAl]."
    };
    int64_t closure = -17, burst = -19, hold = -23;
    for (size_t c = 0; c < sizeof uncovered / sizeof uncovered[0]; ++c) {
        CHECK(begin(&f, uncovered[c], 4) == 0);
        CHECK(reloq_german_stop_fricative_vowel_append(
            f.phones, f.count, 65, &f.builder,
            &closure, &burst, &hold) == 1);
        CHECK(closure == -17 && burst == -19 && hold == -23);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    CHECK(begin(&f, "`[.1pAs].", 4) == 0);
    CHECK(reloq_german_stop_fricative_vowel_append(
        f.phones, f.count, -1, &f.builder,
        &closure, &burst, &hold) == -1);
    CHECK(reloq_german_stop_fricative_vowel_append(
        f.phones, f.count, INT64_MAX, &f.builder,
        &closure, &burst, &hold) == -1);
    CHECK(begin(&f, "`[.1pAs].", 3) == 0);
    CHECK(reloq_german_stop_fricative_vowel_append(
        f.phones, f.count, 65, &f.builder,
        &closure, &burst, &hold) == -1);
    CHECK(closure == -17 && burst == -19 && hold == -23);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        CHECK(f.builder.stream[s].count == 0);
    return 0;
}

static int query(const char *annotation, const char *onset_text)
{
    errno = 0;
    char *end = NULL;
    intmax_t parsed = strtoimax(onset_text, &end, 10);
    if (errno || end == onset_text || *end || parsed < 0 ||
        parsed > INT64_MAX) return -1;
    struct fixture f;
    int64_t closure, burst, hold;
    if (begin(&f, annotation, 4) ||
        reloq_german_stop_fricative_vowel_append(
            f.phones, f.count, (int64_t)parsed, &f.builder,
            &closure, &burst, &hold)) return -1;
    printf("closure=%" PRId64 " burst=%" PRId64 " second=%" PRId64,
           (int64_t)parsed + closure, burst, hold);
    const enum reloq_target_stream streams[] = {
        RELOQ_TARGET_F1, RELOQ_TARGET_F2, RELOQ_TARGET_F3,
        RELOQ_TARGET_AV, RELOQ_TARGET_AH};
    for (size_t s = 0; s < sizeof streams / sizeof streams[0]; ++s) {
        const enum reloq_target_stream stream = streams[s];
        printf(" %s=", reloq_target_stream_name(stream));
        for (size_t i = 0; i < f.builder.stream[stream].count; ++i)
            printf("%s%" PRId64 ":%" PRId32, i ? "," : "",
                   f.points[stream][i].tick,
                   f.points[stream][i].value);
    }
    puts("");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) return reference_cases() || failure_cases() ?
        EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 4 && strcmp(argv[1], "--query") == 0)
        return query(argv[2], argv[3]) ? EXIT_FAILURE : EXIT_SUCCESS;
    fprintf(stderr, "usage: %s [--query SPR VOWEL_ONSET_TICK]\n", argv[0]);
    return EXIT_FAILURE;
}
