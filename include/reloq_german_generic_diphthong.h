#ifndef RELOQ_GERMAN_GENERIC_DIPHTHONG_H
#define RELOQ_GERMAN_GENERIC_DIPHTHONG_H

#include "reloq_german_phone_sequence.h"

#include <stdint.h>

/* A single phoneme contains an oral-vowel nucleus, a timed formant glide,
 * and a short endpoint plateau. The context fields are phonetic, not word
 * spellings; no reference runtime or measured trajectory is loaded. */
struct reloq_german_generic_diphthong {
    int32_t endpoint[3];
    int32_t duration_ticks;
    int32_t glide_ticks;
    int32_t endpoint_hold_ticks;
    int32_t before_coda_ticks;
    int32_t final_av;
};

/* connected_word means the current word is followed immediately by another
 * word in the same clause, with no written pause. It is determined by the
 * complete utterance planner, beyond the diphthong's immediate neighbor.
 * Return 1 for aj/aw/oj; 0 for another phone; -1 for bad arguments. */
int reloq_german_generic_diphthong_plan(
    const struct reloq_german_phone_node *node,
    int connected_word,
    struct reloq_german_generic_diphthong *out);

#endif
