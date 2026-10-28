#include "reloq_german_stop_fricative_vowel.h"

#include <stdint.h>

struct vowel_profile {
    enum reloq_german_phone phone;
    int32_t f1, f2, f3, av;
    int32_t closure, s_lead, f_lead;
    int32_t p_prefix_f2, k_burst_f2, k_burst_f3;
    int32_t p_f2_delta, t_f2_delta;
    int32_t k_f1_delta, k_f2_delta, k_f3_delta;
};

/* The initial place changes the vowel's F1-F3 exactly as in stop-final
 * contexts. The following fricative instead sets the vowel's closure and
 * its second send lead. Each row describes a vowel class and adjacent-phone
 * interactions, not a recorded word or a whole-word control contour. */
static const struct vowel_profile profiles[] = {
    {RELOQ_DE_PHONE_A, 780,1160,2500,51, 96,40,25,
     900,1600,2100, -10,140, 0,90,-100},
    {RELOQ_DE_PHONE_E, 550,1800,2600,52, 103,25,25,
     1150,2150,2600, 0,0, 0,0,0},
    {RELOQ_DE_PHONE_I, 380,2000,2800,53, 97,25,25,
     1250,2200,2700, 0,0, -30,0,0},
    {RELOQ_DE_PHONE_O, 570,900,2500,53, 97,50,25,
     850,1400,2000, 0,0, 0,0,0},
    {RELOQ_DE_PHONE_U, 420,930,2400,54, 83,40,25,
     800,1250,2000, -30,70, 0,0,-340},
    {RELOQ_DE_PHONE_a, 800,1220,2500,50, 166,50,25,
     900,1600,2100, -120,30, 0,60,0},
    {RELOQ_DE_PHONE_e, 370,2150,2700,51, 180,25,25,
     1250,2200,2850, -150,0, 0,0,0},
    {RELOQ_DE_PHONE_i, 265,2150,2900,53, 166,25,25,
     1250,2200,2850, 0,0, 0,0,0},
    {RELOQ_DE_PHONE_o, 390,735,2400,54, 180,40,30,
     800,1250,2000, 0,0, 0,0,0},
    {RELOQ_DE_PHONE_u, 250,750,2400,55, 166,40,30,
     800,1250,2000, 0,0, 0,0,0}
};

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_stop_fricative_vowel_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t vowel_onset_tick, struct reloq_target_builder *builder,
    int64_t *closure_offset, int64_t *first_window_end,
    int64_t *second_window_end)
{
    if (!phones || !builder || !closure_offset || !first_window_end ||
        !second_window_end || vowel_onset_tick < 0) return -1;
    if (phone_count != 3 ||
        (phones[0].phone != RELOQ_DE_PHONE_p &&
         phones[0].phone != RELOQ_DE_PHONE_t &&
         phones[0].phone != RELOQ_DE_PHONE_k) ||
        (phones[2].phone != RELOQ_DE_PHONE_s &&
         phones[2].phone != RELOQ_DE_PHONE_f)) return 1;
    for (size_t i = 0; i < 3; ++i)
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
    const enum reloq_german_phone left = phones[0].phone;
    const int s = phones[2].phone == RELOQ_DE_PHONE_s;
    const int front_t = v->phone == RELOQ_DE_PHONE_E ||
        v->phone == RELOQ_DE_PHONE_I || v->phone == RELOQ_DE_PHONE_e ||
        v->phone == RELOQ_DE_PHONE_i;
    const int64_t burst = left == RELOQ_DE_PHONE_p ? 15 :
        left == RELOQ_DE_PHONE_t && front_t ? 23 : 17;
    const int64_t hold = v->closure - (s ? v->s_lead : v->f_lead);
    if (vowel_onset_tick <= burst ||
        vowel_onset_tick > INT64_MAX - v->closure ||
        vowel_onset_tick + hold <= burst) return -1;
    const int64_t closure_tick = vowel_onset_tick + v->closure;
    const int64_t hold_tick = vowel_onset_tick + hold;

    int32_t prefix[3] = {left == RELOQ_DE_PHONE_p ? 350 : 300,
                         left == RELOQ_DE_PHONE_p ? v->p_prefix_f2 :
                         left == RELOQ_DE_PHONE_t ? 1600 : 1500,
                         left == RELOQ_DE_PHONE_k ? 2300 : 2500};
    int32_t release[3] = {prefix[0], prefix[1], prefix[2]};
    if (left == RELOQ_DE_PHONE_t) {
        if (front_t) { release[1] = 1700; release[2] = 2700; }
        if (v->phone == RELOQ_DE_PHONE_u) release[0] = 280;
    } else if (left == RELOQ_DE_PHONE_k) {
        release[1] = v->k_burst_f2;
        release[2] = v->k_burst_f3;
    }
    int32_t nucleus[3] = {v->f1, v->f2, v->f3};
    if (left == RELOQ_DE_PHONE_p) nucleus[1] += v->p_f2_delta;
    else if (left == RELOQ_DE_PHONE_t) nucleus[1] += v->t_f2_delta;
    else {
        nucleus[0] += v->k_f1_delta;
        nucleus[1] += v->k_f2_delta;
        nucleus[2] += v->k_f3_delta;
    }
    if (s && v->phone == RELOQ_DE_PHONE_u) nucleus[0] = 320;
    const int32_t held_f2 = s && v->phone == RELOQ_DE_PHONE_u ? 820 :
        v->phone == RELOQ_DE_PHONE_a ||
        (v->phone == RELOQ_DE_PHONE_e && left == RELOQ_DE_PHONE_p) ?
        v->f2 : nucleus[1];
    const int32_t av = s && v->phone == RELOQ_DE_PHONE_U ? 58 : v->av;

    struct reloq_target_event events[16];
    size_t count = 0;
    for (size_t i = 0; i < 3; ++i) {
        const enum reloq_target_stream formant =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)i);
        put(events, &count, formant, 0, prefix[i]);
        put(events, &count, formant, burst, release[i]);
        put(events, &count, formant, vowel_onset_tick, nucleus[i]);
        put(events, &count, formant, hold_tick,
            i == 1 ? held_f2 : nucleus[i]);
    }
    put(events, &count, RELOQ_TARGET_AV, vowel_onset_tick + 1, av);
    put(events, &count, RELOQ_TARGET_AV, closure_tick, av - 2);
    put(events, &count, RELOQ_TARGET_AH, vowel_onset_tick + 1, 34);
    put(events, &count, RELOQ_TARGET_AH, closure_tick, 34);
    if (reloq_target_builder_append(builder, 0, events, count)) return -1;
    *closure_offset = v->closure;
    *first_window_end = burst;
    *second_window_end = hold_tick;
    return 0;
}
