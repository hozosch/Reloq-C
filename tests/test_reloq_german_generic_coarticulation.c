#include "reloq_german_generic_coarticulation.h"
#include "reloq_german_generic_voice.h"
#include "reloq_german_syllable_plan.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_cv_classes(void)
{
    const int32_t low[3] = {700, 1150, 2500};
    const int32_t front[3] = {380, 2000, 2800};
    struct reloq_german_phone_node node = {
        .phone = RELOQ_DE_PHONE_p,
        .right_phone = RELOQ_DE_PHONE_A,
        .same_word_right = 1,
        .same_syllable_right = 1
    };
    struct reloq_german_generic_cv_gesture plan;
    assert(reloq_german_generic_cv_gesture(&node, low, &plan) == 1);
    assert(plan.first_formant[0] == 350 &&
           plan.first_formant[1] == 900 &&
           plan.anticipate_during_consonant &&
           plan.replace_consonant_tract);
    node.right_phone = RELOQ_DE_PHONE_I;
    assert(reloq_german_generic_cv_gesture(&node, front, &plan) == 1);
    assert(plan.first_formant[1] == 1250);
    node.phone = RELOQ_DE_PHONE_k;
    assert(reloq_german_generic_cv_gesture(&node, front, &plan) == 1);
    assert(plan.first_formant[1] == 1500 &&
           plan.release_formant[1] == 2200 &&
           plan.release_formant[2] == 2700);
    node.phone = RELOQ_DE_PHONE_s;
    assert(reloq_german_generic_cv_gesture(&node, front, &plan) == 1);
    assert(plan.first_formant[1] == 1760 &&
           !plan.anticipate_during_consonant &&
           plan.vowel_glide_ticks == 30);
    node.phone = RELOQ_DE_PHONE_v;
    assert(reloq_german_generic_cv_gesture(&node, low, &plan) == 1);
    assert(plan.first_formant[0] == 300 &&
           plan.release_formant[0] == 300 &&
           plan.vowel_glide_ticks == 25);
    node.phone = RELOQ_DE_PHONE_f;
    assert(reloq_german_generic_cv_gesture(&node, low, &plan) == 1);
    assert(plan.first_formant[0] == 400 &&
           plan.release_formant[0] == 400 &&
           plan.vowel_glide_ticks == 25);
    node.phone = RELOQ_DE_PHONE_m;
    assert(reloq_german_generic_cv_gesture(&node, front, &plan) == 1);
    assert(!plan.replace_consonant_tract &&
           plan.vowel_glide_ticks == 15);
    node.phone = RELOQ_DE_PHONE_h;
    assert(reloq_german_generic_cv_gesture(&node, front, &plan) == 1);
    assert(plan.first_formant[0] == front[0] &&
           plan.first_formant[1] == front[1] &&
           plan.first_formant[2] == front[2]);
    node.same_syllable_right = 0;
    assert(reloq_german_generic_cv_gesture(&node, front, &plan) == 0);
    assert(reloq_german_generic_cv_gesture(NULL, front, &plan) == -1);
}

