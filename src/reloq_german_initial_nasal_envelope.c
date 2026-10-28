#include "reloq_german_initial_nasal_envelope.h"

#include <limits.h>

static int is_vowel(enum reloq_german_phone phone)
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

int reloq_german_initial_nasal_envelope_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    enum reloq_german_terminal_mark mark,
    int64_t origin_tick, int64_t onset_tick, int64_t closure_tick,
    int64_t release_tick, struct reloq_target_builder *builder)
{
    if (!phones || !builder || !phone_count || origin_tick < 0 ||
        onset_tick <= origin_tick || closure_tick <= onset_tick ||
        release_tick <= closure_tick) return -1;
    if (mark != RELOQ_DE_TERMINAL_PERIOD || phone_count != 3 ||
        (phones[0].phone != RELOQ_DE_PHONE_m &&
         phones[0].phone != RELOQ_DE_PHONE_n) ||
        !is_vowel(phones[1].phone) ||
        (phones[2].phone != RELOQ_DE_PHONE_p &&
         phones[2].phone != RELOQ_DE_PHONE_t &&
         phones[2].phone != RELOQ_DE_PHONE_k)) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index || phones[i].syllable_index ||
            phones[i].syllable_in_word || phones[i].stress != 1 ||
            !phones[i].stress_explicit) return 1;

    const int64_t onset = onset_tick - origin_tick;
    const int64_t span = closure_tick - onset_tick;
    const int64_t release = release_tick - origin_tick;
    if (onset != (phones[0].phone == RELOQ_DE_PHONE_m ? 85 : 70) ||
        span > INT32_MAX / 99 || release_tick > INT64_MAX - 460)
        return -1;
    const int64_t pitch[] = {onset / 10, onset + 35 * span / 100,
                              onset + 70 * span / 100,
                              onset + 99 * span / 100};
    if (pitch[0] >= pitch[1] || pitch[1] >= pitch[2] ||
        pitch[2] >= pitch[3] || pitch[3] >= onset + span) return -1;

    const struct reloq_target_event events[] = {
        {RELOQ_TARGET_F0, pitch[0], 1046},
        {RELOQ_TARGET_F0, pitch[1], 1296},
        {RELOQ_TARGET_F0, pitch[2], 796},
        {RELOQ_TARGET_F0, pitch[3], 743},
        {RELOQ_TARGET_FL, 0, 0},
        {RELOQ_TARGET_FL, release, 0},
        {RELOQ_TARGET_FTP, 0, 250},
        {RELOQ_TARGET_FTP, release, 250},
        {RELOQ_TARGET_FTZ, 0, 250},
        {RELOQ_TARGET_FTZ, release, 250}
    };
    return reloq_target_builder_append(builder, origin_tick, events,
        sizeof events / sizeof events[0]) ? -1 : 0;
}
