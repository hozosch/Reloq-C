#ifndef RELOQ_GERMAN_REDUCED_CLOSED_FIRST_TRACT_H
#define RELOQ_GERMAN_REDUCED_CLOSED_FIRST_TRACT_H

#include "reloq_german_reduced_landmarks.h"
#include "reloq_target_builder.h"

/* First, primary-stressed CVC tract preceding a reduced second syllable:
 * .1[p/t/k]V[p/k].0(@|@n|t@|t@n). The phoneme-context source of the first
 * four formants is the reusable German stop/vowel model; coda constriction
 * depends on the first vowel and the coda place. If the follower begins
 * with /t/, this stage includes first_coda_release; otherwise its own
 * targets end before that release, which the onsetless second stage owns.
 * Source controls F0/AV/AH/AF/A1F..AB belong to another module.
 * Returns 0 modeled, 1 uncovered, -1 invalid/storage. No change on error. */
int reloq_german_reduced_closed_first_tract_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_reduced_landmarks *landmarks,
    struct reloq_target_builder *builder);

#endif
