#include "reloq_german_phrase_f0.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][8];
    struct reloq_target_builder builder;
    size_t count;
};

static void begin(struct fixture *f, const char *annotation)
{
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_german_phone_sequence_info info = {0};
    memset(f, 0, sizeof *f);
    assert(reloq_german_phone_sequence_build(
        (const unsigned char *)annotation, strlen(annotation), 0,
        f->phones, 8, &info) == 0);
    f->count = info.phone_count;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){f->points[i], 8, 0};
    assert(reloq_target_builder_begin(&f->builder, buffers) == 0);
}

/* Independently checked reference holdouts. These are isolated annotations,
 * not a captured PCM frame stream or a table loaded by the voice runtime. */
static void reference_shapes(void)
{
    static const struct {
        const char *annotation;
        enum reloq_german_terminal_mark mark;
        int64_t first_window, onset, closure, release, end;
        int64_t ticks[4];
        int32_t values[4];
    } cases[] = {
        {"`[.1A].", RELOQ_DE_TERMINAL_PERIOD,
         -1, 0, 97, 97, 557, {1,33,67,96}, {1217,1296,796,743}},
        {"`[.1A]?", RELOQ_DE_TERMINAL_QUESTION,
         -1, 0, 97, 97, 557, {1,58,87,96}, {1046,957,1224,1403}},
        {"`[.1hAk].", RELOQ_DE_TERMINAL_PERIOD,
         137, 80, 177, 299, 759, {13,113,147,176}, {1049,1296,796,743}},
        {"`[.1hAk]?", RELOQ_DE_TERMINAL_QUESTION,
         137, 80, 177, 299, 759, {13,138,167,176}, {1049,957,1224,1403}},
        {"`[.1hAk]!", RELOQ_DE_TERMINAL_EXCLAMATION,
         137, 80, 177, 299, 759, {13,94,147,176}, {1013,1331,832,743}},
        {"`[.1hAk]:", RELOQ_DE_TERMINAL_COLON,
         137, 80, 177, 299, 759, {13,113,147,176}, {1049,1296,796,743}},
        {"`[.1hAk];", RELOQ_DE_TERMINAL_SEMICOLON,
         137, 80, 177, 299, 609, {13,113,147,176}, {1049,1296,939,868}},
        {"`[.1hAk],", RELOQ_DE_TERMINAL_COMMA,
         137, 80, 177, 299, 509, {13,113,147,176}, {1049,1296,939,868}},
        {"`[.1hAk]...", RELOQ_DE_TERMINAL_ELLIPSIS,
         137, 80, 177, 299, 509, {13,89,147,176}, {1049,1210,957,992}},
        {"`[.1hEt]?", RELOQ_DE_TERMINAL_QUESTION,
         160, 80, 185, 293, 753, {16,143,174,183}, {1049,957,1224,1403}},
        {"`[.1hIp]!", RELOQ_DE_TERMINAL_EXCLAMATION,
         153, 80, 178, 303, 763, {15,94,148,177}, {1013,1331,832,743}},
        {"`[.1hat];", RELOQ_DE_TERMINAL_SEMICOLON,
         198, 80, 248, 356, 666, {19,138,197,246}, {1049,1296,939,868}},
        {"`[.1hU]...", RELOQ_DE_TERMINAL_ELLIPSIS,
         164, 80, 164, 164, 374, {16,88,138,163}, {1049,1210,957,992}},
        {"`[.1uk],", RELOQ_DE_TERMINAL_COMMA,
         -1, 0, 168, 290, 500, {3,58,117,166}, {1217,1296,939,868}},
        {"`[.1pAt].", RELOQ_DE_TERMINAL_PERIOD,
         15, 65, 152, 260, 720, {1,95,125,151}, {1046,1296,796,743}},
        {"`[.1pAt]?", RELOQ_DE_TERMINAL_QUESTION,
         15, 65, 152, 260, 720, {1,117,143,151}, {1046,957,1224,1403}},
        {"`[.1pAt]!", RELOQ_DE_TERMINAL_EXCLAMATION,
         15, 65, 152, 260, 720, {1,78,125,151}, {1010,1331,832,743}},
        {"`[.1pAt]:", RELOQ_DE_TERMINAL_COLON,
         15, 65, 152, 260, 720, {1,95,125,151}, {1046,1296,796,743}},
        {"`[.1pAt];", RELOQ_DE_TERMINAL_SEMICOLON,
         15, 65, 152, 260, 570, {1,95,125,151}, {1046,1296,939,868}},
        {"`[.1pAt],", RELOQ_DE_TERMINAL_COMMA,
         15, 65, 152, 260, 470, {1,95,125,151}, {1046,1296,939,868}},
        {"`[.1pAt]...", RELOQ_DE_TERMINAL_ELLIPSIS,
         15, 65, 152, 260, 470, {1,73,125,151}, {1046,1296,957,992}},
        {"`[.1tE].", RELOQ_DE_TERMINAL_PERIOD,
         23, 95, 189, 189, 649, {2,127,160,188}, {1046,1296,796,743}},
        {"`[.1kU]!", RELOQ_DE_TERMINAL_EXCLAMATION,
         17, 78, 153, 153, 613, {1,89,130,152}, {1010,1331,832,743}},
        {"`[.1As].", RELOQ_DE_TERMINAL_PERIOD,
         -1, 0, 107, 262, 692, {2,37,74,105}, {1217,1296,796,743}},
        {"`[.1Af].", RELOQ_DE_TERMINAL_PERIOD,
         -1, 0, 107, 257, 687, {2,37,74,105}, {1217,1296,796,743}},
        {"`[.1hAs].", RELOQ_DE_TERMINAL_PERIOD,
         147, 80, 187, 342, 772, {14,117,154,185}, {1049,1296,796,743}},
        {"`[.1hAf].", RELOQ_DE_TERMINAL_PERIOD,
         162, 80, 187, 337, 767, {16,117,154,185}, {1049,1296,796,743}},
        {"`[.1es].", RELOQ_DE_TERMINAL_PERIOD,
         -1, 0, 200, 355, 785, {4,70,140,198}, {1217,1296,796,743}},
        {"`[.1hef].", RELOQ_DE_TERMINAL_PERIOD,
         255, 80, 280, 430, 860, {25,150,220,278}, {1049,1296,796,743}},
        {"`[.1hUs].", RELOQ_DE_TERMINAL_PERIOD,
         132, 80, 172, 327, 757, {13,112,144,171}, {1049,1296,796,743}},
        {"`[.1As]?", RELOQ_DE_TERMINAL_QUESTION,
         -1, 0, 107, 262, 692, {2,64,96,105}, {1046,957,1224,1403}},
        {"`[.1As];", RELOQ_DE_TERMINAL_SEMICOLON,
         -1, 0, 107, 262, 542, {2,37,74,105}, {1217,1296,939,868}},
        {"`[.1hAs]!", RELOQ_DE_TERMINAL_EXCLAMATION,
         147, 80, 187, 342, 772, {14,96,154,185}, {1013,1331,832,743}},
        {"`[.1hAf],", RELOQ_DE_TERMINAL_COMMA,
         162, 80, 187, 337, 517, {16,117,154,185}, {1049,1296,939,868}},
        {"`[.1es]...", RELOQ_DE_TERMINAL_ELLIPSIS,
         -1, 0, 200, 355, 535, {4,20,140,198}, {1217,1210,957,992}},
        {"`[.1hAs]:", RELOQ_DE_TERMINAL_COLON,
         147, 80, 187, 342, 772, {14,117,154,185}, {1049,1296,796,743}},
        {"`[.1Af]", RELOQ_DE_TERMINAL_IMPLICIT,
         -1, 0, 107, 257, 687, {2,37,74,105}, {1217,1296,796,743}},
        {"`[.1pAs].", RELOQ_DE_TERMINAL_PERIOD,
         15, 65, 161, 316, 746, {1,98,132,160}, {1046,1296,796,743}},
        {"`[.1tis].", RELOQ_DE_TERMINAL_PERIOD,
         23, 113, 279, 434, 864, {2,171,229,277}, {1046,1296,796,743}},
        {"`[.1kus].", RELOQ_DE_TERMINAL_PERIOD,
         17, 93, 259, 414, 844, {1,151,209,257}, {1046,1296,796,743}}
    };
    struct fixture f;
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
        begin(&f, cases[i].annotation);
        int64_t end = -1;
        int r = reloq_german_phrase_f0_append(
            f.phones, f.count, cases[i].mark, cases[i].first_window,
            cases[i].onset, cases[i].closure, cases[i].release,
            &f.builder, &end);
        if (r || end != cases[i].end) {
            fprintf(stderr, "phrase case %zu failed: r=%d end=%lld\n",
                    i, r, (long long)end);
            assert(0);
        }
        assert(f.builder.stream[RELOQ_TARGET_F0].count == 4);
        for (size_t j = 0; j < 4; ++j) {
            assert(f.points[RELOQ_TARGET_F0][j].tick == cases[i].ticks[j]);
            assert(f.points[RELOQ_TARGET_F0][j].value == cases[i].values[j]);
        }
        for (size_t s = 1; s < RELOQ_TARGET_STREAM_COUNT; ++s)
            assert(f.builder.stream[s].count == 0);
    }
}