static void test_vc_classes(void)
{
    const int32_t low[3] = {780, 1160, 2500};
    const int32_t front[3] = {380, 2000, 2800};
    const int32_t consonant[3] = {200, 750, 2500};
    struct reloq_german_phone_node vowel = {
        .phone = RELOQ_DE_PHONE_A,
        .right_phone = RELOQ_DE_PHONE_p,
        .same_word_right = 1,
        .same_syllable_right = 1
    };
    struct reloq_german_phone_node coda = {.phone = RELOQ_DE_PHONE_p};
    struct reloq_german_generic_vc_gesture plan;
    assert(reloq_german_generic_vc_gesture(
               &vowel, &coda, low, consonant, &plan) == 1);
    assert(plan.coda_formant[0] == 500 &&
           plan.coda_formant[1] == 900 &&
           plan.anticipation_ticks == 30);
    vowel.phone = RELOQ_DE_PHONE_I;
    assert(reloq_german_generic_vc_gesture(
               &vowel, &coda, front, consonant, &plan) == 1);
    assert(plan.coda_formant[0] == 300 &&
           plan.coda_formant[1] == 1000 &&
           plan.anticipation_ticks == 25);
    vowel.phone = RELOQ_DE_PHONE_A;
    vowel.right_phone = coda.phone = RELOQ_DE_PHONE_m;
    assert(reloq_german_generic_vc_gesture(
               &vowel, &coda, low, consonant, &plan) == 1);
    assert(plan.coda_formant[0] == consonant[0] &&
           plan.coda_formant[1] == consonant[1] &&
           plan.anticipation_ticks == 30);
    /* Velar and palatal fricatives need an oral-place movement during the
     * preceding vowel.  That movement differs by the vowel's front/back
     * tract position, without consulting a spelling or word list. */
    vowel.right_phone = coda.phone = RELOQ_DE_PHONE_x;
    assert(reloq_german_generic_vc_gesture(
               &vowel, &coda, low, consonant, &plan) == 1);
    assert(plan.coda_formant[0] == 300 &&
           plan.coda_formant[1] == 1220 &&
           plan.coda_formant[2] == low[2] &&
           plan.anticipation_ticks == 30);
    vowel.phone = RELOQ_DE_PHONE_U;
    assert(reloq_german_generic_vc_gesture(
               &vowel, &coda, low, consonant, &plan) == 1);
    assert(plan.coda_formant[1] == 900 &&
           plan.anticipation_ticks == 40);
    vowel.phone = RELOQ_DE_PHONE_i;
    vowel.right_phone = coda.phone = RELOQ_DE_PHONE_X;
    assert(reloq_german_generic_vc_gesture(
               &vowel, &coda, front, consonant, &plan) == 1);
    assert(plan.coda_formant[0] == 300 &&
           plan.coda_formant[1] == 2500 &&
           plan.coda_formant[2] == 3100 &&
           plan.anticipation_ticks == 25);
    vowel.same_syllable_right = 0;
    assert(reloq_german_generic_vc_gesture(
               &vowel, &coda, low, consonant, &plan) == 0);
    assert(reloq_german_generic_vc_gesture(
               NULL, &coda, low, consonant, &plan) == -1);
}

static void test_vowel_center_classes(void)
{
    struct reloq_german_phone_node node = {
        .phone = RELOQ_DE_PHONE_a, .left_phone = RELOQ_DE_PHONE_p,
        .same_syllable_left = 1
    };
    struct reloq_german_generic_vowel_center center = {-1, -1};
    assert(reloq_german_generic_vowel_center(&node, 1100, &center) == 1);
    assert(center.neutral_f2 == 1220 && center.before_coda_ticks == 0);
    node.right_phone = RELOQ_DE_PHONE_t;
    node.same_syllable_right = 1;
    node.same_word_right = 1;
    assert(reloq_german_generic_vowel_center(&node, 1100, &center) == 1);
    assert(center.before_coda_ticks == 50);
    node.right_phone = RELOQ_DE_PHONE_f;
    assert(reloq_german_generic_vowel_center(&node, 1100, &center) == 1);
    assert(center.before_coda_ticks == 25);
    node.right_phone = RELOQ_DE_PHONE_m;
    assert(reloq_german_generic_vowel_center(&node, 1100, &center) == 1);
    assert(center.before_coda_ticks == 30);
    node.phone = RELOQ_DE_PHONE_e;
    node.right_phone = RELOQ_DE_PHONE_s;
    assert(reloq_german_generic_vowel_center(&node, 2000, &center) == 1);
    assert(center.neutral_f2 == 2150 && center.before_coda_ticks == 25);
    node.left_phone = RELOQ_DE_PHONE_t;
    center.neutral_f2 = -1;
    assert(reloq_german_generic_vowel_center(&node, 2150, &center) == 0);
    assert(center.neutral_f2 == -1);
    node.phone = RELOQ_DE_PHONE_A;
    assert(reloq_german_generic_vowel_center(&node, 1150, &center) == 0);
    node.phone = RELOQ_DE_PHONE_i;
    assert(reloq_german_generic_vowel_center(&node, 2150, &center) == 0);
    node.phone = RELOQ_DE_PHONE_a;
    node.same_syllable_right = 0;
    assert(reloq_german_generic_vowel_center(&node, 1250, &center) == 0);
    assert(reloq_german_generic_vowel_center(NULL, 1100, &center) == -1);
    assert(reloq_german_generic_vowel_center(&node, -1, &center) == -1);
    assert(reloq_german_generic_vowel_center(&node, 1100, NULL) == -1);
}

