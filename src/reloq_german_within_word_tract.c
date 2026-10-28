#include "reloq_german_within_word_tract.h"

#include <limits.h>
#include <stddef.h>

struct vowel_shape {
    enum reloq_german_phone phone;
    int32_t f1, f2, f3;
    int32_t p_prefix_f2, k_burst_f2, k_burst_f3;
    int32_t closure_p[3], closure_k[3];
};

/* The vowel rows are phonetic classes. In an unstressed first syllable
 * short I has a lower F3 than the same vowel in the stressed second
 * syllable; a and A share the same shortened, pre-coda nucleus here. */
static const struct vowel_shape vowels[] = {
    {RELOQ_DE_PHONE_A, 780,1160,2500, 900,1600,2100,
     {500,900,2500}, {500,1750,2200}},
    {RELOQ_DE_PHONE_E, 550,1800,2600, 1150,2150,2600,
     {300,1000,2200}, {300,1900,2300}},
    {RELOQ_DE_PHONE_I, 380,2000,2520, 1250,2200,2700,
     {300,1000,2200}, {300,1900,2300}},
    {RELOQ_DE_PHONE_O, 570,900,2500, 850,1400,2000,
     {300,700,2500}, {300,1500,2300}},
    {RELOQ_DE_PHONE_U, 420,930,2400, 800,1250,2000,
     {300,600,2100}, {300,1000,2300}},
    {RELOQ_DE_PHONE_a, 780,1160,2500, 900,1600,2100,
     {500,900,2500}, {500,1750,2200}},
    {RELOQ_DE_PHONE_e, 370,2150,2700, 1250,2200,2850,
     {300,1500,2500}, {300,1900,2500}},
    {RELOQ_DE_PHONE_i, 265,2150,2900, 1250,2200,2850,
     {300,1500,2500}, {300,2200,2400}},
    {RELOQ_DE_PHONE_o, 390,735,2400, 800,1250,2000,
     {300,600,2100}, {300,960,2300}},
    {RELOQ_DE_PHONE_u, 250,750,2400, 800,1250,2000,
     {300,600,2100}, {300,1000,2300}}
};

/* Second-syllable vowel is stressed and follows /t/. The first row's weak
 * F3 and rounded-vowel closure reductions do not apply to this nucleus. */
struct stressed_vowel_shape {
    enum reloq_german_phone phone;
    int32_t nucleus[3];
    int32_t closure[3][3]; /* following p, t, k */
    int8_t front_release, t_high_f3;
};

static const struct stressed_vowel_shape stressed[] = {
    {RELOQ_DE_PHONE_A, {780,1300,2500},
     {{500,900,2500},{300,1600,2500},{500,1750,2200}},0,0},
    {RELOQ_DE_PHONE_E, {550,1800,2600},
     {{300,1000,2200},{300,1800,2500},{300,1900,2300}},1,0},
    {RELOQ_DE_PHONE_I, {380,2000,2800},
     {{300,1000,2200},{300,1800,2700},{300,1900,2300}},1,1},
    {RELOQ_DE_PHONE_O, {570,900,2500},
     {{300,700,2500},{300,1400,2500},{500,1500,2300}},0,0},
    {RELOQ_DE_PHONE_U, {420,1000,2400},
     {{300,600,2100},{300,1300,2050},{400,1000,2300}},0,0},
    {RELOQ_DE_PHONE_a, {800,1250,2500},
     {{500,900,2500},{300,1600,2500},{500,1750,2200}},0,0},
    {RELOQ_DE_PHONE_e, {370,2150,2700},
     {{300,1500,2500},{300,1900,2700},{300,1900,2500}},1,1},
    {RELOQ_DE_PHONE_i, {265,2150,2900},
     {{300,1500,2500},{300,1900,2700},{300,2200,2400}},1,1},
    {RELOQ_DE_PHONE_o, {390,735,2400},
     {{300,600,2100},{300,1300,2050},{400,960,2300}},0,0},
    {RELOQ_DE_PHONE_u, {250,750,2400},
     {{300,600,2100},{300,1300,2050},{300,1000,2300}},0,0}
};

