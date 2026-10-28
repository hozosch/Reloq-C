#include "reloq_german_terminal_fricative.h"

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

static int check(const struct fixture *f, enum reloq_target_stream stream,
                 int64_t closure, int64_t duration, int32_t first,
                 int32_t last, size_t n)
{
    CHECK(f->builder.stream[stream].count == n);
    if (n > 0)
        CHECK(f->points[stream][0].tick == closure &&
              f->points[stream][0].value == first);
    if (n > 1)
        CHECK(f->points[stream][1].tick == closure + duration &&
              f->points[stream][1].value == first);
    if (n > 2)
        CHECK(f->points[stream][2].tick == closure + duration + 30 &&
              f->points[stream][2].value == last);
    return 0;
}

static int self_test(void)
{
    static const struct {
        const char *annotation;
        int64_t closure, duration;
        int32_t f1, f2, f3, f4, f5, af, active;
        enum reloq_target_stream resonator;
    } cases[] = {
        {"`[.1As].", 107, 155, 300, 1500, 2600, 3400, 4400, 62, 50, RELOQ_TARGET_A5F},
        {"`[.1hes].", 280, 155, 300, 1800, 2600, 3400, 4400, 62, 50, RELOQ_TARGET_A5F},
        {"`[.1Us].", 92, 155, 300, 1500, 2600, 3400, 4400, 62, 50, RELOQ_TARGET_A5F},
        {"`[.1Af].", 107, 150, 400, 1000, 2300, 3600, 3900, 58, 60, RELOQ_TARGET_AB},
        {"`[.1hEf].", 195, 150, 400, 1450, 2500, 3600, 3900, 58, 60, RELOQ_TARGET_AB},
        {"`[.1uf].", 184, 150, 400, 750, 2300, 3600, 3900, 58, 60, RELOQ_TARGET_AB}
    };
    struct fixture f;
    int64_t release = -19;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        CHECK(begin(&f, cases[i].annotation, 4) == 0);
        CHECK(reloq_german_terminal_fricative_append(
            f.phones, f.count, f.vowel_index, cases[i].closure,
            &f.builder, &release) == 0);
        CHECK(release == cases[i].closure + cases[i].duration);
        const int32_t oral[] = {cases[i].f1, cases[i].f2,
            cases[i].f3, cases[i].f4, cases[i].f5};
        const int32_t bandwidth[] = {120, 100, 150, 330, 260};
        for (size_t j = 0; j < 5; ++j) {
            enum reloq_target_stream s =
                (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)j);
            if (j < 3) CHECK(check(&f, s, cases[i].closure,
                                   cases[i].duration, oral[j], 0, 2) == 0);
            else CHECK(check(&f, s, cases[i].closure,
                             cases[i].duration, oral[j],
                             j == 3 ? 3600 : 3900, 3) == 0);
            s = (enum reloq_target_stream)(RELOQ_TARGET_B1 + (int)j);
            CHECK(check(&f, s, cases[i].closure, cases[i].duration,
                        bandwidth[j], bandwidth[j], 3) == 0);
        }
        for (enum reloq_target_stream s = RELOQ_TARGET_FNP;
             s <= RELOQ_TARGET_FNZ; ++s)
            CHECK(check(&f, s, cases[i].closure, cases[i].duration,
                        200, 200, 3) == 0);
        CHECK(check(&f, cases[i].resonator, cases[i].closure,
                    cases[i].duration, cases[i].active, 0, 3) == 0);
        CHECK(check(&f, RELOQ_TARGET_AF, cases[i].closure,
                    cases[i].duration, cases[i].af, 0, 3) == 0);
        CHECK(check(&f, RELOQ_TARGET_OQ, cases[i].closure,
                    cases[i].duration, 56, 56, 3) == 0);
        CHECK(f.builder.stream[RELOQ_TARGET_F0].count == 0);
        CHECK(f.builder.stream[RELOQ_TARGET_FL].count == 1 &&
              f.points[RELOQ_TARGET_FL][0].tick == release &&
              f.points[RELOQ_TARGET_FL][0].value == 0);
    }
    const char *uncovered[] = {
        "`[.1A].", "`[.1hAp].", "`[.1As.0A].", "`[.0hAs].",
        "`[.1hAs] `[.1s].", "`[.1hAl].", "`[.1Ah]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        CHECK(begin(&f, uncovered[i], 4) == 0);
        release = -19;
        CHECK(reloq_german_terminal_fricative_append(
            f.phones, f.count, f.vowel_index, 100,
            &f.builder, &release) == 1);
        CHECK(release == -19);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    CHECK(begin(&f, "`[.1hAs].", 4) == 0);
    CHECK(reloq_german_terminal_fricative_append(
        f.phones, f.count, f.vowel_index, -1, &f.builder, &release) == -1);
    CHECK(reloq_german_terminal_fricative_append(
        f.phones, f.count, f.vowel_index, INT64_MAX,
        &f.builder, &release) == -1);
    CHECK(release == -19);
    CHECK(begin(&f, "`[.1hAs].", 1) == 0);
    CHECK(reloq_german_terminal_fricative_append(
        f.phones, f.count, f.vowel_index, 187,
        &f.builder, &release) == -1);
    CHECK(release == -19);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        CHECK(f.builder.stream[s].count == 0);
    return 0;
}

