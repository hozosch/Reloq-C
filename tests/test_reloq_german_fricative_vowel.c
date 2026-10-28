#include "reloq_german_fricative_vowel.h"

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
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][2];
    struct reloq_target_builder builder;
    size_t count, vowel_index;
};

static int is_vowel(enum reloq_german_phone p)
{
    return p == RELOQ_DE_PHONE_A || p == RELOQ_DE_PHONE_E ||
           p == RELOQ_DE_PHONE_I || p == RELOQ_DE_PHONE_O ||
           p == RELOQ_DE_PHONE_U || p == RELOQ_DE_PHONE_a ||
           p == RELOQ_DE_PHONE_e || p == RELOQ_DE_PHONE_i ||
           p == RELOQ_DE_PHONE_o || p == RELOQ_DE_PHONE_u;
}

static int begin(struct fixture *f, const char *annotation, size_t capacity)
{
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    memset(f, 0, sizeof *f);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            f->phones, 8, &info)) return -1;
    f->count = info.phone_count;
    f->vowel_index = f->count;
    for (size_t i = 0; i < f->count; ++i)
        if (is_vowel(f->phones[i].phone)) {
            f->vowel_index = i;
            break;
        }
    if (f->vowel_index == f->count) return -1;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        storage[s] = (struct reloq_target_buffer){f->points[s], capacity, 0};
    return reloq_target_builder_begin(&f->builder, storage);
}

static int self_test(void)
{
    static const struct {
        const char *annotation;
        int64_t onset, closure, first;
        int32_t f1, f2, f3, av;
    } cases[] = {
        {"`[.1As].", 0, 107, 67, 780, 1160, 2500, 51},
        {"`[.1hAs].", 80, 187, 147, 780, 1160, 2500, 51},
        {"`[.1Uf].", 0, 92, 67, 420, 930, 2400, 54},
        {"`[.1hUs].", 80, 172, 132, 420, 930, 2400, 58},
        {"`[.1us].", 0, 184, 144, 320, 750, 2400, 55},
        {"`[.1hus].", 80, 264, 224, 320, 750, 2400, 55},
        {"`[.1huf].", 80, 264, 234, 250, 750, 2400, 55},
        {"`[.1ef].", 0, 200, 175, 370, 2150, 2700, 51}
    };
    struct fixture f;
    int64_t closure = -17, first = -19;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        CHECK(begin(&f, cases[i].annotation, 2) == 0);
        CHECK(reloq_german_fricative_vowel_append(
            f.phones, f.count, f.vowel_index, cases[i].onset,
            &f.builder, &closure, &first) == 0);
        CHECK(closure == cases[i].closure && first == cases[i].first);
        const int32_t formants[] = {cases[i].f1, cases[i].f2, cases[i].f3};
        for (size_t j = 0; j < 3; ++j) {
            enum reloq_target_stream s =
                (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)j);
            const int u_s = cases[i].f1 == 320;
            CHECK(f.builder.stream[s].count ==
                  (s == RELOQ_TARGET_F2 && u_s ? 2u : 1u) &&
                  f.points[s][0].tick == cases[i].onset &&
                  f.points[s][0].value == formants[j]);
            if (s == RELOQ_TARGET_F2 && u_s)
                CHECK(f.points[s][1].tick == first &&
                      f.points[s][1].value == 820);
        }
        CHECK(f.builder.stream[RELOQ_TARGET_AV].count == 2 &&
              f.points[RELOQ_TARGET_AV][0].tick == cases[i].onset + 1 &&
              f.points[RELOQ_TARGET_AV][0].value == cases[i].av &&
              f.points[RELOQ_TARGET_AV][1].tick == closure &&
              f.points[RELOQ_TARGET_AV][1].value == cases[i].av - 2);
        CHECK(f.builder.stream[RELOQ_TARGET_AH].count == 2 &&
              f.points[RELOQ_TARGET_AH][0].tick == cases[i].onset + 1 &&
              f.points[RELOQ_TARGET_AH][0].value == 34 &&
              f.points[RELOQ_TARGET_AH][1].tick == closure &&
              f.points[RELOQ_TARGET_AH][1].value == 34);
    }
    const char *uncovered[] = {
        "`[.1A].", "`[.1hAp].", "`[.1hAs.0A].", "`[.0hAs].",
        "`[.1hAs] `[.1s].", "`[.1hAl].", "`[.1Ah].", "`[.1pAs]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        CHECK(begin(&f, uncovered[i], 2) == 0);
        closure = -17; first = -19;
        CHECK(reloq_german_fricative_vowel_append(
            f.phones, f.count, f.vowel_index,
            f.vowel_index ? 80 : 0, &f.builder, &closure, &first) == 1);
        CHECK(closure == -17 && first == -19);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    CHECK(begin(&f, "`[.1hAs].", 2) == 0);
    CHECK(reloq_german_fricative_vowel_append(
        f.phones, f.count, f.vowel_index, -1,
        &f.builder, &closure, &first) == -1);
    CHECK(reloq_german_fricative_vowel_append(
        f.phones, f.count, f.vowel_index, 0,
        &f.builder, &closure, &first) == 1);
    CHECK(begin(&f, "`[.1hAs].", 1) == 0);
    CHECK(reloq_german_fricative_vowel_append(
        f.phones, f.count, f.vowel_index, 80,
        &f.builder, &closure, &first) == -1);
    CHECK(closure == -17 && first == -19);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        CHECK(f.builder.stream[s].count == 0);
    return 0;
}

static int query(const char *annotation)
{
    struct fixture f;
    int64_t closure, first;
    if (begin(&f, annotation, 2) ||
        reloq_german_fricative_vowel_append(
            f.phones, f.count, f.vowel_index,
            f.vowel_index ? 80 : 0, &f.builder, &closure, &first)) return -1;
    printf("closure=%" PRId64 " first=%" PRId64, closure, first);
    const enum reloq_target_stream streams[] = {
        RELOQ_TARGET_F1, RELOQ_TARGET_F2, RELOQ_TARGET_F3,
        RELOQ_TARGET_AV, RELOQ_TARGET_AH
    };
    for (size_t i = 0; i < sizeof streams / sizeof streams[0]; ++i) {
        enum reloq_target_stream s = streams[i];
        printf(" %s=", reloq_target_stream_name(s));
        for (size_t j = 0; j < f.builder.stream[s].count; ++j)
            printf("%s%" PRId64 ":%" PRId32, j ? "," : "",
                   f.points[s][j].tick, f.points[s][j].value);
    }
    puts("");
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
