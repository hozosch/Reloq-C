#include "reloq_german_generic_diphthong.h"
#include "reloq_german_generic_voice.h"
#include "reloq_german_syllable_plan.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_context_classes(void)
{
    struct reloq_german_phone_node node = {
        .phone = RELOQ_DE_PHONE_Q_aj, .stress = 1
    };
    struct reloq_german_generic_diphthong d;
    assert(reloq_german_generic_diphthong_plan(&node, 0, &d) == 1);
    assert(d.endpoint[0] == 420 && d.endpoint[1] == 1900 &&
           d.endpoint[2] == 2450 && d.glide_ticks == 75 &&
           d.duration_ticks == 232 && d.endpoint_hold_ticks == 20 &&
           d.final_av == 45);
    node.same_syllable_right = 1;
    node.right_phone = RELOQ_DE_PHONE_s;
    assert(reloq_german_generic_diphthong_plan(&node, 0, &d) == 1);
    assert(d.duration_ticks == 207 && d.before_coda_ticks == 15);
    assert(reloq_german_generic_diphthong_plan(&node, 1, &d) == 1);
    assert(d.duration_ticks == 159 && d.before_coda_ticks == 15);
    node.phone = RELOQ_DE_PHONE_Q_aw;
    assert(reloq_german_generic_diphthong_plan(&node, 0, &d) == 1);
    assert(d.duration_ticks == 188 && d.before_coda_ticks == 40 &&
           d.endpoint[0] == 400 && d.endpoint[1] == 940 &&
           d.endpoint[2] == 2260);
    assert(reloq_german_generic_diphthong_plan(&node, 1, &d) == 1);
    assert(d.duration_ticks == 145);
    node.phone = RELOQ_DE_PHONE_Q_oj;
    node.right_phone = RELOQ_DE_PHONE_m;
    assert(reloq_german_generic_diphthong_plan(&node, 0, &d) == 1);
    assert(d.duration_ticks == 203 && d.glide_ticks == 45 &&
           d.endpoint[1] == 1780 && d.endpoint[2] == 2400);
    node.same_syllable_right = 0;
    node.same_word_right = 1;
    node.right_phone = RELOQ_DE_PHONE_t;
    assert(reloq_german_generic_diphthong_plan(&node, 0, &d) == 1);
    assert(d.duration_ticks == 159 && d.before_coda_ticks == 0);
    node.same_word_right = 0;
    assert(reloq_german_generic_diphthong_plan(&node, 0, &d) == 1);
    assert(d.duration_ticks == 174);
    node.phone = RELOQ_DE_PHONE_E;
    assert(reloq_german_generic_diphthong_plan(&node, 0, &d) == 0);
    assert(reloq_german_generic_diphthong_plan(NULL, 0, &d) == -1);
    assert(reloq_german_generic_diphthong_plan(&node, 2, &d) == -1);
    assert(reloq_german_generic_diphthong_plan(&node, 0, NULL) == -1);
}

struct emitted {
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][100];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_target_builder builder;
};

static void plan(const char *spr, struct emitted *out)
{
    struct reloq_german_phone_node nodes[12];
    struct reloq_german_phone_sequence_info info = {0};
    assert(reloq_german_phone_sequence_build(
        (const unsigned char *)spr, strlen(spr), 0,
        nodes, 12, &info) == 0);
    struct reloq_german_syllable syllables[12];
    size_t syllable_count = 0;
    assert(reloq_german_syllable_plan_build(
        nodes, info.phone_count, syllables, 12, &syllable_count) == 0);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        out->buffers[i] = (struct reloq_target_buffer){out->points[i], 100, 0};
    assert(reloq_target_builder_begin(&out->builder, out->buffers) == 0);
    int64_t end = 0;
    assert(reloq_german_generic_plan(
        nodes, info.phone_count, syllables, syllable_count,
        &out->builder, &end, NULL, NULL) == 0);
    assert(end > 250);
}

