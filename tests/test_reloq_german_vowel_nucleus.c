#include "reloq_german_vowel_nucleus.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "test_reloq_german_vowel_nucleus.c:%d: %s\n", \
            __LINE__, #test); return -1; \
} } while (0)

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][8];
    struct reloq_target_builder builder;
    size_t count, vowel_index;
};

static int vowel(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_A || phone == RELOQ_DE_PHONE_E ||
           phone == RELOQ_DE_PHONE_I || phone == RELOQ_DE_PHONE_O ||
           phone == RELOQ_DE_PHONE_U || phone == RELOQ_DE_PHONE_a ||
           phone == RELOQ_DE_PHONE_e || phone == RELOQ_DE_PHONE_i ||
           phone == RELOQ_DE_PHONE_o || phone == RELOQ_DE_PHONE_u;
}

static int begin(struct fixture *f, const char *annotation)
{
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_german_phone_sequence_info info = {0};
    memset(f, 0, sizeof *f);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            f->phones, 8, &info)) return -1;
    f->count = info.phone_count;
    f->vowel_index = f->count;
    for (size_t i = 0; i < f->count; ++i)
        if (vowel(f->phones[i].phone)) {
            f->vowel_index = i;
            break;
        }
    if (f->vowel_index == f->count) return -1;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){f->points[i], 8, 0};
    return reloq_target_builder_begin(&f->builder, buffers);
}

static int check(const struct fixture *f, int64_t onset, int64_t closure,
                 int32_t f1, int32_t f2, int32_t f3, int32_t av,
                 int stop)
{
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        size_t expected = i >= RELOQ_TARGET_F1 && i <= RELOQ_TARGET_F3 ? 1 :
            i == RELOQ_TARGET_F0 ? (f->vowel_index ? 3 : 4) :
            i == RELOQ_TARGET_AV ? 3 :
            i == RELOQ_TARGET_AH ? (stop ? 3 : 4) : 0;
        CHECK(f->builder.stream[i].count == expected);
    }
    const int32_t formants[] = {f1, f2, f3};
    for (size_t i = 0; i < 3; ++i) {
        CHECK(f->points[RELOQ_TARGET_F1 + i][0].tick == onset);
        CHECK(f->points[RELOQ_TARGET_F1 + i][0].value == formants[i]);
    }
    const struct reloq_acoustic_point *a = f->points[RELOQ_TARGET_AV];
    CHECK(a[0].tick == onset + 1 && a[0].value == av);
    CHECK(a[1].tick == onset + closure && a[1].value == av - 2);
    CHECK(a[2].tick == onset + closure + (stop ? 0 : 60));
    CHECK(a[2].value == 0);
    const struct reloq_acoustic_point *h = f->points[RELOQ_TARGET_AH];
    CHECK(h[0].tick == onset + 1 && h[0].value == 34);
    CHECK(h[1].tick == onset + closure && h[1].value == 34);
    CHECK(h[2].tick == onset + closure && h[2].value == (stop ? 0 : 35));
    if (!stop) {
        CHECK(h[3].tick == onset + closure + 60);
        CHECK(h[3].value == 0);
    }
    const int64_t f0_ticks[] = {closure * 2 / 100,
                                closure * 35 / 100,
                                closure * 70 / 100,
                                closure * 99 / 100};
    const int32_t f0_values[] = {1217, 1296, 796, 743};
    size_t skip = f->vowel_index ? 1 : 0;
    for (size_t i = skip; i < 4; ++i) {
        CHECK(f->points[RELOQ_TARGET_F0][i - skip].tick ==
              onset + f0_ticks[i]);
        CHECK(f->points[RELOQ_TARGET_F0][i - skip].value == f0_values[i]);
    }
    return 0;
}