static int marked_windows(void)
{
    static const struct {
        enum reloq_german_terminal_mark mark;
        int32_t window;
    } marks[] = {
        {RELOQ_DE_TERMINAL_IMPLICIT, 430},
        {RELOQ_DE_TERMINAL_PERIOD, 430},
        {RELOQ_DE_TERMINAL_QUESTION, 430},
        {RELOQ_DE_TERMINAL_EXCLAMATION, 430},
        {RELOQ_DE_TERMINAL_COLON, 430},
        {RELOQ_DE_TERMINAL_SEMICOLON, 280},
        {RELOQ_DE_TERMINAL_COMMA, 180},
        {RELOQ_DE_TERMINAL_ELLIPSIS, 180}
    };
    static const struct {
        const char *annotation;
        int64_t closure, duration;
    } shapes[] = {
        {"`[.1As].", 107, 155},
        {"`[.1hAf].", 187, 150}
    };
    for (size_t shape = 0; shape < sizeof shapes / sizeof shapes[0]; ++shape) {
        struct fixture baseline;
        int64_t base_release = -1;
        CHECK(begin(&baseline, shapes[shape].annotation, 4) == 0);
        CHECK(reloq_german_terminal_fricative_append(
            baseline.phones, baseline.count, baseline.vowel_index,
            shapes[shape].closure, &baseline.builder, &base_release) == 0);
        for (size_t mark = 0; mark < sizeof marks / sizeof marks[0]; ++mark) {
            struct fixture f;
            int64_t release = -31, end = -31;
            CHECK(begin(&f, shapes[shape].annotation, 4) == 0);
            CHECK(reloq_german_terminal_fricative_append_marked(
                f.phones, f.count, f.vowel_index, shapes[shape].closure,
                marks[mark].mark, &f.builder, &release, &end) == 0);
            CHECK(release == base_release);
            CHECK(end == shapes[shape].closure + shapes[shape].duration +
                  marks[mark].window);
            for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
                CHECK(f.builder.stream[s].count ==
                      baseline.builder.stream[s].count);
                for (size_t j = 0; j < f.builder.stream[s].count; ++j) {
                    CHECK(f.points[s][j].tick ==
                          baseline.points[s][j].tick);
                    CHECK(f.points[s][j].value ==
                          baseline.points[s][j].value);
                }
            }
        }
    }
    struct fixture f;
    int64_t release = -31, end = -29;
    CHECK(begin(&f, "`[.1hAs]?", 4) == 0);
    CHECK(reloq_german_terminal_fricative_append_marked(
        f.phones, f.count, f.vowel_index, 187,
        (enum reloq_german_terminal_mark)999, &f.builder,
        &release, &end) == -1);
    CHECK(release == -31 && end == -29);
    CHECK(reloq_german_terminal_fricative_append_marked(
        f.phones, f.count, f.vowel_index, INT64_MAX - 155 - 429,
        RELOQ_DE_TERMINAL_PERIOD, &f.builder, &release, &end) == -1);
    CHECK(release == -31 && end == -29);
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        CHECK(f.builder.stream[s].count == 0);
    CHECK(begin(&f, "`[.1hAp]?", 4) == 0);
    CHECK(reloq_german_terminal_fricative_append_marked(
        f.phones, f.count, f.vowel_index, 187,
        RELOQ_DE_TERMINAL_QUESTION, &f.builder, &release, &end) == 1);
    CHECK(release == -31 && end == -29);
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
        reloq_german_terminal_fricative_append(
            f.phones, f.count, f.vowel_index, (int64_t)parsed,
            &f.builder, &release)) return -1;
    printf("release=%" PRId64, release);
    for (size_t s = RELOQ_TARGET_F1; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        printf(" %s=", reloq_target_stream_name((enum reloq_target_stream)s));
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
    if (argc == 1) return self_test() || marked_windows() ?
        EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 4 && strcmp(argv[1], "--query") == 0)
        return query(argv[2], argv[3]) ? EXIT_FAILURE : EXIT_SUCCESS;
    fprintf(stderr, "usage: %s [--query SPR VOWEL_CLOSURE_TICK]\n", argv[0]);
    return EXIT_FAILURE;
}
