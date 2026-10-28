#include "reloq_german_reduced_landmarks.h"

#include "reloq_german_hv_timing.h"

#include <stddef.h>

struct primary_vowel {
    enum reloq_german_phone phone;
    int16_t p_anchor, t_anchor, k_anchor, nonfinal_ticks;
};

/* The first syllable retains its primary-stress initial-stop timing; its
 * nonfinal vowel is shortened by the following reduced syllable. The common
 * coda hold lead is shared with other German stop-vowel contexts. */
static const struct primary_vowel vowels[] = {
    {RELOQ_DE_PHONE_A, 65,  89, 78,  74},
    {RELOQ_DE_PHONE_E, 65,  95, 78,  81},
    {RELOQ_DE_PHONE_I, 65,  95, 78,  75},
    {RELOQ_DE_PHONE_O, 65,  89, 78,  75},
    {RELOQ_DE_PHONE_U, 50,  67, 78,  64},
    {RELOQ_DE_PHONE_a, 65,  89, 78, 151},
    {RELOQ_DE_PHONE_e, 78, 113, 93, 128},
    {RELOQ_DE_PHONE_i, 78, 113, 93, 118},
    {RELOQ_DE_PHONE_o, 78, 107, 93, 128},
    {RELOQ_DE_PHONE_u, 59,  80, 93, 118}
};

static int first_stop(enum reloq_german_phone phone)
{
    return phone == RELOQ_DE_PHONE_p || phone == RELOQ_DE_PHONE_t ||
           phone == RELOQ_DE_PHONE_k;
}

