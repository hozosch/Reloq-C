#include "reloq_german_cross_word_contour.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static enum reloq_german_terminal_mark mark_for(const char *annotation)
{
    size_t length = strlen(annotation);
    if (length >= 3 && !strcmp(annotation + length - 3, "..."))
        return RELOQ_DE_TERMINAL_ELLIPSIS;
    switch (annotation[length - 1]) {
    case '.': return RELOQ_DE_TERMINAL_PERIOD;
    case '?': return RELOQ_DE_TERMINAL_QUESTION;
    case '!': return RELOQ_DE_TERMINAL_EXCLAMATION;
    case ';': return RELOQ_DE_TERMINAL_SEMICOLON;
    case ',': return RELOQ_DE_TERMINAL_COMMA;
    case ':': return RELOQ_DE_TERMINAL_COLON;
    default: return (enum reloq_german_terminal_mark)99;
    }
}

static int prepare(const char *annotation,
                   struct reloq_german_cross_word_word words[20],
                   size_t *word_count, int64_t *planned_ticks,
                   struct reloq_german_phone_node phones[64],
                   struct reloq_german_syllable syllables[20])
{
    struct reloq_german_phone_sequence_info info = {0};
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)annotation, strlen(annotation), 0,
            phones, 64, &info)) return -1;
    if (reloq_german_syllable_plan_build(
            phones, info.phone_count, syllables, 20, word_count)) return -1;
    return reloq_german_cross_word_landmarks_plan(
        phones, info.phone_count, syllables, *word_count,
        mark_for(annotation), words, 20, planned_ticks);
}

static int run(const char *annotation, int dump)
{
    struct reloq_german_cross_word_word words[20];
    struct reloq_german_phone_node phones[64];
    struct reloq_german_syllable syllables[20];
    size_t word_count;
    int64_t planned_ticks;
    int status = prepare(annotation, words, &word_count, &planned_ticks,
                         phones, syllables);
    if (status) return status;
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][256];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        buffers[i].points = points[i];
        buffers[i].capacity = 256;
    }
    struct reloq_target_builder builder;
    if (reloq_target_builder_begin(&builder, buffers)) return -1;
    status = reloq_german_cross_word_contour_append(
        words, word_count, mark_for(annotation), planned_ticks, &builder);
    if (status) return status;
    if (dump) {
        struct reloq_acoustic_curve f0_curve = {
            0, builder.stream[RELOQ_TARGET_F0].points,
            builder.stream[RELOQ_TARGET_F0].count};
        struct reloq_acoustic_curve av_curve = {
            0, builder.stream[RELOQ_TARGET_AV].points,
            builder.stream[RELOQ_TARGET_AV].count};
        struct reloq_acoustic_curve ah_curve = {
            0, builder.stream[RELOQ_TARGET_AH].points,
            builder.stream[RELOQ_TARGET_AH].count};
        struct reloq_acoustic_cursor f0_cursor, av_cursor, ah_cursor;
        if (reloq_acoustic_begin(&f0_curve, &f0_cursor) ||
            reloq_acoustic_begin(&av_curve, &av_cursor) ||
            reloq_acoustic_begin(&ah_curve, &ah_cursor)) return -1;
        for (int64_t tick = 0; tick < planned_ticks; ++tick) {
            float f0, av, ah;
            if (reloq_acoustic_value(&f0_cursor, tick, &f0) ||
                reloq_acoustic_value(&av_cursor, tick, &av) ||
                reloq_acoustic_value(&ah_cursor, tick, &ah)) return -1;
            printf("%lld %d %d %d\n", (long long)tick,
                   (int)f0, (int)av, (int)ah);
        }
    }
    return 0;
}

static void check_transaction(void)
{
    struct reloq_german_cross_word_word words[20];
    struct reloq_german_phone_node phones[64];
    struct reloq_german_syllable syllables[20];
    size_t word_count;
    int64_t planned_ticks;
    assert(prepare(" `[.1pAk] `2 `[.1tIp] `2 `[.1kAk].",
                   words, &word_count, &planned_ticks, phones,
                   syllables) == 0);
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][256];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i) {
        buffers[i].points = points[i];
        buffers[i].capacity = 256;
    }
    struct reloq_target_builder builder;
    assert(reloq_target_builder_begin(&builder, buffers) == 0);
    words[1].vowel_onset = words[0].vowel_closure;
    assert(reloq_german_cross_word_contour_append(
        words, word_count, RELOQ_DE_TERMINAL_PERIOD,
        planned_ticks, &builder) == -1);
    assert(builder.stream[RELOQ_TARGET_F0].count == 0);
    assert(prepare(" `[.1pAk] `2 `[.1tIp] `2 `[.1kAk].",
                   words, &word_count, &planned_ticks, phones,
                   syllables) == 0);
    buffers[RELOQ_TARGET_AV].capacity = 1;
    assert(reloq_target_builder_begin(&builder, buffers) == 0);
    assert(reloq_german_cross_word_contour_append(
        words, word_count, RELOQ_DE_TERMINAL_PERIOD,
        planned_ticks, &builder) == -1);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        assert(builder.stream[i].count == 0);
    assert(reloq_german_cross_word_contour_append(
        words, 1, RELOQ_DE_TERMINAL_PERIOD, planned_ticks,
        &builder) == 1);
}

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "--query"))
        return run(argv[2], 1) ? 1 : 0;
    if (argc != 1) return 2;
    assert(run(" `[.1pAk] `2 `[.1tIp].", 0) == 0);
    assert(run(" `[.1pA] `2 `[.1tI] `2 `[.1kA]?", 0) == 0);
    assert(run(" `[.1kIp] `2 `[.1pA] `2 `[.1tAk];", 0) == 0);
    check_transaction();
    puts("German cross-word F0/AV/AH contour: OK");
    return 0;
}