struct emitted {
    struct reloq_acoustic_point points[RELOQ_TARGET_STREAM_COUNT][64];
    struct reloq_target_buffer buffers[RELOQ_TARGET_STREAM_COUNT];
    struct reloq_target_builder builder;
};

static void plan(const char *spr, struct emitted *out)
{
    struct reloq_german_phone_node nodes[8];
    struct reloq_german_phone_sequence_info info = {0};
    assert(reloq_german_phone_sequence_build(
               (const unsigned char *)spr, strlen(spr), 0,
               nodes, 8, &info) == 0);
    struct reloq_german_syllable syllables[8];
    size_t syllable_count = 0;
    assert(reloq_german_syllable_plan_build(
               nodes, info.phone_count, syllables, 8,
               &syllable_count) == 0);
    for (size_t i = 0; i < RELOQ_TARGET_STREAM_COUNT; ++i)
        out->buffers[i] = (struct reloq_target_buffer){
            out->points[i], 64, 0};
    assert(reloq_target_builder_begin(&out->builder, out->buffers) == 0);
    int64_t end = 0;
    assert(reloq_german_generic_plan(
               nodes, info.phone_count, syllables, syllable_count,
               &out->builder, &end, NULL, NULL) == 0);
    assert(end > 300);
}

static int64_t first_tick_after(const struct emitted *out,
                                enum reloq_target_stream stream,
                                int32_t value, int64_t after)
{
    for (size_t i = 0; i < out->builder.stream[stream].count; ++i)
        if (out->points[stream][i].value == value &&
            out->points[stream][i].tick >= after)
            return out->points[stream][i].tick;
    return -1;
}

static int64_t first_positive_after(const struct emitted *out,
                                    enum reloq_target_stream stream,
                                    int64_t after)
{
    for (size_t i = 0; i < out->builder.stream[stream].count; ++i)
        if (out->points[stream][i].value > 0 &&
            out->points[stream][i].tick >= after)
            return out->points[stream][i].tick;
    return -1;
}

