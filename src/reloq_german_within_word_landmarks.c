#include "reloq_german_within_word_landmarks.h"

#include "reloq_german_hv_timing.h"

#include <stddef.h>

/* Weak-vowel nonfinal nucleus: the unstressed open duration gets the same
 * nine-tenths stop-onset modifier as the isolated vowel model. Keeping the
 * vowel coefficients separate from consonant boundary timing lets the same
 * closure rule transfer across all three initial stops and both codas. */
struct weak_vowel {
    enum reloq_german_phone phone;
    int16_t weak_open_ticks;
    int16_t stressed_open_ticks;
    int16_t stressed_coda_shortening;
    int16_t t_onset_ticks;
    int16_t k_coda_bridge_ticks;
};

static const struct weak_vowel weak_vowels[] = {
    {RELOQ_DE_PHONE_A, 75,  97,  0,  89, 58},
    {RELOQ_DE_PHONE_E, 75, 105,  0,  95, 58},
    {RELOQ_DE_PHONE_I, 70,  98,  0,  95, 58},
    {RELOQ_DE_PHONE_O, 70,  98,  0,  89, 58},
    {RELOQ_DE_PHONE_U, 60,  84,  0,  67, 67},
    {RELOQ_DE_PHONE_a, 75, 210, 42,  89, 58},
    {RELOQ_DE_PHONE_e, 90, 208, 26, 113, 51},
    {RELOQ_DE_PHONE_i, 80, 192, 24, 113, 51},
    {RELOQ_DE_PHONE_o, 90, 208, 26, 107, 51},
    {RELOQ_DE_PHONE_u, 80, 192, 24,  80, 61}
};

static int is_stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

