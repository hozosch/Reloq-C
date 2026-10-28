#include "reloq_german_generic_transition.h"
#include "reloq_german_generic_voice.h"
#include "reloq_german_syllable_plan.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_prenasal_bandwidth(void)
{
    const enum reloq_german_phone vowels[] = {
        RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_E, RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_O, RELOQ_DE_PHONE_U, RELOQ_DE_PHONE_a,
        RELOQ_DE_PHONE_e, RELOQ_DE_PHONE_i, RELOQ_DE_PHONE_o,
        RELOQ_DE_PHONE_u, RELOQ_DE_PHONE_AT
    };
    const int b1[] = {270, 220, 220, 320, 320, 270, 220, 270, 320,
                      320, 270};
    const int b2[] = {100, 100, 100, 200, 200, 100, 100, 100, 200,
                      200, 100};
    const enum reloq_german_phone nasals[] = {
        RELOQ_DE_PHONE_m, RELOQ_DE_PHONE_n, RELOQ_DE_PHONE_G
    };
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i) {
        for (size_t j = 0; j < sizeof nasals / sizeof nasals[0]; ++j) {
            struct reloq_german_phone_node node = {
                .phone = vowels[i], .right_phone = nasals[j],
                .same_syllable_right = 1
            };
            int32_t targets[RELOQ_TARGET_STREAM_COUNT] = {0};
            targets[RELOQ_TARGET_B1] = 120;
            targets[RELOQ_TARGET_B2] = 100;
            targets[RELOQ_TARGET_F1] = 777;
            assert(reloq_german_generic_prenasal_bandwidth(
                       &node, targets) == 1);
            assert(targets[RELOQ_TARGET_B1] == b1[i] &&
                   targets[RELOQ_TARGET_B2] == b2[i] &&
                   targets[RELOQ_TARGET_F1] == 777);
            node.same_syllable_right = 0;
            node.same_word_right = 1;
            targets[RELOQ_TARGET_B1] = 120;
            targets[RELOQ_TARGET_B2] = 100;
            assert(reloq_german_generic_prenasal_bandwidth(
                       &node, targets) == 1);
            assert(targets[RELOQ_TARGET_B1] == b1[i] &&
                   targets[RELOQ_TARGET_B2] == b2[i]);
            node.same_word_right = 0;
            assert(reloq_german_generic_prenasal_bandwidth(
                       &node, targets) == 0);
            assert(targets[RELOQ_TARGET_B1] == b1[i]);
            node.same_word_right = 1;
            node.right_phone = RELOQ_DE_PHONE_t;
            assert(reloq_german_generic_prenasal_bandwidth(
                       &node, targets) == 0);
            assert(targets[RELOQ_TARGET_B1] == b1[i]);
        }
    }
    struct reloq_german_phone_node unknown = {
        .phone = RELOQ_DE_PHONE_y,
        .right_phone = RELOQ_DE_PHONE_n, .same_syllable_right = 1
    };
    int32_t targets[RELOQ_TARGET_STREAM_COUNT] = {0};
    assert(reloq_german_generic_prenasal_bandwidth(&unknown, targets) == 0);
    assert(reloq_german_generic_prenasal_bandwidth(NULL, targets) == -1);
    assert(reloq_german_generic_prenasal_bandwidth(&unknown, NULL) == -1);
}

