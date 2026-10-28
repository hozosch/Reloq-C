#include "reloq_german_stop_vowel.h"

#include "reloq_german_hv_timing.h"

#include <limits.h>

/* Voice phoneme coefficients and stop-to-vowel transitions. These describe
 * a vowel and adjacent phone classes, not recorded word trajectories. */
struct vowel_model {
    enum reloq_german_phone phone;
    int32_t f1, f2, f3, av;
    int32_t open_ticks, coda_shortening;
    int32_t p_prefix_f2, k_burst_f2, k_burst_f3;
    int32_t p_f2_delta, t_f2_delta;
    int32_t k_f1_delta, k_f2_delta, k_f3_delta;
};

static const struct vowel_model vowel_models[] = {
    {RELOQ_DE_PHONE_A, 700,1160,2500,51, 97,0,
     900,1600,2100, -10,140, 0,90,-100},
    {RELOQ_DE_PHONE_E, 550,1800,2600,52, 105,0,
     1150,2150,2600, 0,0, 0,0,0},
    {RELOQ_DE_PHONE_I, 380,2000,2800,53, 98,0,
     1250,2200,2700, 0,0, -30,0,0},
    {RELOQ_DE_PHONE_O, 570,900,2500,53, 98,0,
     850,1400,2000, 0,0, 0,0,0},
    {RELOQ_DE_PHONE_U, 420,930,2400,54, 84,0,
     800,1250,2000, -30,70, 0,0,-340},
    {RELOQ_DE_PHONE_a, 800,1220,2500,50, 210,42,
     900,1600,2100, -120,30, 0,60,0},
    {RELOQ_DE_PHONE_e, 370,2150,2700,51, 208,26,
     1250,2200,2850, -150,0, 0,0,0},
    {RELOQ_DE_PHONE_i, 265,2150,2900,53, 192,24,
     1250,2200,2850, 0,0, 0,0,0},
    {RELOQ_DE_PHONE_o, 390,735,2400,54, 208,26,
     800,1250,2000, 0,0, 0,0,0},
    {RELOQ_DE_PHONE_u, 250,750,2400,55, 192,24,
     800,1250,2000, 0,0, 0,0,0}
};