static int valid_shape(const struct reloq_german_phone_node *p,
                       const struct reloq_german_syllable *s)
{
    if (s[0].first_phone != 0 || s[1].first_phone != 3 ||
        s[0].phone_count != 3 || s[1].phone_count != 3 ||
        s[0].onset_count != 1 || s[0].nucleus_count != 1 ||
        s[0].coda_count != 1 || s[1].onset_count != 1 ||
        s[1].nucleus_count != 1 || s[1].coda_count != 1 ||
        !s[0].nucleus_contiguous || !s[1].nucleus_contiguous ||
        s[0].stress != 0 || s[1].stress != 1 ||
        !s[0].stress_explicit || !s[1].stress_explicit ||
        s[0].word_index != s[1].word_index ||
        s[0].syllable_in_word != 0 || s[1].syllable_in_word != 1 ||
        (p[0].phone != RELOQ_DE_PHONE_p &&
         p[0].phone != RELOQ_DE_PHONE_t &&
         p[0].phone != RELOQ_DE_PHONE_k) ||
        (p[2].phone != RELOQ_DE_PHONE_p &&
         p[2].phone != RELOQ_DE_PHONE_k) ||
        p[3].phone != RELOQ_DE_PHONE_t ||
        (p[5].phone != RELOQ_DE_PHONE_p &&
         p[5].phone != RELOQ_DE_PHONE_t &&
         p[5].phone != RELOQ_DE_PHONE_k))
        return 0;
    for (size_t i = 0; i < 6; ++i)
        if (p[i].word_index != s[0].word_index ||
            p[i].syllable_index != p[i < 3 ? 0 : 3].syllable_index ||
            p[i].syllable_in_word != (i < 3 ? 0U : 1U) ||
            p[i].stress != (i < 3 ? 0U : 1U) ||
            !p[i].stress_explicit) return 0;
    return 1;
}

static int valid_ticks(const struct reloq_german_within_word_landmarks *t)
{
    return t->first_burst_end > 5 &&
        t->first_vowel_onset > t->first_burst_end &&
        t->first_vowel_hold > t->first_vowel_onset &&
        t->first_closure > t->first_vowel_hold &&
        t->first_coda_burst_start > t->first_closure &&
        t->first_coda_release > t->first_coda_burst_start &&
        t->second_onset_origin > t->first_coda_release &&
        t->second_burst_start > t->second_onset_origin &&
        t->second_burst_end > t->second_burst_start &&
        t->second_vowel_onset > t->second_burst_end &&
        t->second_vowel_hold > t->second_vowel_onset &&
        t->second_closure > t->second_vowel_hold &&
        t->second_coda_release <= INT64_MAX - 60 &&
        t->second_coda_release >= 13 &&
        t->second_closure < t->second_coda_release - 12 &&
        t->planned_ticks > t->second_coda_release + 60;
}