static void test_nasal_timing(void)
{
    const enum reloq_german_phone nasals[] = {
        RELOQ_DE_PHONE_m, RELOQ_DE_PHONE_n, RELOQ_DE_PHONE_G
    };
    const int spans[] = {30, 42, 45, 52, 60, 72, 85, 120, 140};
    for (size_t i = 0; i < sizeof nasals / sizeof nasals[0]; ++i)
        for (size_t j = 0; j < sizeof spans / sizeof spans[0]; ++j) {
            struct reloq_german_phone_node node = {.phone = nasals[i]};
            struct reloq_german_generic_nasal_transition transition;
            assert(reloq_german_generic_nasal_transition(
                       &node, spans[j], &transition) == 1);
            assert(transition.initial_fnz == 200 &&
                   transition.final_fnz == (i == 2 ? 400 : 350) &&
                   transition.rise_after_ticks == spans[j] / 2);
        }
    const enum reloq_german_phone vowels[] = {
        RELOQ_DE_PHONE_A, RELOQ_DE_PHONE_E, RELOQ_DE_PHONE_I,
        RELOQ_DE_PHONE_O, RELOQ_DE_PHONE_U, RELOQ_DE_PHONE_a,
        RELOQ_DE_PHONE_e, RELOQ_DE_PHONE_i, RELOQ_DE_PHONE_o,
        RELOQ_DE_PHONE_u
    };
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i) {
        for (size_t j = 0; j < sizeof nasals / sizeof nasals[0]; ++j) {
            struct reloq_german_phone_node node = {
                .phone = nasals[j], .left_phone = vowels[i],
                .same_word_left = 1
            };
            struct reloq_german_generic_nasal_transition transition;
            assert(reloq_german_generic_nasal_transition(
                       &node, 68, &transition) == 1);
            assert(transition.rise_after_ticks == 34);
            node.same_syllable_left = 1;
            assert(reloq_german_generic_nasal_transition(
                       &node, 68, &transition) == 1);
            assert(transition.rise_after_ticks == 34);
            node.same_syllable_left = 0;
            node.same_word_left = 0;
            assert(reloq_german_generic_nasal_transition(
                       &node, 68, &transition) == 1);
            assert(transition.rise_after_ticks == 34);
        }
    }
    struct reloq_german_phone_node cross_consonant = {
        .phone = RELOQ_DE_PHONE_n, .left_phone = RELOQ_DE_PHONE_t,
        .same_word_left = 1
    };
    struct reloq_german_generic_nasal_transition cross_transition;
    assert(reloq_german_generic_nasal_transition(
               &cross_consonant, 68, &cross_transition) == 1);
    assert(cross_transition.rise_after_ticks == 34);
    cross_consonant.left_phone = RELOQ_DE_PHONE_A;
    assert(reloq_german_generic_nasal_transition(
               &cross_consonant, 18, &cross_transition) == 1);
    assert(cross_transition.rise_after_ticks == 9);
    struct reloq_german_phone_node other = {.phone = RELOQ_DE_PHONE_l};
    struct reloq_german_generic_nasal_transition transition = {
        999, 999, 999};
    assert(reloq_german_generic_nasal_transition(
               &other, 60, &transition) == 0);
    assert(transition.initial_fnz == 999);
    struct reloq_german_phone_node nasal = {
        .phone = RELOQ_DE_PHONE_m,
        .left_phone = RELOQ_DE_PHONE_Q_a_TILDE,
        .same_syllable_left = 1
    };
    assert(reloq_german_generic_nasal_transition(
               &nasal, 60, &transition) == 0);
    assert(reloq_german_generic_nasal_transition(
               &nasal, 0, &transition) == -1);
    assert(reloq_german_generic_nasal_transition(
               NULL, 60, &transition) == -1);
}

