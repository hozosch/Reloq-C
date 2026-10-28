#include "reloq_german_within_word_source.h"

#include <limits.h>

struct weak_vowel_source {
    enum reloq_german_phone phone;
    int32_t weak_av, primary_av;
    int32_t k_initial_af;
    enum reloq_target_stream k_initial_noise;
    int32_t p_initial_ab;
    int32_t p_final_ab, k_final_ah;
    enum reloq_target_stream k_final_noise;
};

/* Relative to the stressed vowel, weak first-syllable /a/ and /e/ have a
 * distinct source level. The same place and vowel classes control the
 * first stop burst independently of word spelling. */
static const struct weak_vowel_source vowels[] = {
    {RELOQ_DE_PHONE_A, 51, 51, 60, RELOQ_TARGET_A2F, 64,
        64, 48, RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_E, 52, 52, 54, RELOQ_TARGET_A2F, 64,
        60, 48, RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_I, 53, 53, 64, RELOQ_TARGET_A3F, 64,
        60, 53, RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_O, 53, 53, 62, RELOQ_TARGET_A2F, 66,
        64, 48, RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_U, 54, 54, 62, RELOQ_TARGET_A2F, 66,
        64, 48, RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_a, 51, 50, 60, RELOQ_TARGET_A2F, 64,
        64, 48, RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_e, 51, 51, 64, RELOQ_TARGET_A3F, 64,
        60, 51, RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_i, 53, 53, 64, RELOQ_TARGET_A3F, 64,
        60, 51, RELOQ_TARGET_A3F},
    {RELOQ_DE_PHONE_o, 54, 54, 62, RELOQ_TARGET_A2F, 66,
        64, 48, RELOQ_TARGET_A2F},
    {RELOQ_DE_PHONE_u, 55, 55, 62, RELOQ_TARGET_A2F, 66,
        64, 48, RELOQ_TARGET_A2F}
};

static void put(struct reloq_target_event *events, size_t *count,
                enum reloq_target_stream stream, int64_t tick, int32_t value)
{
    events[(*count)++] = (struct reloq_target_event){stream, tick, value};
}

