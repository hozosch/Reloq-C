#ifndef RELOQ_GERMAN_INITIAL_STOP_H
#define RELOQ_GERMAN_INITIAL_STOP_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* Initial p/t/k burst-to-aspiration gesture for an isolated, explicitly
 * primary-stressed, one-syllable German SPR word [p|t|k] V [p|t|k|s|f],
 * with V = A/E/I/O/U/a/e/i/o/u, or no final phone. The caller supplies
 * the absolute stop origin. The output gives the vowel onset offset relative
 * to it, so a later vowel rule can be placed consistently.
 *
 * Appends only the initial stop's AV/AH/AF and one active noise-resonator
 * stream (AB, A5F, A2F or A3F) through vowel onset. The inactive noise
 * streams must be initialized to zero by the caller's voice state. It does
 * not supply F0, formant transitions, the vowel, or the final consonant. A phone
 * sequence omits intervening controls and punctuation, so the caller must
 * provide an isolated group in the controlled reference setup.
 *
 * Returns 0 on success, 1 if uncovered, -1 on invalid arguments or builder
 * error. The builder and *vowel_onset_offset are unchanged on failure. */
int reloq_german_initial_stop_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t stop_origin_tick, struct reloq_target_builder *builder,
    int64_t *vowel_onset_offset);

#endif
