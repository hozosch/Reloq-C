#include "reloq_german_initial_voiced_stop.h"

/* Per-vowel coarticulation coefficients for one default German voice. The
 * model is phoneme-and-place based; final p/t/k does not change this first
 * gesture's coefficients. The future phrase contour and release timing are
 * planned independently by the complete voice. */
struct vowel_context {
    enum reloq_german_phone phone;
    int16_t b_onset, b_end_f2;
    int16_t d_onset, d_end_f2, d_end_f3, d_af;
    int16_t g_end_f2, g_end_f3, g_af, g_noise;
    enum reloq_target_stream g_stream;
};

static const struct vowel_context vowels[] = {
    {RELOQ_DE_PHONE_A,  81,1000, 100,1600,2600,53, 1600,2300,56,70,RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_E,  93,1300, 100,1600,2600,50, 2000,2300,63,65,RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_I, 106,1500, 100,1700,2700,50, 2150,2400,63,60,RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_O,  93, 700, 100,1600,2600,50, 1420,2000,56,70,RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_U, 106, 700, 100,1400,2600,50, 1300,2000,56,70,RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_a,  81,1000, 100,1600,2600,53, 1600,2300,56,70,RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_e,  93,1500, 105,1700,2700,52, 2250,2450,63,60,RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_i, 106,1500, 105,1800,2700,52, 2250,2450,63,60,RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_o,  93, 600, 100,1200,2600,50, 1300,2000,56,70,RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_u, 106, 600, 100,1200,2600,50,  900,2000,56,70,RELOQ_TARGET_A2F}
};

static int is_coda(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

int reloq_german_initial_voiced_stop_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t origin_tick, struct reloq_target_builder *builder,
    int64_t *vowel_onset_offset, int64_t *burst_offset)
{
    if (!phones || !builder || !vowel_onset_offset || !burst_offset ||
        origin_tick < 0 || !phone_count ||
        vowel_onset_offset == burst_offset) return -1;
    if ((phone_count != 2 && phone_count != 3) ||
        (phones[0].phone != RELOQ_DE_PHONE_b &&
         phones[0].phone != RELOQ_DE_PHONE_d &&
         phones[0].phone != RELOQ_DE_PHONE_g) ||
        (phone_count == 3 && !is_coda(phones[2].phone))) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index || phones[i].syllable_index ||
            phones[i].syllable_in_word || phones[i].stress != 1 ||
            !phones[i].stress_explicit) return 1;
    const struct vowel_context *v = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phones[1].phone == vowels[i].phone) {
            v = &vowels[i];
            break;
        }
    if (!v) return 1;

    const enum reloq_german_phone stop = phones[0].phone;
    const int64_t onset = stop == RELOQ_DE_PHONE_b ? v->b_onset :
        stop == RELOQ_DE_PHONE_d ? v->d_onset : 90;
    const int64_t burst = stop == RELOQ_DE_PHONE_b ? onset - 11 :
        stop == RELOQ_DE_PHONE_d ? 90 : 70;
    const int32_t start_f1 = stop == RELOQ_DE_PHONE_g ? 300 : 200;
    const int32_t start_f2 = stop == RELOQ_DE_PHONE_b ? 1000 :
        stop == RELOQ_DE_PHONE_d ? 1600 : 1500;
    const int32_t start_f3 = stop == RELOQ_DE_PHONE_b ? 2500 :
        stop == RELOQ_DE_PHONE_d ? 2600 : 1890;
    const int32_t end_f2 = stop == RELOQ_DE_PHONE_b ? v->b_end_f2 :
        stop == RELOQ_DE_PHONE_d ? v->d_end_f2 : v->g_end_f2;
    const int32_t end_f3 = stop == RELOQ_DE_PHONE_d ? v->d_end_f3 :
        stop == RELOQ_DE_PHONE_g ? v->g_end_f3 : start_f3;
    const int32_t af = stop == RELOQ_DE_PHONE_b ? 56 :
        stop == RELOQ_DE_PHONE_d ? v->d_af : v->g_af;
    const enum reloq_target_stream noise = stop == RELOQ_DE_PHONE_b ?
        RELOQ_TARGET_AB : stop == RELOQ_DE_PHONE_d ?
        RELOQ_TARGET_A5F : v->g_stream;
    const int32_t gain = stop == RELOQ_DE_PHONE_b ? 64 :
        stop == RELOQ_DE_PHONE_d ? 55 : v->g_noise;
    const struct reloq_target_event events[] = {
        {RELOQ_TARGET_F1, 0, start_f1},
        {RELOQ_TARGET_F1, onset, start_f1},
        {RELOQ_TARGET_F2, 0, start_f2},
        {RELOQ_TARGET_F2, onset, end_f2},
        {RELOQ_TARGET_F3, 0, start_f3},
        {RELOQ_TARGET_F3, onset, end_f3},
        {RELOQ_TARGET_AV, 0, 20},
        {RELOQ_TARGET_AV, burst, 20},
        {RELOQ_TARGET_AV, burst, 0},
        {RELOQ_TARGET_AV, onset, 0},
        {RELOQ_TARGET_TL, 0, 35},
        {RELOQ_TARGET_TL, burst, 35},
        {RELOQ_TARGET_TL, burst, 0},
        {RELOQ_TARGET_TL, onset, 0},
        {RELOQ_TARGET_AH, 0, 0},
        {RELOQ_TARGET_AH, burst, 0},
        {RELOQ_TARGET_AH, burst, 0},
        {RELOQ_TARGET_AH, onset, 0},
        {RELOQ_TARGET_AF, 0, 0},
        {RELOQ_TARGET_AF, burst, 0},
        {RELOQ_TARGET_AF, burst, af},
        {RELOQ_TARGET_AF, onset, af},
        {noise, 0, 0},
        {noise, burst, 0},
        {noise, burst, gain},
        {noise, onset, gain}
    };
    if (reloq_target_builder_append(builder, origin_tick, events,
                                    sizeof events / sizeof events[0]))
        return -1;
    *vowel_onset_offset = onset;
    *burst_offset = burst;
    return 0;
}