int reloq_german_within_word_source_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_within_word_landmarks *landmarks,
    struct reloq_target_builder *builder)
{
    if (!phones || !syllables || !landmarks || !builder) return -1;
    if (phone_count != 6 || syllable_count != 2 ||
        syllables[0].first_phone != 0 || syllables[1].first_phone != 3 ||
        syllables[0].phone_count != 3 || syllables[1].phone_count != 3 ||
        syllables[0].word_index != syllables[1].word_index ||
        syllables[0].boundary_after != RELOQ_DE_BOUNDARY_SYLLABLE ||
        syllables[1].boundary_after != RELOQ_DE_BOUNDARY_UTTERANCE ||
        syllables[0].stress != 0 || syllables[1].stress != 1 ||
        !syllables[0].stress_explicit ||
        !syllables[1].stress_explicit ||
        syllables[0].onset_count != 1 ||
        syllables[0].nucleus_count != 1 ||
        syllables[0].coda_count != 1 ||
        syllables[1].onset_count != 1 ||
        syllables[1].nucleus_count != 1 ||
        syllables[1].coda_count != 1 ||
        (phones[0].phone != RELOQ_DE_PHONE_p &&
         phones[0].phone != RELOQ_DE_PHONE_t &&
         phones[0].phone != RELOQ_DE_PHONE_k) ||
        (phones[2].phone != RELOQ_DE_PHONE_p &&
         phones[2].phone != RELOQ_DE_PHONE_k) ||
        phones[3].phone != RELOQ_DE_PHONE_t ||
        (phones[5].phone != RELOQ_DE_PHONE_p &&
         phones[5].phone != RELOQ_DE_PHONE_t &&
         phones[5].phone != RELOQ_DE_PHONE_k)) return 1;
    const struct weak_vowel_source *vowel = NULL, *second = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phones[1].phone == vowels[i].phone) {
            vowel = &vowels[i];
        }
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phones[4].phone == vowels[i].phone)
            second = &vowels[i];
    if (!vowel || !second) return 1;

    const struct reloq_german_within_word_landmarks *l = landmarks;
    if (l->first_burst_end < 5 ||
        l->first_vowel_onset <= l->first_burst_end ||
        l->first_vowel_onset >= INT64_MAX ||
        l->first_closure <= l->first_vowel_onset + 1 ||
        l->first_coda_burst_start <= l->first_closure ||
        l->first_coda_release <= l->first_coda_burst_start ||
        l->second_burst_start <= l->first_coda_release ||
        l->second_burst_end <= l->second_burst_start ||
        l->second_vowel_onset <= l->second_burst_end ||
        l->second_vowel_onset >= INT64_MAX ||
        l->second_closure <= l->second_vowel_onset + 1 ||
        l->second_coda_release <= l->second_closure ||
        l->second_coda_release > INT64_MAX - 60 ||
        l->planned_ticks < l->second_coda_release + 60 ||
        (phones[2].phone == RELOQ_DE_PHONE_k &&
         l->first_coda_release > INT64_MAX - 20)) return -1;

    const enum reloq_german_phone initial = phones[0].phone;
    const int first_p = phones[2].phone == RELOQ_DE_PHONE_p;
    const enum reloq_german_phone final = phones[5].phone;
    const int64_t final_preburst =
        final == RELOQ_DE_PHONE_p ? 10 :
        final == RELOQ_DE_PHONE_t ? 8 : 12;
    if (l->second_coda_release - l->second_closure <= final_preburst)
        return -1;
    const enum reloq_target_stream final_noise =
        final == RELOQ_DE_PHONE_p ? RELOQ_TARGET_AB :
        final == RELOQ_DE_PHONE_t ? RELOQ_TARGET_A5F :
        second->k_final_noise;
    const int32_t final_noise_gain = final == RELOQ_DE_PHONE_p ?
        second->p_final_ab : 60;
    const int32_t final_ah = final == RELOQ_DE_PHONE_p ? 35 :
        final == RELOQ_DE_PHONE_t ? 48 : second->k_final_ah;
    const int32_t final_af = final == RELOQ_DE_PHONE_p ? 60 :
        final == RELOQ_DE_PHONE_t ? 48 : 62;
    const int32_t initial_ah = initial == RELOQ_DE_PHONE_p ? 49 :
                               initial == RELOQ_DE_PHONE_t ? 47 : 48;
    const int32_t initial_af = initial == RELOQ_DE_PHONE_p ? 60 :
                               initial == RELOQ_DE_PHONE_t ? 59 :
                               vowel->k_initial_af;
    const enum reloq_target_stream initial_noise =
        initial == RELOQ_DE_PHONE_p ? RELOQ_TARGET_AB :
        initial == RELOQ_DE_PHONE_t ? RELOQ_TARGET_A5F :
        vowel->k_initial_noise;
    const int32_t initial_noise_gain =
        initial == RELOQ_DE_PHONE_p ? vowel->p_initial_ab :
        initial == RELOQ_DE_PHONE_t ? 56 :
        initial_noise == RELOQ_TARGET_A3F ? 60 : 70;
    const enum reloq_target_stream first_coda_noise =
        first_p ? RELOQ_TARGET_AB : RELOQ_TARGET_A2F;
    const int64_t first_noise_end = l->first_coda_release +
                                    (first_p ? 1 : 20);
    if (first_noise_end >= l->second_burst_start) return -1;

    /* At most 114 events: one word-level atomic append preserves the same
     * tick replacement order around closures and stop releases. */
    struct reloq_target_event events[160];
    size_t n = 0;
