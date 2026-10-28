#ifndef RELOQ_GERMAN_GENERIC_TRACT_H
#define RELOQ_GERMAN_GENERIC_TRACT_H

#include "reloq_german_phone_sequence.h"

#include <stdint.h>

/* A small, independently authored context model for the experimental voice.
 * Values are target controls, not Klatt frame dumps. The rest of the target
 * profile (F4/F5, bandwidths, source and timing) remains with the caller.
 * A phone node supplies structural neighbors; a neighbor across a syllable
 * or word boundary is not treated as direct coarticulation. */
struct reloq_german_generic_tract {
    int32_t f1, f2, f3;
    int32_t b1, b2, fnp, fnz;
    int32_t av, ah;
};

/* Base vowel values are calibrated for the simple German vowels. Less common
 * vowels and nasal vowels are corrected inside the model. Return -1 on an
 * invalid pointer/phone, otherwise zero and a complete tract/source profile. */
int reloq_german_generic_vowel_tract(
    const struct reloq_german_phone_node *node,
    int32_t base_f1, int32_t base_f2, int32_t base_f3, int32_t base_av,
    struct reloq_german_generic_tract *out);

/* The same place-of-articulation model for nasal, liquid and glide gestures.
 * Adjacent vowel class affects consonant formants; onset and coda use
 * different gestures. No individual text or word spelling enters here. */
int reloq_german_generic_sonorant_tract(
    const struct reloq_german_phone_node *node,
    struct reloq_german_generic_tract *out);

#endif
