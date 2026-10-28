#ifndef RELOQ_GERMAN_REDUCED_LANDMARKS_H
#define RELOQ_GERMAN_REDUCED_LANDMARKS_H

#include "reloq_german_syllable_plan.h"

#include <stdint.h>

/* Shared 1-ms clock for a primary CV or CVC followed by a weak schwa
 * syllable. An open CV may be followed by /p,t,k/ + schwa, or CVC by
 * onsetless schwa or /t/ + schwa. The weak syllable may end in /n/.
 * These are phone-class timing landmarks, never captured sound data.
 *
 * first_* all refer to the primary syllable. second_vowel_onset is the AV
 * zero-target one tick before positive schwa voicing. If the second syllable
 * has no consonant onset, its F1/F2 schwa target comes later at
 * second_schwa_target; otherwise those fields coincide. second_vowel_hold
 * is the end of that F1/F2 target window. second_closure and
 * second_coda_release coincide: the schwa's end without /n/, or final /n/
 * release. Fields for absent stop onset or /n/ tail are zero. With an open
 * first syllable, first_coda_burst_start is zero and first_coda_release
 * denotes the intervocalic consonant release; its burst appears in the
 * second_burst fields. */
struct reloq_german_reduced_landmarks {
    int64_t first_burst_end;
    int64_t first_vowel_onset;
    int64_t first_vowel_hold;
    int64_t first_closure;
    int64_t first_coda_burst_start;
    int64_t first_coda_release;
    int64_t second_onset_origin;
    int64_t second_burst_start;
    int64_t second_burst_end;
    int64_t second_vowel_onset;
    int64_t second_schwa_target;
    int64_t second_vowel_hold;
    int64_t second_noise_fade;
    int64_t second_nasal_onset;
    int64_t second_nasal_mid;
    int64_t second_closure;
    int64_t second_coda_release;
    int64_t planned_ticks;
};

/* Covered groups .1[p/t/k]V[p/k].0(@|@n|t@|t@n) and
 * .1[p/t/k]V.0[p/t/k]@{,n}, V one of ten simple vowels. Caller validates
 * trailing punctuation and other SPR
 * controls. Returns 0 modeled, 1 uncovered pattern, -1 invalid pointers;
 * *out stays unchanged on failure. This layer uses no reference runtime. */
int reloq_german_reduced_landmarks_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    struct reloq_german_reduced_landmarks *out);

#endif
