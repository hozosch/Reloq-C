#include "reloq_german_generic_voice.h"
#include "reloq_german_syllable_plan.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

struct target_capture {
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][128];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_target_builder builder;
};

static void plan(const char *spr, struct target_capture *capture)
{
    struct reloq_german_phone_node nodes[16];
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_german_syllable syllables[16];
    size_t syllable_count = 0;
    assert(reloq_german_phone_sequence_build((const unsigned char *)spr,
               strlen(spr), 0, nodes, 16, &info) == 0);
    assert(reloq_german_syllable_plan_build(nodes, info.phone_count,
               syllables, 16, &syllable_count) == 0);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        capture->buffers[i] = (struct reloq_target_buffer){
            capture->points[i], 128, 0};
    assert(reloq_target_builder_begin(&capture->builder,
               capture->buffers) == 0);
    int64_t end_tick = 0;
    assert(reloq_german_generic_plan(nodes, info.phone_count, syllables,
               syllable_count, &capture->builder, &end_tick, NULL, NULL) == 0);
    assert(end_tick > 100);
}

static int has_oq(const struct target_capture *capture, int64_t tick,
                  int32_t value)
{
    const struct reloq_target_buffer *track =
        &capture->builder.stream[RELOQ_TARGET_OQ];
    for (size_t i = 0; i < track->count; ++i)
        if (track->points[i].tick == tick &&
            track->points[i].value == value) return 1;
    return 0;
}

int main(void)
{
    static const char *const vowel_first[] = {
        "`[.1A].", "`[.1E].", "`[.1I].", "`[.1O].",
        "`[.1U].", "`[.1a].", "`[.1e]."
    };
    static struct target_capture capture;
    for (size_t i = 0;
         i < sizeof vowel_first / sizeof vowel_first[0]; ++i) {
        plan(vowel_first[i], &capture);
        assert(has_oq(&capture, 0, 18));
        assert(has_oq(&capture, 21, 56));
    }
    static const char *const consonant_first[] = {
        "`[.1hA].", "`[.1mA].", "`[.1sA].", "`[.1pA]."
    };
    for (size_t i = 0;
         i < sizeof consonant_first / sizeof consonant_first[0]; ++i) {
        plan(consonant_first[i], &capture);
        assert(!has_oq(&capture, 0, 18));
        const struct reloq_target_buffer *track =
            &capture.builder.stream[RELOQ_TARGET_OQ];
        for (size_t j = 0; j < track->count; ++j)
            assert(track->points[j].value == 56);
    }
    puts("generic OQ: utterance-initial vowel transient, voiced source 56");
    return 0;
}
