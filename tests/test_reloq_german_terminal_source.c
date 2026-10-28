#include "reloq_german_terminal_source.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "test_reloq_german_terminal_source.c:%d: %s\n", \
            __LINE__, #test); return -1; \
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

static int begin(struct fixture *f, const char *annotation)
{
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
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
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){f->points[i], 4, 0};
    return reloq_target_builder_begin(&f->builder, buffers);
}

static int check(const struct fixture *f, int64_t origin, int32_t av,
                 int32_t ah_first, int32_t ah, int32_t af,
                 enum reloq_target_stream active, int32_t gain)
{
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        size_t count = s == RELOQ_TARGET_AH ? 4 :
            s == RELOQ_TARGET_AV || s == RELOQ_TARGET_AF ||
            (s >= RELOQ_TARGET_A1F && s <= RELOQ_TARGET_AB) ? 2 : 0;
        CHECK(f->builder.stream[s].count == count);
    }
    const struct reloq_acoustic_point *a = f->points[RELOQ_TARGET_AV];
    CHECK(a[0].tick == origin && a[0].value == av);
    CHECK(a[1].tick == origin + 60 && a[1].value == 0);
    a = f->points[RELOQ_TARGET_AH];
    CHECK(a[0].tick == origin && a[0].value == ah_first);
    CHECK(a[1].tick == origin && a[1].value == ah);
    CHECK(a[2].tick == origin + 60 && a[2].value == ah);
    CHECK(a[3].tick == origin + 60 && a[3].value == 0);
    for (size_t s = RELOQ_TARGET_AF; s <= RELOQ_TARGET_AB; ++s) {
        if (s > RELOQ_TARGET_AF && s < RELOQ_TARGET_A1F) continue;
        a = f->points[s];
        int32_t expected = s == RELOQ_TARGET_AF ? af :
            s == (size_t)active ? gain : 0;
        CHECK(a[0].tick == origin && a[0].value == expected);
        CHECK(a[1].tick == origin + 60 && a[1].value == 0);
    }
    return 0;
}

static int self_test(void)
{
    static const struct {
        const char *annotation;
        int64_t closure, release;
        int32_t av, ah_first, ah, af, gain;
        enum reloq_target_stream active;
    } cases[] = {
        {"`[.1A].", 97, 97, 49, 34, 35, 0, 0, RELOQ_TARGET_AB},
        {"`[.1hA].", 177, 177, 49, 34, 35, 0, 0, RELOQ_TARGET_AB},
        {"`[.1a].", 210, 210, 48, 34, 35, 0, 0, RELOQ_TARGET_AB},
        {"`[.1hA].", 177, 177, 49, 34, 35, 0, 0, RELOQ_TARGET_AB},
        {"`[.1Ap].", 97, 222, 0, 0, 35, 60, 64, RELOQ_TARGET_AB},
        {"`[.1hEp].", 185, 310, 0, 0, 35, 60, 60, RELOQ_TARGET_AB},
        {"`[.1pAp].", 152, 277, 0, 0, 35, 60, 64, RELOQ_TARGET_AB},
        {"`[.1It].", 98, 206, 0, 0, 48, 48, 60, RELOQ_TARGET_A5F},
        {"`[.1hAt].", 177, 285, 0, 0, 48, 48, 60, RELOQ_TARGET_A5F},
        {"`[.1tAt].", 176, 284, 0, 0, 48, 48, 60, RELOQ_TARGET_A5F},
        {"`[.1Ek].", 105, 227, 0, 0, 48, 62, 60, RELOQ_TARGET_A3F},
        {"`[.1hIk].", 178, 300, 0, 0, 53, 62, 60, RELOQ_TARGET_A3F},
        {"`[.1kIk].", 166, 288, 0, 0, 53, 62, 60, RELOQ_TARGET_A3F},
        {"`[.1ek].", 182, 304, 0, 0, 51, 62, 60, RELOQ_TARGET_A3F},
        {"`[.1Ok].", 98, 220, 0, 0, 48, 62, 60, RELOQ_TARGET_A2F}
    };
    struct fixture f;
    int64_t release = -19;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        CHECK(begin(&f, cases[i].annotation) == 0);
        CHECK(reloq_german_terminal_source_append(
            f.phones, f.count, f.vowel_index, cases[i].closure,
            &f.builder, &release) == 0);
        CHECK(release == cases[i].release);
        CHECK(check(&f, release, cases[i].av, cases[i].ah_first,
                    cases[i].ah, cases[i].af, cases[i].active,
                    cases[i].gain) == 0);
    }
    const char *uncovered[] = {
        "`[.1hAk.0A].", "`[.0hA].",
        "`[.1hA] `[.1k].", "`[.1hAl].", "`[.1Ah]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        CHECK(begin(&f, uncovered[i]) == 0);
        release = -19;
        CHECK(reloq_german_terminal_source_append(
            f.phones, f.count, f.vowel_index, 100,
            &f.builder, &release) == 1);
        CHECK(release == -19);
        for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            CHECK(f.builder.stream[s].count == 0);
    }
    CHECK(begin(&f, "`[.1hA].") == 0);
    CHECK(reloq_german_terminal_source_append(
        f.phones, f.count, f.vowel_index, -1,
        &f.builder, &release) == -1);
    CHECK(reloq_german_terminal_source_append(
        f.phones, f.count, f.vowel_index, INT64_MAX,
        &f.builder, &release) == -1);
    CHECK(release == -19);
    CHECK(f.builder.stream[RELOQ_TARGET_AV].count == 0);
    return 0;
}

static int query(const char *annotation, const char *closure_text)
{
    char *end = NULL;
    errno = 0;
    intmax_t parsed = strtoimax(closure_text, &end, 10);
    if (errno || end == closure_text || *end || parsed < 0 ||
        parsed > INT64_MAX) return -1;
    struct fixture f;
    int64_t release;
    if (begin(&f, annotation) || reloq_german_terminal_source_append(
            f.phones, f.count, f.vowel_index, (int64_t)parsed,
            &f.builder, &release)) return -1;
    printf("release=%" PRId64, release);
    for (size_t s = RELOQ_TARGET_AV; s <= RELOQ_TARGET_AB; ++s) {
        if (s > RELOQ_TARGET_AF && s < RELOQ_TARGET_A1F) continue;
        printf(" %s=", reloq_target_stream_name((enum reloq_target_stream)s));
        for (size_t i = 0; i < f.builder.stream[s].count; ++i)
            printf("%s%" PRId64 ":%" PRId32, i ? "," : "",
                   f.points[s][i].tick - release,
                   f.points[s][i].value);
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
