#ifndef RELOQ_GERMAN_GENERIC_STOP_CONNECTION_H
#define RELOQ_GERMAN_GENERIC_STOP_CONNECTION_H

#include "reloq_german_phone_sequence.h"

#include <stdint.h>

/* Word-adjacent p/t/k gestures for the approximate German voice. The
 * phonetic pair selects a closure duration, and equal stops share one burst.
 * Values are native 1-ms model ticks. Return 1 when applicable, 0 when the
 * ordinary phone planner owns the sequence, -1 on invalid arguments. */
struct reloq_german_generic_connected_coda {
    int32_t span_ticks;
    int32_t burst_at_ticks;
    int merge_with_next;
};

int reloq_german_generic_connected_coda(
    const struct reloq_german_phone_node *node,
    struct reloq_german_generic_connected_coda *out);

/* Number of ticks from the beginning of the next word's stop closure to
 * its burst. The caller preserves that stop's native burst and aspiration
 * durations, and must emit the closure before the burst. */
int reloq_german_generic_connected_onset(
    const struct reloq_german_phone_node *node,
    int32_t *burst_at_ticks);

/* A vowel followed by two voiceless stops in one stressed syllable has two
 * separate closures and releases.  The first stop can either merge with an
 * identical second stop or carry a short release and, in some pairs, a
 * breath that remains audible during the second closure.  These values are
 * articulatory durations and source levels; the generic planner owns the
 * actual target interpolation.  `first` is true for the vowel-adjacent stop,
 * false for the word-final stop.  The caller verifies that the other stop is
 * immediately adjacent and that the complete VCC sequence ends the word.
 * Return 1 for the modeled phoneme classes, 0 otherwise, -1 for bad args. */
struct reloq_german_generic_cluster_stop {
    int32_t span_ticks;
    int32_t burst_ticks;
    int32_t carry_aspiration_ticks;
    int32_t carry_aspiration_level;
    int32_t af_level;
    int32_t a2f_level;
    int32_t a5f_level;
    int32_t ab_level;
    int32_t following_aspiration_level;
    int32_t release_formants[3];
    int merge_with_next;
};

int reloq_german_generic_cluster_stop(
    const struct reloq_german_phone_node *node, int first,
    struct reloq_german_generic_cluster_stop *out);

/* Stressed vowel length in a final VCC sequence.  The caller validates the
 * next two phones as a same-word, same-syllable stop pair. */
int reloq_german_generic_cluster_vowel_ticks(
    const struct reloq_german_phone_node *node, int32_t *out);

#endif