static void test_planner(void)
{
    struct emitted a = {0};
    plan("`[.1pA].", &a);
    int64_t labial = first_tick_after(&a, RELOQ_TARGET_F1, 350, 1);
    int64_t vowel = first_tick_after(&a, RELOQ_TARGET_F1, 700, 1);
    assert(labial >= 0 && vowel - labial >= 25);
    plan("`[.1kI].", &a);
    /* The measured stop tract starts at the closure boundary, tick zero.
     * Its departure precedes aspiration and the vowel's acoustic target. */
    int64_t velar = first_tick_after(&a, RELOQ_TARGET_F2, 1500, 0);
    int64_t release = first_tick_after(&a, RELOQ_TARGET_F2, 2200, 1);
    int64_t palatal_vowel = first_tick_after(&a, RELOQ_TARGET_F2, 2000, 1);
    /* The original F2 knot is at 78: the vowel target does not wait an
     * extra tick after the stop's 61-tick aspirated transition. */
    assert(velar == 0 && release == 17 && palatal_vowel == 78);
    plan("`[.1sA].", &a);
    int64_t fricative = first_tick_after(&a, RELOQ_TARGET_F1, 300, 0);
    int64_t fricative_to_vowel = first_tick_after(
        &a, RELOQ_TARGET_F1, 700, fricative + 1);
    assert(fricative >= 0 && fricative_to_vowel - fricative > 30);
    plan("`[.1vA].", &a);
    assert(first_tick_after(&a, RELOQ_TARGET_F1, 300, 1) > 0);
    assert(first_tick_after(&a, RELOQ_TARGET_F1, 700, 1) >
           first_tick_after(&a, RELOQ_TARGET_F1, 300, 1));
    plan("`[.1fA].", &a);
    assert(first_tick_after(&a, RELOQ_TARGET_F1, 400, 1) > 0);
    plan("`[.1Af].", &a);
    int64_t oral = first_tick_after(&a, RELOQ_TARGET_F1, 780, 1);
    int64_t labiodental = first_tick_after(
        &a, RELOQ_TARGET_F1, 400, oral + 1);
    assert(oral >= 0 && labiodental > oral + 20);
    /* The noise reaches its plateau at the coda boundary; its preceding
     * rise shares the outgoing vowel transition observed in the reference. */
    assert(first_positive_after(&a, RELOQ_TARGET_AF, 0) == labiodental);
    plan("`[.1hA].", &a);
    /* Reference: an utterance-initial onset h is already on the vowel's
     * tract target in its first frame. */
    int64_t aspiration_tract = first_tick_after(
        &a, RELOQ_TARGET_F1, 700, 0);
    assert(aspiration_tract == 0);
    plan("`[.1AX].", &a);
    assert(first_tick_after(&a, RELOQ_TARGET_F2, 2150, 50) > 50);
    plan("`[.1ux].", &a);
    assert(first_tick_after(&a, RELOQ_TARGET_F2, 750, 100) > 100);
    plan("`[.1pa].", &a);
    int64_t colored_a = first_tick_after(&a, RELOQ_TARGET_F2, 1100, 40);
    int64_t center_a = first_tick_after(&a, RELOQ_TARGET_F2, 1220, 40);
    assert(colored_a > 0 && center_a > colored_a);
    plan("`[.1pat].", &a);
    int64_t coda_a = first_tick_after(&a, RELOQ_TARGET_F2, 1600, 60);
    assert(first_tick_after(&a, RELOQ_TARGET_F2, 1220, 60) > 60 &&
           first_tick_after(&a, RELOQ_TARGET_F2, 1220, 60) < coda_a);
    plan("`[.1pe].", &a);
    int64_t colored_e = first_tick_after(&a, RELOQ_TARGET_F2, 2000, 60);
    int64_t center_e = first_tick_after(&a, RELOQ_TARGET_F2, 2150, 60);
    assert(colored_e > 0 && center_e > colored_e);
    plan("`[.1pA].", &a);
    assert(first_tick_after(&a, RELOQ_TARGET_F2, 1220, 0) < 0);
    /* A diphthong has its own internal formant movement. In a shortened
     * unstressed vowel that target can otherwise be appended after the
     * anticipatory coda, producing an invalid non-monotonic curve. */
    plan("`[.0'aj'n].", &a);
    plan("`[.1Ar.0b'aj't].", &a);
}

/* The CV rise and the following VC anticipation may overlap for short
 * vowels. Exercise the time ordering over independent phone combinations,
 * including voiced stops and nasal/liquid codas; a rejected curve was found
 * in dAt when these intervals had initially been planned independently. */
static void test_overlapping_windows(void)
{
    static const char onset[] = "ptkbdgfsSmnlrh";
    static const char nucleus[] = "AEIOUaeiou@";
    static const char coda[] = "ptkfsSmnl";
    struct emitted out = {0};
    char spr[32];
    for (size_t a = 0; a < sizeof onset - 1; ++a)
        for (size_t v = 0; v < sizeof nucleus - 1; ++v)
            for (size_t c = 0; c < sizeof coda - 1; ++c) {
                int n = snprintf(spr, sizeof spr, "`[.1%c%c%c].",
                                 onset[a], nucleus[v], coda[c]);
                assert(n > 0 && (size_t)n < sizeof spr);
                plan(spr, &out);
            }
}

int main(void)
{
    test_cv_classes();
    test_vc_classes();
    test_vowel_center_classes();
    test_planner();
    test_overlapping_windows();
    puts("generic CV/VC tract gestures and integration passed");
    return 0;
}
