#ifndef RELOQ_GERMAN_REDUCED_OPEN_FIRST_TRACT_H
#define RELOQ_GERMAN_REDUCED_OPEN_FIRST_TRACT_H

#include "reloq_german_reduced_landmarks.h"
#include "reloq_target_builder.h"

/* Primary-stressed open CV tract before an intervocalic /p,t,k/ and weak
 * schwa, with or without final /n/. This writes only the first syllable's
 * F1..F5, B1..B5, FNP/FNZ, FTP/FTZ, FL, TL, OQ and DI through the first
 * vowel hold. The reduced-tract module owns the closure and all later
 * targets on those same streams. F0 and noise/voice sources are separate.
 * Return 0 appended, 1 uncovered shape, -1 invalid arguments/storage;
 * all nonzero returns leave the caller's builder unchanged. */
int reloq_german_reduced_open_first_tract_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_reduced_landmarks *landmarks,
    struct reloq_target_builder *builder);

#endif