static void put(struct reloq_target_event *events, size_t *used,
                enum reloq_target_stream stream, int64_t tick,
                int32_t value)
{
    events[(*used)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_within_word_tract_append(
    const struct reloq_german_phone_node *phones, size_t count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_within_word_landmarks *t,
    struct reloq_target_builder *builder)
{
    if (!phones || !syllables || !t || !builder) return -1;
    if (count != 6 || syllable_count != 2 ||
        !valid_shape(phones, syllables)) return 1;
    const struct vowel_shape *v = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phones[1].phone == vowels[i].phone) {
            v = &vowels[i]; break;
        }
    if (!v) return 1;
    const struct stressed_vowel_shape *second = NULL;
    for (size_t i = 0; i < sizeof stressed / sizeof stressed[0]; ++i)
        if (phones[4].phone == stressed[i].phone) {
            second = &stressed[i]; break;
        }
    if (!second) return 1;
    if (!valid_ticks(t)) return -1;

    const enum reloq_german_phone onset = phones[0].phone;
    const int p_coda = phones[2].phone == RELOQ_DE_PHONE_p;
    const size_t second_place = phones[5].phone == RELOQ_DE_PHONE_p ? 0 :
        phones[5].phone == RELOQ_DE_PHONE_t ? 1 : 2;
    const int64_t second_preburst = second_place == 0 ? 10 :
        second_place == 1 ? 8 : 12;
    const int front = v->phone == RELOQ_DE_PHONE_E ||
        v->phone == RELOQ_DE_PHONE_I || v->phone == RELOQ_DE_PHONE_e ||
        v->phone == RELOQ_DE_PHONE_i;
    int32_t prefix[3] = {
        onset == RELOQ_DE_PHONE_p ? 350 : 300,
        onset == RELOQ_DE_PHONE_p ? v->p_prefix_f2 :
            onset == RELOQ_DE_PHONE_t ? 1600 : 1500,
        onset == RELOQ_DE_PHONE_k ? 2300 : 2500
    };
    int32_t burst[3] = {prefix[0], prefix[1], prefix[2]};
    if (onset == RELOQ_DE_PHONE_t) {
        if (front) { burst[1] = 1700; burst[2] = 2700; }
        if (v->phone == RELOQ_DE_PHONE_u) burst[0] = 280;
    } else if (onset == RELOQ_DE_PHONE_k) {
        burst[1] = v->k_burst_f2;
        burst[2] = v->k_burst_f3;
    }
    int32_t nucleus[3] = {v->f1, v->f2, v->f3};
    if (onset == RELOQ_DE_PHONE_p) {
        if (v->phone == RELOQ_DE_PHONE_A ||
            v->phone == RELOQ_DE_PHONE_a) nucleus[1] = 1150;
        else if (v->phone == RELOQ_DE_PHONE_U) nucleus[1] = 900;
        else if (v->phone == RELOQ_DE_PHONE_e) nucleus[1] = 2000;
    } else if (onset == RELOQ_DE_PHONE_t) {
        if (v->phone == RELOQ_DE_PHONE_A ||
            v->phone == RELOQ_DE_PHONE_a) nucleus[1] = 1300;
        else if (v->phone == RELOQ_DE_PHONE_U) nucleus[1] = 1000;
    } else {
        if (v->phone == RELOQ_DE_PHONE_A ||
            v->phone == RELOQ_DE_PHONE_a) {
            nucleus[1] = 1250; nucleus[2] = 2400;
        } else if (v->phone == RELOQ_DE_PHONE_I) nucleus[0] = 350;
        else if (v->phone == RELOQ_DE_PHONE_U) nucleus[2] = 2060;
    }
    int32_t held[3] = {nucleus[0], nucleus[1], nucleus[2]};
    if (onset == RELOQ_DE_PHONE_p &&
        v->phone == RELOQ_DE_PHONE_e) held[1] = v->f2;

    const int32_t *constriction = p_coda ? v->closure_p : v->closure_k;
    const int64_t first_timeline[] = {
        0, t->first_burst_end, t->first_vowel_onset,
        t->first_vowel_hold, t->first_closure,
        t->first_coda_release, t->second_onset_origin,
        t->second_burst_end, t->second_vowel_onset,
        t->second_vowel_hold, t->second_closure,
        t->second_coda_release
    };
    struct reloq_target_event events[320];
    size_t used = 0;
    for (size_t i = 0; i < 3; ++i) {
        const int32_t values[] = {
            prefix[i], burst[i], nucleus[i], held[i],
            constriction[i],
            i == 0 ? 300 : i == 1 ? (p_coda ? 1300 : 1800) :
                (p_coda ? 2500 : 2300),
            i == 0 ? 300 : i == 1 ? 1600 : 2500,
            i == 0 && second->phone == RELOQ_DE_PHONE_u ? 280 :
                i == 0 ? 300 : i == 1 ? (second->front_release ? 1700 :
                                           1600) :
                (second->front_release ? 2700 : 2500),
            second->nucleus[i],
            second->phone == RELOQ_DE_PHONE_a && i == 1 ?
                1220 : second->nucleus[i],
            second->closure[second_place][i],
            i == 0 ? 300 : i == 1 ?
                (second_place == 0 ? 1000 : second_place == 1 ?
                    (second->front_release ? 1800 : 1600) :
                    (second->front_release ? 1900 : 1500)) :
                (second_place == 0 ? 2500 : second_place == 1 ?
                    (second->t_high_f3 ? 2700 : 2500) : 2300)
        };
        for (size_t j = 0; j < 12; ++j)
            put(events, &used, (enum reloq_target_stream)
                (RELOQ_TARGET_F1 + (int)i), first_timeline[j], values[j]);
    }
    const int64_t tract_timeline[] = {
        0, t->first_burst_end, t->first_vowel_onset,
        t->first_vowel_hold, t->first_closure,
        t->first_coda_release, t->second_onset_origin,
        t->second_burst_end, t->second_vowel_onset,
        t->second_vowel_hold, t->second_closure,
        t->second_coda_release, t->second_coda_release + 60,
        t->planned_ticks
    };
    const int32_t widths[] = {120, 100, 150, 330, 260};
    for (size_t i = 0; i < 2; ++i)
        for (size_t j = 0; j < 14; ++j) {
            const int t_initial = onset == RELOQ_DE_PHONE_t && j < 2;
            const int t_second = j == 6 || j == 7;
            const int t_final = second_place == 1 && (j == 10 || j == 11);
            const int t_tract = t_initial || t_second || t_final;
            put(events, &used, (enum reloq_target_stream)
                (RELOQ_TARGET_F4 + (int)i), tract_timeline[j],
                i == 0 ? (t_tract ? 3500 : 3600) :
                         (t_tract ? 4200 : 3900));
        }
    for (size_t i = 0; i < 5; ++i)
        for (size_t j = 0; j < 14; ++j)
            put(events, &used, (enum reloq_target_stream)
                (RELOQ_TARGET_B1 + (int)i), tract_timeline[j], widths[i]);
    for (enum reloq_target_stream stream = RELOQ_TARGET_FNP;
         stream <= RELOQ_TARGET_FNZ; ++stream)
        for (size_t j = 0; j < 14; ++j)
            put(events, &used, stream, tract_timeline[j], 200);

    const int64_t flutter_ticks[] = {
        0, t->first_coda_release, t->second_onset_origin,
        t->second_coda_release
    };
    const enum reloq_target_stream flutter[] = {
        RELOQ_TARGET_FTP, RELOQ_TARGET_FTZ, RELOQ_TARGET_FL};
    for (size_t i = 0; i < sizeof flutter / sizeof flutter[0]; ++i)
        for (size_t j = 0; j < 4; ++j)
            put(events, &used, flutter[i], flutter_ticks[j],
                flutter[i] == RELOQ_TARGET_FL ? 0 : 250);

    const int64_t oq_ticks[] = {
        0, 5, t->first_vowel_onset, t->first_vowel_hold,
        t->first_closure, t->first_coda_burst_start,
        t->second_onset_origin, t->second_burst_start,
        t->second_vowel_onset, t->second_vowel_hold,
        t->second_closure, t->second_coda_release - second_preburst,
        t->second_coda_release + 60, t->planned_ticks
    };
    const int64_t tl_ticks[] = {
        0, 5, 5, t->first_burst_end, t->first_vowel_onset + 1,
        t->first_closure, t->first_closure,
        t->first_coda_burst_start, t->first_coda_burst_start,
        t->first_coda_release, t->second_onset_origin,
        t->second_burst_start, t->second_burst_start,
        t->second_burst_end, t->second_vowel_onset + 1,
        t->second_closure, t->second_closure,
        t->second_coda_release - second_preburst,
        t->second_coda_release - second_preburst,
        t->second_coda_release, t->second_coda_release + 60,
        t->planned_ticks
    };
    for (size_t j = 0; j < sizeof tl_ticks / sizeof tl_ticks[0]; ++j)
        put(events, &used, RELOQ_TARGET_TL, tl_ticks[j], 0);
    for (size_t j = 0; j < sizeof oq_ticks / sizeof oq_ticks[0]; ++j) {
        put(events, &used, RELOQ_TARGET_OQ, oq_ticks[j], 56);
        put(events, &used, RELOQ_TARGET_DI, oq_ticks[j], 0);
    }
    if (used > sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder, 0, events, used)) return -1;
    return 0;
}