int reloq_german_reduced_landmarks_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    struct reloq_german_reduced_landmarks *out)
{
    if (!phones || !syllables || !out) return -1;
    if (syllable_count != 2 || phone_count < 4 || phone_count > 6 ||
        !first_stop(phones[0].phone)) return 1;
    const size_t second = syllables[1].first_phone;
    const int first_open = second == 2;
    if ((!first_open && second != 3) ||
        syllables[0].first_phone != 0 ||
        syllables[0].phone_count != second ||
        syllables[1].phone_count != phone_count - second ||
        syllables[0].onset_count != 1 ||
        syllables[0].nucleus_count != 1 ||
        syllables[0].coda_count != (size_t)!first_open ||
        !syllables[0].nucleus_contiguous ||
        !syllables[1].nucleus_contiguous ||
        syllables[0].word_index != 0 ||
        syllables[1].word_index != 0 ||
        syllables[0].syllable_in_word != 0 ||
        syllables[1].syllable_in_word != 1 ||
        syllables[0].boundary_after != RELOQ_DE_BOUNDARY_SYLLABLE ||
        syllables[1].boundary_after != RELOQ_DE_BOUNDARY_UTTERANCE ||
        syllables[0].stress != 1 || syllables[1].stress != 0 ||
        !syllables[0].stress_explicit ||
        !syllables[1].stress_explicit) return 1;
    const int has_onset = syllables[1].onset_count == 1;
    const int has_n = syllables[1].coda_count == 1;
    const enum reloq_german_phone second_stop = has_onset ?
        phones[second].phone : RELOQ_DE_PHONE_NONE;
    if ((syllables[1].onset_count != 0 && !has_onset) ||
        syllables[1].nucleus_count != 1 ||
        (syllables[1].coda_count != 0 && !has_n) ||
        syllables[1].phone_count != (size_t)(1 + has_onset + has_n) ||
        (first_open ? !first_stop(second_stop) :
         ((phones[2].phone != RELOQ_DE_PHONE_p &&
           phones[2].phone != RELOQ_DE_PHONE_k) ||
          (has_onset && second_stop != RELOQ_DE_PHONE_t))) ||
        phones[second + (size_t)has_onset].phone != RELOQ_DE_PHONE_AT ||
        (has_n && phones[phone_count - 1].phone != RELOQ_DE_PHONE_n))
        return 1;
    for (size_t i = 0; i < phone_count; ++i)
        if (phones[i].word_index != 0 ||
            phones[i].syllable_index != (i >= second) ||
            phones[i].syllable_in_word != (i >= second) ||
            phones[i].stress != (i < second) ||
            !phones[i].stress_explicit) return 1;

    const struct primary_vowel *v = NULL;
    for (size_t i = 0; i < sizeof vowels / sizeof vowels[0]; ++i)
        if (phones[1].phone == vowels[i].phone) {
            v = &vowels[i];
            break;
        }
    if (!v) return 1;
    int64_t lead;
    const enum reloq_german_phone boundary = first_open ?
        second_stop : phones[2].phone;
    if (reloq_german_coda_hold_lead(v->phone, boundary, &lead))
        return 1;
    const int k_coda = !first_open && boundary == RELOQ_DE_PHONE_k;
    const int front = v->phone == RELOQ_DE_PHONE_E ||
        v->phone == RELOQ_DE_PHONE_I ||
        v->phone == RELOQ_DE_PHONE_e ||
        v->phone == RELOQ_DE_PHONE_i;
    /* An intervocalic /k/ starts nearer the open first vowel's closure
     * than a syllable-final /k/; front vowels retain their 25-tick lead. */
    if (first_open && boundary == RELOQ_DE_PHONE_k && !front)
        lead = 20;
    struct reloq_german_reduced_landmarks next = {0};
    next.first_burst_end = phones[0].phone == RELOQ_DE_PHONE_p ? 15 :
        phones[0].phone == RELOQ_DE_PHONE_t && front ? 23 : 17;
    next.first_vowel_onset = phones[0].phone == RELOQ_DE_PHONE_p ?
        v->p_anchor : phones[0].phone == RELOQ_DE_PHONE_t ?
        v->t_anchor : v->k_anchor;
    next.first_closure = next.first_vowel_onset + v->nonfinal_ticks;
    next.first_vowel_hold = next.first_closure - lead;
    if (first_open) {
        const int long_vowel = v->phone == RELOQ_DE_PHONE_a ||
            v->phone == RELOQ_DE_PHONE_e ||
            v->phone == RELOQ_DE_PHONE_i ||
            v->phone == RELOQ_DE_PHONE_o ||
            v->phone == RELOQ_DE_PHONE_u;
        next.first_coda_release = next.first_closure +
            (second_stop == RELOQ_DE_PHONE_p ?
             100 - (long_vowel ? 15 : 0) :
             second_stop == RELOQ_DE_PHONE_t ? has_n ? 50 : 46 : 82);
        next.second_onset_origin = next.first_closure;
        next.second_burst_end = next.first_coda_release;
        next.second_burst_start = next.second_burst_end -
            (second_stop == RELOQ_DE_PHONE_p ? 10 : 12);
        next.second_vowel_onset = next.second_burst_end +
            (second_stop == RELOQ_DE_PHONE_p ? 19 :
             second_stop == RELOQ_DE_PHONE_t ? 28 : 50);
        next.second_schwa_target = next.second_vowel_onset;
        next.second_noise_fade = next.second_vowel_onset;
        next.second_vowel_hold = next.second_vowel_onset +
            (has_n ? 39 : 81);
        next.second_closure = next.second_vowel_onset +
            (has_n ? 169 : 81);
        if (has_n) next.second_nasal_onset =
            next.second_vowel_onset + 59;
    } else if (has_onset) {
        next.first_coda_release = next.first_closure +
            (k_coda ? 60 : 100);
        next.first_coda_burst_start = next.first_coda_release -
            (k_coda ? 8 : 10);
        next.second_onset_origin = next.first_coda_release +
            (k_coda ? 20 : 1);
        next.second_burst_start = next.first_coda_release +
            (k_coda ? 92 : 53);
        next.second_burst_end = next.second_burst_start + 12;
        next.second_vowel_onset = next.second_burst_end + 28;
        next.second_schwa_target = next.second_vowel_onset;
        next.second_noise_fade = next.second_vowel_onset;
        next.second_vowel_hold = next.second_vowel_onset +
            (has_n ? 39 : 81);
        next.second_closure = next.second_vowel_onset +
            (has_n ? 169 : 81);
        if (has_n) next.second_nasal_onset =
            next.second_vowel_onset + 59;
    } else {
        next.first_coda_release = next.first_closure +
            (k_coda ? 70 : 100);
        next.first_coda_burst_start = next.first_coda_release -
            (k_coda ? 12 : 10);
        next.second_vowel_onset = next.first_coda_release;
        next.second_schwa_target = next.second_vowel_onset +
            (has_n ? k_coda ? 48 : 45 : k_coda ? 61 : 50);
        next.second_noise_fade = next.second_schwa_target;
        next.second_vowel_hold = has_n ?
            next.second_schwa_target + 3 :
            next.second_vowel_onset + 90;
        next.second_closure = next.second_vowel_onset +
            (has_n ? 176 : 90);
        if (has_n) next.second_nasal_onset =
            next.second_vowel_onset + 66;
    }
    if (has_n) next.second_nasal_mid = next.second_closure - 22;
    next.second_coda_release = next.second_closure;
    next.planned_ticks = next.second_closure + (has_n ? 430 : 460);
    if (next.first_burst_end >= next.first_vowel_onset ||
        next.first_vowel_onset >= next.first_vowel_hold ||
        next.first_vowel_hold >= next.first_closure ||
        (!first_open && next.first_closure >=
                        next.first_coda_burst_start) ||
        next.first_coda_release > next.second_vowel_onset ||
        (has_onset && (next.second_onset_origin >=
                    next.second_burst_start ||
                   next.second_burst_start >= next.second_burst_end ||
                   next.second_burst_end >= next.second_vowel_onset)) ||
        next.second_vowel_onset > next.second_schwa_target ||
        next.second_schwa_target > next.second_vowel_hold ||
        next.second_vowel_hold > next.second_closure ||
        (has_n && (next.second_vowel_hold >=
                   next.second_nasal_onset ||
                   next.second_nasal_onset >= next.second_nasal_mid ||
                   next.second_nasal_mid >= next.second_closure)))
        return 1;
    *out = next;
    return 0;
}
