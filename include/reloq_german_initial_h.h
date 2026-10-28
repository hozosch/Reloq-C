#ifndef RELOQ_GERMAN_INITIAL_H_H
#define RELOQ_GERMAN_INITIAL_H_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

/* The source precursor of an isolated, explicitly primary-stressed German
 * SPR word h V [p|t|k], V = A/E/I/O/U/a/e/i/o/u, in the default reference
 * voice. The caller supplies h_origin_tick and the first vowel AV strength
 * from its independent vowel model (50..55 in the currently covered voice).
 * The source gesture leaves AV and AF at zero through the vowel's first
 * formant anchor and sets h aspiration AH to vowel_av - 9.
 *
 * first_window_ticks is an independently predicted first-send end relative
 * to h_origin_tick. For an isolated period phrase, it also places the first
 * F0 target at floor(first_window_ticks / 10), value 1049. Pass -1 to omit
 * F0 until a validated timing/prosody rule supplies that window. A different
 * punctuation or an unknown window must not request this period F0 point.
 * The exact AV-positive vowel onset occurs one tick after the returned
 * acoustic anchor. The function emits no formants, OQ, vowel or coda; its
 * h aspiration is shaped by later vowel formant targets held backward.
 *
 * Returns 0 on success, 1 for an uncovered shape, -1 for invalid input or
 * builder failure. Output and builder remain unchanged on failure. */
int reloq_german_initial_h_append(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    int64_t h_origin_tick, int32_t vowel_av,
    int64_t first_window_ticks, struct reloq_target_builder *builder,
    int64_t *vowel_anchor_offset);

#endif