static int self_test(void)
{
    static const struct {
        const char *annotation;
        int64_t onset, closure;
        int32_t f1, f2, f3, av;
        int stop;
    } cases[] = {
        {"`[.1A].", 0, 97, 700, 1160, 2500, 51, 0},
        {"`[.1hAt].", 80, 97, 780, 1300, 2500, 51, 1},
        {"`[.1hAp].", 80, 97, 780, 1160, 2500, 51, 1},
        {"`[.1hEk].", 80, 105, 550, 1800, 2600, 52, 1},
        {"`[.1hIp].", 80, 98, 380, 2000, 2800, 53, 1},
        {"`[.1hOt].", 80, 98, 570, 900, 2500, 53, 1},
        {"`[.1hUk].", 80, 84, 420, 930, 2400, 54, 1},
        {"`[.1ha].", 80, 210, 800, 1220, 2500, 50, 0},
        {"`[.1hat].", 80, 168, 800, 1220, 2500, 50, 1},
        {"`[.1hek].", 80, 182, 370, 2150, 2700, 51, 1},
        {"`[.1hip].", 80, 168, 265, 2150, 2900, 53, 1},
        {"`[.1hot].", 80, 182, 390, 735, 2400, 54, 1},
        {"`[.1huk].", 80, 168, 250, 750, 2400, 55, 1}
    };
    struct fixture f;
    int64_t closure = -17;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        CHECK(begin(&f, cases[i].annotation) == 0);
        CHECK(reloq_german_vowel_nucleus_append(
            f.phones, f.count, f.vowel_index, cases[i].onset,
            1, &f.builder, &closure) == 0);
        CHECK(closure == cases[i].closure);
        CHECK(check(&f, cases[i].onset, cases[i].closure,
                    cases[i].f1, cases[i].f2, cases[i].f3,
                    cases[i].av, cases[i].stop) == 0);
    }

    const char *uncovered[] = {
        "`[.1hAk.0A].", "`[.1hAkl].", "`[.1Ah].",
        "`[.1pA].", "`[.0A].", "`[.1hA] `[.1k]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        CHECK(begin(&f, uncovered[i]) == 0);
        closure = -17;
        CHECK(reloq_german_vowel_nucleus_append(
            f.phones, f.count, f.vowel_index, 80, 1, &f.builder,
            &closure) == 1);
        CHECK(closure == -17);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    CHECK(begin(&f, "`[.1A].") == 0);
    closure = -17;
    CHECK(reloq_german_vowel_nucleus_append(
        f.phones, f.count, f.vowel_index, -1, 1, &f.builder,
        &closure) == -1);
    CHECK(closure == -17);
    CHECK(f.builder.stream[RELOQ_TARGET_AV].count == 0);
    CHECK(reloq_german_vowel_nucleus_append(
        f.phones, f.count, f.vowel_index, INT64_MAX, 1, &f.builder,
        &closure) == -1);
    CHECK(closure == -17);
    CHECK(f.builder.stream[RELOQ_TARGET_AV].count == 0);
    CHECK(reloq_german_vowel_nucleus_append(
        f.phones, f.count, f.vowel_index, 0, 2, &f.builder,
        &closure) == -1);
    CHECK(closure == -17);
    CHECK(f.builder.stream[RELOQ_TARGET_AV].count == 0);
    return 0;
}

static int query(const char *annotation, const char *tick_text)
{
    char *end = NULL;
    errno = 0;
    intmax_t parsed = strtoimax(tick_text, &end, 10);
    if (errno || end == tick_text || *end || parsed < 0 ||
        parsed > INT64_MAX) return -1;
    struct fixture f;
    int64_t closure;
    if (begin(&f, annotation) || reloq_german_vowel_nucleus_append(
            f.phones, f.count, f.vowel_index, (int64_t)parsed,
            1, &f.builder, &closure)) return -1;
    printf("closure=%" PRId64 " F1=%" PRId32 " F2=%" PRId32
           " F3=%" PRId32 " AV=", closure,
           f.points[RELOQ_TARGET_F1][0].value,
           f.points[RELOQ_TARGET_F2][0].value,
           f.points[RELOQ_TARGET_F3][0].value);
    for (size_t i = 0; i < 3; ++i)
        printf("%s%" PRId64 ":%" PRId32, i ? "," : "",
               f.points[RELOQ_TARGET_AV][i].tick,
               f.points[RELOQ_TARGET_AV][i].value);
    printf(" AH=");
    for (size_t i = 0; i < f.builder.stream[RELOQ_TARGET_AH].count; ++i)
        printf("%s%" PRId64 ":%" PRId32, i ? "," : "",
               f.points[RELOQ_TARGET_AH][i].tick,
               f.points[RELOQ_TARGET_AH][i].value);
    printf(" F0=");
    for (size_t i = 0; i < f.builder.stream[RELOQ_TARGET_F0].count; ++i)
        printf("%s%" PRId64 ":%" PRId32, i ? "," : "",
               f.points[RELOQ_TARGET_F0][i].tick,
               f.points[RELOQ_TARGET_F0][i].value);
    puts("");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) return self_test() ? EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 4 && strcmp(argv[1], "--query") == 0)
        return query(argv[2], argv[3]) ? EXIT_FAILURE : EXIT_SUCCESS;
    fprintf(stderr, "usage: %s [--query SPR ONSET_TICK]\n", argv[0]);
    return EXIT_FAILURE;
}
