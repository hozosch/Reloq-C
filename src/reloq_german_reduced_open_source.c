#include "reloq_german_reduced_open_source.h"

#include <limits.h>

struct vowel_source {
    enum reloq_german_phone phone;
    int32_t av, pitch_offset, k_af;
    enum reloq_target_stream k_noise;
    int32_t p_ab;
};

/* The stressed vowel sets source amplitude and the onset's velar burst.
 * The pitch-offset is relative to first_vowel_onset, in 1-ms ticks. */
static const struct vowel_source vowels[] = {
    {RELOQ_DE_PHONE_A, 51, 25, 60, RELOQ_TARGET_A2F, 64},
    {RELOQ_DE_PHONE_E, 52, 28, 54, RELOQ_TARGET_A2F, 64},
    {RELOQ_DE_PHONE_I, 53, 26, 64, RELOQ_TARGET_A3F, 64},
    {RELOQ_DE_PHONE_O, 53, 26, 62, RELOQ_TARGET_A2F, 66},
    {RELOQ_DE_PHONE_U, 54, 22, 62, RELOQ_TARGET_A2F, 66},
    {RELOQ_DE_PHONE_a, 50, 52, 60, RELOQ_TARGET_A2F, 64},
    {RELOQ_DE_PHONE_e, 51, 44, 64, RELOQ_TARGET_A3F, 64},
    {RELOQ_DE_PHONE_i, 53, 41, 64, RELOQ_TARGET_A3F, 64},
    {RELOQ_DE_PHONE_o, 54, 44, 62, RELOQ_TARGET_A2F, 66},
    {RELOQ_DE_PHONE_u, 55, 41, 62, RELOQ_TARGET_A2F, 66}
};

static int stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_reduced_open_source_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_reduced_landmarks *l,
    struct reloq_target_builder *builder)
{
    if (!phones || !syllables || !l || !builder) return -1;
    const int nasal = phone_count == 5;
    if ((phone_count != 4 && !nasal) || syllable_count != 2 ||
        syllables[0].first_phone != 0 || syllables[0].phone_count != 2 ||
        syllables[0].onset_count != 1 ||
        syllables[0].nucleus_count != 1 ||
        syllables[0].coda_count != 0 ||
        syllables[1].first_phone != 2 ||
        syllables[1].phone_count != phone_count - 2 ||
        syllables[1].onset_count != 1 ||
        syllables[1].nucleus_count != 1 ||
        syllables[1].coda_count != (size_t)nasal ||
        syllables[0].word_index != 0 || syllables[1].word_index != 0 ||
        syllables[0].boundary_after != RELOQ_DE_BOUNDARY_SYLLABLE ||
        syllables[1].boundary_after != RELOQ_DE_BOUNDARY_UTTERANCE ||
        syllables[0].stress != 1 || syllables[1].stress != 0 ||
        !syllables[0].stress_explicit ||
        !syllables[1].stress_explicit ||
        !stop(phones[0].phone) || !stop(phones[2].phone) ||
        phones[3].phone != RELOQ_DE_PHONE_AT ||
        (nasal && phones[4].phone != RELOQ_DE_PHONE_n)) return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index != 0 ||
            phones[i].syllable_index != (i >= 2) ||
            phones[i].syllable_in_word != (i >= 2) ||
            phones[i].stress != (i < 2) ||
            !phones[i].stress_explicit) return 1;
    const struct vowel_source *v = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phones[1].phone == vowels[i].phone) {
            v = &vowels[i];
            break;
        }
    if (!v) return 1;

    /* Verify the ordering before constructing an atomic event batch. */
    if (l->first_burst_end <= 5 ||
        l->first_burst_end >= l->first_vowel_onset ||
        l->first_vowel_onset >= l->first_vowel_hold ||
        l->first_vowel_hold >= l->first_closure ||
        l->first_closure != l->second_onset_origin ||
        l->first_coda_burst_start != 0 ||
        l->second_burst_start <= l->first_closure ||
        l->second_burst_end <= l->second_burst_start ||
        l->first_coda_release != l->second_burst_end ||
        l->second_vowel_onset <= l->second_burst_end ||
        l->second_vowel_onset >= INT64_MAX - 60 ||
        l->second_closure <= l->second_vowel_onset + 1 ||
        l->second_closure >= INT64_MAX - 60 ||
        l->second_coda_release != l->second_closure ||
        l->planned_ticks < l->second_closure + (nasal ? 30 : 60) ||
        (nasal && (l->second_vowel_hold <= l->second_vowel_onset ||
                   l->second_vowel_hold >= l->second_nasal_onset ||
                   l->second_nasal_onset >= l->second_nasal_mid ||
                   l->second_nasal_mid >= l->second_closure))) return -1;

    const enum reloq_german_phone initial = phones[0].phone;
    const enum reloq_german_phone middle = phones[2].phone;
    const int32_t first_ah = initial == RELOQ_DE_PHONE_p ? 49 :
        initial == RELOQ_DE_PHONE_t ? 47 : 45;
    const int32_t second_ah = middle == RELOQ_DE_PHONE_p ? 49 :
        middle == RELOQ_DE_PHONE_t ? 47 : 48;
    const int32_t first_af = initial == RELOQ_DE_PHONE_p ? 60 :
        initial == RELOQ_DE_PHONE_t ? 59 : v->k_af;
    const int32_t second_af = middle == RELOQ_DE_PHONE_p ? 60 :
        middle == RELOQ_DE_PHONE_t ? 56 : 64;
    const enum reloq_target_stream first_noise =
        initial == RELOQ_DE_PHONE_p ? RELOQ_TARGET_AB :
        initial == RELOQ_DE_PHONE_t ? RELOQ_TARGET_A5F : v->k_noise;
    const enum reloq_target_stream second_noise =
        middle == RELOQ_DE_PHONE_p ? RELOQ_TARGET_AB :
        middle == RELOQ_DE_PHONE_t ? RELOQ_TARGET_A5F : RELOQ_TARGET_A3F;
    const int32_t first_gain = initial == RELOQ_DE_PHONE_p ? v->p_ab :
        initial == RELOQ_DE_PHONE_t ? 56 :
        first_noise == RELOQ_TARGET_A2F ? 70 : 60;
    const int32_t second_gain = middle == RELOQ_DE_PHONE_p ? 54 :
        middle == RELOQ_DE_PHONE_t ? 56 : 60;

    /* These are interpolated Klatt control gestures, not sound segments.
     * Repeated-tick zero/replacement events follow the renderer's ordering
     * at stop releases and nasal closure. */
    struct reloq_target_event events[240];
    size_t n = 0;