static int has_target(const struct emitted *out,
                      enum reloq_target_stream track,
                      int64_t tick, int32_t value)
{
    for (size_t i = 0; i < out->builder.stream[track].count; ++i)
        if (out->points[track][i].tick == tick &&
            out->points[track][i].value == value) return 1;
    return 0;
}

static void test_planner(void)
{
    struct emitted out = {0};
    plan("`[.1'aj'].", &out);
    assert(has_target(&out, RELOQ_TARGET_F1, 137, 820));
    assert(has_target(&out, RELOQ_TARGET_F1, 212, 420));
    assert(has_target(&out, RELOQ_TARGET_F2, 212, 1900));
    plan("`[.1'aj's].", &out);
    assert(has_target(&out, RELOQ_TARGET_F1, 97, 820));
    assert(has_target(&out, RELOQ_TARGET_F1, 172, 420));
    assert(has_target(&out, RELOQ_TARGET_F1, 192, 420));
    plan("`[.1'aw's].", &out);
    assert(has_target(&out, RELOQ_TARGET_F1, 53, 780));
    assert(has_target(&out, RELOQ_TARGET_F1, 128, 400));
    assert(has_target(&out, RELOQ_TARGET_F1, 148, 400));
    plan("`[.1'oj'n].", &out);
    assert(has_target(&out, RELOQ_TARGET_F1, 123, 550));
    assert(has_target(&out, RELOQ_TARGET_F1, 168, 350));
    assert(has_target(&out, RELOQ_TARGET_F2, 168, 1900));
    plan("`[.1'aj's] `[.1pA].", &out);
    assert(has_target(&out, RELOQ_TARGET_F1, 49, 820));
    assert(has_target(&out, RELOQ_TARGET_F1, 124, 420));
    assert(has_target(&out, RELOQ_TARGET_F1, 144, 420));
    plan("`[.1'aw's] `[.1pA].", &out);
    assert(has_target(&out, RELOQ_TARGET_F1, 10, 780));
    assert(has_target(&out, RELOQ_TARGET_F1, 85, 400));
    assert(has_target(&out, RELOQ_TARGET_F1, 105, 400));
    plan("`[.1'E:'t].", &out);
    /* Scheduled vowels start immediately; the free interval and the
     * outgoing transition share the whole nucleus budget. */
    assert(has_target(&out, RELOQ_TARGET_F1, 0, 500));
    assert(has_target(&out, RELOQ_TARGET_F2, 0, 2005));
    assert(has_target(&out, RELOQ_TARGET_F3, 0, 2600));
    /* Diphthong + next-syllable onset and diphthong + coda both require
     * target order; these used to fail when glide and VC windows crossed. */
    plan("`[.1fr'aj'.0h'aj't].", &out);
    plan("`[.1m'aw's] `[.1n'oj'].", &out);
}

/* Onset gestures, the internal glide, and the anticipatory coda may all
 * compete for one short target window. Exercise combinations instead of
 * blessing isolated words or merely checking a pure timing function. */
static void test_overlap(void)
{
    const char *const ph[] = {"'aj'", "'aw'", "'oj'"};
    const char onsets[] = "ptkbfsmnlrh";
    const char codas[] = "ptksmnlrf";
    char spr[40];
    struct emitted out = {0};
    for (size_t d = 0; d < sizeof ph / sizeof ph[0]; ++d)
        for (size_t a = 0; a < sizeof onsets - 1; ++a)
            for (size_t c = 0; c < sizeof codas - 1; ++c)
                for (int stress = 0; stress < 3; ++stress) {
                    int n = snprintf(spr, sizeof spr, "`[.%d%c%s%c].",
                                     stress, onsets[a], ph[d], codas[c]);
                    assert(n > 0 && (size_t)n < sizeof spr);
                    plan(spr, &out);
                }
}

int main(void)
{
    test_context_classes();
    test_planner();
    test_overlap();
    puts("generic German diphthong gestures: OK");
    return 0;
}
