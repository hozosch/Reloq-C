#ifndef RELOQ_GERMAN_PHONE_SEQUENCE_H
#define RELOQ_GERMAN_PHONE_SEQUENCE_H

#include "reloq_german_annotation.h"
#include "reloq_german_input.h"

/* Exact numeric control preceding a phonetic word.  The name describes the
 * annotation token, not a guessed lexical stress or spelling class. */
enum reloq_german_word_prosody_control {
    RELOQ_DE_WORD_CONTROL_NONE = 0,
    RELOQ_DE_WORD_CONTROL_0,
    RELOQ_DE_WORD_CONTROL_1,
    RELOQ_DE_WORD_CONTROL_2,
    RELOQ_DE_WORD_CONTROL_00,
    RELOQ_DE_WORD_CONTROL_3,
    RELOQ_DE_WORD_CONTROL_4
};

/* One phone in the original SPR order. The left/right fields include phones
 * across word boundaries; the corresponding flags distinguish lexical and
 * syllable neighbors. A NONE neighbor denotes the utterance edge. This is
 * linguistic context only, not a timing or acoustic target prediction. */
struct reloq_german_phone_node {
    enum reloq_german_phone phone;
    enum reloq_german_phone left_phone;
    enum reloq_german_phone right_phone;
    size_t source_offset;
    size_t source_length;
    size_t word_index;
    size_t syllable_index; /* global index across all SPR words */
    size_t syllable_in_word;
    uint8_t stress; /* 0, 1, 2; 0 if no marker exists */
    uint8_t stress_explicit; /* 0 distinguishes missing from written .0 */
    uint8_t same_word_left;
    uint8_t same_word_right;
    uint8_t same_syllable_left;
    uint8_t same_syllable_right;
    uint8_t weak_word_control; /* exact `00 before this SPR group */
    uint8_t prosody_control; /* enum reloq_german_word_prosody_control */
    uint8_t affricate_role; /* Acoustic: closure/frication 1..8, following j 9. */
    uint8_t primary_ahead; /* A primary accent follows in this phonetic word. */
    uint8_t primary_long_closed; /* Its long nucleus has at least two coda phones. */
    uint8_t prosody_tail; /* Private `10: final parenthetical pitch register. */
};

struct reloq_german_phone_sequence_info {
    size_t phone_count;
    size_t word_count;
    size_t syllable_count;
};

/* Parse a complete German callback, preserving absolute source offsets.
 * Exact `0/`1/`2/`00 controls are retained on the following group; `00
 * also keeps its existing weak_word_control field. Unknown controls,
 * punctuation, explicit pauses and NUL fragments remain in the input for
 * reloq_german_annotation_next. This parser does not infer acoustic targets.
 *
 * Returns 0 on success, -1 on invalid input or insufficient capacity. On
 * failure, the output nodes and info remain unchanged. `nodes` may be NULL
 * only when capacity is zero; an empty valid input needs no nodes.
 * Input must stay immutable for both validation and output passes. */
int reloq_german_phone_sequence_build(
    const unsigned char *input, size_t input_length, uint32_t max_pause_ms,
    struct reloq_german_phone_node *nodes, size_t capacity,
    struct reloq_german_phone_sequence_info *info);

/* The same builder with cooperative cancellation during both parser passes
 * and retroactive stress assignment. Return 1 on cancellation, -2 if
 * private scratch allocation fails, -1 on invalid input/capacity and 0 on
 * success. On every failure, nodes and info remain unchanged; the callback
 * and input must stay stable for the call. A NULL predicate gives the
 * original no-allocation behavior. One annotation scanner event (for example
 * a long literal TEXT span) is not individually interruptible. */
int reloq_german_phone_sequence_build_checked(
    const unsigned char *input, size_t input_length, uint32_t max_pause_ms,
    struct reloq_german_phone_node *nodes, size_t capacity,
    struct reloq_german_phone_sequence_info *info,
    reloq_german_cancel_check cancelled, void *cancel_context);

#endif
