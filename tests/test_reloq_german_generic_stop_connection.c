#include "reloq_german_generic_stop_connection.h"
#include "reloq_german_generic_voice.h"
#include "reloq_german_syllable_plan.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static const enum reloq_german_phone stops[3] = {
    RELOQ_DE_PHONE_p, RELOQ_DE_PHONE_t, RELOQ_DE_PHONE_k
};

static void test_context_matrix(void)
{
    static const int coda[3][3] = {
        {40, 100, 100}, {58, 30, 58}, {52, 52, 65}
    };
    static const int onset[3][3] = {
        {91, 69, 56}, {91, 69, 105}, {91, 88, 56}
    };
    for (int a = 0; a < 3; ++a)
        for (int b = 0; b < 3; ++b) {
            struct reloq_german_phone_node first = {
                .phone = stops[a], .left_phone = RELOQ_DE_PHONE_A,
                .right_phone = stops[b], .same_syllable_left = 1,
                .stress = 1
            };
            struct reloq_german_generic_connected_coda fit;
            assert(reloq_german_generic_connected_coda(&first, &fit) == 1);
            assert(fit.span_ticks == coda[a][b] &&
                   fit.burst_at_ticks == coda[a][b] - (a ? 12 : 10) &&
                   fit.merge_with_next == (a == b));
            struct reloq_german_phone_node second = {
                .phone = stops[b], .left_phone = stops[a],
                .right_phone = RELOQ_DE_PHONE_A,
                .same_syllable_right = 1, .stress = 1
            };
            int32_t rise = 0;
            assert(reloq_german_generic_connected_onset(&second, &rise) == 1);
            assert(rise == onset[a][b]);
        }
    struct reloq_german_phone_node candidate = {
        .phone = RELOQ_DE_PHONE_p,
        .left_phone = RELOQ_DE_PHONE_A,
        .right_phone = RELOQ_DE_PHONE_t,
        .same_syllable_left = 1, .stress = 1
    };
    struct reloq_german_generic_connected_coda fit;
    candidate.same_word_right = 1;
    assert(reloq_german_generic_connected_coda(&candidate, &fit) == 0);
    candidate.same_word_right = 0;
    candidate.stress = 0;
    assert(reloq_german_generic_connected_coda(&candidate, &fit) == 0);
    candidate.stress = 1;
    candidate.phone = RELOQ_DE_PHONE_f;
    assert(reloq_german_generic_connected_coda(&candidate, &fit) == 0);
    assert(reloq_german_generic_connected_coda(NULL, &fit) == -1);
    assert(reloq_german_generic_connected_coda(&candidate, NULL) == -1);
}

struct emitted {
    struct reloq_acoustic_point point[RELOQ_TARGET_STREAM_COUNT][128];
    struct reloq_target_builder builder;
};

static void plan(const char *spr, struct emitted *out)
{
    struct reloq_german_phone_node phones[16];
    struct reloq_german_syllable syllables[16];
    struct reloq_german_phone_sequence_info info = {0};
    size_t syllable_count = 0;
    assert(reloq_german_phone_sequence_build(
               (const unsigned char *)spr, strlen(spr), 0,
               phones, 16, &info) == 0);
    assert(reloq_german_syllable_plan_build(
               phones, info.phone_count, syllables, 16,
               &syllable_count) == 0);
    struct reloq_target_buffer buffer[RELOQ_TARGET_STREAM_COUNT];
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        buffer[i] = (struct reloq_target_buffer){out->point[i], 128, 0};
    assert(reloq_target_builder_begin(&out->builder, buffer) == 0);
    int64_t end_tick = 0;
    assert(reloq_german_generic_plan(
               phones, info.phone_count, syllables, syllable_count,
               &out->builder, &end_tick, NULL, NULL) == 0);
    assert(end_tick > 100);
}

static int at(const struct emitted *out, enum reloq_target_stream stream,
              int64_t tick, int32_t value)
{
    const struct reloq_target_buffer *b = &out->builder.stream[stream];
    for (size_t i = 0; i < b->count; ++i)
        if (b->points[i].tick == tick && b->points[i].value == value)
            return 1;
    return 0;
}

static int64_t first_positive_after(
    const struct emitted *out, enum reloq_target_stream stream,
    int64_t after)
{
    const struct reloq_target_buffer *b = &out->builder.stream[stream];
    for (size_t i = 0; i < b->count; ++i)
        if (b->points[i].tick >= after && b->points[i].value > 0)
            return b->points[i].tick;
    return -1;
}

