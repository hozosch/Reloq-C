#include "reloq_german_stop_vowel_interaction.h"

#include <limits.h>

static int stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

int reloq_german_stop_vowel_interaction(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index,
    const struct reloq_german_vowel_onset_prediction *additive,
    struct reloq_german_vowel_onset_prediction *result)
{
    if (!phones || !additive || !result || !phone_count ||
        vowel_index >= phone_count ||
        additive->anchor_tick < 0 ||
        additive->first_send_end <= additive->anchor_tick) return -1;
    if (phone_count != 3 || vowel_index != 1 ||
        !stop(phones[0].phone) || !stop(phones[2].phone)) return 1;
    for (size_t i = 0; i < phone_count; ++i) {
        if (!phones[i].stress_explicit || phones[i].stress != 1 ||
            phones[i].word_index != phones[0].word_index ||
            phones[i].syllable_index != phones[0].syllable_index)
            return 1;
    }
    if (!phones[0].same_syllable_right ||
        !phones[1].same_syllable_left ||
        !phones[1].same_syllable_right ||
        !phones[2].same_syllable_left) return 1;

    struct reloq_german_vowel_onset_prediction corrected = *additive;
    int64_t window_delta = 0;
    switch (phones[1].phone) {
    case RELOQ_DE_PHONE_a: window_delta = 4; break;
    case RELOQ_DE_PHONE_e: window_delta = 2; break;
    case RELOQ_DE_PHONE_i: window_delta = 3; break;
    case RELOQ_DE_PHONE_o: window_delta = 2; break;
    case RELOQ_DE_PHONE_u: window_delta = 3; break;
    case RELOQ_DE_PHONE_A:
        if (phones[2].phone == RELOQ_DE_PHONE_t) {
            if (corrected.f2 < INT32_MIN + 140) return -1;
            corrected.f2 -= 140;
        }
        break;
    case RELOQ_DE_PHONE_E: case RELOQ_DE_PHONE_I:
    case RELOQ_DE_PHONE_O: case RELOQ_DE_PHONE_U:
        break;
    default:
        return 1;
    }
    if (corrected.first_send_end > INT64_MAX - window_delta)
        return -1;
    corrected.first_send_end += window_delta;
    *result = corrected;
    return 0;
}
