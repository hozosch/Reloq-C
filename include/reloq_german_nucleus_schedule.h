#ifndef RELOQ_GERMAN_NUCLEUS_SCHEDULE_H
#define RELOQ_GERMAN_NUCLEUS_SCHEDULE_H

#include "reloq_german_nucleus_prosody.h"
#include "reloq_german_syllable_plan.h"
#include "reloq_german_terminal_timing.h"
#include "reloq_german_word_features.h"

/* A scheduled vowel owns all listed intervals. The surrounding consonant
 * keeps its own duration; no transition is added again by the renderer.
 * Zero span leaves a nucleus with unsupported geometry on its old path. */
struct reloq_german_nucleus_schedule {
    int32_t span_ticks;
    int32_t incoming_ticks;
    int32_t outgoing_ticks;
    int32_t vowel_ticks;
    int32_t glide_ticks;
    int32_t closing_ticks;
    struct reloq_german_nucleus_budget budget;
};

/* Bridge from the current native linguistic sequence to shared timing.
 * Linguistic quantities absent from SPR are not invented here: explicitly
 * long phone symbols retain their quantity. Word controls are preserved;
 * absent prominence uses the existing phrase position. This bridge is
 * experimental, whereas the normalized budget API is separately verified.
 * On cancellation/failure, discard out. Returns 0, 1 cancelled, -1 error.
 * Scratch storage is linear and is released before returning. */
int reloq_german_nucleus_schedule_build(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const enum reloq_german_terminal_mark *marks, size_t mark_count,
    struct reloq_german_nucleus_schedule *out,
    reloq_german_cancel_check cancelled, void *cancel_context);

/* Normal-text voice preferences. With no metadata this is identical to
 * build. Unstressed final-a quality uses the midpoint of the contextual
 * short and long budgets, before allocating its existing transitions. */
int reloq_german_nucleus_schedule_build_with_features(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    const enum reloq_german_terminal_mark *marks, size_t mark_count,
    const struct reloq_german_word_features *features, size_t feature_count,
    struct reloq_german_nucleus_schedule *out,
    reloq_german_cancel_check cancelled, void *cancel_context);

#endif