static void test_fricative_context_timing(void)
{
    const enum reloq_german_phone phones[] = {
        RELOQ_DE_PHONE_s, RELOQ_DE_PHONE_f, RELOQ_DE_PHONE_x,
        RELOQ_DE_PHONE_S, RELOQ_DE_PHONE_X
    };
    const int onset[] = {85, 95, 95, 110, 90};
    const int intra[] = {64, 95, 60, 80, 90};
    const int connected[] = {96, 118, 60, 110, 90};
    const int final[] = {155, 150, 155, 160, 160};
    for (size_t i = 0; i < sizeof phones / sizeof phones[0]; ++i) {
        struct reloq_german_phone_node node = {
            .phone = phones[i], .right_phone = RELOQ_DE_PHONE_A,
            .same_syllable_right = 1
        };
        int32_t got = -1;
        assert(reloq_german_generic_fricative_ticks(&node, 0, 0, &got) == 1 &&
               got == onset[i]);
        if (i<2) {
            node.stress=1;
            assert(reloq_german_generic_fricative_ticks(&node,0,0,&got)==1 &&
                got==(i==0?125:118));
            node.stress=0;
        }
        node.same_syllable_right = 0;
        node.same_syllable_left = 1;
        node.left_phone = RELOQ_DE_PHONE_A;
        node.same_word_right = 1;
        assert(reloq_german_generic_fricative_ticks(&node, 0, 0, &got) == 1 &&
               got == intra[i]);
        node.same_word_right = 0;
        node.right_phone = RELOQ_DE_PHONE_t;
        node.stress = 1;
        assert(reloq_german_generic_fricative_ticks(&node, 0, 0, &got) == 1 &&
               got == connected[i]);
        assert(reloq_german_generic_fricative_ticks(&node, 1, 0, &got) == 1 &&
               got == final[i]);
        node.same_syllable_left = 0;
        got = -1;
        assert(reloq_german_generic_fricative_ticks(&node, 1, 0, &got) == 0 &&
               got == -1);
    }
    struct reloq_german_phone_node s = {
        .phone = RELOQ_DE_PHONE_S, .right_phone = RELOQ_DE_PHONE_I,
        .same_syllable_right = 1
    };
    int32_t got = 0;
    assert(reloq_german_generic_fricative_ticks(&s, 0, 0, &got) == 1 &&
           got == 87);
    s.phone = RELOQ_DE_PHONE_p;
    assert(reloq_german_generic_fricative_ticks(&s, 0, 0, &got) == 0);
    assert(reloq_german_generic_fricative_ticks(NULL, 0, 0, &got) == -1);
    assert(reloq_german_generic_fricative_ticks(&s, 2, 0, &got) == -1);
    assert(reloq_german_generic_fricative_ticks(&s, 0, 2, &got) == -1);
    assert(reloq_german_generic_fricative_ticks(&s, 0, 0, NULL) == -1);

    /* Controlled reference pairs distinguish a connected simple onset from
     * a consonant cluster, and a syllable onset within the same word from
     * the first syllable. None of these choices keys on written words. */
    struct reloq_german_phone_node coda = {
        .phone = RELOQ_DE_PHONE_f, .left_phone = RELOQ_DE_PHONE_I,
        .same_syllable_left = 1, .right_phone = RELOQ_DE_PHONE_S
    };
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 118);
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 1, &got) == 1 &&
           got == 60);
    coda.phone = RELOQ_DE_PHONE_S;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 70);
    coda.right_phone = RELOQ_DE_PHONE_l;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 110);
    coda.phone = RELOQ_DE_PHONE_s;
    coda.right_phone = RELOQ_DE_PHONE_f;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 85);
    coda.right_phone = RELOQ_DE_PHONE_t;
    coda.stress = 0;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 75);
    coda.stress = 1;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 96);
    coda.stress = 0;
    coda.right_phone = RELOQ_DE_PHONE_n;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 85);
    coda.stress = 1;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 102);
    coda.stress = 0;
    coda.right_phone = RELOQ_DE_PHONE_h;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 96);
    coda.phone = RELOQ_DE_PHONE_x;
    coda.right_phone = RELOQ_DE_PHONE_l;
    coda.stress = 1;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 128);
    coda.stress = 0;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 90);
    coda.right_phone = RELOQ_DE_PHONE_t;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 60);
    coda.right_phone = RELOQ_DE_PHONE_A;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 95);
    coda.phone = RELOQ_DE_PHONE_f;
    coda.right_phone = RELOQ_DE_PHONE_r;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 95);
    coda.phone = RELOQ_DE_PHONE_S;
    coda.right_phone = RELOQ_DE_PHONE_h;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 125);
    coda.phone = RELOQ_DE_PHONE_X;
    coda.right_phone = RELOQ_DE_PHONE_S;
    assert(reloq_german_generic_fricative_ticks(&coda, 0, 0, &got) == 1 &&
           got == 85);

    struct reloq_german_phone_node medial = {
        .phone = RELOQ_DE_PHONE_f, .left_phone = RELOQ_DE_PHONE_A,
        .same_word_left = 1, .right_phone = RELOQ_DE_PHONE_A,
        .same_syllable_right = 1
    };
    assert(reloq_german_generic_fricative_ticks(&medial, 0, 0, &got) == 1 &&
           got == 95);
    medial.phone = RELOQ_DE_PHONE_s;
    assert(reloq_german_generic_fricative_ticks(&medial, 0, 0, &got) == 1 &&
           got == 85);
    medial.phone = RELOQ_DE_PHONE_S;
    assert(reloq_german_generic_fricative_ticks(&medial, 0, 0, &got) == 1 &&
           got == 125);
    medial.right_phone = RELOQ_DE_PHONE_AT;
    assert(reloq_german_generic_fricative_ticks(&medial, 0, 0, &got) == 1 &&
           got == 95);
}

static void test_planner_integration(const char *annotation, int expected_b1,
                                     int expected_fnz)
{
    struct reloq_german_phone_node phones[8];
    struct reloq_german_phone_sequence_info info;
    size_t length = strlen(annotation);
    assert(reloq_german_phone_sequence_build(
               (const unsigned char *)annotation, length, 0,
               phones, 8, &info) == 0);
    struct reloq_german_syllable syllables[8];
    size_t syllable_count = 0;
    assert(reloq_german_syllable_plan_build(
               phones, info.phone_count, syllables, 8,
               &syllable_count) == 0);
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][64];
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        storage[i] = (struct reloq_target_buffer){points[i], 64, 0};
    struct reloq_target_builder builder;
    assert(reloq_target_builder_begin(&builder, storage) == 0);
    int64_t end_tick;
    assert(reloq_german_generic_plan(
               phones, info.phone_count, syllables, syllable_count,
               &builder, &end_tick, NULL, NULL) == 0);
    assert(end_tick > 0);
    int saw_b1 = 0;
    for (size_t i = 0; i < builder.stream[RELOQ_TARGET_B1].count; ++i)
        saw_b1 |= points[RELOQ_TARGET_B1][i].value == expected_b1;
    assert(saw_b1);
    int64_t start = -1, rise = -1;
    int32_t consonant_f1 = annotation[5] == 'm' ? 200 : 350;
    for (size_t i = 0; i < builder.stream[RELOQ_TARGET_F1].count; ++i)
        if (points[RELOQ_TARGET_F1][i].value == consonant_f1 &&
            points[RELOQ_TARGET_F1][i].tick > 40) {
            start = points[RELOQ_TARGET_F1][i].tick;
            break;
        }
    for (size_t i = 0; i < builder.stream[RELOQ_TARGET_FNZ].count; ++i) {
        int32_t value = points[RELOQ_TARGET_FNZ][i].value;
        if (expected_fnz == 200) assert(value == 200);
        if (value == expected_fnz &&
            points[RELOQ_TARGET_FNZ][i].tick > 40 && rise < 0)
            rise = points[RELOQ_TARGET_FNZ][i].tick;
    }
    assert(start >= 0);
    /* F1 now starts moving toward a nasal in the preceding vowel. The
     * independent nasal timing test above still fixes FNZ at its midpoint. */
    if (expected_fnz == 350) assert(rise > start);
}

