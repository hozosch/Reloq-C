#include "reloq_german_initial_nasal_envelope.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "nasal envelope test line %d: %s\n", __LINE__, #test); \
    return -1; \
} } while (0)

struct fixture {
    struct reloq_german_phone_node phones[8];
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][4];
    struct reloq_target_builder builder;
    size_t phone_count;
};

static int begin(struct fixture *f, const char *annotation, size_t f0_capacity)
{
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    memset(f, 0, sizeof *f);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            f->phones, 8, &info)) return -1;
    f->phone_count = info.phone_count;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffers[i] = (struct reloq_target_buffer){
            f->points[i], i == RELOQ_TARGET_F0 ? f0_capacity : 4, 0};
    return reloq_target_builder_begin(&f->builder, buffers);
}

static size_t total_points(const struct fixture *f)
{
    size_t total = 0;
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        total += f->builder.stream[i].count;
    return total;
}

static int check_example(const char *annotation, int64_t origin,
                         int64_t onset, int64_t closure, int64_t release,
                         const int64_t pitch[4])
{
    struct fixture f;
    CHECK(begin(&f, annotation, 4) == 0);
    CHECK(reloq_german_initial_nasal_envelope_append(
        f.phones, f.phone_count, RELOQ_DE_TERMINAL_PERIOD,
        origin, onset, closure, release, &f.builder) == 0);
    CHECK(total_points(&f) == 10);
    const int32_t values[] = {1046, 1296, 796, 743};
    CHECK(f.builder.stream[RELOQ_TARGET_F0].count == 4);
    for (size_t i = 0; i < 4; ++i) {
        CHECK(f.points[RELOQ_TARGET_F0][i].tick == pitch[i] + origin);
        CHECK(f.points[RELOQ_TARGET_F0][i].value == values[i]);
    }
    const enum reloq_target_stream streams[] = {
        RELOQ_TARGET_FL, RELOQ_TARGET_FTP, RELOQ_TARGET_FTZ};
    for (size_t i = 0; i < 3; ++i) {
        const enum reloq_target_stream s = streams[i];
        CHECK(f.builder.stream[s].count == 2);
        CHECK(f.points[s][0].tick == origin);
        CHECK(f.points[s][1].tick == release);
        CHECK(f.points[s][0].value == (i ? 250 : 0));
        CHECK(f.points[s][1].value == (i ? 250 : 0));
    }
    return 0;
}

static int self_test(void)
{
    const int64_t m_a[] = {8, 118, 152, 181};
    const int64_t n_o_t[] = {7, 104, 139, 168};
    const int64_t m_e[] = {8, 121, 158, 188};
    const int64_t n_i[] = {7, 128, 187, 236};
    CHECK(check_example("`[.1mAp].", 0, 85, 182, 307, m_a) == 0);
    CHECK(check_example("`[.1nOt].", 0, 70, 169, 277, n_o_t) == 0);
    CHECK(check_example("`[.1mEt].", 0, 85, 190, 298, m_e) == 0);
    CHECK(check_example("`[.1nip].", 31, 101, 269, 394, n_i) == 0);

    struct fixture f;
    const char *uncovered[] = {
        "`[.1lAt].", "`[.1mAs].", "`[.1nAf].", "`[.0mAt].",
        "`[.1mA].", "`[.1mA.0nA]."
    };
    for (size_t i = 0; i < sizeof uncovered / sizeof uncovered[0]; ++i) {
        CHECK(begin(&f, uncovered[i], 4) == 0);
        CHECK(reloq_german_initial_nasal_envelope_append(
            f.phones, f.phone_count, RELOQ_DE_TERMINAL_PERIOD,
            0, 85, 182, 307, &f.builder) == 1);
        CHECK(total_points(&f) == 0);
    }
    CHECK(begin(&f, "`[.1mAp].", 2) == 0);
    CHECK(reloq_german_initial_nasal_envelope_append(
        f.phones, f.phone_count, RELOQ_DE_TERMINAL_QUESTION,
        0, 85, 182, 307, &f.builder) == 1);
    CHECK(reloq_german_initial_nasal_envelope_append(
        f.phones, f.phone_count, RELOQ_DE_TERMINAL_PERIOD,
        0, 84, 182, 307, &f.builder) == -1);
    CHECK(reloq_german_initial_nasal_envelope_append(
        f.phones, f.phone_count, RELOQ_DE_TERMINAL_PERIOD,
        0, 85, 182, INT64_MAX - 1, &f.builder) == -1);
    CHECK(reloq_german_initial_nasal_envelope_append(
        f.phones, f.phone_count, RELOQ_DE_TERMINAL_PERIOD,
        0, 85, 182, 307, &f.builder) == -1);
    CHECK(total_points(&f) == 0);
    return 0;
}

static int query(const char *annotation, int64_t closure, int64_t release)
{
    struct fixture f;
    if (begin(&f, annotation, 4)) return -1;
    const int64_t onset = f.phones[0].phone == RELOQ_DE_PHONE_m ? 85 : 70;
    if (reloq_german_initial_nasal_envelope_append(
            f.phones, f.phone_count, RELOQ_DE_TERMINAL_PERIOD,
            0, onset, closure, release, &f.builder)) return -1;
    for (size_t s = 0; s < RELOQ_TARGET_STREAM_COUNT; ++s)
        for (size_t i = 0; i < f.builder.stream[s].count; ++i)
            printf("%s %" PRId64 " %" PRId32 "\n",
                   reloq_target_stream_name((enum reloq_target_stream)s),
                   f.points[s][i].tick, f.points[s][i].value);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) return self_test() ? EXIT_FAILURE : EXIT_SUCCESS;
    if (argc == 5 && strcmp(argv[1], "--points") == 0) {
        char *end = NULL;
        int64_t closure = strtoll(argv[3], &end, 10);
        if (!end || *end) return EXIT_FAILURE;
        int64_t release = strtoll(argv[4], &end, 10);
        if (!end || *end) return EXIT_FAILURE;
        return query(argv[2], closure, release) ? EXIT_FAILURE : EXIT_SUCCESS;
    }
    fprintf(stderr, "usage: %s [--points SPR closure release]\n", argv[0]);
    return EXIT_FAILURE;
}
