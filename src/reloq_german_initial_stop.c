#include "reloq_german_initial_stop.h"

/* One common closure/burst/aspiration gesture with place-specific active
 * noise resonator. The ten following-vowel classes adjust release onset;
 * p and k noise selection also depends on the vowel. These are authored
 * phone model parameters, not per-word recordings or reference code. */
struct following_vowel {
    enum reloq_german_phone phone;
    int64_t p_onset, t_onset, k_onset;
    int8_t t_front;
    int8_t p_back;
    int8_t k_af;
    int8_t k_front;
};

static const struct following_vowel vowels[] = {
    {RELOQ_DE_PHONE_A, 65, 89, 78, 0, 0, 60, 0},
    {RELOQ_DE_PHONE_E, 65, 95, 78, 1, 0, 54, 0},
    {RELOQ_DE_PHONE_I, 65, 95, 78, 1, 0, 64, 1},
    {RELOQ_DE_PHONE_O, 65, 89, 78, 0, 1, 62, 0},
    {RELOQ_DE_PHONE_U, 50, 67, 78, 0, 1, 62, 0},
    {RELOQ_DE_PHONE_a, 65, 89, 78, 0, 0, 60, 0},
    {RELOQ_DE_PHONE_e, 78, 113, 93, 1, 0, 64, 1},
    {RELOQ_DE_PHONE_i, 78, 113, 93, 1, 0, 64, 1},
    {RELOQ_DE_PHONE_o, 78, 107, 93, 0, 1, 62, 0},
    {RELOQ_DE_PHONE_u, 59, 80, 93, 0, 1, 62, 0}
};

int reloq_german_initial_stop_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t stop_origin_tick, struct reloq_target_builder *builder,
    int64_t *vowel_onset_offset)
{
    if (!phones || !builder || !vowel_onset_offset || !phone_count ||
        stop_origin_tick < 0) return -1;
    if (phone_count != 2 && phone_count != 3) return 1;
    enum reloq_german_phone stop = phones[0].phone;
    if (stop != RELOQ_DE_PHONE_p && stop != RELOQ_DE_PHONE_t &&
        stop != RELOQ_DE_PHONE_k) return 1;
    if (phone_count == 3 && phones[2].phone != RELOQ_DE_PHONE_p &&
        phones[2].phone != RELOQ_DE_PHONE_t &&
        phones[2].phone != RELOQ_DE_PHONE_k &&
        phones[2].phone != RELOQ_DE_PHONE_s &&
        phones[2].phone != RELOQ_DE_PHONE_f) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index != 0 || phones[i].syllable_index !=
                phones[1].syllable_index ||
            phones[i].syllable_in_word != 0 || phones[i].stress != 1 ||
            !phones[i].stress_explicit) return 1;

    const struct following_vowel *next = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phones[1].phone == vowels[i].phone) {
            next = &vowels[i];
            break;
        }
    if (!next) return 1;

    int64_t onset, burst_end;
    int32_t af, ah, resonator_gain;
    enum reloq_target_stream resonator;
    if (stop == RELOQ_DE_PHONE_p) {
        onset = next->p_onset;
        burst_end = 15;
        af = 60;
        ah = 49;
        resonator = RELOQ_TARGET_AB;
        resonator_gain = next->p_back ? 66 : 64;
    } else if (stop == RELOQ_DE_PHONE_t) {
        onset = next->t_onset;
        burst_end = next->t_front ? 23 : 17;
        af = 59;
        ah = 47;
        resonator = RELOQ_TARGET_A5F;
        resonator_gain = 56;
    } else {
        onset = next->k_onset;
        burst_end = 17;
        af = next->k_af;
        ah = 45;
        resonator = next->k_front ? RELOQ_TARGET_A3F : RELOQ_TARGET_A2F;
        resonator_gain = next->k_front ? 60 : 70;
    }

    const struct reloq_target_event points[] = {
        {RELOQ_TARGET_AV, 0, 0},
        {RELOQ_TARGET_AV, onset, 0},
        {RELOQ_TARGET_AH, 0, 0},
        {RELOQ_TARGET_AH, burst_end, 0},
        {RELOQ_TARGET_AH, burst_end, ah},
        {RELOQ_TARGET_AH, onset, ah},
        {RELOQ_TARGET_AF, 0, 0},
        {RELOQ_TARGET_AF, 5, 0},
        {RELOQ_TARGET_AF, 5, af},
        {RELOQ_TARGET_AF, burst_end, af},
        {RELOQ_TARGET_AF, onset, 0},
        {resonator, 0, 0},
        {resonator, 5, 0},
        {resonator, 5, resonator_gain},
        {resonator, burst_end, resonator_gain},
        {resonator, onset, 0}
    };
    if (reloq_target_builder_append(builder, stop_origin_tick, points,
                                    sizeof points / sizeof points[0]))
        return -1;
    *vowel_onset_offset = onset;
    return 0;
}
