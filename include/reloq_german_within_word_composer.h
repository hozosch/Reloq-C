#ifndef RELOQ_GERMAN_WITHIN_WORD_COMPOSER_H
#define RELOQ_GERMAN_WITHIN_WORD_COMPOSER_H

#include "reloq_german_within_word_landmarks.h"
#include "reloq_target_builder.h"

/* Compose one weak CVC followed by a primary tVC inside the same German
 * phonetic word. This is a phoneme-class acoustic rule: it needs no recorded
 * word contour or reference runtime. Supported first onsets p/t/k, ten
 * simple vowels and p/k codas; second syllable currently tIp.
 *
 * All targets use the caller's global 1-ms timeline. `builder` must already
 * be bound to caller-owned point arrays. The append is transactional: on
 * uncovered, invalid timing, or insufficient target capacity its contents
 * and *landmarks remain unchanged. 0 modeled, 1 uncovered, -1 error. */
int reloq_german_within_word_compose(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    struct reloq_target_builder *builder,
    struct reloq_german_within_word_landmarks *landmarks);

#endif