static void test_cross_syllable_planner(const char *annotation,
                                        int expected_b1, int expected_b2,
                                        int expected_nasal_lag)
{
    struct reloq_german_phone_node phones[12];
    struct reloq_german_phone_sequence_info info;
    size_t length = strlen(annotation);
    assert(reloq_german_phone_sequence_build(
               (const unsigned char *)annotation, length, 0,
               phones, 12, &info) == 0);
    struct reloq_german_syllable syllables[12];
    size_t syllable_count = 0;
    assert(reloq_german_syllable_plan_build(
               phones, info.phone_count, syllables, 12,
               &syllable_count) == 0);
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][96];
    struct reloq_target_buffer storage[RELOQ_TARGET_STREAM_COUNT];
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        storage[i] = (struct reloq_target_buffer){points[i], 96, 0};
    struct reloq_target_builder builder;
    assert(reloq_target_builder_begin(&builder, storage) == 0);
    int64_t end_tick;
    assert(reloq_german_generic_plan(
               phones, info.phone_count, syllables, syllable_count,
               &builder, &end_tick, NULL, NULL) == 0);
    assert(end_tick > 0);
    int32_t largest_b1 = 0, largest_b2 = 0;
    for (size_t i = 0; i < builder.stream[RELOQ_TARGET_B1].count; ++i)
        if (points[RELOQ_TARGET_B1][i].value > largest_b1)
            largest_b1 = points[RELOQ_TARGET_B1][i].value;
    for (size_t i = 0; i < builder.stream[RELOQ_TARGET_B2].count; ++i)
        if (points[RELOQ_TARGET_B2][i].value > largest_b2)
            largest_b2 = points[RELOQ_TARGET_B2][i].value;
    assert(largest_b1 == expected_b1);
    assert(largest_b2 == expected_b2);
    if (expected_nasal_lag >= 0) {
        int64_t onset = -1, rise = -1;
        for (size_t i = 0; i < builder.stream[RELOQ_TARGET_F1].count; ++i)
            if (points[RELOQ_TARGET_F1][i].value == 200 &&
                points[RELOQ_TARGET_F1][i].tick > 20) {
                onset = points[RELOQ_TARGET_F1][i].tick;
                break;
            }
        for (size_t i = 0; i < builder.stream[RELOQ_TARGET_FNZ].count; ++i)
            if (points[RELOQ_TARGET_FNZ][i].value == 350 &&
                points[RELOQ_TARGET_FNZ][i].tick > onset) {
                rise = points[RELOQ_TARGET_FNZ][i].tick;
                break;
            }
        assert(onset >= 0 && rise - onset == expected_nasal_lag);
    }
}

int main(void)
{
    test_prenasal_bandwidth();
    test_nasal_timing();
    test_fricative_context_timing();
    test_planner_integration("`[.1Am]", 270, 350);
    test_planner_integration("`[.1Al]", 120, 200);
    test_cross_syllable_planner("`[.1A.0m@].", 270, 150, -1);
    /* The shared VC interval now reaches the nasal's F1 at the consonant
     * boundary, 7 ticks before its old first local point. FNZ retains its
     * independently checked midpoint of the short 30-ms nasal. */
    test_cross_syllable_planner("`[.1A.0n@].", 270, 200, 15);
    test_cross_syllable_planner("`[.1hE.0n@].", 220, 200, 15);
    test_cross_syllable_planner("`[.1O.0m@].", 320, 200, -1);
    test_cross_syllable_planner("`[.1A.0p@].", 120, 100, -1);
    test_cross_syllable_planner("`[.1A.0t@].", 120, 100, -1);
    test_cross_syllable_planner("`[.1A] `2 `[.1m@].", 120, 150, -1);
    test_cross_syllable_planner("`[.1A] `2 `[.1n@].", 120, 200, 35);
    test_cross_syllable_planner("`[.1An].", 270, 200, 60);
    puts("generic nasal transitions: bandwidth and onset checks passed");
    return 0;
}
