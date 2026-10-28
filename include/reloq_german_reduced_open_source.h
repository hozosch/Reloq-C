#ifndef RELOQ_GERMAN_REDUCED_OPEN_SOURCE_H
#define RELOQ_GERMAN_REDUCED_OPEN_SOURCE_H

#include "reloq_german_reduced_landmarks.h"
#include "reloq_target_builder.h"

/* Source, aspiration, frication and pitch for an open stressed syllable
 * followed by /p,t,k/ + schwa (+ optional /n/):
 *     .1[p/t/k]V.0[p/t/k]@{,n}
 * V ranges over the ten simple German vowel phones. All timing comes from
 * the independently planned landmarks; no spelling or captured PCM is used.
 * Owns F0, AV, AH, AF, A1F..AB. Other streams stay untouched.
 * Returns 0 modeled, 1 uncovered, -1 invalid/storage failure. On failure
 * the target builder remains unchanged. */
int reloq_german_reduced_open_source_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_reduced_landmarks *landmarks,
    struct reloq_target_builder *builder);

#endif