static void test_source_plateau(void)
{
    struct emitted out = {0};
    plan("`[.1pA].", &out);
    assert(at(&out, RELOQ_TARGET_AB, 5, 0) &&
           at(&out, RELOQ_TARGET_AB, 5, 64) &&
           at(&out, RELOQ_TARGET_AB, 15, 64) &&
           at(&out, RELOQ_TARGET_AB, 65, 0));
    /* One source dB for /p/; burst and aspiration retain their clocks. */
    assert(at(&out, RELOQ_TARGET_AF, 5, 61) &&
           at(&out, RELOQ_TARGET_AH, 15, 50));
    plan("`[.1tA].", &out);
    assert(at(&out, RELOQ_TARGET_A5F, 5, 0) &&
           at(&out, RELOQ_TARGET_A5F, 5, 56) &&
           at(&out, RELOQ_TARGET_A5F, 17, 56) &&
           at(&out, RELOQ_TARGET_A5F, 89, 0));
    plan("`[.1kA].", &out);
    assert(at(&out, RELOQ_TARGET_A2F, 5, 0) &&
           at(&out, RELOQ_TARGET_A2F, 5, 70) &&
           at(&out, RELOQ_TARGET_A2F, 17, 70) &&
           at(&out, RELOQ_TARGET_A2F, 78, 0));
    plan("`[.1fA].", &out);
    assert(!at(&out, RELOQ_TARGET_AF, 5, 60) &&
           !at(&out, RELOQ_TARGET_AB, 5, 64));
}

static void test_connected_chain(void)
{
    struct emitted out = {0};
    plan("`[.1pAk] `2 `[.1tIp].", &out);
    const int64_t k = first_positive_after(&out, RELOQ_TARGET_A2F, 100);
    const int64_t t = first_positive_after(&out, RELOQ_TARGET_A5F, 100);
    assert(k > 100 && t - k == 100);
    assert(at(&out, RELOQ_TARGET_A2F, k, 0) &&
           at(&out, RELOQ_TARGET_A2F, k, 60));
    assert(at(&out, RELOQ_TARGET_A5F, t, 0) &&
           at(&out, RELOQ_TARGET_A5F, t, 56));
    plan("`[.1pAp] `2 `[.1pA].", &out);
    const int64_t first = first_positive_after(&out, RELOQ_TARGET_AB, 0);
    const int64_t second = first_positive_after(&out, RELOQ_TARGET_AB, 30);
    assert(first == 5 && second > 150);
    /* The equal-pair coda has no middle release; the next onset owns one. */
    const struct reloq_target_buffer *b = &out.builder.stream[RELOQ_TARGET_AB];
    for (size_t i = 0; i < b->count; ++i)
        if (b->points[i].value > 0)
            assert(b->points[i].tick <= 15 || b->points[i].tick >= second);
}

static void test_unlisted_following_vowel(void)
{
    struct emitted out = {0};
    /* /'E:/ is a legal long front vowel outside the small calibrated
     * p/t/k timing table. Its connected burst must shift the whole stop
     * gesture; otherwise the aspiration target precedes the burst and the
     * complete sentence is rejected before the first audio frame. */
    plan("`[.0der] `2 `[.1k'E:'.0fR].", &out);
    plan("`[.0di] `2 `[.1k'E:'.0fR].", &out);
    plan("`[.1k'E:'.0fR].", &out);
}

