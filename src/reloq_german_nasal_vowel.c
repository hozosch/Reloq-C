#include "reloq_german_nasal_vowel.h"

#include "reloq_german_hv_timing.h"

#include <limits.h>

/* Vowel and neighboring-place coefficients are shared across coda places.
 * The duration is measured from the end of the initial nasal send. */
struct nasal_vowel {
    enum reloq_german_phone phone;
    int32_t f1, f2, f3, av;
    int64_t open_ticks;
};

static const struct nasal_vowel vowels[] = {
    {RELOQ_DE_PHONE_A, 780, 1160, 2500, 51, 97},
    {RELOQ_DE_PHONE_E, 550, 1800, 2600, 52, 105},
    {RELOQ_DE_PHONE_I, 380, 2000, 2800, 53, 98},
    {RELOQ_DE_PHONE_O, 570, 900, 2500, 53, 98},
    {RELOQ_DE_PHONE_U, 420, 930, 2400, 53, 84},
    {RELOQ_DE_PHONE_a, 800, 1220, 2500, 50, 168},
    {RELOQ_DE_PHONE_e, 370, 2150, 2700, 51, 182},
    {RELOQ_DE_PHONE_i, 265, 2150, 2900, 53, 168},
    {RELOQ_DE_PHONE_o, 390, 735, 2400, 54, 182},
    {RELOQ_DE_PHONE_u, 250, 750, 2400, 55, 168}
};

static int is_coda(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_nasal_vowel_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t origin_tick, struct reloq_target_builder *builder,
    int64_t *vowel_target_offset, int64_t *hold_end_offset,
    int64_t *closure_offset)
{
    if (!phones || !builder || !vowel_target_offset || !hold_end_offset ||
        !closure_offset || !phone_count || origin_tick < 0) return -1;
    if (phone_count != 3 ||
        (phones[0].phone != RELOQ_DE_PHONE_m &&
         phones[0].phone != RELOQ_DE_PHONE_n) ||
        !is_coda(phones[2].phone)) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index != 0 || phones[i].syllable_index !=
                phones[0].syllable_index ||
            phones[i].syllable_in_word != 0 || phones[i].stress != 1 ||
            !phones[i].stress_explicit) return 1;

    const struct nasal_vowel *v = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phones[1].phone == vowels[i].phone) {
            v = &vowels[i];
            break;
        }
    if (!v) return 1;

    const int m = phones[0].phone == RELOQ_DE_PHONE_m;
    const enum reloq_german_phone coda = phones[2].phone;
    const int64_t nasal_end = m ? 85 : 70;
    int64_t target = 100;
    if (!m) {
        if (v->phone == RELOQ_DE_PHONE_A || v->phone == RELOQ_DE_PHONE_a)
            target = 95;
        else if (v->phone == RELOQ_DE_PHONE_O)
            target = coda == RELOQ_DE_PHONE_t ? 118 : 120;
        else if (v->phone == RELOQ_DE_PHONE_U ||
                 v->phone == RELOQ_DE_PHONE_o ||
                 v->phone == RELOQ_DE_PHONE_u)
            target = 110;
    }
    /* In /n O t/ the vowel target occurs two ticks earlier and AV closure
     * one tick later than /n O p,k/. This is a vowel/coda context, never
     * a natural-spelling or complete-word exception. */
    const int64_t closure = nasal_end + v->open_ticks +
        (!m && v->phone == RELOQ_DE_PHONE_O &&
         coda == RELOQ_DE_PHONE_t);
    int64_t lead = 0;
    if (reloq_german_coda_hold_lead(v->phone, coda, &lead)) return 1;
    if (!m && v->phone == RELOQ_DE_PHONE_O &&
        coda == RELOQ_DE_PHONE_t) lead = 48;
    const int64_t hold = closure - lead;
    if (hold <= target || origin_tick > INT64_MAX - closure) return -1;

    int32_t f2 = v->f2, f3 = v->f3;
    if (v->phone == RELOQ_DE_PHONE_A) {
        f2 = m ? 1150 : coda == RELOQ_DE_PHONE_t ? 1300 : 1160;
    } else if (v->phone == RELOQ_DE_PHONE_U) {
        f2 = m ? 900 : 1000;
    } else if (v->phone == RELOQ_DE_PHONE_a && m) {
        f2 = 1100;
    } else if (v->phone == RELOQ_DE_PHONE_u && m) {
        f3 = 2600;
    }
    const int32_t held_f2 = v->phone == RELOQ_DE_PHONE_a ? v->f2 : f2;
    const int32_t values[] = {
        v->f1, f2, f3, 3600, 3900, 120, 100, 150, 330, 260,
        200, 200
    };
    struct reloq_target_event events[50];
    size_t count = 0;
    for (enum reloq_target_stream stream = RELOQ_TARGET_F1;
         stream <= RELOQ_TARGET_FNZ; ++stream) {
        int32_t value = values[(size_t)(stream - RELOQ_TARGET_F1)];
        put(events, &count, stream, target, value);
        put(events, &count, stream, hold,
            stream == RELOQ_TARGET_F2 ? held_f2 : value);
    }
    put(events, &count, RELOQ_TARGET_AV, nasal_end + 1, v->av);
    put(events, &count, RELOQ_TARGET_AV, closure, v->av - 2);
    put(events, &count, RELOQ_TARGET_AV, closure, 0);
    put(events, &count, RELOQ_TARGET_TL, nasal_end + 1, 0);
    put(events, &count, RELOQ_TARGET_TL, closure, 0);
    put(events, &count, RELOQ_TARGET_AH, nasal_end + 1, 34);
    put(events, &count, RELOQ_TARGET_AH, closure, 34);
    put(events, &count, RELOQ_TARGET_AH, closure, 0);
    for (enum reloq_target_stream stream = RELOQ_TARGET_A1F;
         stream <= RELOQ_TARGET_AB; ++stream) {
        put(events, &count, stream, target, 0);
        put(events, &count, stream, hold, 0);
    }
    put(events, &count, RELOQ_TARGET_AF, target, 0);
    put(events, &count, RELOQ_TARGET_AF, hold, 0);
    put(events, &count, RELOQ_TARGET_DI, target, 0);
    put(events, &count, RELOQ_TARGET_DI, hold, 0);
    put(events, &count, RELOQ_TARGET_OQ, target, 56);
    put(events, &count, RELOQ_TARGET_OQ, hold, 56);
    if (count != sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder, origin_tick, events, count))
        return -1;
    *vowel_target_offset = target;
    *hold_end_offset = hold;
    *closure_offset = closure;
    return 0;
}