static int is_stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_stop_vowel_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t vowel_onset_tick, struct reloq_target_builder *builder,
    int64_t *closure_offset, int64_t *first_window_end,
    int64_t *second_window_end)
{
    if (!phones || !builder || !closure_offset || !first_window_end ||
        !second_window_end || vowel_onset_tick < 0) return -1;
    if ((phone_count != 2 && phone_count != 3) ||
        !is_stop(phones[0].phone) ||
        (phone_count == 3 && !is_stop(phones[2].phone))) return 1;
    for (size_t i = 0; i < phone_count; ++i) {
        if (phones[i].word_index || phones[i].syllable_index ||
            phones[i].stress != 1 || !phones[i].stress_explicit)
            return 1;
    }
    const struct vowel_model *v = NULL;
    for (size_t i = 0;
         i < sizeof vowel_models / sizeof vowel_models[0]; ++i)
        if (phones[1].phone == vowel_models[i].phone) {
            v = &vowel_models[i];
            break;
        }
    if (!v) return 1;

    const enum reloq_german_phone left = phones[0].phone;
    const enum reloq_german_phone right = phone_count == 3 ?
        phones[2].phone : RELOQ_DE_PHONE_NONE;
    const int front_t = v->phone == RELOQ_DE_PHONE_E ||
        v->phone == RELOQ_DE_PHONE_I || v->phone == RELOQ_DE_PHONE_e ||
        v->phone == RELOQ_DE_PHONE_i;
    const int64_t burst_end = left == RELOQ_DE_PHONE_p ? 15 :
        left == RELOQ_DE_PHONE_t && front_t ? 23 : 17;
    const int64_t base = v->open_ticks -
        (right == RELOQ_DE_PHONE_NONE ? 0 : v->coda_shortening);
    const int64_t closure = base * 9 / 10;
    int64_t lead = 0;
    if (right != RELOQ_DE_PHONE_NONE &&
        reloq_german_coda_hold_lead(v->phone, right, &lead)) return 1;
    if (vowel_onset_tick <= burst_end ||
        vowel_onset_tick > INT64_MAX - closure - 60 ||
        vowel_onset_tick + closure - lead <= burst_end) return -1;
    const int64_t closure_tick = vowel_onset_tick + closure;
    const int64_t hold_end = closure_tick - lead;

    int32_t prefix_f1 = left == RELOQ_DE_PHONE_p ? 350 : 300;
    int32_t prefix_f2 = left == RELOQ_DE_PHONE_p ?
        v->p_prefix_f2 : left == RELOQ_DE_PHONE_t ? 1600 : 1500;
    int32_t prefix_f3 = left == RELOQ_DE_PHONE_k ? 2300 : 2500;
    int32_t burst_f1 = prefix_f1;
    int32_t burst_f2 = prefix_f2;
    int32_t burst_f3 = prefix_f3;
    if (left == RELOQ_DE_PHONE_t) {
        if (front_t) { burst_f2 = 1700; burst_f3 = 2700; }
        if (v->phone == RELOQ_DE_PHONE_u) burst_f1 = 280;
    } else if (left == RELOQ_DE_PHONE_k) {
        burst_f2 = v->k_burst_f2;
        burst_f3 = v->k_burst_f3;
    }
    int32_t vowel_f1 = v->f1;
    int32_t vowel_f2 = v->f2;
    int32_t vowel_f3 = v->f3;
    if (right != RELOQ_DE_PHONE_NONE && v->phone == RELOQ_DE_PHONE_A)
        vowel_f1 += 80;
    if (left == RELOQ_DE_PHONE_p) vowel_f2 += v->p_f2_delta;
    else if (left == RELOQ_DE_PHONE_t) vowel_f2 += v->t_f2_delta;
    else {
        vowel_f1 += v->k_f1_delta;
        vowel_f2 += v->k_f2_delta;
        vowel_f3 += v->k_f3_delta;
    }

    struct reloq_target_event events[28];
    size_t count = 0;
    const int32_t initial[] = {prefix_f1,prefix_f2,prefix_f3};
    const int32_t burst[] = {burst_f1,burst_f2,burst_f3};
    const int32_t nucleus[] = {vowel_f1,vowel_f2,vowel_f3};
    /* The a-vowel's F2 onset coarticulation relaxes to its neutral vowel
     * target, and initial p does the same before e. Other tested context
     * values remain steady through the second numeric send endpoint. */
    const int32_t held_f2 = v->phone == RELOQ_DE_PHONE_a ||
        (v->phone == RELOQ_DE_PHONE_e && left == RELOQ_DE_PHONE_p) ?
        v->f2 : vowel_f2;
    for (size_t i = 0; i < 3; ++i) {
        const enum reloq_target_stream s =
            (enum reloq_target_stream)(RELOQ_TARGET_F1 + (int)i);
        put(events, &count, s, 0, initial[i]);
        put(events, &count, s, burst_end, burst[i]);
        put(events, &count, s, vowel_onset_tick, nucleus[i]);
        put(events, &count, s, hold_end,
            i == 1 ? held_f2 : nucleus[i]);
    }
    put(events, &count, RELOQ_TARGET_AV, vowel_onset_tick + 1, v->av);
    put(events, &count, RELOQ_TARGET_AV, closure_tick, v->av - 2);
    put(events, &count, RELOQ_TARGET_AV,
        right == RELOQ_DE_PHONE_NONE ? closure_tick + 60 : closure_tick, 0);
    put(events, &count, RELOQ_TARGET_AH, vowel_onset_tick + 1, 34);
    put(events, &count, RELOQ_TARGET_AH, closure_tick, 34);
    put(events, &count, RELOQ_TARGET_AH, closure_tick,
        right == RELOQ_DE_PHONE_NONE ? 35 : 0);
    if (right == RELOQ_DE_PHONE_NONE)
        put(events, &count, RELOQ_TARGET_AH, closure_tick + 60, 0);

    if (reloq_target_builder_append(builder, 0, events, count)) return -1;
    *closure_offset = closure;
    *first_window_end = burst_end;
    *second_window_end = hold_end;
    return 0;
}
