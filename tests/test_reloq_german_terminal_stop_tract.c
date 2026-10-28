#include "reloq_german_terminal_stop_tract.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); \
    return -1; \
} } while (0)

static const enum reloq_target_stream streams[] = {
    RELOQ_TARGET_F1, RELOQ_TARGET_F2, RELOQ_TARGET_F3,
    RELOQ_TARGET_F4, RELOQ_TARGET_F5,
    RELOQ_TARGET_B1, RELOQ_TARGET_B2, RELOQ_TARGET_B3,
    RELOQ_TARGET_B4, RELOQ_TARGET_B5,
    RELOQ_TARGET_FNP, RELOQ_TARGET_FNZ,
    RELOQ_TARGET_FTP, RELOQ_TARGET_FTZ,
    RELOQ_TARGET_OQ, RELOQ_TARGET_DI, RELOQ_TARGET_FL
};

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][4];
    struct reloq_target_builder builder;
    size_t count, vowel_index;
};

static int is_vowel(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_A || phone == RELOQ_DE_PHONE_E ||
           phone == RELOQ_DE_PHONE_I || phone == RELOQ_DE_PHONE_O ||
           phone == RELOQ_DE_PHONE_U || phone == RELOQ_DE_PHONE_a ||
           phone == RELOQ_DE_PHONE_e || phone == RELOQ_DE_PHONE_i ||
           phone == RELOQ_DE_PHONE_o || phone == RELOQ_DE_PHONE_u;
}

static int begin(struct fixture *f, const char *annotation,
                 size_t capacity)
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

static int check_pair(const struct fixture *f, enum reloq_target_stream s,
                      int64_t first, int32_t a, int64_t second, int32_t b)
{
    CHECK(f->builder.stream[s].count == 2);
    CHECK(f->points[s][0].tick == first && f->points[s][0].value == a);
    CHECK(f->points[s][1].tick == second && f->points[s][1].value == b);
    return 0;
}

