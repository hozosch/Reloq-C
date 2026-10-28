#ifndef RELOQ_GERMAN_STOP_VOWEL_H
#define RELOQ_GERMAN_STOP_VOWEL_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* Independently authored initial-stop/vowel coarticulation for one isolated,
 * explicitly primary-stressed German [p|t|k] V [p|t|k] syllable. V is one
 * of A/E/I/O/U/a/e/i/o/u at the default 11,025-Hz voice. The caller first
 * predicts vowel_onset_tick with reloq_german_initial_stop_append. The word
 * origin must be tick zero, and there must be no intervening control,
 * punctuation or pause within the SPR group.
 *
 * Appends initial stop F1-F3 at word origin and burst end, vowel F1-F3 at
 * its acoustic onset and hold end, and the vowel's AV/AH source gesture.
 * The independent initial-stop source/noise gesture and F4/F5, bandwidths,
 * nasal/source controls, final stop tract, phrase F0 and terminal release
 * must be supplied separately. The source interval is a common 90% factor
 * of the base vowel's open/coda interval; initial place changes some vowel
 * formants without changing that factor. The three outputs are the relative
 * vowel closure offset and the absolute first/second numeric send ends.
 * These send ends are schedule landmarks, not phone durations.
 *
 * Return 0 on covered success, 1 for an uncovered valid context, -1 for
 * invalid input, overflow or builder failure. All builder streams and
 * outputs remain unchanged on failure. */
int reloq_german_stop_vowel_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t vowel_onset_tick, struct reloq_target_builder *builder,
    int64_t *closure_offset, int64_t *first_window_end,
    int64_t *second_window_end);

#endif
