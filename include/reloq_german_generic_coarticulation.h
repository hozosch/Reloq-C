#ifndef RELOQ_GERMAN_GENERIC_COARTICULATION_H
#define RELOQ_GERMAN_GENERIC_COARTICULATION_H

#include "reloq_german_phone_sequence.h"

#include <stdint.h>

/* Phone-class tract gestures around a syllable's vowel. Source excitation
 * stays with the source/noise planner: these three values are only F1–F3.
 * Both functions return 1 for an applicable adjacent pair, 0 otherwise,
 * and -1 for invalid pointers. No word spelling is inspected. */
struct reloq_german_generic_cv_gesture {
    int32_t first_formant[3];
    int32_t release_formant[3];
    int32_t vowel_glide_ticks;
    int replace_consonant_tract;
    int anticipate_during_consonant;
};

int reloq_german_generic_cv_gesture(
    const struct reloq_german_phone_node *consonant,
    const int32_t vowel_formant[3],
    struct reloq_german_generic_cv_gesture *out);

struct reloq_german_generic_vc_gesture {
    int32_t coda_formant[3];
    int32_t anticipation_ticks;
};

int reloq_german_generic_vc_gesture(
    const struct reloq_german_phone_node *vowel,
    const struct reloq_german_phone_node *consonant,
    const int32_t vowel_formant[3],
    const int32_t consonant_formant[3],
    struct reloq_german_generic_vc_gesture *out);

/* A place-colored long a/e may return to its uncolored F2 centre before
 * entering a following coda. This point changes F2 only; F1, the source,
 * vowel duration and the coda tract retain their own gestures. The lead
 * is measured backwards from the coda boundary, or zero at a free vowel
 * ending. A result of zero leaves all outputs unchanged. */
struct reloq_german_generic_vowel_center {
    int32_t neutral_f2;
    int32_t before_coda_ticks;
};
int reloq_german_generic_vowel_center(
    const struct reloq_german_phone_node *node, int32_t onset_f2,
    struct reloq_german_generic_vowel_center *out);

#endif
