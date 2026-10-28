#ifndef RELOQ_GERMAN_VOICED_STOP_VOWEL_H
#define RELOQ_GERMAN_VOICED_STOP_VOWEL_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* The lower vowel formants and source for a single isolated, explicitly
 * primary-stressed German [b|d|g]V[p|t|k] syllable at the default voice and
 * 11,025 Hz. V is A/E/I/O/U/a/e/i/o/u. The word origin is zero and
 * vowel_onset_tick is the first send end returned by
 * reloq_german_initial_voiced_stop_append. That separate onset gesture owns
 * F1-F3 through this onset and the closure/burst source controls.
 *
 * This gesture supplies F1-F3 at the vowel's context-dependent target and
 * second-send end, AV/AH at the first positive source tick and source
 * closure. The tract, coda and phrase F0 are separately planned. Outputs are
 * the relative source closure offset and absolute onset burst/second-send
 * ends and first vowel formant target. Coda place affects the second send
 * without changing source closure.
 *
 * Return 0 for covered success, 1 for an uncovered well-formed context, and
 * -1 for invalid arguments, overflow or builder failure. On failure the
 * builder and outputs remain unchanged. */
int reloq_german_voiced_stop_vowel_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t vowel_onset_tick, struct reloq_target_builder *builder,
    int64_t *closure_offset, int64_t *first_window_end,
    int64_t *second_window_end, int64_t *vowel_formant_start);

#endif
