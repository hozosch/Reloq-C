#ifndef RELOQ_GERMAN_STOP_VOWEL_INTERACTION_H
#define RELOQ_GERMAN_STOP_VOWEL_INTERACTION_H

#include "reloq_german_phone_sequence.h"

#include <stdint.h>

/* Small interaction of initial and final stops on a German vowel.
 * All fields are the *factorially predicted* onset measurements, obtained
 * from independently authored first-onset and following-coda rules.
 * first_send_end is the absolute end of the first send containing the vowel
 * onset; it is NOT a phone duration. No measured reference targets are
 * embedded in this structure. */
struct reloq_german_vowel_onset_prediction {
    int64_t anchor_tick;
    int64_t first_send_end;
    int32_t f1, f2, f3;
};

/* Apply two general phonological-context interactions, measured and held
 * out in reference-apple/factorial_target_rule_probe.py. Validated only for
 * one isolated, explicitly primary-stressed p/t/k + one of ten German
 * simple vowels + p/t/k, with all three nodes in one word/syllable.
 * Return 0 on success; 1 means no validated model for this input; -1 means
 * invalid arguments or arithmetic overflow. *result is untouched on both
 * failure codes. This helper neither selects vowel formants nor plans F0,
 * full durations or coarticulation with a later syllable. */
int reloq_german_stop_vowel_interaction(
    const struct reloq_german_phone_node *phones, size_t phone_count,
    size_t vowel_index,
    const struct reloq_german_vowel_onset_prediction *additive,
    struct reloq_german_vowel_onset_prediction *result);

#endif
