#ifndef RELOQ_GERMAN_NUCLEUS_PROSODY_H
#define RELOQ_GERMAN_NUCLEUS_PROSODY_H

#include "reloq_german_spr.h"
#include "reloq_german_nucleus_duration.h"
#include <stddef.h>
#include <stdint.h>

/* Acoustic phone spelling uses separate A+j/A+w/O+j members for a
 * diphthong. A postvocalic r is distinct from a standalone syllabic R.
 * Quantity and local phone stress can differ from the syllable's stress.
 * short_quantity includes inherently short vowels as well as reduced long
 * vowels; it is not the narrower `shortened` flag of the base-duration API.
 * Phone features must already be normalized. NONE may mark a hard boundary
 * outside this context's word. No phone or feature is inferred from text. */
struct reloq_german_duration_phone {
    enum reloq_german_phone phone;
    uint8_t stress;
    uint8_t short_quantity;
    uint8_t voiceless;
    uint8_t vocalized_r;
};

/* All ranges are half-open in the same immutable phone array. Word and
 * syllable boundaries are linguistic inputs, not inferred from timing.
 * The context contains no reference-engine addresses or field numbers. */
struct reloq_german_nucleus_context {
    const struct reloq_german_duration_phone *phones;
    size_t phone_count;
    size_t word_begin, word_end;
    size_t syllable_begin, syllable_end;
    size_t nucleus_begin, nucleus_end;
    size_t word_syllables, syllable_in_word;
    unsigned stress;
    int prominence;
    unsigned final_word_in_phrase;
    unsigned final_syllable_in_phrase;
    unsigned following_unstressed_syllables;
    unsigned following_syllables;
    unsigned incoming_transition_left_owned;
};

/* Boundary lengthening of a nucleus in the phrase-final duration region.
 * Input/output remain real-valued budgets; interval rounding happens later.
 * Returns 1, 0 for unsupported nucleus shape, -1 for invalid input.
 * Failure never changes out. This is not a complete text duration planner. */
int reloq_german_nucleus_phrase_final(
    const struct reloq_german_nucleus_context *context,
    double input_ticks, double *out);

/* Phrase-medial shaping, including weak words and unstressed syllables. */
int reloq_german_nucleus_phrase_medial(
    const struct reloq_german_nucleus_context *context,
    double input_ticks, double *out);

/* Additional contextual shortening/lengthening after phrase shaping. */
int reloq_german_nucleus_adjust(
    const struct reloq_german_nucleus_context *context,
    double input_ticks, double *out);

enum reloq_german_nucleus_phrase_region {
    RELOQ_DE_NUCLEUS_MEDIAL,
    RELOQ_DE_NUCLEUS_FINAL,
    RELOQ_DE_NUCLEUS_BEFORE_LATER_STRESS
};

struct reloq_german_nucleus_budget {
    struct reloq_german_nucleus_duration base;
    enum reloq_german_nucleus_phrase_region region;
    double phrase_ticks;
    double adjusted_ticks;
};

/* Complete pre-rate nucleus budget from linguistic context. Chooses the
 * phrase region, computes the base and applies both contextual stages.
 * The result can be passed to reloq_german_nucleus_distribute after rate
 * adjustment. No observed duration is an input to this function. */
int reloq_german_nucleus_budget(
    const struct reloq_german_nucleus_context *context,
    struct reloq_german_nucleus_budget *out);

#endif