#define PUT(stream, tick, value) put(events, &n, stream, tick, value)
    const int front = v->phone == RELOQ_DE_PHONE_E ||
        v->phone == RELOQ_DE_PHONE_I ||
        v->phone == RELOQ_DE_PHONE_e ||
        v->phone == RELOQ_DE_PHONE_i;
    PUT(RELOQ_TARGET_F0,
        initial == RELOQ_DE_PHONE_t && front ? 2 : 1, 1046);
    PUT(RELOQ_TARGET_F0, l->first_vowel_onset + v->pitch_offset, 1296);
    PUT(RELOQ_TARGET_F0, l->second_closure - (nasal ? 51 : 25), 793);
    PUT(RELOQ_TARGET_F0, l->second_closure - (nasal ? 2 : 1), 740);

    PUT(RELOQ_TARGET_AV, 0, 0);
    PUT(RELOQ_TARGET_AV, 5, 0);
    PUT(RELOQ_TARGET_AV, 5, 0);
    PUT(RELOQ_TARGET_AV, l->first_burst_end, 0);
    PUT(RELOQ_TARGET_AV, l->first_burst_end, 0);
    PUT(RELOQ_TARGET_AV, l->first_vowel_onset, 0);
    PUT(RELOQ_TARGET_AV, l->first_vowel_onset + 1, v->av);
    PUT(RELOQ_TARGET_AV, l->first_closure, v->av - 2);
    PUT(RELOQ_TARGET_AV, l->first_closure, 0);
    PUT(RELOQ_TARGET_AV, l->second_burst_start, 0);
    PUT(RELOQ_TARGET_AV, l->second_burst_start, 0);
    PUT(RELOQ_TARGET_AV, l->second_burst_end, 0);
    PUT(RELOQ_TARGET_AV, l->second_burst_end, 0);
    PUT(RELOQ_TARGET_AV, l->second_vowel_onset, 0);
    PUT(RELOQ_TARGET_AV, l->second_vowel_onset + 1, nasal ? 47 : 50);
    if (nasal) {
        PUT(RELOQ_TARGET_AV, l->second_nasal_onset, 45);
        PUT(RELOQ_TARGET_AV, l->second_nasal_mid, 40);
        PUT(RELOQ_TARGET_AV, l->second_closure, 38);
        PUT(RELOQ_TARGET_AV, l->second_closure + 15, 38);
        PUT(RELOQ_TARGET_AV, l->second_closure + 15, 0);
        PUT(RELOQ_TARGET_AV, l->second_closure + 30, 0);
        PUT(RELOQ_TARGET_AV, l->second_closure + 30, 0);
    } else {
        PUT(RELOQ_TARGET_AV, l->second_closure, 48);
        PUT(RELOQ_TARGET_AV, l->second_closure + 60, 0);
    }
    PUT(RELOQ_TARGET_AV, l->planned_ticks, 0);

    PUT(RELOQ_TARGET_AH, 0, 0);
    PUT(RELOQ_TARGET_AH, 5, 0);
    PUT(RELOQ_TARGET_AH, 5, 0);
    PUT(RELOQ_TARGET_AH, l->first_burst_end, 0);
    PUT(RELOQ_TARGET_AH, l->first_burst_end, first_ah);
    PUT(RELOQ_TARGET_AH, l->first_vowel_onset, first_ah);
    PUT(RELOQ_TARGET_AH, l->first_vowel_onset + 1, 34);
    PUT(RELOQ_TARGET_AH, l->first_closure, 34);
    PUT(RELOQ_TARGET_AH, l->first_closure, 0);
    PUT(RELOQ_TARGET_AH, l->second_burst_start, 0);
    PUT(RELOQ_TARGET_AH, l->second_burst_start, 0);
    PUT(RELOQ_TARGET_AH, l->second_burst_end, 0);
    PUT(RELOQ_TARGET_AH, l->second_burst_end, second_ah);
    PUT(RELOQ_TARGET_AH, l->second_vowel_onset, second_ah);
    PUT(RELOQ_TARGET_AH, l->second_vowel_onset + 1, 34);
    if (nasal) {
        PUT(RELOQ_TARGET_AH, l->second_nasal_onset, 34);
        PUT(RELOQ_TARGET_AH, l->second_nasal_onset, 0);
        PUT(RELOQ_TARGET_AH, l->second_closure, 0);
        PUT(RELOQ_TARGET_AH, l->second_closure, 36);
        PUT(RELOQ_TARGET_AH, l->second_closure + 30, 36);
        PUT(RELOQ_TARGET_AH, l->second_closure + 30, 0);
    } else {
        PUT(RELOQ_TARGET_AH, l->second_closure, 34);
        PUT(RELOQ_TARGET_AH, l->second_closure, 35);
        PUT(RELOQ_TARGET_AH, l->second_closure + 60, 35);
        PUT(RELOQ_TARGET_AH, l->second_closure + 60, 0);
    }
    PUT(RELOQ_TARGET_AH, l->planned_ticks, 0);

    PUT(RELOQ_TARGET_AF, 0, 0);
    PUT(RELOQ_TARGET_AF, 5, 0);
    PUT(RELOQ_TARGET_AF, 5, first_af);
    PUT(RELOQ_TARGET_AF, l->first_burst_end, first_af);
    PUT(RELOQ_TARGET_AF, l->first_vowel_onset, 0);
    PUT(RELOQ_TARGET_AF, l->first_vowel_hold, 0);
    PUT(RELOQ_TARGET_AF, l->first_closure, 0);
    PUT(RELOQ_TARGET_AF, l->second_burst_start, 0);
    PUT(RELOQ_TARGET_AF, l->second_burst_start, second_af);
    PUT(RELOQ_TARGET_AF, l->second_burst_end, second_af);
    PUT(RELOQ_TARGET_AF, l->second_vowel_onset, 0);
    if (nasal) {
        PUT(RELOQ_TARGET_AF, l->second_vowel_hold, 0);
        PUT(RELOQ_TARGET_AF, l->second_nasal_onset, 0);
        PUT(RELOQ_TARGET_AF, l->second_closure, 0);
        PUT(RELOQ_TARGET_AF, l->second_closure + 30, 0);
    } else {
        PUT(RELOQ_TARGET_AF, l->second_closure, 0);
        PUT(RELOQ_TARGET_AF, l->second_closure + 60, 0);
    }
    PUT(RELOQ_TARGET_AF, l->planned_ticks, 0);

    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s) {
        PUT(s, 0, 0);
        PUT(s, 5, 0);
        PUT(s, 5, s == first_noise ? first_gain : 0);
        PUT(s, l->first_burst_end, s == first_noise ? first_gain : 0);
        PUT(s, l->first_vowel_onset, 0);
        PUT(s, l->first_vowel_hold, 0);
        PUT(s, l->first_closure, 0);
        PUT(s, l->second_burst_start, 0);
        PUT(s, l->second_burst_start, s == second_noise ? second_gain : 0);
        PUT(s, l->second_burst_end, s == second_noise ? second_gain : 0);
        PUT(s, l->second_vowel_onset, 0);
        if (nasal) {
            PUT(s, l->second_vowel_hold, 0);
            PUT(s, l->second_nasal_onset, 0);
            PUT(s, l->second_closure, 0);
            PUT(s, l->second_closure + 30, 0);
        } else {
            PUT(s, l->second_closure, 0);
            PUT(s, l->second_closure + 60, 0);
        }
        PUT(s, l->planned_ticks, 0);
    }
#undef PUT
    if (n > sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder, 0, events, n)) return -1;
    return 0;
}
