#include "reloq_german_nucleus_acoustics.h"
#include "reloq_german_generic_voice.h"
#include "reloq_german_syllable_plan.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_input_contract(void)
{
    struct reloq_german_nucleus_acoustics result, before;
    memset(&result, 0x5a, sizeof result);
    before = result;
    assert(reloq_german_nucleus_acoustics_plan(RELOQ_DE_PHONE_p,
        RELOQ_DE_PHONE_NONE, RELOQ_DE_PHONE_NONE, 1, 0, &result) == 0);
    assert(!memcmp(&result, &before, sizeof result));
    assert(reloq_german_nucleus_acoustics_plan(RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_NONE, RELOQ_DE_PHONE_NONE, 3, 0, &result) == -1);
    assert(!memcmp(&result, &before, sizeof result));
    assert(reloq_german_nucleus_acoustics_plan(RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_NONE, RELOQ_DE_PHONE_NONE, 1, 0, NULL) == -1);
}

struct capture {
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][128];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_target_builder builder;
};

static void plan(const char *spr, struct capture *capture)
{
    struct reloq_german_phone_node nodes[16];
    struct reloq_german_phone_sequence_info info = {0};
    struct reloq_german_syllable syllables[16];
    size_t count = 0;
    int64_t end = 0;
    assert(!reloq_german_phone_sequence_build((const unsigned char *)spr,
        strlen(spr), 0, nodes, 16, &info));
    assert(!reloq_german_syllable_plan_build(nodes, info.phone_count,
        syllables, 16, &count));
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        capture->buffers[i] = (struct reloq_target_buffer){
            capture->points[i], 128, 0};
    assert(!reloq_target_builder_begin(&capture->builder, capture->buffers));
    assert(!reloq_german_generic_plan(nodes, info.phone_count, syllables,
        count, &capture->builder, &end, NULL, NULL));
    assert(end > 100);
}

static int has(const struct capture *c, enum reloq_target_stream stream,
                int32_t value)
{
    const struct reloq_target_buffer *track = &c->builder.stream[stream];
    for (size_t i = 0; i < track->count; ++i)
        if (track->points[i].value == value) return 1;
    return 0;
}

static void test_formant_ownership(void)
{
    static struct capture c;
    plan("`[.1pe].", &c);
    const struct reloq_target_buffer *f2 = &c.builder.stream[RELOQ_TARGET_F2];
    int gesture = 0;
    for (size_t i = 1; i < f2->count; ++i) {
        if (f2->points[i-1].value == 2000 && f2->points[i].value == 2150 &&
            f2->points[i].tick - f2->points[i-1].tick > 80) {
            /* A separate source point must not hold/overwrite this entire
             * observed entering-to-departing tongue gesture. */
            gesture = 1;
        }
        assert(f2->points[i].tick >= f2->points[i-1].tick);
    }
    assert(gesture);
    plan("`[.2I].", &c);
    assert(has(&c, RELOQ_TARGET_F3, 2520));
    plan("`[.1I].", &c);
    assert(has(&c, RELOQ_TARGET_F3, 2800));
    plan("`[.1Un].", &c);
    assert(has(&c, RELOQ_TARGET_B1, 320));
    assert(has(&c, RELOQ_TARGET_B2, 200));
    assert(has(&c, RELOQ_TARGET_B4, 280));
    assert(has(&c, RELOQ_TARGET_B5, 210));
    plan("`[.1tR].", &c);
    assert(has(&c, RELOQ_TARGET_F1, 600));
    assert(has(&c, RELOQ_TARGET_F2, 1500));
    assert(has(&c, RELOQ_TARGET_F3, 2280));
    plan("`[.1tR] `[.1nA].", &c);
    assert(!has(&c, RELOQ_TARGET_B1, 320));
}

static int has_at(const struct capture *c,enum reloq_target_stream stream,
                  int64_t tick,int32_t value)
{
    const struct reloq_target_buffer *track=&c->builder.stream[stream];
    for (size_t i=0;i<track->count;++i)
        if (track->points[i].tick==tick && track->points[i].value==value) return 1;
    return 0;
}

static void test_nasal_handoffs(void)
{
    static struct capture c;
    /* Original t landmarks: 5-ms closure, 12-ms burst, 72-ms aspiration.
     * The selected eloquence_64 a~ targets must already be present at the
     * following nucleus, rather than drifting in from a late fallback. */
    plan("`[.1t'a~'].",&c);
    assert(has_at(&c,RELOQ_TARGET_B1,89,180));
    assert(has_at(&c,RELOQ_TARGET_FNP,89,280));
    assert(has_at(&c,RELOQ_TARGET_FNZ,89,350));
    assert(has_at(&c,RELOQ_TARGET_B1,278,180));
    plan("`[.1m'a~'].",&c);
    assert(has_at(&c,RELOQ_TARGET_FNZ,100,350));
    assert(has_at(&c,RELOQ_TARGET_FNZ,295,350));
    plan("`[.1m'E~'].",&c);
    assert(has_at(&c,RELOQ_TARGET_FNP,100,250));
    assert(has_at(&c,RELOQ_TARGET_FNZ,100,350));
}

int main(void)
{
    test_input_contract();
    test_formant_ownership();
    test_nasal_handoffs();
    puts("nucleus integration: independent tract curves, stress, nasal bandwidths and vocalic R passed");
    return 0;
}
