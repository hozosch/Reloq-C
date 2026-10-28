#ifndef RELOQ_GERMAN_WITHIN_WORD_LANDMARKS_H
#define RELOQ_GERMAN_WITHIN_WORD_LANDMARKS_H

#include "reloq_german_syllable_plan.h"

#include <stdint.h>

/* Common 1-ms acoustic clock for one weak CVC syllable followed directly by
 * a stressed /tV[p/t/k]/ syllable inside the same German word. All onset fields
 * refer to the F1/F2/F3 target; positive AV starts one tick later. Send
 * endpoints are first_burst_end, first_vowel_hold, first_coda_release,
 * second_burst_end, second_vowel_hold, second_coda_release, planned_ticks.
 *
 * The /t/ onset begins at second_onset_origin. Its actual aspirated burst
 * starts later at second_burst_start; these differ in a /k/-coda context.
 * No field is a recorded sample position or stored utterance trajectory. */
struct reloq_german_within_word_landmarks {
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
    int64_t second_vowel_hold;
    int64_t second_closure;
    int64_t second_coda_release;
    int64_t planned_ticks;
};

/* Covered class: one phonetic group .0[p/t/k]V[p/k].1tW[p/t/k], with ten
 * simple vowels V and W and no word boundary between syllables. Caller validates the
 * surrounding SPR controls and terminal mark separately. Return 0 modeled,
 * 1 uncovered phone/stress pattern, -1 invalid pointers. On nonzero return
 * *out remains unchanged. No Apple library is needed at runtime. */
int reloq_german_within_word_landmarks_plan(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    const struct reloq_german_syllable *syllables, size_t syllable_count,
    struct reloq_german_within_word_landmarks *out);

#endif
