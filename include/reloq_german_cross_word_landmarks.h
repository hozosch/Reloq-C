#ifndef RELOQ_GERMAN_CROSS_WORD_LANDMARKS_H
#define RELOQ_GERMAN_CROSS_WORD_LANDMARKS_H

#include "reloq_german_syllable_plan.h"
#include "reloq_german_terminal_timing.h"

#include <stdint.h>

/* One absolute 1-ms clock for an arbitrary number of connected, explicitly
 * stressed CV/CVC German words. Each word has one initial /p,t,k/, one of
 * the ten simple oral vowels, and an optional final /p,t,k/. No generic
 * silence is inserted at a word boundary. The caller must independently
 * verify that the SPR control between groups is precisely `2 and choose
 * the final punctuation mark; the phone/syllable sequence does not carry
 * those controls. Do not use these timings for an unverified word boundary.
 *
 * boundary_release is the send endpoint at the following word transition.
 * With an open CV word it coincides with the *next* word's burst_end;
 * with CVC it ends the current coda before the next onset. For the last
 * word it ends its own coda, or equals vowel_closure when there is no coda.
 * coda_burst_start is zero for open words and merged identical stops.
 * Initial burst_start/end refer to this word, including the first word.
 * f0_early_tick is the first F0 target of this word, affected by position
 * and punctuation. All ticks are absolute from the callback's origin. */
struct reloq_german_cross_word_word {
    enum reloq_german_phone initial, vowel, coda;
    int64_t onset_origin;
    int64_t burst_start, burst_end;
    int64_t vowel_onset, vowel_hold, vowel_closure;
    int64_t coda_burst_start, boundary_release;
    int64_t f0_early_tick;
};

/* 0 modeled, 1 uncovered phones/phrase shape, -1 invalid pointers or
 * capacity. On failure words and *planned_ticks stay unchanged. `words`
 * must have room for syllable_count entries. The last word's release plus
 * the punctuation window gives *planned_ticks. No reference module or
 * generated recording is needed by this portable planner. */
int reloq_german_cross_word_landmarks_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    enum reloq_german_terminal_mark mark,
    struct reloq_german_cross_word_word *words, size_t word_capacity,
    int64_t *planned_ticks);

#endif
