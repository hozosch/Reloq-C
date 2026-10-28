#include "reloq_german_reduced_open_source.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static int run(const char *annotation, int dump)
{
    struct reloq_german_phone_node phones[5];
    struct reloq_german_phone_sequence_info info = {0};
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            phones, 5, &info)) return -1;
    struct reloq_german_syllable syllables[2];
    size_t count = 0;
    if (reloq_german_syllable_plan_build(
            phones, info.phone_count, syllables, 2, &count)) return -1;
    struct reloq_german_reduced_landmarks landmarks;
    if (reloq_german_reduced_landmarks_plan(
            phones, info.phone_count, syllables, count, &landmarks)) return 1;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][32];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        buffers[i].points = points[i];
        buffers[i].capacity = 32;
    }
    struct reloq_target_builder builder;
    if (reloq_target_builder_begin(&builder, buffers)) return -1;
    if (reloq_german_reduced_open_source_append(
            phones, info.phone_count, syllables, count,
            &landmarks, &builder)) return -1;
    static const enum reloq_target_stream streams[] = {
        RELOQ_TARGET_F0, RELOQ_TARGET_AV, RELOQ_TARGET_AH,
        RELOQ_TARGET_AF, RELOQ_TARGET_A1F, RELOQ_TARGET_A2F,
        RELOQ_TARGET_A3F, RELOQ_TARGET_A4F, RELOQ_TARGET_A5F,
        RELOQ_TARGET_AB
    };
    if (dump) {
        for (size_t i = 0; i < sizeof streams / sizeof streams[0]; ++i) {
            enum reloq_target_stream s = streams[i];
            printf("%s", reloq_target_stream_name(s));
            for (size_t j = 0; j < builder.stream[s].count; ++j)
                printf(" %lld:%d",
                       (long long)builder.stream[s].points[j].tick,
                       builder.stream[s].points[j].value);
            putchar('\n');
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "--points"))
        return run(argv[2], 1) ? 1 : 0;
    if (argc != 1) return 2;
    assert(run("`[.1pA.0p@].", 0) == 0);
    assert(run("`[.1tI.0k@n].", 0) == 0);
    assert(run("`[.1ka.0t@].", 0) == 0);
    assert(run("`[.1pe.0p@n].", 0) == 0);

    /* An invalid stream append leaves the builder unchanged. */
    struct reloq_german_phone_node phones[5];
    struct reloq_german_phone_sequence_info info = {0};
    const char *annotation = "`[.1pA.0p@n].";
    assert(reloq_german_phone_sequence_build(
        (const unsigned char *)annotation, strlen(annotation), 0,
        phones, 5, &info) == 0);
    struct reloq_german_syllable syllables[2];
    size_t count = 0;
    assert(reloq_german_syllable_plan_build(
        phones, 5, syllables, 2, &count) == 0);
    struct reloq_german_reduced_landmarks landmarks;
    assert(reloq_german_reduced_landmarks_plan(
        phones, 5, syllables, count, &landmarks) == 0);
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][32];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        buffers[i].points = points[i];
        buffers[i].capacity = 32;
    }
    struct reloq_target_builder builder;
    assert(reloq_target_builder_begin(&builder, buffers) == 0);
    phones[2].phone = RELOQ_DE_PHONE_f;
    assert(reloq_german_reduced_open_source_append(
        phones, 5, syllables, 2, &landmarks, &builder) == 1);
    assert(builder.stream[RELOQ_TARGET_F0].count == 0);
    phones[2].phone = RELOQ_DE_PHONE_p;
    landmarks.second_nasal_onset = landmarks.second_vowel_onset;
    assert(reloq_german_reduced_open_source_append(
        phones, 5, syllables, 2, &landmarks, &builder) == -1);
    assert(builder.stream[RELOQ_TARGET_F0].count == 0);

    puts("German reduced open source/noise gestures: OK");
    return 0;
}
