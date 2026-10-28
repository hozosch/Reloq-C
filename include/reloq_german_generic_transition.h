#ifndef RELOQ_GERMAN_GENERIC_TRANSITION_H
#define RELOQ_GERMAN_GENERIC_TRANSITION_H

#include "reloq_german_phone_sequence.h"
#include "reloq_target_builder.h"

#include <stdint.h>

/* A directly adjacent nasal modifies the preceding vowel's bandwidth
 * without moving every formant centre. It may belong to the next syllable
 * of the same word, but anticipation stops at a word boundary. Returns 1
 * if applied, zero if unsupported, -1 on an invalid argument; zero/-1
 * leave the target array unchanged. */
int reloq_german_generic_prenasal_bandwidth(
    const struct reloq_german_phone_node *vowel,
    int32_t targets[RELOQ_TARGET_STREAM_COUNT]);

struct reloq_german_generic_nasal_transition {
    int32_t initial_fnz;
    int32_t final_fnz;
    int32_t rise_after_ticks;
};

/* During an oral nasal m/n/G, antiresonance FNZ begins at its neutral value.
 * It reaches the place-specific value halfway through the gesture, except
 * for /n/ after a vowel in the preceding syllable of the same word, where
 * it rises at one third. The caller emits this extra target.
 * Return 1 for a nasal, zero for another phone, -1 for invalid duration or
 * pointers. A nasalized vowel before the phone is left to its own profile. */
int reloq_german_generic_nasal_transition(
    const struct reloq_german_phone_node *node, int32_t phone_ticks,
    struct reloq_german_generic_nasal_transition *out);

/* An obstruent has a different sustained interval at a syllable onset,
 * inside a word, at a connected word edge and at a phrase edge.  These are
 * target-time gesture spans in milliseconds, before rate scaling; the
 * subsequent source release is planned separately.  Return 1 if this phone
 * class has a calibrated span, 0 if the caller should keep its default, and
 * -1 on invalid input. */
int reloq_german_generic_fricative_ticks(
    const struct reloq_german_phone_node *node, int phrase_final,
    int following_onset_cluster,
    int32_t *out_ticks);

#endif
