#include "reloq_german_within_word_source.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static int prepare(const char *annotation,
                   struct reloq_german_phone_node phones[6],
                   struct reloq_german_syllable syllables[2],
                   struct reloq_german_within_word_landmarks *landmarks)
{
    struct reloq_german_phone_sequence_info info = {0};
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            phones, 6, &info) || info.phone_count != 6) return -1;
    size_t count = 0;
    if (reloq_german_syllable_plan_build(
            phones, 6, syllables, 2, &count) || count != 2) return -1;
    return reloq_german_within_word_landmarks_plan(
        phones, 6, syllables, 2, landmarks);
}

static int run(const char *annotation, int dump)
{
    struct reloq_german_phone_node phones[6];
    struct reloq_german_syllable syllables[2];
    struct reloq_german_within_word_landmarks landmarks;
    int status = prepare(annotation, phones, syllables, &landmarks);
    if (status) return status;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][64];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        buffers[i].points = points[i];
        buffers[i].capacity = 64;
    }
    struct reloq_target_builder builder;
    if (reloq_target_builder_begin(&builder, buffers)) return -1;
    status = reloq_german_within_word_source_append(
        phones, 6, syllables, 2, &landmarks, &builder);
    if (status) return status;
    static const enum reloq_target_stream streams[] = {
        RELOQ_TARGET_AV, RELOQ_TARGET_AH, RELOQ_TARGET_AF,
        RELOQ_TARGET_A1F, RELOQ_TARGET_A2F, RELOQ_TARGET_A3F,
        RELOQ_TARGET_A4F, RELOQ_TARGET_A5F, RELOQ_TARGET_AB
    };
    if (dump) {
        struct reloq_acoustic_cursor cursors[9];
        struct reloq_acoustic_curve curves[9];
        for (size_t i = 0; i < 9; ++i) {
            curves[i] = (struct reloq_acoustic_curve){
                0, builder.stream[streams[i]].points,
                builder.stream[streams[i]].count};
            if (reloq_acoustic_begin(&curves[i], &cursors[i])) return -1;
        }
        for (int64_t tick = 0; tick < landmarks.planned_ticks; ++tick) {
            printf("%lld", (long long)tick);
            for (size_t i = 0; i < 9; ++i) {
                float value;
                if (reloq_acoustic_value(&cursors[i], tick, &value))
                    return -1;
                printf(" %d", (int)value);
            }
            putchar('\n');
        }
    }
    return 0;
}

static void check_rejection(void)
{
    struct reloq_german_phone_node phones[6];
    struct reloq_german_syllable syllables[2];
    struct reloq_german_within_word_landmarks landmarks;
    assert(prepare("`[.0pAp.1tIp].", phones, syllables, &landmarks) == 0);
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][64];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        buffers[i].points = points[i];
        buffers[i].capacity = 64;
    }
    struct reloq_target_builder builder;
    assert(reloq_target_builder_begin(&builder, buffers) == 0);
    phones[3].phone = RELOQ_DE_PHONE_k;
    assert(reloq_german_within_word_source_append(
        phones, 6, syllables, 2, &landmarks, &builder) == 1);
    assert(builder.stream[RELOQ_TARGET_AV].count == 0);
    phones[3].phone = RELOQ_DE_PHONE_t;
    landmarks.second_vowel_onset = landmarks.second_burst_end;
    assert(reloq_german_within_word_source_append(
        phones, 6, syllables, 2, &landmarks, &builder) == -1);
    assert(builder.stream[RELOQ_TARGET_AV].count == 0);
    assert(prepare("`[.0pAp.1tIp].", phones, syllables, &landmarks) == 0);
    buffers[RELOQ_TARGET_AF].capacity = 1;
    assert(reloq_target_builder_begin(&builder, buffers) == 0);
    assert(reloq_german_within_word_source_append(
        phones, 6, syllables, 2, &landmarks, &builder) == -1);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        assert(builder.stream[i].count == 0);
}

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "--query"))
        return run(argv[2], 1) ? 1 : 0;
    if (argc != 1) return 2;
    assert(run("`[.0pAp.1tIp].", 0) == 0);
    assert(run("`[.0tEk.1tIp].", 0) == 0);
    assert(run("`[.0kUk.1tIp].", 0) == 0);
    check_rejection();
    puts("German within-word source/noise gestures: OK");
    return 0;
}
