#ifndef RELOQ_GERMAN_SYLLABLE_PLAN_H
#define RELOQ_GERMAN_SYLLABLE_PLAN_H

#include "reloq_german_phone_sequence.h"

/* Structural syllables for an arbitrary-length German SPR callback. This
 * layer has no duration or voice coefficients: it describes where a future
 * context-sensitive acoustic model must place onset, nucleus and coda
 * gestures. A word boundary alone does not imply silence: the annotation
 * controls between SPR groups still need separate prosodic analysis. */
enum reloq_german_syllable_boundary {
    RELOQ_DE_BOUNDARY_SYLLABLE = 1,
    RELOQ_DE_BOUNDARY_WORD = 2,
    RELOQ_DE_BOUNDARY_UTTERANCE = 3
};

struct reloq_german_syllable {
    size_t first_phone;
    size_t phone_count;
    size_t word_index;
    size_t syllable_in_word;
    size_t onset_count;
    size_t nucleus_count;
    size_t coda_count;
    /* An unusual group with two separated vowel runs needs an explicit
     * phonological decision; do not pretend it has one compact nucleus. */
    uint8_t nucleus_contiguous;
    uint8_t stress;
    uint8_t stress_explicit;
    enum reloq_german_syllable_boundary boundary_after;
};

/* 0 success, -2 capacity query (required count in *count), -1 invalid
 * sequence, -3 allocation failure, 1 cooperative cancellation. Nothing in
 * output/count changes on invalid input or cancellation. Input must stay
 * immutable for the two validation/output passes. NULL output and capacity
 * zero is an ordinary query even when the sequence has zero phones. */
int reloq_german_syllable_plan_build_checked(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    struct reloq_german_syllable *syllables, size_t capacity, size_t *count,
    reloq_german_cancel_check cancelled, void *cancel_context);

int reloq_german_syllable_plan_build(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    struct reloq_german_syllable *syllables, size_t capacity, size_t *count);

#endif