static void test_within_word_stop_pair(void)
{
    struct reloq_german_generic_cluster_stop fit;
    static const int first_spans[3][3] = {
        {40, 100, 100}, {42, 30, 42}, {73, 73, 65}
    };
    static const int last_spans[3][3] = {
        {126, 61, 123}, {126, 66, 152}, {126, 100, 123}
    };
    for (int a = 0; a != 3; ++a)
        for (int b = 0; b != 3; ++b) {
            struct reloq_german_phone_node first = {
                .phone = stops[a], .left_phone = RELOQ_DE_PHONE_A,
                .right_phone = stops[b], .stress = 1,
                .same_word_left = 1, .same_word_right = 1,
                .same_syllable_left = 1, .same_syllable_right = 1
            };
            struct reloq_german_phone_node last = first;
            last.phone = stops[b];
            last.left_phone = stops[a];
            last.right_phone = RELOQ_DE_PHONE_NONE;
            last.same_word_right = 0;
            last.same_syllable_right = 0;
            assert(reloq_german_generic_cluster_stop(&first, 1, &fit) == 1);
            assert(fit.span_ticks == first_spans[a][b]);
            assert(fit.merge_with_next == (a == b));
            assert(reloq_german_generic_cluster_stop(&last, 0, &fit) == 1);
            assert(fit.span_ticks == last_spans[a][b]);
        }
    struct reloq_german_phone_node vowel = {
        .phone = RELOQ_DE_PHONE_A, .right_phone = RELOQ_DE_PHONE_k,
        .stress = 1, .same_word_right = 1, .same_syllable_right = 1
    };
    int32_t ticks = 0;
    assert(reloq_german_generic_cluster_vowel_ticks(&vowel, &ticks) == 1);
    assert(ticks == 87);
    vowel.phone = RELOQ_DE_PHONE_E;
    assert(reloq_german_generic_cluster_vowel_ticks(&vowel, &ticks) == 1);
    assert(ticks == 94);
    vowel.same_syllable_right = 0;
    assert(reloq_german_generic_cluster_vowel_ticks(&vowel, &ticks) == 0);
    assert(reloq_german_generic_cluster_stop(NULL, 1, &fit) == -1);
    assert(reloq_german_generic_cluster_vowel_ticks(NULL, &ticks) == -1);

    static const struct {
        enum reloq_german_phone following;
        int span, af, a2f, a5f, ah, merged;
    } linked[] = {
        {RELOQ_DE_PHONE_NONE, 100, 48, 0, 60, 48, 0},
        {RELOQ_DE_PHONE_p, 100, 70, 0, 0, 51, 0},
        {RELOQ_DE_PHONE_t, 92, 0, 0, 0, 0, 1},
        {RELOQ_DE_PHONE_k, 100, 70, 50, 40, 51, 0},
        {RELOQ_DE_PHONE_b, 100, 70, 0, 0, 51, 0},
        {RELOQ_DE_PHONE_d, 90, 0, 0, 0, 0, 1},
        {RELOQ_DE_PHONE_g, 100, 70, 50, 40, 0, 0},
        {RELOQ_DE_PHONE_m, 100, 54, 0, 51, 51, 0},
        {RELOQ_DE_PHONE_s, 73, 66, 0, 53, 0, 0},
        {RELOQ_DE_PHONE_A, 100, 56, 0, 56, 0, 0}
    };
    struct reloq_german_phone_node last = {
        .phone = RELOQ_DE_PHONE_t, .left_phone = RELOQ_DE_PHONE_k,
        .stress = 1, .same_word_left = 1, .same_syllable_left = 1
    };
    for (size_t j = 0; j < sizeof linked / sizeof linked[0]; ++j) {
        last.right_phone = linked[j].following;
        assert(reloq_german_generic_cluster_stop(&last, 0, &fit) == 1);
        assert(fit.span_ticks == linked[j].span);
        assert(fit.af_level == linked[j].af);
        assert(fit.a2f_level == linked[j].a2f);
        assert(fit.a5f_level == linked[j].a5f);
        assert(fit.following_aspiration_level == linked[j].ah);
        assert(fit.merge_with_next == linked[j].merged);
    }

    struct emitted out = {0};
    /* The modeled final VCC gesture uses the same target landmarks for
     * phoneme classes; these spellings are example SPR, never lookup keys. */
    plan("`[.1pAkt].", &out);
    assert(at(&out, RELOQ_TARGET_A2F, 217, 60));
    assert(at(&out, RELOQ_TARGET_AH, 225, 51));
    /* Reference k->t: hold carried aspiration until closure, while the
     * burst noise interpolates from its last endpoint to zero. */
    assert(at(&out, RELOQ_TARGET_AH, 245, 51));
    assert(at(&out, RELOQ_TARGET_AH, 245, 0));
    assert(at(&out, RELOQ_TARGET_AF, 225, 55));
    assert(!at(&out, RELOQ_TARGET_AF, 225, 0));
    assert(at(&out, RELOQ_TARGET_AF, 245, 0));
    assert(at(&out, RELOQ_TARGET_A5F, 317, 60));
    assert(at(&out, RELOQ_TARGET_AH, 325, 49));
    plan("`[.1pEpt].", &out);
    assert(at(&out, RELOQ_TARGET_AB, 249, 60));
    assert(at(&out, RELOQ_TARGET_A5F, 312, 60));
    plan("`[.1tIpt].", &out);
    plan("`[.1klApt] `2 `[.0di].", &out);
    /* An uncalibrated long-ä vowel must not activate only the *second*
     * half of a VCC gesture; that used to generate out-of-order targets. */
    plan("`[.1'E:'kt].", &out);
    plan("`[.1tr'E:'kt] `00 `[.0di].", &out);
    plan("`[.0Es] `2 `[.1tIpt].", &out);
    plan("`[.1pe.0tR] `[.1pAkt] `2 `[.0pA.1ke.0t@].", &out);
    const struct reloq_target_buffer *af =
        &out.builder.stream[RELOQ_TARGET_AF];
    int connected_t_burst = 0;
    for (size_t j = 0; j < af->count; ++j)
        connected_t_burst |= af->points[j].value == 71;
    assert(connected_t_burst);
    plan("`[.1pAkt] `2 `[.1tA].", &out);
    plan("`[.1pAkt] `2 `[.1kA].", &out);
    plan("`[.1pAkt] `2 `[.1bA].", &out);
    plan("`[.1pAkt] `2 `[.1gA].", &out);
    plan("`[.1pAkt] `2 `[.1mA].", &out);
    plan("`[.1pAkt] `2 `[.1sA].", &out);
    plan("`[.1pAkt] `2 `[.1A].", &out);
}

int main(void)
{
    test_context_matrix();
    test_source_plateau();
    test_connected_chain();
    test_unlisted_following_vowel();
    test_within_word_stop_pair();
    puts("generic stop burst and connected closure classes passed");
    return 0;
}