static int self_test(void)
{
    static const struct {
        const char *annotation;
        int64_t closure, release;
        int32_t f1, f2, f3, final_f2, final_f3;
        int32_t f4, f5;
        int64_t preburst;
    } cases[] = {
        {"`[.1Ap].", 97, 222, 500, 900, 2500, 1000, 2500, 3600, 3900, 10},
        {"`[.1hAt].", 177, 285, 300, 1600, 2500, 1600, 2500, 3500, 4200, 8},
        {"`[.1hIk].", 178, 300, 300, 1900, 2300, 1900, 2300, 3600, 3900, 12},
        {"`[.1ek].", 182, 304, 300, 1900, 2500, 1900, 2300, 3600, 3900, 12},
        {"`[.1it].", 168, 276, 300, 1900, 2700, 1800, 2700, 3500, 4200, 8},
        {"`[.1ok].", 182, 304, 400, 960, 2300, 1500, 2300, 3600, 3900, 12}
    };
    struct fixture f;
    int64_t release = -19;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        CHECK(begin(&f, cases[i].annotation, 4) == 0);
        CHECK(reloq_german_terminal_stop_tract_append(
            f.phones, f.count, f.vowel_index, cases[i].closure,
            &f.builder, &release) == 0);
        CHECK(release == cases[i].release);
        CHECK(check_pair(&f, RELOQ_TARGET_F1, cases[i].closure,
                         cases[i].f1, release, 300) == 0);
        CHECK(check_pair(&f, RELOQ_TARGET_F2, cases[i].closure,
                         cases[i].f2, release, cases[i].final_f2) == 0);
        CHECK(check_pair(&f, RELOQ_TARGET_F3, cases[i].closure,
                         cases[i].f3, release, cases[i].final_f3) == 0);
        CHECK(check_pair(&f, RELOQ_TARGET_F4, cases[i].closure,
                         cases[i].f4, release, cases[i].f4) == 0);
        CHECK(check_pair(&f, RELOQ_TARGET_F5, cases[i].closure,
                         cases[i].f5, release, cases[i].f5) == 0);
        const int32_t widths[] = {120, 100, 150, 330, 260};
        for (size_t b = 0; b < 5; ++b)
            CHECK(check_pair(&f, (enum reloq_target_stream)
                             (RELOQ_TARGET_B1 + (int)b), cases[i].closure,
                             widths[b], release, widths[b]) == 0);
        CHECK(check_pair(&f, RELOQ_TARGET_FNP, cases[i].closure,
                         200, release, 200) == 0);
        CHECK(check_pair(&f, RELOQ_TARGET_FNZ, cases[i].closure,
                         200, release, 200) == 0);
        CHECK(check_pair(&f, RELOQ_TARGET_OQ, cases[i].closure,
                         56, release - cases[i].preburst, 56) == 0);
        CHECK(check_pair(&f, RELOQ_TARGET_DI, cases[i].closure,
                         0, release - cases[i].preburst, 0) == 0);
        const enum reloq_target_stream once[] = {
            RELOQ_TARGET_FTP, RELOQ_TARGET_FTZ, RELOQ_TARGET_FL
        };
        for (size_t j = 0; j < sizeof once / sizeof once[0]; ++j) {
            enum reloq_target_stream s = once[j];
            CHECK(f.builder.stream[s].count == 1);
            CHECK(f.points[s][0].tick == release);
            CHECK(f.points[s][0].value ==
                  (s == RELOQ_TARGET_FL ? 0 : 250));
        }
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
            int used = 0;
            for (size_t j = 0; j < sizeof streams / sizeof streams[0]; ++j)
                used |= s == (size_t)streams[j];
            if (!used) CHECK(f.builder.stream[s].count == 0);
        }
    }
    const char *uncovered[] = {
        "`[.1A].", "`[.1pA].", "`[.1hAk.0A].", "`[.0hAk].",
        "`[.1hAk] `[.1k].", "`[.1hAl].", "`[.1Ah]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        CHECK(begin(&f, uncovered[i], 4) == 0);
        release = -19;
        CHECK(reloq_german_terminal_stop_tract_append(
            f.phones, f.count, f.vowel_index, 100,
            &f.builder, &release) == 1);
        CHECK(release == -19);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    CHECK(begin(&f, "`[.1hAk].", 4) == 0);
    CHECK(reloq_german_terminal_stop_tract_append(
        f.phones, f.count, f.vowel_index, -1, &f.builder, &release) == -1);
    CHECK(reloq_german_terminal_stop_tract_append(
        f.phones, f.count, f.vowel_index, INT64_MAX,
        &f.builder, &release) == -1);
    CHECK(release == -19);
    CHECK(begin(&f, "`[.1hAk].", 1) == 0);
    CHECK(reloq_german_terminal_stop_tract_append(
        f.phones, f.count, f.vowel_index, 177,
        &f.builder, &release) == -1);
    CHECK(release == -19);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        CHECK(f.builder.stream[s].count == 0);
    return 0;
}

static int query(const char *annotation, const char *closure_text)
{
    errno = 0;
    char *end = NULL;
    intmax_t parsed = strtoimax(closure_text, &end, 10);
    if (errno || end == closure_text || *end || parsed < 0 ||
        parsed > INT64_MAX) return -1;
    struct fixture f;
    int64_t release;
    if (begin(&f, annotation, 4) ||
        reloq_german_terminal_stop_tract_append(
            f.phones, f.count, f.vowel_index, (int64_t)parsed,
            &f.builder, &release)) return -1;
    printf("release=%" PRId64, release);
    for (size_t i = 0; i < sizeof streams / sizeof streams[0]; ++i) {
        enum reloq_target_stream s = streams[i];
        printf(" %s=", reloq_target_stream_name(s));
        for (size_t j = 0; j < f.builder.stream[s].count; ++j)
            printf("%s%" PRId64 ":%" PRId32, j ? "," : "",
                   f.points[s][j].tick - (int64_t)parsed,
                   f.points[s][j].value);
    }
    puts("");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) return self_test() ? EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 4 && strcmp(argv[1], "--query") == 0)
        return query(argv[2], argv[3]) ? EXIT_FAILURE : EXIT_SUCCESS;
    fprintf(stderr, "usage: %s [--query SPR VOWEL_CLOSURE_TICK]\n", argv[0]);
    return EXIT_FAILURE;
}