#define PUT(stream, tick, value) put(events, &n, stream, tick, value)
    PUT(RELOQ_TARGET_AV, 0, 0);
    PUT(RELOQ_TARGET_AV, l->first_vowel_onset, 0);
    PUT(RELOQ_TARGET_AV, l->first_vowel_onset + 1, vowel->weak_av);
    PUT(RELOQ_TARGET_AV, l->first_closure, vowel->weak_av - 2);
    PUT(RELOQ_TARGET_AV, l->first_closure, 0);
    PUT(RELOQ_TARGET_AV, l->second_vowel_onset, 0);
    PUT(RELOQ_TARGET_AV, l->second_vowel_onset + 1, second->primary_av);
    PUT(RELOQ_TARGET_AV, l->second_closure, second->primary_av - 2);
    PUT(RELOQ_TARGET_AV, l->second_closure, 0);
    PUT(RELOQ_TARGET_AV, l->planned_ticks, 0);

    PUT(RELOQ_TARGET_AH, 0, 0);
    PUT(RELOQ_TARGET_AH, l->first_burst_end, 0);
    PUT(RELOQ_TARGET_AH, l->first_burst_end, initial_ah);
    PUT(RELOQ_TARGET_AH, l->first_vowel_onset, initial_ah);
    PUT(RELOQ_TARGET_AH, l->first_vowel_onset + 1, 34);
    PUT(RELOQ_TARGET_AH, l->first_closure, 34);
    PUT(RELOQ_TARGET_AH, l->first_closure, 0);
    if (!first_p) {
        PUT(RELOQ_TARGET_AH, l->first_coda_release, 0);
        PUT(RELOQ_TARGET_AH, l->first_coda_release, 50);
        PUT(RELOQ_TARGET_AH, first_noise_end, 50);
        PUT(RELOQ_TARGET_AH, first_noise_end, 0);
    }
    PUT(RELOQ_TARGET_AH, l->second_burst_end, 0);
    PUT(RELOQ_TARGET_AH, l->second_burst_end, 47);
    PUT(RELOQ_TARGET_AH, l->second_vowel_onset, 47);
    PUT(RELOQ_TARGET_AH, l->second_vowel_onset + 1, 34);
    PUT(RELOQ_TARGET_AH, l->second_closure, 34);
    PUT(RELOQ_TARGET_AH, l->second_closure, 0);
    PUT(RELOQ_TARGET_AH, l->second_coda_release, 0);
    PUT(RELOQ_TARGET_AH, l->second_coda_release, final_ah);
    PUT(RELOQ_TARGET_AH, l->second_coda_release + 60, final_ah);
    PUT(RELOQ_TARGET_AH, l->second_coda_release + 60, 0);
    PUT(RELOQ_TARGET_AH, l->planned_ticks, 0);

    PUT(RELOQ_TARGET_AF, 0, 0);
    PUT(RELOQ_TARGET_AF, 5, 0);
    PUT(RELOQ_TARGET_AF, 5, initial_af);
    PUT(RELOQ_TARGET_AF, l->first_burst_end, initial_af);
    PUT(RELOQ_TARGET_AF, l->first_vowel_onset, 0);
    PUT(RELOQ_TARGET_AF, l->first_coda_burst_start, 0);
    PUT(RELOQ_TARGET_AF, l->first_coda_burst_start, first_p ? 60 : 54);
    PUT(RELOQ_TARGET_AF, l->first_coda_release, first_p ? 60 : 54);
    PUT(RELOQ_TARGET_AF, first_noise_end, 0);
    PUT(RELOQ_TARGET_AF, l->second_burst_start, 0);
    PUT(RELOQ_TARGET_AF, l->second_burst_start, 59);
    PUT(RELOQ_TARGET_AF, l->second_burst_end, 59);
    PUT(RELOQ_TARGET_AF, l->second_vowel_onset, 0);
    PUT(RELOQ_TARGET_AF, l->second_coda_release - final_preburst, 0);
    PUT(RELOQ_TARGET_AF, l->second_coda_release - final_preburst, final_af);
    PUT(RELOQ_TARGET_AF, l->second_coda_release, final_af);
    PUT(RELOQ_TARGET_AF, l->second_coda_release + 60, 0);
    PUT(RELOQ_TARGET_AF, l->planned_ticks, 0);

    for (enum reloq_target_stream s = RELOQ_TARGET_A1F;
         s <= RELOQ_TARGET_AB; ++s) {
        PUT(s, 0, 0);
        if (s == initial_noise) {
            PUT(s, 5, 0);
            PUT(s, 5, initial_noise_gain);
            PUT(s, l->first_burst_end, initial_noise_gain);
        }
        PUT(s, l->first_vowel_onset, 0);
        if (s == first_coda_noise) {
            PUT(s, l->first_coda_burst_start, 0);
            PUT(s, l->first_coda_burst_start, 60);
            PUT(s, l->first_coda_release, 60);
        }
        PUT(s, first_noise_end, 0);
        if (s == RELOQ_TARGET_A5F) {
            PUT(s, l->second_burst_start, 0);
            PUT(s, l->second_burst_start, 56);
            PUT(s, l->second_burst_end, 56);
        }
        PUT(s, l->second_vowel_onset, 0);
        if (s == final_noise) {
            PUT(s, l->second_coda_release - final_preburst, 0);
            PUT(s, l->second_coda_release - final_preburst,
                final_noise_gain);
            PUT(s, l->second_coda_release, final_noise_gain);
        }
        PUT(s, l->second_coda_release + 60, 0);
        PUT(s, l->planned_ticks, 0);
    }
#undef PUT
    if (n > sizeof events / sizeof events[0] ||
        reloq_target_builder_append(builder, 0, events, n)) return -1;
    return 0;
}
