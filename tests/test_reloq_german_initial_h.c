#include "reloq_german_initial_h.h"

#include "reloq_german_hv_timing.h"
#include "reloq_german_vowel_nucleus.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
    return -1; \
} } while (0)

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_german_phone_sequence_info info;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][8];
    struct reloq_target_builder builder;
};

static int begin(struct fixture *f, const char *annotation)
{
    memset(f, 0, sizeof *f);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            f->phones, 8, &f->info)) return -1;
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){f->points[i], 8, 0};
    return reloq_target_builder_begin(&f->builder, buffers);
}

/* Source the AV coefficient and window from authored C models rather than
 * supplying observed word-point sequences to the initial-h rule. */
static int build_native(const char *annotation, struct fixture *h,
                        int64_t *onset, int32_t *av, int64_t *window)
{
    struct fixture vowel;
    int64_t anchor, closure;
    if (begin(&vowel, annotation) ||
        reloq_german_hv_acoustic_anchor(
            vowel.phones, vowel.info.phone_count, &anchor) ||
        reloq_german_vowel_nucleus_append(
            vowel.phones, vowel.info.phone_count, 1, anchor, 1,
            &vowel.builder, &closure)) return -1;
    *av = vowel.points[RELOQ_TARGET_AV][0].value;
    if (vowel.info.phone_count == 3) {
        if (reloq_german_hvc_first_send_end(
                vowel.phones, vowel.info.phone_count, window)) return -1;
    } else {
        *window = anchor + closure;
    }
    if (begin(h, annotation) ||
        reloq_german_initial_h_append(h->phones, h->info.phone_count, 0,
                                      *av, *window, &h->builder, onset))
        return -1;
    return 0;
}

static int check_points(const struct fixture *f, int64_t origin,
                        int64_t onset, int32_t ah, int64_t f0)
{
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        size_t expected = s == RELOQ_TARGET_AV || s == RELOQ_TARGET_AH ||
            s == RELOQ_TARGET_AF ? 2 : s == RELOQ_TARGET_F0 ? 1 : 0;
        CHECK(f->builder.stream[s].count == expected);
    }
    CHECK(f->points[RELOQ_TARGET_F0][0].tick == origin + f0);
    CHECK(f->points[RELOQ_TARGET_F0][0].value == 1049);
    const enum reloq_target_stream streams[] = {
        RELOQ_TARGET_AV, RELOQ_TARGET_AH, RELOQ_TARGET_AF
    };
    const int32_t values[] = {0, ah, 0};
    for (size_t s = 0; s < 3; ++s) {
        CHECK(f->points[streams[s]][0].tick == origin);
        CHECK(f->points[streams[s]][1].tick == origin + onset);
        CHECK(f->points[streams[s]][0].value == values[s]);
        CHECK(f->points[streams[s]][1].value == values[s]);
    }
    return 0;
}

