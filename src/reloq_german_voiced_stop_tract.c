#include "reloq_german_voiced_stop_tract.h"

#include <stdint.h>

static int covered_vowel(enum reloq_german_phone phone)
{
    switch (phone) {
    case RELOQ_DE_PHONE_A: case RELOQ_DE_PHONE_E:
    case RELOQ_DE_PHONE_I: case RELOQ_DE_PHONE_O:
    case RELOQ_DE_PHONE_U: case RELOQ_DE_PHONE_a:
    case RELOQ_DE_PHONE_e: case RELOQ_DE_PHONE_i:
    case RELOQ_DE_PHONE_o: case RELOQ_DE_PHONE_u:
        return 1;
    default:
        return 0;
    }
}

static int coda_stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

int reloq_german_voiced_stop_tract_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t onset_tick, int64_t formant_target_tick, int64_t hold_tick,
    int64_t closure_tick, struct reloq_target_builder *builder)
{
    if (!phones || !builder || onset_tick < 0 ||
        formant_target_tick <= onset_tick ||
        hold_tick < formant_target_tick || closure_tick <= hold_tick)
        return -1;
    if (phone_count != 3 ||
        (phones[0].phone != RELOQ_DE_PHONE_b &&
         phones[0].phone != RELOQ_DE_PHONE_d &&
         phones[0].phone != RELOQ_DE_PHONE_g) ||
        !covered_vowel(phones[1].phone) || !coda_stop(phones[2].phone))
        return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index || phones[i].syllable_index ||
            phones[i].syllable_in_word || phones[i].stress != 1 ||
            !phones[i].stress_explicit) return 1;

    static const int32_t widths[5] = {120, 100, 150, 330, 260};
    struct reloq_target_event events[38];
    size_t count = 0;
#define PUT(s, t, v) do { events[count++] = (struct reloq_target_event) \
    {(s), (t), (v)}; } while (0)
    for (enum reloq_target_stream s = RELOQ_TARGET_F4;
         s <= RELOQ_TARGET_F5; ++s) {
        int32_t value = s == RELOQ_TARGET_F4 ? 3600 : 3900;
        PUT(s, formant_target_tick, value);
        PUT(s, hold_tick, value);
    }
    for (size_t i = 0; i < 5; ++i) {
        enum reloq_target_stream s = (enum reloq_target_stream)
            (RELOQ_TARGET_B1 + (int)i);
        PUT(s, formant_target_tick, widths[i]);
        PUT(s, hold_tick, widths[i]);
    }
    for (enum reloq_target_stream s = RELOQ_TARGET_FNP;
         s <= RELOQ_TARGET_FNZ; ++s) {
        PUT(s, formant_target_tick, 200);
        PUT(s, hold_tick, 200);
    }
    PUT(RELOQ_TARGET_TL, onset_tick + 1, 0);
    PUT(RELOQ_TARGET_TL, closure_tick, 0);
    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s) {
        PUT(s, formant_target_tick, 0);
        PUT(s, hold_tick, 0);
    }
    PUT(RELOQ_TARGET_AF, formant_target_tick, 0);
    PUT(RELOQ_TARGET_AF, hold_tick, 0);
    PUT(RELOQ_TARGET_DI, formant_target_tick, 0);
    PUT(RELOQ_TARGET_DI, hold_tick, 0);
    PUT(RELOQ_TARGET_OQ, formant_target_tick, 56);
    PUT(RELOQ_TARGET_OQ, hold_tick, 56);
#undef PUT
    return count == sizeof events / sizeof events[0] ?
        reloq_target_builder_append(builder, 0, events, count) : -1;
}