int reloq_german_within_word_landmarks_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    struct reloq_german_within_word_landmarks *out)
{
    if (!phones || !syllables || !out) return -1;
    if (phone_count != 6 || syllable_count != 2 ||
        !is_stop(phones[0].phone) ||
        (phones[2].phone != RELOQ_DE_PHONE_p &&
         phones[2].phone != RELOQ_DE_PHONE_k) ||
        phones[3].phone != RELOQ_DE_PHONE_t ||
        !is_stop(phones[5].phone))
        return 1;
    if (syllables[0].first_phone != 0 ||
        syllables[1].first_phone != 3 ||
        syllables[0].phone_count != 3 ||
        syllables[1].phone_count != 3 ||
        syllables[0].onset_count != 1 ||
        syllables[1].onset_count != 1 ||
        syllables[0].nucleus_count != 1 ||
        syllables[1].nucleus_count != 1 ||
        syllables[0].coda_count != 1 ||
        syllables[1].coda_count != 1 ||
        !syllables[0].nucleus_contiguous ||
        !syllables[1].nucleus_contiguous ||
        syllables[0].word_index != 0 ||
        syllables[1].word_index != 0 ||
        syllables[0].syllable_in_word != 0 ||
        syllables[1].syllable_in_word != 1 ||
        syllables[0].boundary_after != RELOQ_DE_BOUNDARY_SYLLABLE ||
        syllables[1].boundary_after != RELOQ_DE_BOUNDARY_UTTERANCE ||
        syllables[0].stress != 0 || syllables[1].stress != 1 ||
        !syllables[0].stress_explicit ||
        !syllables[1].stress_explicit)
        return 1;
    for (size_t i = 0; i < 6; ++i)
        if (phones[i].word_index != 0 ||
            phones[i].syllable_index != i / 3 ||
            phones[i].syllable_in_word != i / 3 ||
            phones[i].stress != i / 3 ||
            !phones[i].stress_explicit) return 1;

    const struct weak_vowel *vowel = NULL, *second_vowel = NULL;
    for (size_t i = 0;
         i < sizeof weak_vowels / sizeof weak_vowels[0]; ++i) {
        if (weak_vowels[i].phone == phones[1].phone)
            vowel = &weak_vowels[i];
        if (weak_vowels[i].phone == phones[4].phone)
            second_vowel = &weak_vowels[i];
    }
    if (!vowel || !second_vowel) return 1;

    int64_t lead;
    if (reloq_german_coda_hold_lead(
            vowel->phone, phones[2].phone, &lead)) return 1;
    int64_t second_lead;
    if (reloq_german_coda_hold_lead(
            second_vowel->phone, phones[5].phone, &second_lead)) return 1;
    /* Isolated final a/k has a 20-tick hold lead; inside this weak first
     * syllable the nonfinal /k/ constriction begins 40 ticks before closure.
     * This is a syllable-boundary rule, shared by p/t/k initial contexts. */
    if (vowel->phone == RELOQ_DE_PHONE_a &&
        phones[2].phone == RELOQ_DE_PHONE_k) lead = 40;
    const int front = vowel->phone == RELOQ_DE_PHONE_E ||
        vowel->phone == RELOQ_DE_PHONE_I ||
        vowel->phone == RELOQ_DE_PHONE_e ||
        vowel->phone == RELOQ_DE_PHONE_i;
    const int k_coda = phones[2].phone == RELOQ_DE_PHONE_k;
    struct reloq_german_within_word_landmarks next = {0};
    next.first_burst_end = phones[0].phone == RELOQ_DE_PHONE_p ? 15 :
        phones[0].phone == RELOQ_DE_PHONE_t && front ? 23 : 17;
    next.first_vowel_onset = phones[0].phone == RELOQ_DE_PHONE_p ? 34 :
        phones[0].phone == RELOQ_DE_PHONE_t ?
        next.first_burst_end + 28 : 67;
    next.first_closure = next.first_vowel_onset +
        vowel->weak_open_ticks * 9 / 10;
    next.first_vowel_hold = next.first_closure - lead;
    next.first_coda_release = next.first_closure + (k_coda ? 60 : 100);
    next.first_coda_burst_start = next.first_coda_release -
        (k_coda ? 8 : 10);
    next.second_onset_origin = next.first_coda_release +
        (k_coda ? 20 : 1);
    next.second_burst_start = next.first_coda_release +
        (k_coda ? 92 : 53);
    const int second_front = second_vowel->phone == RELOQ_DE_PHONE_E ||
        second_vowel->phone == RELOQ_DE_PHONE_I ||
        second_vowel->phone == RELOQ_DE_PHONE_e ||
        second_vowel->phone == RELOQ_DE_PHONE_i;
    next.second_burst_end = next.second_burst_start +
        (second_front ? 18 : 12);
    next.second_vowel_onset = next.first_coda_release +
        second_vowel->t_onset_ticks +
        (k_coda ? second_vowel->k_coda_bridge_ticks : 48);
    next.second_closure = next.second_vowel_onset +
        (second_vowel->stressed_open_ticks -
         second_vowel->stressed_coda_shortening) * 9 / 10;
    next.second_vowel_hold = next.second_closure - second_lead;
    next.second_coda_release = next.second_closure +
        (phones[5].phone == RELOQ_DE_PHONE_p ? 125 :
         phones[5].phone == RELOQ_DE_PHONE_t ? 108 : 122);
    next.planned_ticks = next.second_coda_release + 460;
    if (next.first_burst_end >= next.first_vowel_onset ||
        next.first_vowel_onset >= next.first_vowel_hold ||
        next.first_vowel_hold >= next.first_closure ||
        next.first_closure >= next.first_coda_burst_start ||
        next.first_coda_release >= next.second_onset_origin ||
        next.second_onset_origin >= next.second_burst_start ||
        next.second_burst_start >= next.second_burst_end ||
        next.second_burst_end >= next.second_vowel_onset ||
        next.second_vowel_onset >= next.second_vowel_hold ||
        next.second_vowel_hold >= next.second_closure ||
        next.second_closure >= next.second_coda_release)
        return 1;
    *out = next;
    return 0;
}
