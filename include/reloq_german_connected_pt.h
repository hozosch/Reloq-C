#ifndef RELOQ_GERMAN_CONNECTED_PT_H
#define RELOQ_GERMAN_CONNECTED_PT_H

#include "reloq_german_connected_pair.h"

/* Acoustic gestures for a nonfinal /p/ before the following word's /t/.
 * The caller validates the complete explicitly stressed two-word SPR and
 * supplies the first vowel's nucleus/closure and the second word's final
 * coda. Ticks are absolute on the shared 1 ms phrase clock. Return 0 when
 * covered, 1 for another phone class, -1 for invalid input or append error;
 * outputs and target buffers stay unchanged on failure. */
int reloq_german_connected_pt_coda_append(
    const struct reloq_german_connected_pair *pair,
    int64_t first_closure_tick, struct reloq_target_builder *builder,
    int64_t *second_onset_origin);

/* Query the /t/ burst-send endpoint and following vowel's acoustic anchor
 * before planning that vowel and its final coda. */
int reloq_german_connected_pt_onset_landmarks(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick,
    int64_t *first_send_end, int64_t *second_vowel_onset);

/* Append the /t/ onset after the second final release has been planned.
 * The paired F0 builder owns its four F0 targets in this send. */
int reloq_german_connected_pt_onset_append(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick, int64_t second_final_release_tick,
    struct reloq_target_builder *builder,
    int64_t *first_send_end, int64_t *second_vowel_onset);

#endif
