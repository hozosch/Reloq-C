#ifndef RELOQ_GERMAN_STOP_FRICATIVE_VOWEL_H
#define RELOQ_GERMAN_STOP_FRICATIVE_VOWEL_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* Vowel and lower-tract coarticulation in one isolated, explicitly
 * primary-stressed [p|t|k] V [s|f] German SPR group. V is one of
 * AEIOUaeiou at the measured default 11,025-Hz voice. The word origin is
 * zero; vowel_onset_tick comes from reloq_german_initial_stop_append.
 *
 * The four F1-F3 targets are at origin, initial burst, vowel onset and the
 * second numeric-send end. AV and AH start at onset+1 and retain a positive
 * target at closure. The terminal fricative gesture supplies the subsequent
 * same-tick AV/AH zeros and coda. Upper formants, bandwidths, other source
 * controls, F0 and the initial stop's noise are separate gestures.
 *
 * Outputs: closure_offset is relative to vowel onset; first_window_end is
 * the initial stop's burst send; second_window_end is the vowel send. The
 * latter is a scheduler landmark, not the vowel's duration. Returns 0 on
 * success, 1 for an uncovered phone context, -1 on invalid input, overflow
 * or target-builder failure. Outputs and builder stay unchanged on failure. */
int reloq_german_stop_fricative_vowel_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t vowel_onset_tick, struct reloq_target_builder *builder,
    int64_t *closure_offset, int64_t *first_window_end,
    int64_t *second_window_end);

#endif
