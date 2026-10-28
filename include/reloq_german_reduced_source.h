#ifndef RELOQ_GERMAN_REDUCED_SOURCE_H
#define RELOQ_GERMAN_REDUCED_SOURCE_H

#include "reloq_german_reduced_landmarks.h"
#include "reloq_target_builder.h"

/* Voicing, aspiration and frication for .1[p/t/k]V[p/k].0(@|@n|t@|t@n).
 * V is one of the ten simple German vowels. This is an acoustic phone-class
 * model driven by the independent reduced-syllable duration planner; it does
 * not contain a spelling list or a reference utterance. It owns F0, AV, AH,
 * AF and A1F..AB for the entire word. The other target streams are untouched.
 * Returns 0 modeled, 1 uncovered, -1 invalid/storage failure, with the
 * builder unchanged on either nonzero result. */
int reloq_german_reduced_source_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_reduced_landmarks *landmarks,
    struct reloq_target_builder *builder);

#endif
