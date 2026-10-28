#include "reloq_german_within_word_f0.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void prepare(const char *annotation,
                    struct reloq_german_phone_node phones[8],
                    struct reloq_german_syllable syllables[2])
{
    struct reloq_german_phone_sequence_info info;
    assert(reloq_german_phone_sequence_build(
        (const unsigned char *)annotation, strlen(annotation), 0,
        phones, 8, &info) == 0 && info.phone_count == 6);
    size_t count = 0;
    assert(reloq_german_syllable_plan_build(
        phones, 6, syllables, 2, &count) == 0 && count == 2);
}

static void check(void)
{
    struct reloq_german_phone_node phones[8];
    struct reloq_german_syllable syllables[2];
    struct reloq_acoustic_point f0[4];
    struct reloq_target_buffer streams[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    streams[RELOQ_TARGET_F0] = (struct reloq_target_buffer){f0, 4, 0};
    struct reloq_target_builder builder;
    assert(reloq_target_builder_begin(&builder, streams) == 0);
    prepare(" `[.0pAk.1tIp].", phones, syllables);
    assert(reloq_german_within_word_f0_append(
        phones, 6, syllables, 2, 161, 314, 402, &builder) == 0);
    const int64_t ticks[] = {190, 344, 375, 401};
    const int32_t values[] = {1043, 1293, 793, 740};
    assert(builder.stream[RELOQ_TARGET_F0].count == 4);
    for (size_t i = 0; i < 4; ++i) {
        assert(f0[i].tick == ticks[i]);
        assert(f0[i].value == values[i]);
    }
    prepare(" `[.1pAk.0tIp].", phones, syllables);
    assert(reloq_german_within_word_f0_append(
        phones, 6, syllables, 2, 161, 314, 402, &builder) == 1);
    assert(builder.stream[RELOQ_TARGET_F0].count == 4);
}

static int query(int argc, char **argv)
{
    if (argc != 6) return 2;
    struct reloq_german_phone_node phones[8];
    struct reloq_german_syllable syllables[2];
    struct reloq_german_phone_sequence_info info;
    size_t length = strlen(argv[2]);
    if (reloq_german_phone_sequence_build(
            (const unsigned char *)argv[2], length, 0,
            phones, 8, &info) || info.phone_count != 6) return 1;
    size_t count = 0;
    if (reloq_german_syllable_plan_build(
            phones, 6, syllables, 2, &count) || count != 2) return 1;
    char *end;
    const int64_t first = strtoll(argv[3], &end, 10);
    if (*end) return 1;
    const int64_t onset = strtoll(argv[4], &end, 10);
    if (*end) return 1;
    const int64_t closure = strtoll(argv[5], &end, 10);
    if (*end) return 1;
    struct reloq_acoustic_point f0[4];
    struct reloq_target_buffer streams[RELOQ_TARGET_STREAM_COUNT] = {{0}};
    streams[RELOQ_TARGET_F0] = (struct reloq_target_buffer){f0, 4, 0};
    struct reloq_target_builder builder;
    if (reloq_target_builder_begin(&builder, streams) ||
        reloq_german_within_word_f0_append(
            phones, 6, syllables, 2, first, onset, closure, &builder))
        return 1;
    for (size_t i = 0; i < builder.stream[RELOQ_TARGET_F0].count; ++i)
        printf("%lld:%d\n", (long long)f0[i].tick, f0[i].value);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc > 1 && !strcmp(argv[1], "--query")) return query(argc, argv);
    if (argc != 1) return 2;
    check();
    puts("German within-word two-syllable F0: OK");
    return 0;
}
