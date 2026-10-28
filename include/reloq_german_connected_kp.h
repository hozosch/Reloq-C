#ifndef RELOQ_GERMAN_CONNECTED_KP_H
#define RELOQ_GERMAN_CONNECTED_KP_H

#include "reloq_german_connected_pair.h"

/* Nonfinal velar closure followed by a labial word onset, both explicitly
 * primary stressed. These builders append phone-class target gestures to a
 * shared phrase clock. They make no claim about the orthographic stress of
 * arbitrary text: the caller validates the complete word-boundary SPR.
 * Return 0 for a covered phone class, 1 if uncovered, -1 if invalid. */
int reloq_german_connected_kp_coda_append(
    const struct reloq_german_connected_pair *pair,
    int64_t first_closure_tick, struct reloq_target_builder *builder,
    int64_t *second_onset_origin);

int reloq_german_connected_kp_onset_landmarks(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick,
    int64_t *first_send_end, int64_t *second_vowel_onset);

int reloq_german_connected_kp_onset_append(
    const struct reloq_german_connected_pair *pair,
    enum reloq_german_phone second_vowel,
    int64_t first_coda_release_tick, int64_t second_final_release_tick,
    struct reloq_target_builder *builder,
    int64_t *first_send_end, int64_t *second_vowel_onset);

#endif
