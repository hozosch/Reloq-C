#include "reloq_german_initial_voiced_stop.h"
#include "reloq_german_initial_voiced_stop_envelope.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "voiced envelope test line %d: %s\n", __LINE__, #x); \
    return -1; \
} } while (0)

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][8];
    struct reloq_target_builder builder;
    size_t count;
};

static int begin(struct fixture *f, const char *annotation,
                 size_t f0_capacity)
{
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_german_phone_sequence_info info = {0};
    memset(f, 0, sizeof *f);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            f->phones, 8, &info)) return -1;
    f->count = info.phone_count;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){
            f->points[i], i == RELOQ_TARGET_F0 ? f0_capacity : 8, 0};
    return reloq_target_builder_begin(&f->builder, buffers);
}

static int build(struct fixture *f, const char *shape, int64_t closure,
                 int64_t release)
{
    int64_t onset, burst;
    CHECK(begin(f, shape, 8) == 0);
    CHECK(reloq_german_initial_voiced_stop_append(
        f->phones, f->count, 0, &f->builder, &onset, &burst) == 0);
    CHECK(reloq_german_initial_voiced_stop_envelope_append(
        f->phones, f->count, RELOQ_DE_TERMINAL_PERIOD,
        0, burst, onset, closure, release, &f->builder) == 0);
    size_t total = 0;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        total += f->builder.stream[s].count;
    CHECK(total == 78);
    const struct reloq_acoustic_point *f0 = f->points[RELOQ_TARGET_F0];
    const int64_t duration = closure - onset;
    CHECK(f0[0].tick == onset / 10 && f0[0].value == 1046);
    CHECK(f0[1].tick == onset + duration * 35 / 100 &&
          f0[1].value == 1296);
    CHECK(f0[2].tick == onset + duration * 70 / 100 &&
          f0[2].value == 796);
    CHECK(f0[3].tick == onset + duration * 99 / 100 &&
          f0[3].value == 743);
    for (enum reloq_target_stream s = RELOQ_TARGET_FTP;
         s <= RELOQ_TARGET_FTZ; ++s) {
        CHECK(f->builder.stream[s].count == 2);
        CHECK(f->points[s][0].tick == 0 && f->points[s][0].value == 250);
        CHECK(f->points[s][1].tick == release &&
              f->points[s][1].value == 250);
    }
    CHECK(f->builder.stream[RELOQ_TARGET_FL].count == 2);
    CHECK(f->points[RELOQ_TARGET_FL][1].tick == release);
    return 0;
}

static int self_test(void)
{
    struct fixture f;
    CHECK(build(&f, "`[.1bAp].", 178, 303) == 0);
    CHECK(f.points[RELOQ_TARGET_F0][1].tick == 114);
    CHECK(f.builder.stream[RELOQ_TARGET_AB].count == 4);
    CHECK(f.builder.stream[RELOQ_TARGET_A5F].count == 4);
    CHECK(f.points[RELOQ_TARGET_A5F][1].tick == 70 &&
          f.points[RELOQ_TARGET_A5F][2].tick == 70);
    CHECK(build(&f, "`[.1gUk].", 174, 296) == 0);
    CHECK(f.builder.stream[RELOQ_TARGET_A2F].count == 4);
    CHECK(f.builder.stream[RELOQ_TARGET_AB].count == 4);
    CHECK(f.points[RELOQ_TARGET_AB][1].tick == 70);
    CHECK(build(&f, "`[.1gIp].", 188, 313) == 0);
    CHECK(f.builder.stream[RELOQ_TARGET_A3F].count == 4);
    CHECK(f.builder.stream[RELOQ_TARGET_A2F].count == 4);
    CHECK(build(&f, "`[.1dEk].", 205, 327) == 0);
    CHECK(f.builder.stream[RELOQ_TARGET_A5F].count == 4);
    CHECK(f.points[RELOQ_TARGET_F0][1].tick == 136);

    int64_t onset, burst;
    CHECK(begin(&f, "`[.1bAp].", 2) == 0);
    CHECK(reloq_german_initial_voiced_stop_append(
        f.phones, f.count, 0, &f.builder, &onset, &burst) == 0);
    CHECK(reloq_german_initial_voiced_stop_envelope_append(
        f.phones, f.count, RELOQ_DE_TERMINAL_PERIOD,
        0, burst, onset, 178, 303, &f.builder) == -1);
    CHECK(f.builder.stream[RELOQ_TARGET_F0].count == 0);
    CHECK(f.builder.stream[RELOQ_TARGET_AV].count == 4);
    CHECK(f.builder.stream[RELOQ_TARGET_FL].count == 0);
    CHECK(reloq_german_initial_voiced_stop_envelope_append(
        f.phones, f.count, RELOQ_DE_TERMINAL_QUESTION,
        0, burst, onset, 178, 303, &f.builder) == 1);
    CHECK(f.builder.stream[RELOQ_TARGET_F0].count == 0);
    CHECK(reloq_german_initial_voiced_stop_envelope_append(
        f.phones, f.count, RELOQ_DE_TERMINAL_PERIOD,
        0, burst, onset, INT64_MAX - 3, INT64_MAX - 2,
        &f.builder) == -1);
    CHECK(f.builder.stream[RELOQ_TARGET_F0].count == 0);
    return 0;
}

static int query(const char *annotation, int64_t closure, int64_t release)
{
    struct fixture f;
    if (build(&f, annotation, closure, release)) return -1;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s) {
        const struct reloq_target_buffer *stream = &f.builder.stream[s];
        if (!stream->count) continue;
        printf("%s", reloq_target_stream_name((enum reloq_target_stream)s));
        for (size_t i = 0; i < stream->count; ++i)
            printf(" %" PRId64 ":%" PRId32,
                   stream->points[i].tick, stream->points[i].value);
        putchar('\n');
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) return self_test() ? EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 3 && strcmp(argv[1], "--onset") == 0) {
        struct fixture f;
        int64_t onset, burst;
        if (begin(&f, argv[2], 8) ||
            reloq_german_initial_voiced_stop_append(
                f.phones, f.count, 0, &f.builder, &onset, &burst))
            return EXIT_FAILURE;
        printf("%" PRId64 "\n", onset);
        return EXIT_SUCCESS;
    }
    if (argc == 5 && strcmp(argv[1], "--query") == 0) {
        char *end = NULL;
        int64_t closure = strtoll(argv[3], &end, 10);
        if (!end || *end) return EXIT_FAILURE;
        int64_t release = strtoll(argv[4], &end, 10);
        if (!end || *end) return EXIT_FAILURE;
        return query(argv[2], closure, release) ? EXIT_FAILURE : EXIT_SUCCESS;
    }
    fprintf(stderr, "usage: %s [--query SPR closure release]\n", argv[0]);
    return EXIT_FAILURE;
}