static void failures_are_atomic(void)
{
    struct fixture f;
    int64_t end = -17;
    begin(&f, "`[.1hAk].");
    assert(reloq_german_phrase_f0_append(
        f.phones, f.count, RELOQ_DE_TERMINAL_PERIOD, 137,
        80, 177, INT64_MAX, &f.builder, &end) == -1);
    assert(end == -17 && f.builder.stream[RELOQ_TARGET_F0].count == 0);
    assert(reloq_german_phrase_f0_append(
        f.phones, f.count, (enum reloq_german_terminal_mark)900,
        137, 80, 177, 299, &f.builder, &end) == -1);
    assert(end == -17 && f.builder.stream[RELOQ_TARGET_F0].count == 0);
    assert(reloq_german_phrase_f0_append(
        f.phones, f.count, RELOQ_DE_TERMINAL_PERIOD,
        -1, 80, 177, 299, &f.builder, &end) == -1);
    assert(end == -17 && f.builder.stream[RELOQ_TARGET_F0].count == 0);
    assert(reloq_german_phrase_f0_append(
        f.phones, f.count, RELOQ_DE_TERMINAL_PERIOD,
        137, 80, 177, 299, &f.builder, &end) == 0);
    assert(end == 759 && f.builder.stream[RELOQ_TARGET_F0].count == 4);
    assert(reloq_german_phrase_f0_append(
        f.phones, f.count, RELOQ_DE_TERMINAL_PERIOD,
        137, 80, 177, 299, &f.builder, &end) == -1);
    assert(end == 759 && f.builder.stream[RELOQ_TARGET_F0].count == 4);

    begin(&f, "`[.0hAk].");
    end = -17;
    assert(reloq_german_phrase_f0_append(
        f.phones, f.count, RELOQ_DE_TERMINAL_PERIOD,
        137, 80, 177, 299, &f.builder, &end) == 1);
    assert(end == -17 && f.builder.stream[RELOQ_TARGET_F0].count == 0);
    begin(&f, "`[.1hA] `[.1k].");
    assert(reloq_german_phrase_f0_append(
        f.phones, f.count, RELOQ_DE_TERMINAL_PERIOD,
        137, 80, 177, 299, &f.builder, &end) == 1);
    assert(end == -17 && f.builder.stream[RELOQ_TARGET_F0].count == 0);

    begin(&f, "`[.1pA.0s].");
    assert(reloq_german_phrase_f0_append(
        f.phones, f.count, RELOQ_DE_TERMINAL_PERIOD,
        15, 65, 161, 316, &f.builder, &end) == 1);
    assert(end == -17 && f.builder.stream[RELOQ_TARGET_F0].count == 0);
}

int main(void)
{
    reference_shapes();
    failures_are_atomic();
    return 0;
}
