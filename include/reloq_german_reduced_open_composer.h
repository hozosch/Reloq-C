#ifndef RELOQ_GERMAN_REDUCED_OPEN_COMPOSER_H
#define RELOQ_GERMAN_REDUCED_OPEN_COMPOSER_H

#include "reloq_german_reduced_landmarks.h"
#include "reloq_target_builder.h"

/* Compose a stressed open first syllable and weak stop+schwa (+ optional n):
 * .1[p/t/k]V.0[p/t/k]@{,n}, with ten German simple vowels.
 * The caller independently validates the terminal period and text frontend.
 * Source, first tract, reduced tract and timing are phone-class C rules.
 * A successful call appends all 28 controls; on any failure neither output
 * builder nor *landmarks changes. 0 success, 1 uncovered shape, -1 error. */
int reloq_german_reduced_open_compose(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    struct reloq_target_builder *builder,
    struct reloq_german_reduced_landmarks *landmarks);

#endif