static int self_test(void)
{
    static const struct {
        const char *annotation;
        int32_t vowel_av;
        int64_t first_send, first_f0;
    } cases[] = {
        {"`[.1hA].", 51, 177, 17},
        {"`[.1hE].", 52, 185, 18},
        {"`[.1hU].", 54, 164, 16},
        {"`[.1ha].", 50, 290, 29},
        {"`[.1hi].", 53, 272, 27},
        {"`[.1hAp].", 51, 147, 14},
        {"`[.1hAt].", 51, 137, 13},
        {"`[.1hap].", 50, 218, 21},
        {"`[.1hak].", 50, 228, 22},
        {"`[.1huk].", 55, 208, 20}
    };
    struct fixture f;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        int64_t onset, window;
        int32_t av;
        CHECK(build_native(cases[i].annotation, &f, &onset, &av, &window) == 0);
        CHECK(onset == 80 && av == cases[i].vowel_av);
        CHECK(window == cases[i].first_send);
        CHECK(check_points(&f, 0, 80, av - 9, cases[i].first_f0) == 0);
    }

    CHECK(begin(&f, "`[.1hA].") == 0);
    int64_t onset = -7;
    CHECK(reloq_german_initial_h_append(
        f.phones, f.info.phone_count, 31, 51, -1,
        &f.builder, &onset) == 0);
    CHECK(onset == 80);
    CHECK(f.builder.stream[RELOQ_TARGET_F0].count == 0);
    CHECK(f.points[RELOQ_TARGET_AV][0].tick == 31);
    CHECK(f.points[RELOQ_TARGET_AH][1].tick == 111);

    /* /s/ shares the h-source rule; the measured U/s vowel AV is 58. */
    CHECK(begin(&f, "`[.1hUs].") == 0);
    CHECK(reloq_german_initial_h_append(
        f.phones, f.info.phone_count, 0, 58, 132,
        &f.builder, &onset) == 0);
    CHECK(onset == 80 && check_points(&f, 0, 80, 49, 13) == 0);

    const char *uncovered[] = {
        "`[.0hA].", "`[.2hA].", "`[.1hAl].", "`[.1pA].",
        "`[.1hA.0A].", "`[.1hA] `[.1k]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        CHECK(begin(&f, uncovered[i]) == 0);
        onset = -7;
        CHECK(reloq_german_initial_h_append(
            f.phones, f.info.phone_count, 0, 51, 177,
            &f.builder, &onset) == 1);
        CHECK(onset == -7 && f.builder.stream[RELOQ_TARGET_AH].count == 0);
    }
    CHECK(begin(&f, "`[.1hA].") == 0);
    const int64_t bad_origins[] = {-1, INT64_MAX};
    for (size_t i = 0; i < sizeof bad_origins / sizeof bad_origins[0]; ++i) {
        CHECK(reloq_german_initial_h_append(
            f.phones, f.info.phone_count, bad_origins[i], 51, 177,
            &f.builder, &onset) == -1);
        CHECK(onset == -7 && f.builder.stream[RELOQ_TARGET_AH].count == 0);
    }
    CHECK(reloq_german_initial_h_append(
        f.phones, f.info.phone_count, 0, 49, 177,
        &f.builder, &onset) == -1);
    CHECK(reloq_german_initial_h_append(
        f.phones, f.info.phone_count, 0, 51, 0,
        &f.builder, &onset) == -1);
    CHECK(onset == -7 && f.builder.stream[RELOQ_TARGET_AH].count == 0);
    return 0;
}

static int query(const char *annotation)
{
    struct fixture f;
    int64_t onset, window;
    int32_t av;
    if (build_native(annotation, &f, &onset, &av, &window)) return -1;
    printf("onset=%" PRId64 " window=%" PRId64 " vowel_av=%" PRId32
           " F0=%" PRId64 ":%" PRId32 " AH=%" PRId32 "\n",
           onset, window, av, f.points[RELOQ_TARGET_F0][0].tick,
           f.points[RELOQ_TARGET_F0][0].value,
           f.points[RELOQ_TARGET_AH][0].value);
    return 0;
}

static int frames_query(const char *annotation)
{
    struct fixture f;
    int64_t onset, window, closure;
    int32_t av;
    if (build_native(annotation, &f, &onset, &av, &window) ||
        reloq_german_vowel_nucleus_append(
            f.phones, f.info.phone_count, 1, onset, 1,
            &f.builder, &closure)) return -1;
    const enum reloq_target_stream streams[] = {
        RELOQ_TARGET_F0, RELOQ_TARGET_AV,
        RELOQ_TARGET_AH, RELOQ_TARGET_AF
    };
    struct reloq_acoustic_curve curves[4];
    struct reloq_acoustic_cursor cursors[4];
    for (size_t s = 0; s < 4; ++s) {
        struct reloq_target_buffer *buffer = &f.builder.stream[streams[s]];
        curves[s] = (struct reloq_acoustic_curve){
            0.0f, buffer->points, buffer->count};
        if (reloq_acoustic_begin(&curves[s], &cursors[s])) return -1;
    }
    for (int64_t tick = 0; tick <= onset; ++tick) {
        printf("frame %" PRId64, tick);
        for (size_t s = 0; s < 4; ++s) {
            float value;
            if (reloq_acoustic_value(&cursors[s], tick, &value)) return -1;
            printf(" %.0f", value);
        }
        putchar('\n');
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) return self_test() ? EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 3 && strcmp(argv[1], "--query") == 0)
        return query(argv[2]) ? EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 3 && strcmp(argv[1], "--frames") == 0)
        return frames_query(argv[2]) ? EXIT_FAILURE : EXIT_SUCCESS;
    fprintf(stderr, "usage: %s [--query|--frames ANNOTATION]\n", argv[0]);
    return EXIT_FAILURE;
}
