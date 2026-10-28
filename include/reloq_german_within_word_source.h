#ifndef RELOQ_GERMAN_WITHIN_WORD_SOURCE_H
#define RELOQ_GERMAN_WITHIN_WORD_SOURCE_H

#include "reloq_german_syllable_plan.h"
#include "reloq_german_within_word_landmarks.h"
#include "reloq_target_builder.h"

/* Source and frication gestures for a weak CVC followed by primary tVC in
 * one German word. The first onset may be p/t/k, the first coda p/k; the
 * second vowel is one of ten simple qualities and final stop is p/t/k. The
 * supplied landmarks come from the independent duration model. The module
 * owns AV, AH, AF and A1F..AB; other streams remain untouched. Each source
 * is shaped over the entire word, including both closures and the 60-tick
 * terminal release. No word spelling or measured utterance trace is stored.
 * 0 modeled, 1 outside this measured phone class, -1 invalid arguments or
 * target storage; on 1/-1 the builder is unchanged. */
int reloq_german_within_word_source_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const struct reloq_german_within_word_landmarks *landmarks,
    struct reloq_target_builder *builder);

#endif
