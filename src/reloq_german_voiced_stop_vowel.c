#include "reloq_german_voiced_stop_vowel.h"

#include "reloq_german_hv_timing.h"

#include <limits.h>

/* Shared vowel nuclei and measured place transitions. A row describes a
 * phoneme, not a word trajectory: source closure follows the base
 * open/coda interval used by the vowel nucleus, while place adjusts early
 * formants. The g formant lead is longer for low A/a. */
struct vowel_profile {
    enum reloq_german_phone phone;
    int32_t f1, f2, f3, av, closure;
    int32_t b_f1, b_f2, d_f2, g_f1, g_f2, g_f3;
    int8_t front, g_low;
};

static const struct vowel_profile profiles[] = {
    {RELOQ_DE_PHONE_A, 700,1160,2500,51, 97, 0,-10,140, 0,90,-100, 0,1},
    {RELOQ_DE_PHONE_E, 550,1800,2600,52,105, 50,-160,0, 0,50,0, 1,0},
    {RELOQ_DE_PHONE_I, 380,2000,2800,53, 98, 0,0,0, -30,0,0, 1,0},
    {RELOQ_DE_PHONE_O, 570,900,2500,53, 98, 0,0,0, 0,0,0, 0,0},
    {RELOQ_DE_PHONE_U, 420,930,2400,54, 84, 0,-30,70, 0,0,-340, 0,0},
    {RELOQ_DE_PHONE_a, 800,1220,2500,50,168, 0,-120,30, 0,60,0, 0,1},
    {RELOQ_DE_PHONE_e, 370,2150,2700,51,182, 0,-150,0, 0,0,0, 1,0},
    {RELOQ_DE_PHONE_i, 265,2150,2900,53,168, 0,0,0, 0,0,0, 1,0},
    {RELOQ_DE_PHONE_o, 390,735,2400,54,182, 0,0,0, 0,0,0, 0,0},
    {RELOQ_DE_PHONE_u, 250,750,2400,55,168, 0,0,0, 0,0,0, 0,0}
};

static int initial_stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_b || phone == RELOQ_DE_PHONE_d ||
           phone == RELOQ_DE_PHONE_g;
}

static int final_stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_voiced_stop_vowel_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t vowel_onset_tick, struct reloq_target_builder *builder,
    int64_t *closure_offset, int64_t *first_window_end,
    int64_t *second_window_end, int64_t *vowel_formant_start)
{
    if (!phones || !builder || !closure_offset || !first_window_end ||
        !second_window_end || !vowel_formant_start ||
        vowel_onset_tick < 0) return -1;
    if (phone_count != 3 || !initial_stop(phones[0].phone) ||
        !final_stop(phones[2].phone)) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index || phones[i].syllable_index ||
            phones[i].syllable_in_word || phones[i].stress != 1 ||
            !phones[i].stress_explicit) return 1;

    const struct vowel_profile *v = NULL;
    for (size_t i = 0; i < sizeof profiles / sizeof profiles[0]; ++i)
        if (phones[1].phone == profiles[i].phone) {
            v = &profiles[i];
            break;
        }
    if (!v) return 1;

    int64_t hold_lead;
    if (reloq_german_coda_hold_lead(v->phone, phones[2].phone,
                                   &hold_lead)) return 1;
    const enum reloq_german_phone left = phones[0].phone;
    const int64_t burst = left == RELOQ_DE_PHONE_b ?
        vowel_onset_tick - 11 : left == RELOQ_DE_PHONE_d ? 90 : 70;
    const int64_t target_lead = left == RELOQ_DE_PHONE_b ?
        (v->front ? 25 : 15) : left == RELOQ_DE_PHONE_d ?
        (v->front ? 25 : 45) : (v->g_low ? 35 : 25);
    if (burst < 0 || vowel_onset_tick <= burst ||
        vowel_onset_tick > INT64_MAX - v->closure) return -1;
    const int64_t closure = vowel_onset_tick + v->closure;
    int64_t hold = closure - hold_lead;
    /* A short U before a coronal/velar coda reaches the d-place target
     * only at the nominal hold. Leave a short final formant transition. */
    if (left == RELOQ_DE_PHONE_d && v->phone == RELOQ_DE_PHONE_U &&
        phones[2].phone != RELOQ_DE_PHONE_p &&
        hold < vowel_onset_tick + target_lead)
        hold = vowel_onset_tick + target_lead;
    int64_t target = vowel_onset_tick + target_lead;
    if (target > hold - 3) target = hold - 3;
    if (target <= vowel_onset_tick || hold >= closure) return -1;

    int32_t f1 = v->f1, f2 = v->f2, f3 = v->f3;
    if (v->phone == RELOQ_DE_PHONE_A) f1 += 80;
    if (left == RELOQ_DE_PHONE_b) {
        f1 += v->b_f1;
        f2 += v->b_f2;
    } else if (left == RELOQ_DE_PHONE_d) {
        f2 += v->d_f2;
    } else {
        f1 += v->g_f1;
        f2 += v->g_f2;
        f3 += v->g_f3;
    }
    const int32_t held_f2 = v->phone == RELOQ_DE_PHONE_a ||
        (left == RELOQ_DE_PHONE_b && v->phone == RELOQ_DE_PHONE_e) ?
        v->f2 : f2;

    struct reloq_target_event events[12];
    size_t count = 0;
    const int32_t formants[] = {f1, f2, f3};
    for (size_t i = 0; i < 3; ++i) {
        enum reloq_target_stream s =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)i);
        put(events, &count, s, target, formants[i]);
        put(events, &count, s, hold, i == 1 ? held_f2 : formants[i]);
    }
    put(events, &count, RELOQ_TARGET_AV, vowel_onset_tick + 1, v->av);
    put(events, &count, RELOQ_TARGET_AV, closure, v->av - 2);
    put(events, &count, RELOQ_TARGET_AV, closure, 0);
    put(events, &count, RELOQ_TARGET_AH, vowel_onset_tick + 1, 34);
    put(events, &count, RELOQ_TARGET_AH, closure, 34);
    put(events, &count, RELOQ_TARGET_AH, closure, 0);
    if (reloq_target_builder_append(builder, 0, events, count)) return -1;
    *closure_offset = v->closure;
    *first_window_end = burst;
    *second_window_end = hold;
    *vowel_formant_start = target;
    return 0;
}
